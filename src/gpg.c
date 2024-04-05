#include <gcrypt.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>

#include "err.h"
#include "debug.h"
#include "gpg.h"
#include "hex.h"
#include "digest.h"

#define BUFLEN 1024 * 1024


const char *gpgVersion = NULL;


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
		return ERR_NOCRYPTO;
	}
	e = gcry_cipher_setkey(*h, key, CHACHA20_KEY_LENGTH_BYTES);
	if (e) {
		return ERR_NOCRYPTO;
	}
	e = gcry_cipher_setiv(*h, nonce, CHACHA20_NONCE_LENGTH_BYTES);
	if (e) {
		return ERR_NOCRYPTO;
	}
	return ERR_OK;
}


static void free_handle(gcry_cipher_hd_t *h) {
	gcry_cipher_close(*h);
}

int encryptb (char *ciphertext, size_t ciphertext_len, const char *indata, size_t indata_len, const char *key, const char *nonce) {
	int r;
	gcry_cipher_hd_t h;
	gcry_error_t e;
	char indata_raw[ciphertext_len];

	r = create_handle(&h, key, nonce);
	if (r) {
		return r;
	}
	memcpy(indata_raw, indata, indata_len);
	padb(indata_raw, ciphertext_len, indata_len);
	e = gcry_cipher_encrypt(h, (unsigned char*)ciphertext, ciphertext_len, (const unsigned char*)indata_raw, ciphertext_len);
	if (e) {
		return ERR_NOCRYPTO;
	}

	free_handle(&h);

	return ERR_OK;
}

int encrypt(char *ciphertext, size_t ciphertext_len, const char *indata, const char *key, const char *nonce) {
	int r;
	gcry_cipher_hd_t h;
	gcry_error_t e;
	char indata_raw[ciphertext_len];

	r = create_handle(&h, key, nonce);
	if (r) {
		return r;
	}

	pad(indata_raw, ciphertext_len, indata);
	e = gcry_cipher_encrypt(h, (unsigned char*)ciphertext, ciphertext_len, (const unsigned char*)indata_raw, ciphertext_len);
	if (e) {
		return ERR_NOCRYPTO;
	}

	free_handle(&h);

	return ERR_OK;
}

int decryptb(char *outdata, const char *ciphertext, size_t ciphertext_len, const char *key, const char *nonce) {
	int r;
	gcry_cipher_hd_t h;
	gcry_error_t e;

	r = create_handle(&h, key, nonce);
	if (r) {
		return r;
	}

	e = gcry_cipher_decrypt(h, outdata, ciphertext_len, ciphertext, ciphertext_len);
	if (e) {
		return ERR_NOCRYPTO;
	}

	free_handle(&h);

	return ERR_OK;

}

int decrypt(char *outdata, const char *ciphertext, size_t ciphertext_len, const char *key, const char *nonce) {
	int r;
	gcry_cipher_hd_t h;
	gcry_error_t e;
	char outdata_raw[ciphertext_len] = {};

	outdata_raw[0] = 0;
	r = create_handle(&h, key, nonce);
	if (r) {
		return r;
	}

	e = gcry_cipher_decrypt(h, outdata_raw, ciphertext_len, ciphertext, ciphertext_len);
	if (e) {
		return ERR_NOCRYPTO;
	}
	//outdata->assign(outdata_raw);
	strcpy(outdata, outdata_raw);

	free_handle(&h);

	return ERR_OK;
}

static int key_from_data(gcry_sexp_t *key, const char *indata, size_t indata_len) {
	gcry_error_t e;

	e = gcry_sexp_new(key, indata, indata_len, 1);
	if (e != GPG_ERR_NO_ERROR) {
		//debug_log(DEBUG_DEBUG, indata);
		return ERR_KEYFAIL;
	}
	return ERR_OK;
}

static int key_from_path(gcry_sexp_t *key, const char *p, const char *passphrase) {
	int l;
	int r;
	char v[BUFLEN];
	size_t c;
	size_t i;
	FILE *f;
	char *fullpath;
	char nonce[CHACHA20_NONCE_LENGTH_BYTES];
	void *outdata;

	fullpath = (char*)malloc(strlen(p) + 8);
	sprintf(fullpath, "%s/%s", p, "key.bin");

	f = fopen(fullpath, "r");
	if (f == NULL) {
		return ERR_NOKEY;
	}
	free(fullpath);

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
		return ERR_NOKEY;
	}
	fclose(f);

	outdata = malloc(i);
	r = decryptb((char*)outdata, v, i, passphrase, nonce);
	if (r) {
		return ERR_NOKEY;
	}
	//r = key_from_data(key, (char*)outdata, l);
	r = key_from_data(key, (char*)(outdata+sizeof(int)), (size_t)(*((int*)outdata)));
	free(outdata);
	return r;
}

/**
 * \todo consistent endianness for key length in persistent storage (fwrite)
 * \todo implement MAC
 * \todo test new data length location (part of ciphertext)
 */
