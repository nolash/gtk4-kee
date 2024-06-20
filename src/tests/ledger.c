#include <string.h>

#include "ledger.h"
#include "hex.h"
#include "digest.h"
#include "testutil.h"


const char *test_ledger_data = "30818e0c035553440201020420788da0e19fe7d885c7b3d37029be22b17a3afb8aa4428db98d80f67a5ca693b7042005c4f411a03048177910396212409487045249ac5bd9271aeb38624e11fc474304408e019a8c44e9483c772e2393a83dbefea6d8f0eb279bf9c69e5932d71f9592479d65799134f676f2d13bf1bb5f5ac1ba2f21c7d6ecd9d639239eac0d1ffdc8af";

const char *test_item_data_a = "3082011d044000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000040866460b143741fbee0202117102020642044009491030794d7d1992ceb4038a77aa628b68ab18db2b50f0bd60c3ef1754c3fba697e18a98caf67ecf19d3993760fe34d31013412bce3087bcc54d739f181a4b044020cc81a5a5be0fde65057dbb5769d5ce2dc2f8f02279388dcea63c5bc9977de853b951a9212f88b560fdba595dd9a7b0d9e4c95d2efb019a59bb663cedc02e010101ff0440bd3f6878a93e8a3b9068ed039d68bb6ab31da99f9daa34cb37a9f111e5f58b3d39a40d06af5653f23d14b87ffbd7b38d06d77a1cd11d0bf1cb0fd1a1c6980c0a00";

const char *test_item_data_b = "3082011d0440051a9dc4e24e19e1a102121f5a514f702ac8c0fa81b7da129fad9154cd83ba12a4672009850b7806c003bc4d106ccb546a3e274509653e0aa72ab05d800b37a1040866460b1437744e63020209af0202058a04407f210ca459c0c309c94dd3017d437d249754eb98753c8a6fbcbbfe94e2d9f8c0d038ce5d5b58696c0cd896f4ffcf1a18e58d6f1d5228f2f27a1b8a5161caab220440bdd91dfeb4887d54bdded6832446b1b1dac0f6d93c793b0309bd2025649d0930873ffd7ad5a956e4247a9145052e5bcec574d925b19964c126478c33d0badb020101ff0440881cc8759416b46ea5aa1328217d7dc9f661c7e8f40cd5ce52c8e587eeccf5d3b93ae1481292b17e6ac5b14d34ec3a5e0b3c58352246de52f558169346531c0a01";


int test_util() {
	int r;
	struct kee_test_t t;

	r = kee_test_generate(&t);
	if (r) {
		return r;
	}
	kee_test_free(&t);
	return 0;
}

int test_parse() {
	int r;
	size_t c;
	struct kee_ledger_t ledger;
	struct kee_ledger_item_t *ledger_item_a;
	struct kee_ledger_item_t *ledger_item_b;
	Cadiz cadiz;
	char data[1024];
	
	cadiz.locator = "./testdata/resource";

	kee_ledger_init(&ledger);
	kee_ledger_reset_cache(&ledger);

	c = h2b(test_ledger_data, (unsigned char*)data);
	r = kee_ledger_parse(&ledger, data, c);
	if (r) {
		return 1;
	}

	c = h2b(test_item_data_a, (unsigned char*)data);
	ledger_item_a = kee_ledger_parse_item_db(&ledger, data, c);
	if (ledger_item_a == NULL) {
		return 1;
	}

	c = h2b(test_item_data_b, (unsigned char*)data);
	ledger_item_b = kee_ledger_parse_item_db(&ledger, data, c);
	if (ledger_item_b == NULL) {
		return 1;
	}

	kee_ledger_resolve(&ledger, &cadiz);

	if (ledger.cache->alice_credit_balance == 0) {
		return 1;
	}
	if (ledger.cache->bob_credit_balance == 0) {
		return 1;
	}
	if (ledger.cache->alice_collateral_balance == 0) {
		return 1;
	}
	if (ledger.cache->bob_collateral_balance == 0) {
		return 1;
	}

	return 0;
}


