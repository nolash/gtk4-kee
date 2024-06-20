#include <gcrypt.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>

#include <rerr.h>

#include "debug.h"
#include "gpg.h"
#include "hex.h"
#include "digest.h"

#define BUFLEN 1024 * 1024

#ifdef RERR
char *_rerr[7] = {
	"",
	"Crypto backend",
	"Auth fail",
	"Unlock fail",
	"Sign reject",
	"Resource fail",
	"No key found",
};
#endif

const char *gpgVersion = NULL;
const char sign_test[64];

size_t get_padsize(size_t insize, size_t blocksize) {
	size_t c;
	size_t l;
	size_t m;

	c = insize + 1;
	l = c / blocksize;
	m = c % blocksize;
	if (m) {
		l++;
	}
	return l * blocksize;
}

static void padb(char *data, size_t outsize, size_t insize) {
	gcry_randomize(data + insize, outsize - insize, GCRY_STRONG_RANDOM);
}

static void pad(char *indata_raw, size_t outsize, const char *indata) { //std::string indata) {
	int l;

	//strcpy(indata_raw, indata.c_str());
	strcpy(indata_raw, indata);
	//l = indata.length() + 1;
	l = strlen(indata) + 1;
	padb(indata_raw, outsize, l);
}

static int create_handle(gcry_cipher_hd_t *h, const char *key, const char *nonce) {
	gcry_error_t e;

	e = gcry_cipher_open(h, GCRY_CIPHER_CHACHA20, GCRY_CIPHER_MODE_POLY1305, GCRY_CIPHER_SECURE);
	if (e) {
		return 1;
	}
	e = gcry_cipher_setkey(*h, key, CHACHA20_KEY_LENGTH_BYTES);
	if (e) {
		return 1;
	}
	e = gcry_cipher_setiv(*h, nonce, CHACHA20_NONCE_LENGTH_BYTES);
	if (e) {
		return 1;
	}
	return 0;
}


static void free_handle(gcry_cipher_hd_t *h) {
	gcry_cipher_close(*h);
}

int encryptb (char *ciphertext, size_t ciphertext_len, const char *indata, size_t indata_len, const char *key, const char *nonce) {
	const char *p;
	int r;
	gcry_cipher_hd_t h;
	gcry_error_t e;
	char indata_raw[ciphertext_len];

	r = create_handle(&h, key, nonce);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_NOCRYPTO, NULL);
	}
	memcpy(indata_raw, indata, indata_len);
	padb(indata_raw, ciphertext_len, indata_len);
	e = gcry_cipher_encrypt(h, (unsigned char*)ciphertext, ciphertext_len, (const unsigned char*)indata_raw, ciphertext_len);
	if (e) {
		p = gcry_strerror(e);
		return debug_logerr(LLOG_ERROR, ERR_NOCRYPTO, (char*)p);
	}

	free_handle(&h);

	return ERR_OK;
}

int encrypt(char *ciphertext, size_t ciphertext_len, const char *indata, const char *key, const char *nonce) {
	char *p;
	int r;
	gcry_cipher_hd_t h;
	gcry_error_t e;
	char indata_raw[ciphertext_len];

	r = create_handle(&h, key, nonce);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_NOCRYPTO, NULL);
	}

	pad(indata_raw, ciphertext_len, indata);
	e = gcry_cipher_encrypt(h, (unsigned char*)ciphertext, ciphertext_len, (const unsigned char*)indata_raw, ciphertext_len);
	if (e) {
		p = (char*)gcry_strerror(e);
		return debug_logerr(LLOG_ERROR, ERR_NOCRYPTO, p);
	}

	free_handle(&h);

	return ERR_OK;
}

int decryptb(char *outdata, const char *ciphertext, size_t ciphertext_len, const char *key, const char *nonce) {
	char *p;
	int r;
	gcry_cipher_hd_t h;
	gcry_error_t e;

	r = create_handle(&h, key, nonce);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_NOCRYPTO, NULL);
	}

	e = gcry_cipher_decrypt(h, outdata, ciphertext_len, ciphertext, ciphertext_len);
	if (e) {
		p = (char*)gcry_strerror(e);
		return debug_logerr(LLOG_ERROR, ERR_NOCRYPTO, p);
	}

	free_handle(&h);

	return ERR_OK;
}

