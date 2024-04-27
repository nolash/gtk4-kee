#include <string.h>

#include "transport.h"


int test_raw() {
	int r;
	size_t c;
	struct kee_transport_t h;
	char out[1024];

	r = kee_transport_single(&h, KEE_TRANSPORT_RAW, 1, 1024);
	if (r) {
		return 1;
	}
	r = kee_transport_write(&h, "foo", 4);
	if (r) {
		return 1;
	}
	c = 1024;
	r = kee_transport_next(&h, out, &c);
	if (r) {
		return 1;
	}
	if (out[0] != 1) {
		return 1;
	}
	if (strcmp("foo", ((char*)out)+1)) {
		return 1;
	}

	return 0;
}

int test_pack() {
	int r;
	size_t c;
	struct kee_transport_t h;
	struct kee_transport_t hi;
	char out[1024];

	r = kee_transport_single(&h, KEE_TRANSPORT_BASE64, 1, 1024);
	if (r) {
		return 1;
	}
	r = kee_transport_write(&h, "foo", 4);
	if (r) {
		return 1;
	}
	c = 1024;
	r = kee_transport_next(&h, out, &c);
	if (r) {
		return 1;
	}
	if (strcmp("eNpjTMvPZwAAA80BRg==", (char*)out)) {
		return 1;
	}

	r = kee_transport_import(&hi, KEE_TRANSPORT_BASE64, out, c);
	if (r) {
		return 1;
	}

	r = kee_transport_read(&hi, out, &c);
	if (r) {
		return 1;
	}

	return 0;
}


int main() {
	int r;

	r = test_raw();
	if (r) {
		return 1;
	}
	r = test_pack();
	if (r) {
		return 1;
	}
	
}