int test_pair() {
	int r;
	struct kee_test_t t;
	char out[1024];
	size_t out_len;
	struct kee_ledger_t ledger;

	r = kee_test_generate(&t);
	if (r) {
		return 1;
	}

	out_len = 1024;
	r = kee_ledger_serialize_open(&t.ledger, out, &out_len, KEE_LEDGER_STATE_RESPONSE);
	if (r) {
		return 1;
	}

	r = kee_ledger_parse_open(&ledger, &t.gpg, out, out_len);
	if (r) {
		return 1;
	}

	if (strcmp(ledger.uoa, "USD")) {
		return 1;
	}

	if (ledger.last_item->alice_credit_delta != 666) {
		return 1;
	}

	if (memcmp(t.ledger.digest, ledger.digest, DIGEST_LENGTH)) {
		return 1;
	}

	return 0;
}

int test_put() {
	int r;
	struct kee_test_t t;

	r = kee_test_generate(&t);
	if (r) {
		return 1;
	}

	r = kee_test_db(&t);
	if (r) {
		return 1;
	}

	r = kee_ledger_put(&t.ledger, &t.db);
	if (r) {
		return 1;
	}

	return 0;
}

int test_sign() {
	int r;
	struct kee_test_t t;

	r = kee_test_generate(&t);
	if (r) {
		return 1;
	}

	r = kee_test_sign_request(&t);
	if (r) {
		return 1;
	}

	return 0;
}

int test_verify() {
	int r;
	int i;
	struct kee_test_t t;

	r = kee_test_generate(&t);
	if (r) {
		return 1;
	}

	r = kee_test_sign_request(&t);
	if (r) {
		return 1;
	}

	r = kee_ledger_verify(&t.ledger, &i);
	if (r) {
		return 1;
	}

	r = kee_test_sign_response(&t);
	if (r) {
		return 1;
	}

	r = kee_ledger_verify(&t.ledger, &i);
	if (r) {
		return 1;
	}

	return 0;
}

int test_initiator() {
	int r;
	struct kee_test_t t;
	struct kee_ledger_item_t *item;
	enum kee_initiator_e initiator;

	r = kee_test_generate(&t);
	if (r) {
		return 1;
	}

	item = t.ledger.last_item;

	r = kee_test_sign_request(&t);
	if (r) {
		return 1;
	}
	r = kee_test_sign_response(&t);
	if (r) {
		return 1;
	}

	item->initiator = NOONE;
	initiator = kee_ledger_item_initiator(&t.ledger, &t.gpg, item);
	if (initiator != ALICE) {
		return 1;
	}

	kee_test_free(&t);
	r = kee_test_generate(&t);
	if (r) {
		return 1;
	}

	kee_test_swap_identities(&t);
	r = kee_test_sign_request(&t);
	if (r) {
		return 1;
	}
	r = kee_test_sign_response(&t);
	if (r) {
		return 1;
	}

	item = t.ledger.last_item;
	item->initiator = NOONE;
	initiator = kee_ledger_item_initiator(&t.ledger, &t.gpg, item);
	if (initiator != BOB) {
		return 1;
	}

	return 0;
}

int main() {
	int i;
	int r;

	i = 1;
	r = test_parse();
	if (r) {
		return i;
	}
	i++;
	r = test_util();
	if (r) {
		return i;
	}
	i++;
	r = test_sign();
	if (r) {
		return i;
	}
	i++;
	r = test_verify();
	if (r) {
		return i;
	}
	i++;
	r = test_initiator();
	if (r) {
		return i;
	}
	i++;
	r = test_pair();
	if (r) {
		return i;
	}
	i++;
	r = test_put();
	if (r) {
		return i;
	}

	return 0;
}