int decrypt(char *outdata, const char *ciphertext, size_t ciphertext_len, const char *key, const char *nonce) {
	char *p;
	int r;
	gcry_cipher_hd_t h;
	gcry_error_t e;
	char outdata_raw[ciphertext_len];

	memset(outdata_raw, 0, sizeof(outdata_raw));

	outdata_raw[0] = 0;
	r = create_handle(&h, key, nonce);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_NOCRYPTO, NULL);
	}

	e = gcry_cipher_decrypt(h, outdata_raw, ciphertext_len, ciphertext, ciphertext_len);
	if (e) {
		p = (char*)gcry_strerror(e);
		return debug_logerr(LLOG_ERROR, ERR_NOCRYPTO, p);
	}
	//outdata->assign(outdata_raw);
	strcpy(outdata, outdata_raw);

	free_handle(&h);

	return ERR_OK;
}

static int key_apply_public(struct gpg_store *gpg, gcry_sexp_t key) {
	char *p;
	size_t c;
	gcry_sexp_t pubkey;

	pubkey = gcry_sexp_find_token(key, "public-key", 10);
	if (pubkey == NULL) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}
	pubkey = gcry_sexp_find_token(pubkey, "q", 1);
	if (pubkey == NULL) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}
	c = PUBKEY_LENGTH;
	p = (char*)gcry_sexp_nth_data(pubkey, 1, &c);
	if (p == NULL) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}
	memcpy(gpg->public_key, p, PUBKEY_LENGTH);
	return ERR_OK;
}

static char *key_filename(struct gpg_store *gpg, char *path) {
	char *p;

	strcpy((char*)path, gpg->path);
	p = (char*)path + strlen((char*)path);

	b2h((unsigned char*)gpg->fingerprint, 20, (unsigned char*)p);

	return path;
}

static int key_from_data(gcry_sexp_t *key, const char *indata, size_t indata_len) {
	gcry_error_t e;

	e = gcry_sexp_new(key, indata, indata_len, 0);
	if (e != GPG_ERR_NO_ERROR) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}
	return ERR_OK;
}

static int key_from_file(gcry_sexp_t *key, const char *path, const char *passphrase) {
	char *p;
	int r;
	char v[BUFLEN];
	size_t c;
	size_t i;
	FILE *f;
	char nonce[CHACHA20_NONCE_LENGTH_BYTES];
	void *outdata;

	f = fopen(path, "r");
	if (f == NULL) {
		return debug_logerr(LLOG_ERROR, ERR_NOKEY, NULL);
	}

	/// \todo length must be in the ciphertext
	//c = fread(&l, sizeof(int), 1, f);
	c = fread(nonce, CHACHA20_NONCE_LENGTH_BYTES, 1, f);
	i = 0;
	c = 1;
	while (c != 0 && i < BUFLEN) {
		c = fread(v+i, 1024, 1, f);
		c *= 1024;
		i += c;
	}
	if (i == 0) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFILE, (char*)path);
	}
	fclose(f);

	outdata = malloc(i);
	r = decryptb((char*)outdata, v, i, passphrase, nonce);
	if (r) {
		return r;
	}
	//r = key_from_data(key, (char*)outdata, l);
	c = (size_t)(*((int*)outdata));
	p = (char*)(outdata+sizeof(int));
	r = key_from_data(key, p, c);
	free(outdata);
	return ERR_OK;
}

static int key_create(struct gpg_store *gpg, gcry_sexp_t *key) {
	int r;
	const char *p;
	const char *sexp_quick = "(genkey(ecc(flags eddsa)(curve Ed25519)))";
	//char *pv;
	gcry_sexp_t in;
	gcry_error_t e;

	e = gcry_sexp_new(&in, (const void*)sexp_quick, strlen(sexp_quick), 0);
	if (e) {
		p = gcry_strerror(e);
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, (char*)p);
	}
	e = gcry_pk_genkey(key, in);
	if (e) {
		p = gcry_strerror(e);
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, (char*)p);
	}
	p = (char*)gcry_pk_get_keygrip(*key, (unsigned char*)gpg->fingerprint);
	if (p == NULL) {
		p = gcry_strerror(e);
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, (char*)p);
	}

	r = key_apply_public(gpg, *key);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}

	return ERR_OK;
}

