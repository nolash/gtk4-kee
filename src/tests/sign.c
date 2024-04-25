#include <gcrypt.h>
#include <stdlib.h>
#include <fcntl.h>

#include "ledger.h"
#include "gpg.h"
#include "digest.h"

const char *content_test = "Subject: foo\n\nsome content\n";
const char *content_test_item = "Subject: bar\n\nsome other content\n";

/// \todo split up function
int test_sign() {
	int r;
	gcry_sexp_t alice;
	gcry_sexp_t bob;
	char alice_fingerprint[20];
	char *p;
	char *out;
	size_t out_len;
	char *out_item;
	size_t out_item_len;
	struct gpg_store gpg;
	struct kee_ledger_t ledger;
	struct kee_ledger_item_t item;
	struct kee_ledger_item_t *item_parsed;
	struct kee_content_t content;
	struct kee_content_t content_item;
	char item_sum[64];
	const char *version;
	char path[1024];

	version = gcry_check_version(NULL);
	if (version == 0x0) {
		return 1;	
	}
	gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
	gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);

	strcpy(path, "/tmp/keetest_key_XXXXXX");
	p = mkdtemp(path);
	if (p == NULL) {
		return 1;
	}

	kee_ledger_init(&ledger);

	gpg_store_init(&gpg, p);
	gpg.k = &alice;
	r = gpg_key_create(&gpg, "1234"); // alice
	if (r) {
		return 1;
	}
	memcpy(ledger.pubkey_alice, gpg.public_key, PUBKEY_LENGTH);
	memcpy(alice_fingerprint, gpg.fingerprint, FINGERPRINT_LENGTH);

	gpg_store_init(&gpg, p);
	gpg.k = &bob;
	r = gpg_key_create(&gpg, "1234"); // bob
	if (r) {
		return 1;
	}
	memcpy(ledger.pubkey_bob, gpg.public_key, PUBKEY_LENGTH);

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

	out_len = 1024*1024;
	out = malloc(out_len);
	r = kee_ledger_serialize(&ledger, out, &out_len);
	if (r) {
		return 1;
	}

	r = calculate_digest_algo(out, out_len, ledger.digest, GCRY_MD_SHA512);
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

	r = calculate_digest_algo(content_test_item, strlen(content_test_item), content_item.key, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	r = kee_content_init(&content_item, content_item.key, 0);
	if (r) {
		return 1;
	}
	r = calculate_digest_algo(content_test_item, strlen(content_test_item), content_item.key, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	out_item_len = 4096;
	out_item = malloc(out_item_len);
	r = kee_ledger_item_serialize(&item, out_item, &out_item_len, KEE_LEDGER_ITEM_SERIALIZE_REQUEST);
	if (r) {
		return 1;
	}
	r = calculate_digest_algo(out_item, out_item_len, item_sum, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	r = gpg_store_sign(&gpg, out_item, out_item_len, "1234");
	if (r) {
		return 1;
	}
	r = gpg_store_verify(gpg.last_signature, item_sum, ledger.pubkey_bob);
	if (r) {
		return 1;
	}
	memcpy(item.bob_signature, gpg.last_signature, SIGNATURE_LENGTH);

	out_item_len = 4096;
	r = kee_ledger_item_serialize(&item, out_item, &out_item_len, KEE_LEDGER_ITEM_SERIALIZE_RESPONSE);
	if (r) {
		return 1;
	}
	r = calculate_digest_algo(out_item, out_item_len, item_sum, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	gpg.k = &alice;
	r = gpg_store_sign_with(&gpg, out_item, out_item_len, "1234", alice_fingerprint);
	if (r) {
		return 1;
	}
	r = gpg_store_verify(gpg.last_signature, item_sum, ledger.pubkey_alice);
	if (r) {
		return 1;
	}
	memcpy(item.alice_signature, gpg.last_signature, SIGNATURE_LENGTH);

	out_item_len = 4096;
	r = kee_ledger_item_serialize(&item, out_item, &out_item_len, KEE_LEDGER_ITEM_SERIALIZE_FINAL);
	if (r) {
		return 1;
	}
	*(out_item+out_item_len) = 1;

	item_parsed = kee_ledger_parse_item(&ledger, out_item, out_item_len + 1);
	if (item_parsed == NULL) {
		return 1;
	}

	free(out_item);
	free(out);
	kee_content_free(&content_item);
	kee_content_free(&content);
	kee_ledger_free(&ledger);

	return 0;
}

int test_create() {
	char *p;
	int r;
	struct gpg_store gpg;
	char path[1024];
	gcry_sexp_t key;

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
	gpg.k = &key;
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
	r = test_sign();
	if (r) {
		return 1;
	}
	return 0;
}
