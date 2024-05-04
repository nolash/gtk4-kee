#ifndef KEE_TEST_H_
#define KEE_TEST_H_

#include <gcrypt.h>

#include "ledger.h"
#include "db.h"

static const char *content_test = "Subject: foo\n\nsome content\n";
static const char *content_test_item = "Subject: bar\n\nsome other content\n";


struct kee_test_t {
	struct kee_ledger_t ledger;
	struct gpg_store gpg;
	gcry_sexp_t alice;
	gcry_sexp_t bob;
	char alice_fingerprint[20];
	char bob_fingerprint[20];
	struct kee_content_t content_ledger;
	struct kee_content_t **content_item; //last first
	char ledger_bytes[1024];
	size_t ledger_bytes_len;
	char ledger_item_bytes[1024];
	size_t ledger_item_bytes_len;
	size_t item_count;
	struct db_ctx db;
};

int kee_test_generate(struct kee_test_t *t);
int kee_test_ledger_data(struct kee_test_t *t);
void kee_test_free(struct kee_test_t *t);
size_t kee_test_get_ledger_data(struct kee_test_t *t, char **out);
size_t kee_test_get_ledger_item_data(struct kee_test_t *t, int idx, char **out);
int kee_test_db(struct kee_test_t *t);
int kee_test_sign_request(struct kee_test_t *t);
int kee_test_sign_response(struct kee_test_t *t);
//void kee_test_swap_identities(struct kee_test_t *t);

#endif