/**
 * \todo consistent endianness for key length in persistent storage (fwrite)
 * \todo doc must have enough in path for path + fingerprint hex
 *
 */
static int key_create_file(struct gpg_store *gpg, gcry_sexp_t *key, const char *passphrase) {
	char *p;
	int r;
	int kl;
	char v[BUFLEN];
	int i;
	int l;
	size_t c;
	size_t m;
	FILE *f;
	char nonce[CHACHA20_NONCE_LENGTH_BYTES];
	char path[1024];
	char ciphertext[BUFLEN];

	r = key_create(gpg, key);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}

	kl = gcry_sexp_sprint(*key, GCRYSEXP_FMT_CANON, NULL, 0);
	m = (size_t)kl + 1;
	p = (char*)v + sizeof(int);
	c = 0;
	kl = gcry_sexp_sprint(*key, GCRYSEXP_FMT_CANON, p, BUFLEN-m);
	m -= (size_t)(kl + 1);
	c += kl;
//	while (m > 0) {
//		kl = gcry_sexp_sprint(*key, GCRYSEXP_FMT_CANON, p, BUFLEN-m);
//		m -= (size_t)(kl + 1);
//		p += kl;
//		c += kl;
//	}
	memcpy(v, &c, sizeof(int));

	m = c;
	c = get_padsize(m, ENCRYPT_BLOCKSIZE);
	gcry_create_nonce(nonce, CHACHA20_NONCE_LENGTH_BYTES);
	r = encryptb(ciphertext, c, v, m+sizeof(int), passphrase, nonce);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}

	p = key_filename(gpg, path);
	if (p == NULL) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}

	f = fopen((char*)path, "w");
	if (f == NULL) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}
	l = c;

	c = fwrite(nonce, CHACHA20_NONCE_LENGTH_BYTES, 1, f);
	if (c != 1) {
		fclose(f);
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}
	i = 0;
	c = 1;
	while (c != 0 && l > 0) {
		c = fwrite(ciphertext+i, 1024, 1, f);
		c *= 1024;
		i += c;
		l -= c;
	}
	fclose(f);

	return ERR_OK;
}


int gpg_key_create(struct gpg_store *gpg, const char *passphrase) {
	size_t c;
	char *p;
	int r;
	gcry_sexp_t key;
	char key_path[1024];
	char link_path[1024];

	r = key_create_file(gpg, &key, passphrase);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}

	p = key_filename(gpg, key_path);
	if (p == NULL) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}

	strcpy(link_path, gpg->path);
	c = strlen(link_path);
	strcpy(link_path + c, "kee.key");

	r = unlink(link_path);
	if (r == -1 && errno != ENOENT) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}

	r = symlink(key_path, link_path);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}
	return ERR_OK;
}

/// \todo add key unload to destroy memory
int gpg_key_load(struct gpg_store *gpg, const char *passphrase, enum gpg_find_mode_e mode, const void *criteria) {
	int r;
	char *p;
	char path[1024];

	switch(mode) {
		case KEE_GPG_FIND_MAIN:
			strcpy(path, gpg->path);
			p = path + strlen(path);
			strcpy(p, "kee.key");
			r = key_from_file(&gpg->k, path, passphrase);
			if (r) {
				return debug_logerr(LLOG_WARNING, ERR_KEYFAIL, NULL);
			}
			break;
		case KEE_GPG_FIND_FINGERPRINT:
			strcpy(path, gpg->path);
			p = path + strlen(path);
			b2h((const unsigned char*)criteria, FINGERPRINT_LENGTH, (unsigned char*)p);
			r = key_from_file(&gpg->k, path, passphrase);
			if (r) {
				return debug_logerr(LLOG_WARNING, ERR_KEYFAIL, NULL);
			}
			break;
		default:
			return debug_logerr(LLOG_WARNING, ERR_FAIL, NULL);
	}

	p = (char*)gcry_pk_get_keygrip(gpg->k, (unsigned char*)gpg->fingerprint);
	if (p == NULL) {
		return debug_logerr(LLOG_ERROR, ERR_KEYFAIL, NULL);
	}

	r = key_apply_public(gpg, gpg->k);
	if (r) {
		return debug_logerr(LLOG_ERROR, ERR_FAIL, NULL);
	}
	
	return ERR_OK;
}


