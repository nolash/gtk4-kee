#include <string.h>

#include "transport.h"
#include "testutil.h"


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

int test_msg() {
	int r;
	char *p;
	size_t c;
	struct kee_test_t t;
	struct kee_transport_t ledger_transport;
	struct kee_transport_t item_transport;
	struct kee_transport_t merged_transport;
	char out[1024];
	size_t out_len;

	r = kee_test_generate(&t);
	if (r) {
		return 1;
	}

	c = kee_test_get_ledger_data(&t, &p);
	if (c == 0) {
		return 1;
	}
	r = kee_transport_single(&ledger_transport, KEE_TRANSPORT_RAW, KEE_CMD_LEDGER, t.ledger_bytes_len);
	if (r) {
		return 1;
	}
	r = kee_transport_write(&ledger_transport, t.ledger_bytes, t.ledger_bytes_len);
	if (r) {
		return 1;
	}

	c = kee_test_get_ledger_item_data(&t, 0, &p);
	if (c == 0) {
		return 1;
	}
	r = kee_transport_single(&item_transport, KEE_TRANSPORT_RAW, KEE_CMD_DELTA, t.ledger_item_bytes_len);
	if (r) {
		return 1;
	}
	r = kee_transport_write(&item_transport, t.ledger_item_bytes, t.ledger_item_bytes_len);
	if (r) {
		return 1;
	}

	out_len = 1024;
	r = kee_transport_encode_ledger(&ledger_transport, &item_transport, &merged_transport, KEE_TRANSPORT_RAW);
	if (r) {
		return 1;
	}

	r = kee_transport_validate(&merged_transport);
	if (r) {
		return 1;
	}

	kee_test_free(&t);

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
	r = test_msg();
	if (r) {
		return 1;
	}
	return 0;	
}
