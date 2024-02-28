#include <gcrypt.h>

#include "err.h"
#include "digest.h"


int calculate_digest(const char *in, size_t in_len, char *out) {
	gcry_error_t e;
	gcry_md_hd_t h;
	unsigned char *v;
	static unsigned int digest_len;

	if (digest_len == 0) {
		digest_len = gcry_md_get_algo_dlen(GCRY_MD_SHA256);
	}

	e = gcry_md_open(&h, GCRY_MD_SHA256, GCRY_MD_FLAG_SECURE);
	if (e) {
		return ERR_DIGESTFAIL;
	}

	gcry_md_write(h, in, in_len);
	v = gcry_md_read(h, 0);
	memcpy(out, v, digest_len);
	gcry_md_close(h);
	return ERR_OK;
}
