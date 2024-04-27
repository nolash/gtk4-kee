#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <gcrypt.h>

#include "testutil.h"
#include "ledger.h"
#include "err.h"
#include "content.h"
#include "digest.h"


int kee_test_generate(struct kee_test_t *t) {
	int r;
	char *p;
	char path[64];
	const char *version;
	char out[1024];
	size_t out_len;
	struct kee_ledger_item_t *item;
	struct kee_content_t *content_item;

	memset(t, 0, sizeof(struct kee_test_t));
	t->content_item = malloc(sizeof(struct kee_content_t*) * 2);		
	*t->content_item = malloc(sizeof(struct kee_content_t));
	*(t->content_item+1) = NULL;

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

	kee_ledger_init(&t->ledger);

	gpg_store_init(&t->gpg, p);
	t->gpg.k = &t->alice;
	r = gpg_key_create(&t->gpg, "1234"); // alice
	if (r) {
		return 1;
	}
	memcpy(t->ledger.pubkey_alice, t->gpg.public_key, PUBKEY_LENGTH);
	memcpy(t->alice_fingerprint, t->gpg.fingerprint, FINGERPRINT_LENGTH);

	gpg_store_init(&t->gpg, p);
	t->gpg.k = &t->bob;
	r = gpg_key_create(&t->gpg, "1234"); // bob
	if (r) {
		return 1;
	}
	memcpy(t->ledger.pubkey_bob, t->gpg.public_key, PUBKEY_LENGTH);
	memcpy(t->alice_fingerprint, t->gpg.fingerprint, FINGERPRINT_LENGTH);

	strcpy(t->ledger.uoa, "USD");
	t->ledger.uoa_decimals = 2;

	r = calculate_digest_algo(content_test, strlen(content_test), t->content_ledger.key, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	r = kee_content_init(&t->content_ledger, t->content_ledger.key, 0);
	if (r) {
		return 1;
	}
	r = calculate_digest_algo(content_test, strlen(content_test), t->content_ledger.key, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	out_len = 1024;
	r = kee_ledger_serialize(&t->ledger, out, &out_len);
	if (r) {
		return 1;
	}

	r = calculate_digest_algo(out, out_len, t->ledger.digest, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	item = kee_ledger_add_item(&t->ledger);
	item->alice_credit_delta = 666;
	item->bob_credit_delta = -42;
	item->alice_collateral_delta = 1024;
	item->bob_collateral_delta = 2048;
	r = clock_gettime(CLOCK_REALTIME, &item->time);
	if (r) {
		return 1;
	}
	item->initiator = ALICE;
	item->response = 1;

	content_item = *t->content_item;
	r = calculate_digest_algo(content_test_item, strlen(content_test_item), content_item->key, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}
	r = kee_content_init(content_item, content_item->key, 0);
	if (r) {
		return 1;
	}
	r = calculate_digest_algo(content_test_item, strlen(content_test_item), content_item->key, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	return ERR_OK;
}

void kee_test_free(struct kee_test_t *t) {
	struct kee_content_t **p;

	p = t->content_item;
	if (p == NULL) {
		return;
	}
	while (*p != NULL) {
		free(*p);
		p++;
	}
}

size_t kee_test_get_ledger_data(struct kee_test_t *t, char **out) {
	int r;

	t->ledger_bytes_len = 1024;
	r = kee_ledger_serialize(&t->ledger, t->ledger_bytes, &t->ledger_bytes_len);
	if (r) {
		return 0;
	}
	return (size_t)t->ledger_bytes_len;
}

size_t kee_test_get_ledger_item_data(struct kee_test_t *t, int idx, char **out) {
	int r;
	int i;
	struct kee_ledger_item_t *item;

	item = t->ledger.last_item;
	i = 0;
	while (i < idx) {
		if (item->prev_item == NULL) {
			return ERR_FAIL;
		}
		item = item->prev_item;
		i++;
	}

	t->ledger_item_bytes_len = 1024;
	r = kee_ledger_item_serialize(item, t->ledger_item_bytes, &t->ledger_item_bytes_len, KEE_LEDGER_ITEM_SERIALIZE_REQUEST);
	if (r) {
		return 0;
	}
	return (size_t)t->ledger_item_bytes_len;
}
