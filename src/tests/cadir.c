#include <gcrypt.h>

#include "digest.h"
#include "cadir.h"

int main(int argc, char **argv) {
	int r;
	const char *data = "foo";
	char digest[64];
	char result[256];
	size_t l;
	calculate_digest_algo(data, 3, digest, GCRY_MD_SHA512);

	l = 256;
	r = cadir_get("./testdata_resource", CADIR_KEYTYPE_ANY, digest, result, &l);
	if (r) {
		return 1;
	}

	return strcmp(result, data);
}
