#include <gcrypt.h>
#include <stdlib.h>
#include <fcntl.h>

#include "ledger.h"
#include "gpg.h"
#include "digest.h"

int test_create() {
	char *p;
	int r;
	struct gpg_store gpg;
	char path[1024];

	strcpy(path, "/tmp/keetest_key_XXXXXX");
	p = mkdtemp(path);
	if (p == NULL) {
		return 1;
	}
	gpg_store_init(&gpg, p)
		;
	r = gpg_key_create(&gpg, "1234");
	if (r) {
		return 1;
	}

	memset(&gpg, 0, sizeof(struct gpg_store));
	gpg_store_init(&gpg, p);
	r = gpg_key_load(&gpg, "1234", KEE_GPG_FIND_MAIN, NULL);
	if (r) {
		return 1;
	}

	return 0;
}

int main() {
	int r;
	r = test_create();
	if (r) {
		return 1;
	}
	return 0;
}
