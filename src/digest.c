#include <gcrypt.h>

#include "err.h"
#include "digest.h"


int calculate_digest_algo(const char *in, size_t in_len, char *out, enum gcry_md_algos algo) {
	gcry_error_t e;
	gcry_md_hd_t h;
	unsigned char *v;
	static unsigned int digest_len;

	if (algo == GCRY_MD_NONE) {
		algo = GCRY_MD_SHA256;
	}
	digest_len = gcry_md_get_algo_dlen(algo);

	e = gcry_md_open(&h, algo, GCRY_MD_FLAG_SECURE);
	if (e) {
		return ERR_DIGESTFAIL;
	}

	gcry_md_write(h, in, in_len);
	v = gcry_md_read(h, 0);
	memcpy(out, v, digest_len);
	gcry_md_close(h);
	return ERR_OK;
}

int calculate_digest(const char *in, size_t in_len, char *out) {
	return calculate_digest_algo(in, in_len, out, GCRY_MD_NONE);
}
