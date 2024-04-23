#include <gcrypt.h>

#include "ledger.h"
#include "gpg.h"
#include "digest.h"

const char *content_test = "Subject: foo\n\nsome content\n";
const char *content_test_item = "Subject: bar\n\nsome other content\n";
const char *content_test_item_two = "Subject: baz\n\nmore more more content\n";

int main() {
	int r;
	gcry_sexp_t alice;
	gcry_sexp_t bob;
	gcry_sexp_t tmp;
	char *out;
	size_t out_len;
	char *out_item;
	size_t out_item_len;
	struct kee_ledger_t ledger;
	struct kee_ledger_item_t item;
	struct kee_ledger_item_t item_two;
	struct kee_content_t content;
	struct kee_content_t content_item;
	struct kee_content_t content_item_two;
	char item_sum[64];

	r = gpg_key_create(&alice);
	if (r) {
		return 1;
	}
	r = gpg_key_create(&bob);
	if (r) {
		return 1;
	}

	kee_ledger_init(&ledger);

	tmp = gcry_sexp_find_token(alice, "public-key", 10);
	if (tmp == NULL) {
		return 1;
	}
	tmp = gcry_sexp_find_token(tmp, "q", 1);
	if (tmp == NULL) {
		return 1;
	}
	out_len = 32;	
	out = gcry_sexp_nth_data(tmp, 1, &out_len);
	if (tmp == NULL) {
		return 1;
	}
	memcpy(ledger.pubkey_alice, out, 32);

	tmp = gcry_sexp_find_token(bob, "public-key", 10);
	if (tmp == NULL) {
		return 1;
	}
	tmp = gcry_sexp_find_token(tmp, "q", 1);
	if (tmp == NULL) {
		return 1;
	}
	out_len = 32;	
	out = gcry_sexp_nth_data(tmp, 1, &out_len);
	if (tmp == NULL) {
		return 1;
	}
	memcpy(ledger.pubkey_bob, out, 32);

	strcpy(ledger.uoa, "USD");
	ledger.uoa_decimals = 2;

	r = calculate_digest_algo(content_test, strlen(content_test), content.key, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	r = kee_content_init(&content, content.key, 0);
	if (r) {
		return 1;
	}

	r = calculate_digest_algo(content_test, strlen(content_test), content.key, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	out_len = 4096;
	out = malloc(out_len);
	r = kee_ledger_serialize(&ledger, out, &out_len);
	if (r) {
		return 1;
	}

	r = calculate_digest_algo(out,  out_len, ledger.digest, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	kee_ledger_item_init(&item);
	item.alice_credit_delta = 666;
	item.bob_credit_delta = -42;
	item.alice_collateral_delta = 1024;
	item.bob_collateral_delta = 2048;
	r = clock_gettime(CLOCK_REALTIME, &item.time);
	if (r) {
		return 1;
	}
	item.initiator = BOB;
	item.response = 1;

	r = calculate_digest_algo(content_test, strlen(content_test_item), content_item.key, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	r = kee_content_init(&content_item, content_item.key, 0);
	if (r) {
		return 1;
	}

	free(out);
	kee_content_free(&content_item);
	kee_content_free(&content);
	kee_ledger_free(&ledger);

	out_item_len = 4096;
	out_item = malloc(out_len);
	r = kee_ledger_item_serialize(&item, out_item, &out_item_len, KEE_LEDGER_ITEM_SERIALIZE_REQUEST);
	if (r) {
		return 1;
	}

	r = calculate_digest_algo(out_item, out_item_len, item_sum, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	return 0;
}