int gpg_verify(gcry_sexp_t *sig, gcry_sexp_t *key, const char *v) {
	const char *p;
	gcry_error_t e;
	gcry_sexp_t data;
	size_t err_offset;

	e = gcry_sexp_build(&data, &err_offset, "(data(flags eddsa)(hash-algo sha512)(value %b))", 64, v);
	if (e) {
		p = gcry_strerror(e);
		return debug_logerr(LLOG_ERROR, ERR_FAIL, (char*)p);
	}
	e = gcry_pk_verify(*sig, data, *key);
	if (e != GPG_ERR_NO_ERROR) {
		p = gcry_strerror(e);
		return debug_logerr(LLOG_ERROR, ERR_FAIL, (char*)p);
	}
	return ERR_OK;
}

char *gpg_store_get_fingerprint(struct gpg_store *gpg) {
	return gpg->fingerprint;
}


int gpg_store_digest(struct gpg_store *gpg, char *out, const char *in) {
	const char *s;
	size_t l;

	//l = in.length();
	l = strlen(in);
	s = in; //.c_str();
	return calculate_digest(s, l, out); //, m_passphrase_digest_len);
}

/// \todo handle path length limit
void gpg_store_init(struct gpg_store *gpg, const char *path) {
#ifdef RERR
	rerr_register(RERR_PFX_GPG, "gpg", _rerr);
#endif

	char *p;
	size_t c;
	memset(gpg, 0, sizeof(struct gpg_store));
	gpg->passphrase_digest_len = gcry_md_get_algo_dlen(GCRY_MD_SHA256);

	strcpy(gpg->path, path);
	c = strlen(gpg->path);
	p = gpg->path+c;
	if (*p != '/') {
		*p = '/';
		*(p+1) = 0;
	}
}

/// \todo conceal passphrase hashing
int gpg_store_check(struct gpg_store *gpg, const char *passphrase) { 
	int r;
	const char *v;
	char d[1024];
	gcry_sexp_t k;
	char *p;
	//unsigned char fingerprint[20] = { 0x00 };
	unsigned char fingerprint[41] = { 0x00 };
	//size_t fingerprint_len = 41;
	//char passphrase_hash[m_passphrase_digest_len];
	char passphrase_hash[gpg->passphrase_digest_len];

	p = gpg->path;

	//digest(passphrase_hash, passphrase);
	gpg_store_digest(gpg, passphrase_hash, passphrase);

	if (gpgVersion == NULL) {
		v = gcry_check_version(GPG_MIN_VERSION);
		//if (v == nullptr) {
		if (v == NULL) {
			return ERR_NOCRYPTO;
		}
	}
	gpgVersion = v;
	sprintf(d, "Using gpg version: %s", gpgVersion);
	debug_log(DEBUG_INFO, d);
//	r = key_from_file(&k, gpg->path, passphrase_hash);
	//r = gpg_key_load(gpg, passphrase_hash, KEE_GPG_FIND_MAIN, NULL);
	r = gpg_key_load(gpg, passphrase_hash, KEE_GPG_FIND_MAIN, NULL);
	if (r == ERR_KEYFAIL) {
		char pp[2048];
		sprintf(pp, "could not decrypt key in %s/kee.key", p);
		debug_log(DEBUG_INFO, pp);
		return 1;
	}
	if (r != ERR_OK) {
		char pp[2048];
		sprintf(pp, "%s/kee.key", p);
		r = key_create_file(gpg, &k, passphrase_hash);
		if (r != ERR_OK) {
			return r;
		}
		//gcry_pk_get_keygrip(k, fingerprint);
		gcry_pk_get_keygrip(k, (unsigned char*)gpg->fingerprint);
		//bin_to_hex(fingerprint, 20, (unsigned char*)gpg->fingerprint, &fingerprint_len);
		b2h((unsigned char*)gpg->fingerprint, 20, (unsigned char*)fingerprint);
		char ppp[4096];
		//sprintf(ppp, "created key %s from %s", m_fingerprint, pp);
		sprintf(ppp, "created key %s from %s", fingerprint, pp);
		debug_log(DEBUG_INFO, ppp);
	} else {
		//gcry_pk_get_keygrip(k, fingerprint);
		gcry_pk_get_keygrip(k, (unsigned char*)gpg->fingerprint);
		//bin_to_hex(fingerprint, 20, (unsigned char*)gpg->fingerprint, &fingerprint_len);
		b2h((unsigned char*)gpg->fingerprint, 20, (unsigned char*)fingerprint);
		char pp[4096];
		//sprintf(pp, "found key %s in %s", (unsigned char*)m_fingerprint, p.c_str());
		sprintf(pp, "found key %s in path: %s", fingerprint, p);
		debug_log(DEBUG_INFO, pp);
	}
	//r = gpg_sign(&o, &k, sign_test);
	//return r;
	return ERR_OK;
}