static int key_create(gcry_sexp_t *key, const char *p, const char *passphrase) {
	int r;
	FILE *f;
	const char *sexp_quick = "(genkey(ecc(curve Ed25519)))";
	//char *pv;
	int i;
	int l;
	int kl;
	size_t c;
	gcry_sexp_t in;
	gcry_error_t e;
	char v[BUFLEN];
	char nonce[CHACHA20_NONCE_LENGTH_BYTES];

	e = gcry_sexp_new(&in, (const void*)sexp_quick, strlen(sexp_quick), 0);
	if (e) {
		printf("error sexp: %s\n", gcry_strerror(e));
		return (int)e;
	}
	e = gcry_pk_genkey(key, in);
	if (e) {
		printf("error gen: %s\n", gcry_strerror(e));
		return (int)e;
	}
	kl = gcry_sexp_sprint(*key, GCRYSEXP_FMT_CANON, v+sizeof(int), BUFLEN);
	memcpy(v, &kl, sizeof(int));

	c = get_padsize(kl, ENCRYPT_BLOCKSIZE);
	char ciphertext[c];

	gcry_create_nonce(nonce, CHACHA20_NONCE_LENGTH_BYTES);
	r = encryptb(ciphertext, c, v, kl, passphrase, nonce);
	if (r) {
		return ERR_KEYFAIL;
	}

	f = fopen(p, "w");
	if (f == NULL) {
		return ERR_KEYFAIL;
	}
	l = c;
//	c = fwrite(&kl, sizeof(int), 1, f);
//	if (c != 1) {
//		fclose(f);
//		return ERR_KEYFAIL;
//	}
	c = fwrite(nonce, CHACHA20_NONCE_LENGTH_BYTES, 1, f);
	if (c != 1) {
		fclose(f);
		return ERR_KEYFAIL;
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

static int sign(gcry_sexp_t *out, gcry_sexp_t *key, const char *v) {
	gcry_error_t e;
	gcry_sexp_t data;
	char in[BUFLEN];

	sprintf(in, "(data(flags eddsa(hash-algo sha512(value %s))))", v);
	gcry_sexp_new(&data, in, strlen(in), 0);
	e = gcry_pk_sign(out, data, *key);
	if (e) {
		sprintf(in, "error sign: %s\n", gcry_strerror(e));
		debug_log(DEBUG_ERROR, in);
		return ERR_KEYFAIL;
	}
	return 0;
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
	gpg->passphrase_digest_len = gcry_md_get_algo_dlen(GCRY_MD_SHA256);
	strcpy(gpg->path, path);

}

int gpg_store_check(struct gpg_store *gpg, const char *passphrase) { 
	int r;
	const char *v;
	char d[1024];
	gcry_sexp_t k;
	gcry_sexp_t o;
	char *p;
	unsigned char fingerprint[20] = { 0x00 };
	size_t fingerprint_len = 41;
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
	//r = key_from_path(&k, p.c_str(), passphrase_hash);
	//r = key_from_path(&k, p, passphrase_hash);
	r = key_from_path(&k, gpg->path, passphrase_hash);
	if (r == ERR_KEYFAIL) {
		char pp[2048];
		//sprintf(pp, "could not decrypt key in %s/key.bin", p.c_str());
		sprintf(pp, "could not decrypt key in %s/key.bin", p);
		debug_log(DEBUG_INFO, pp);
		return 1;
	}
	if (r != ERR_OK) {
		char pp[2048];
		//sprintf(pp, "%s/key.bin", p.c_str());
		sprintf(pp, "%s/key.bin", p);
		r = key_create(&k, pp, passphrase_hash);
		if (r != ERR_OK) {
			return r;
		}
		gcry_pk_get_keygrip(k, fingerprint);
		//bin_to_hex(fingerprint, 20, (unsigned char*)m_fingerprint, &fingerprint_len);
		bin_to_hex(fingerprint, 20, (unsigned char*)gpg->fingerprint, &fingerprint_len);
		char ppp[4096];
		//sprintf(ppp, "created key %s from %s", m_fingerprint, pp);
		sprintf(ppp, "created key %s from %s", gpg->fingerprint, pp);
		debug_log(DEBUG_INFO, ppp);
	} else {
		gcry_pk_get_keygrip(k, fingerprint);
		//bin_to_hex(fingerprint, 20, (unsigned char*)m_fingerprint, &fingerprint_len);
		bin_to_hex(fingerprint, 20, (unsigned char*)gpg->fingerprint, &fingerprint_len);
		char pp[4096];
		//sprintf(pp, "found key %s in %s", (unsigned char*)m_fingerprint, p.c_str());
		sprintf(pp, "found key %s in %s", (unsigned char*)gpg->fingerprint, p);
		debug_log(DEBUG_INFO, pp);
	}
	r = sign(&o, &k, "foo");
	return r;
}
