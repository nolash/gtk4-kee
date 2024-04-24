#include <gcrypt.h>

#include "ledger.h"
#include "gpg.h"
#include "digest.h"

const char *content_test = "Subject: foo\n\nsome content\n";
const char *content_test_item = "Subject: bar\n\nsome other content\n";

int main() {
	int r;
	size_t c;
	gcry_sexp_t alice;
	gcry_sexp_t alice_pub;
	gcry_sexp_t bob;
	gcry_sexp_t bob_pub;
	gcry_sexp_t tmp;
	gcry_sexp_t sig;
	gcry_sexp_t msg;
	gcry_mpi_t sr;
	gcry_mpi_t ss;
	gcry_error_t err;
	char *p;
	char *out;
	size_t out_len;
	char *out_item;
	size_t out_item_len;
	struct kee_ledger_t ledger;
	struct kee_ledger_item_t item;
	struct kee_ledger_item_t *item_parsed;
	struct kee_content_t content;
	struct kee_content_t content_item;
	char item_sum[64];
	const char *version;

	version = gcry_check_version(NULL);
	if (version == 0x0) {
		return 1;	
	}
	gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
	gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);

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

	out_len = 1024*1024;
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
	r = gpg_sign(&sig, &bob, item_sum);
	if (r) {
		return 1;
	}
	c = 0;
	bob_pub = gcry_sexp_find_token(bob, "public-key", 10);
	if (bob_pub == NULL) {
		return 1;
	}
	err = gcry_sexp_build(&bob_pub, &c, "(key-data%S)", bob_pub);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	r = gpg_verify(&sig, &bob, item_sum);
	if (r) {
		return 1;
	}

	tmp = gcry_sexp_find_token(sig, "r", 1);
	if (tmp == NULL) {
		return 1;
	}
	sr = gcry_sexp_nth_mpi(tmp, 1, GCRYMPI_FMT_STD);
	if (sr == NULL) {
		return 1;
	}
	tmp = gcry_sexp_find_token(sig, "s", 1);
	if (tmp == NULL) {
		return 1;
	}
	ss = gcry_sexp_nth_mpi(tmp, 1, GCRYMPI_FMT_STD);
	if (ss == NULL) {
		return 1;
	}
	c = 0;
	err = gcry_mpi_print(GCRYMPI_FMT_STD, item_sum, 32, &c, sr);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	if (c != 32) {
		return 1;
	}
	memcpy(item.bob_signature, item_sum, 32);
	c = 0;
	err = gcry_mpi_print(GCRYMPI_FMT_STD, item_sum, 32, &c, ss);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	if (c != 32) {
		return 1;
	}
	memcpy(item.bob_signature+32, item_sum, 32);

	out_item_len = 4096;
	r = kee_ledger_item_serialize(&item, out_item, &out_item_len, KEE_LEDGER_ITEM_SERIALIZE_RESPONSE);
	if (r) {
		return 1;
	}
	r = calculate_digest_algo(out_item, out_item_len, item_sum, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	r = gpg_sign(&sig, &alice, item_sum);
	if (r) {
		return 1;
	}
	c = 0;
	alice_pub = gcry_sexp_find_token(alice, "public-key", 10);
	if (alice_pub == NULL) {
		return 1;
	}
	err = gcry_sexp_build(&alice_pub, &c, "(key-data%S)", alice_pub);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	r = gpg_verify(&sig, &alice, item_sum);
	if (r) {
		return 1;
	}
	tmp = NULL;
	tmp = gcry_sexp_find_token(sig, "r", 1);
	if (tmp == NULL) {
		return 1;
	}
	sr = gcry_sexp_nth_mpi(tmp, 1, GCRYMPI_FMT_STD);
	if (sr == NULL) {
		return 1;
	}
	tmp = NULL;
	tmp = gcry_sexp_find_token(sig, "s", 1);
	if (tmp == NULL) {
		return 1;
	}
	ss = gcry_sexp_nth_mpi(tmp, 1, GCRYMPI_FMT_STD);
	if (ss == NULL) {
		return 1;
	}
	c = 0;
	err = gcry_mpi_print(GCRYMPI_FMT_STD, item_sum, 32, &c, sr);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	if (c != 32) {
		return 1;
	}
	memcpy(item.alice_signature, item_sum, 32);
	c = 0;
	err = gcry_mpi_print(GCRYMPI_FMT_STD, item_sum, 32, &c, ss);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	if (c != 32) {
		return 1;
	}
	memcpy(item.alice_signature+32, item_sum, 32);

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
	tmp = NULL;

	free(out_item);
	free(out);
	kee_content_free(&content_item);
	kee_content_free(&content);
	kee_ledger_free(&ledger);

	return 0;
}