int gpg_store_sign(struct gpg_store *gpg, char *data, size_t data_len, const char *passphrase) {
	return gpg_store_sign_with(gpg, data, data_len, passphrase, NULL);
}

int gpg_store_sign_with(struct gpg_store *gpg, char *data, size_t data_len, const char *passphrase, const char *fingerprint) {
	int r;
	size_t c;
	gcry_sexp_t pnt;
	gcry_sexp_t msg;
	gcry_sexp_t sig;
	gcry_error_t e;
	char *p;

	r = calculate_digest_algo(data, data_len, gpg->last_data, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	if (fingerprint == NULL) {
		r = gpg_key_load(gpg, passphrase, KEE_GPG_FIND_MAIN, NULL);
	} else {
		r = gpg_key_load(gpg, passphrase, KEE_GPG_FIND_FINGERPRINT, fingerprint);
	}
	if (r) {
		return 1;
	}
		 
	c = 0;
	e = gcry_sexp_build(&msg, &c, "(data(flags eddsa)(hash-algo sha512)(value %b))", 64, gpg->last_data);
	if (e != GPG_ERR_NO_ERROR) {
		return 1;
	}

	e = gcry_pk_sign(&sig, msg, gpg->k);
	if (e != GPG_ERR_NO_ERROR) {
		return 1;
	}

	// retrieve r and write it
	pnt = NULL;
	pnt = gcry_sexp_find_token(sig, "r", 1);
	if (pnt == NULL) {
		return 1;
	}
	c = POINT_LENGTH;
	p = (char*)gcry_sexp_nth_data(pnt, 1, &c);
	if (p == NULL) {
		return 1;
	}
	memcpy(gpg->last_signature, p, c);

	// retrieve s and write it
	pnt = NULL;
	pnt = gcry_sexp_find_token(sig, "s", 1);
	if (pnt == NULL) {
		return 1;
	}
	c = POINT_LENGTH;
	p = (char*)gcry_sexp_nth_data(pnt, 1, &c);
	if (p == NULL) {
		return 1;
	}
	memcpy(gpg->last_signature + POINT_LENGTH, p, c);

	gcry_sexp_release(gpg->k);

	return 0;
}

/// \todo data input
int gpg_store_verify(const char *sig_bytes, const char *digest, const char *pubkey_bytes) {
	gcry_mpi_t sig_r;
	gcry_mpi_t sig_s;
	size_t c;
	gcry_error_t err;
	gcry_sexp_t sig;
	gcry_sexp_t msg;
	gcry_sexp_t pubkey;

	c = 0;
	err = gcry_sexp_build(&pubkey, &c, "(key-data(public-key(ecc(curve Ed25519)(q %b))))", PUBKEY_LENGTH, pubkey_bytes);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}

	c = 0;
	err = gcry_mpi_scan(&sig_r, GCRYMPI_FMT_STD, sig_bytes, POINT_LENGTH, &c);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	if (c != 32) {
		return 1;
	}

	c = 0;
	err = gcry_mpi_scan(&sig_s, GCRYMPI_FMT_STD, sig_bytes + POINT_LENGTH, POINT_LENGTH, &c);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	if (c != 32) {
		return 1;
	}

	c = 0;
	err = gcry_sexp_build(&sig, &c, "(sig-val(eddsa(r %m)(s %m)))", sig_r, sig_s);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}

	c = 0;
	err = gcry_sexp_build(&msg, &c, "(data(flags eddsa)(hash-algo sha512)(value %b))", DIGEST_LENGTH, digest);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}

	err = gcry_pk_verify(sig, msg, pubkey);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}

	return 0;
}
