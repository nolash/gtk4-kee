#include <string.h>

#include "ledger.h"
#include "hex.h"
#include "digest.h"
#include "testutil.h"


const char *test_ledger_data = "30818e0c035553440201020420c67ee54f93d63d00f4b8c9a7e1c11b39657b55c525704bb32e15ec85bc140d140420adcaf6474132ac36e97d3dbee693d3b186cd8399d402dc505073069c46b5bd780440878102c19c032fd0d06f6b054a01e969b823ccfe7d5ba37a37beef3e64feb5f9b38e1a0f7413b781a4626b884f89bb3052f662692c53578453dc7c7d911d8609";

const char *test_item_data_a = "3082011d044000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000020817c94f8dec0e485d0202150e0202107a04404919455218d5bfb6ae9f0de9af37c23c140f976d4a0cb8652fc6897e6f043bc454021128b8daa18eb36c28687cfc33c3e1aa9b4e37059822ae14a0cefd1087a704409bd071c737b9342ab87de3f3e43ccba508366d09b28a6e331f1255d5668b211694fe011c78b6bba376590a5dae47e2ff880facad68e9ab4fca15309c00a6bd0a0101ff04402a3eac6ff818857883fb26d052fb17f2384f9fdb60f5bbae7d849bf621dfc65e68c2b82359b6c54b041732f11919ab0c1ae1a68504870c872f30cc74f9b9ae0400";

const char *test_item_data_b = "3082011d0440c2b795d9d3183bcc9d6ae1ae2960c302d7364a04996013dd9f31be628c46d2ee87b0cba51db67cd851a64dba04cc3e191dd48e7d7f3e063b0c850fd7b9b82218020817c94f8dec3e67aa02020ce20202049504401f78629f3015afa72f443005fc6711f7a7e2e20072eac86c98874c1dbe42095de3408d5711fb8fca56428461139992e8ff0452dc2092d2ba6ddb9658607f90ac0440d5d6cd6d905d0eb104ff3ab825cfc1be27f69a5377a3c84c33b3c5a0e6902e2af74d9024db58e1b90375be316e687a928edb881f8b6b3795682c20e533f9ed040101ff04409e8ffbbd5684b75aed7bf42a044914ea5813b1fccd9645462664317fa92dd9766c9ede39ea381e9648ef88bad220d0808660be63c94bf9954cf00daddad1150e01";


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

/// \todo split up function (use util.c)
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
	memcpy(item.bob_signature, gpg.last_signature, SIGNATURE_LENGTH);
	r = gpg_store_verify(gpg.last_signature, item_sum, ledger.pubkey_bob);
	if (r) {
		return 1;
	}

	gpg.k = &alice;
	r = gpg_key_load(&gpg, "1234", KEE_GPG_FIND_FINGERPRINT, alice_fingerprint);
	if (r) {
		return 1;
	}

	out_item_len = 1024;
	r = kee_ledger_sign(&ledger, &item, &gpg, out_item, &out_item_len, "1234");
	if (r) {
		return 1;
	}
	memcpy(item.alice_signature, gpg.last_signature, SIGNATURE_LENGTH);
	r = calculate_digest_algo(out_item, out_item_len, item_sum, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	r = gpg_store_verify(gpg.last_signature, item_sum, ledger.pubkey_alice);
	if (r) {
		return 1;
	}

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

int test_alice() {
	char *p;
	int r;
	const char *version;
	struct kee_ledger_t ledger;
	struct kee_ledger_item_t item;
	struct gpg_store gpg;
	gcry_sexp_t alice;
	char path[1024];
	struct kee_content_t content;
	struct kee_content_t content_item;
	char item_sum[64];
	size_t out_len;
	size_t out_item_len;
	char *out;
	char *out_item;

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
	item.initiator = ALICE;
	item.response = 0;

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

	out_item_len = 1024;
	out_item = malloc(out_item_len);
	r = kee_ledger_item_serialize(&item, out_item, &out_item_len, KEE_LEDGER_ITEM_SERIALIZE_REQUEST);
	if (r) {
		return 1;
	}
	r = calculate_digest_algo(out_item, out_item_len, item_sum, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	out_item_len = 1024;
	r = kee_ledger_sign(&ledger, &item, &gpg, out_item, &out_item_len, "1234");
	if (r) {
		return 1;
	}
	memcpy(item.alice_signature, gpg.last_signature, SIGNATURE_LENGTH);
	r = calculate_digest_algo(out_item, out_item_len, item_sum, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	r = gpg_store_verify(gpg.last_signature, item_sum, ledger.pubkey_alice);
	if (r) {
		return 1;
	}

	free(out_item);
	free(out);
	kee_content_free(&content_item);
	kee_content_free(&content);
	kee_ledger_free(&ledger);

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
	
	cadiz.locator = "./testdata_resource";

	kee_ledger_init(&ledger);
	kee_ledger_reset_cache(&ledger);

	c = hex2bin(test_ledger_data, (unsigned char*)data);
	r = kee_ledger_parse(&ledger, data, c);
	if (r) {
		return 1;
	}

	c = hex2bin(test_item_data_a, (unsigned char*)data);
	ledger_item_a = kee_ledger_parse_item(&ledger, data, c);
	if (ledger_item_a == NULL) {
		return 1;
	}

	c = hex2bin(test_item_data_b, (unsigned char*)data);
	ledger_item_b = kee_ledger_parse_item(&ledger, data, c);
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
	r = kee_ledger_serialize_open(&t.ledger, out, &out_len);
	if (r) {
		return 1;
	}

	r = kee_ledger_parse_open(&ledger, out, out_len);
	if (r) {
		return 1;
	}

	if (strcmp(ledger.uoa, "USD")) {
		return 1;
	}

	if (ledger.last_item->alice_credit_delta == 666) {
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

int main() {
	int r;

	r = test_parse();
	if (r) {
		return 1;
	}
	r = test_util();
	if (r) {
		return 1;
	}
	r = test_sign();
	if (r) {
		return 1;
	}
	r = test_alice();
	if (r) {
		return 1;
	}
	r = test_pair();
	if (r) {
		return 1;
	}
	r = test_put();
	if (r) {
		return 1;
	}

	return 0;
}
