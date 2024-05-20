#ifndef KEE_LEDGER_H_
#define KEE_LEDGER_H_

#include <time.h>

#include "content.h"
#include "cadiz.h"
#include "gpg.h"
#include "db.h"
#include "digest.h"

#define ERRR_PFX_LEDGER 0x200
/// Ledger state has already been signed
#define ERR_ALREADY_SIGNED 0x201

enum kee_initiator_e {
	NOONE,
	ALICE,
	BOB,
};

enum kee_ledger_state_e {
	KEE_LEDGER_STATE_REQUEST,
	KEE_LEDGER_STATE_RESPONSE,
	KEE_LEDGER_STATE_FINAL,
};

struct kee_ledger_item_t {
	struct kee_ledger_item_t *prev_item;
	int alice_credit_delta;
	int bob_credit_delta;
	int alice_collateral_delta;
	int bob_collateral_delta;
	struct timespec time;
	enum kee_initiator_e initiator;
	char response;
	char alice_signature[SIGNATURE_LENGTH];
	char bob_signature[SIGNATURE_LENGTH];
	struct kee_content_t content;
};

struct kee_ledger_cache_t {
	int count;
	int alice_credit_balance;
	int bob_credit_balance;
	int alice_collateral_balance;
	int bob_collateral_balance;
};

struct kee_ledger_t {
	char digest[DIGEST_LENGTH];
	struct kee_ledger_item_t *last_item;
	char pubkey_alice[PUBKEY_LENGTH];
	char pubkey_bob[PUBKEY_LENGTH];
	char uoa_decimals;
	char uoa[64];
	struct kee_content_t content;
	struct kee_ledger_cache_t *cache;
};

int kee_ledger_parse(struct kee_ledger_t *ledger, const char *data, size_t data_len);
int kee_ledger_serialize(struct kee_ledger_t *ledger, char *out, size_t *out_len);
void kee_ledger_init(struct kee_ledger_t *ledger);
void kee_ledger_free(struct kee_ledger_t *ledger);
void kee_ledger_resolve(struct kee_ledger_t *ledger, Cadiz *cadiz);
void kee_ledger_reset_cache(struct kee_ledger_t *ledger);
int kee_ledger_sign(struct kee_ledger_t *ledger, struct kee_ledger_item_t *item, struct gpg_store *gpg, const char *passphrase);
int kee_ledger_serialize_open(struct kee_ledger_t *ledger, char *out, size_t *out_len);
int kee_ledger_parse_open(struct kee_ledger_t *ledger, struct gpg_store *gpg, const char *in, size_t in_len);
int kee_ledger_put(struct kee_ledger_t *ledger, struct db_ctx *db);
int kee_ledger_item_put(struct kee_ledger_t *ledger, struct db_ctx *db, int idx);
int kee_ledger_verify(struct kee_ledger_t *ledger, int *err_idx);

struct kee_ledger_item_t *kee_ledger_parse_item(struct kee_ledger_t *ledger, const char *data, size_t data_len, enum kee_initiator_e initiator);
struct kee_ledger_item_t *kee_ledger_parse_item_db(struct kee_ledger_t *ledger, const char *data, size_t data_len);
struct kee_ledger_item_t *kee_ledger_add_item(struct kee_ledger_t *ledger);
void kee_ledger_item_init(struct kee_ledger_item_t *item);
int kee_ledger_item_serialize(struct kee_ledger_item_t *item, char *out, size_t *out_len, enum kee_ledger_state_e mode);
void kee_ledger_item_free(struct kee_ledger_item_t *item);
enum kee_ledger_state_e kee_ledger_item_state(struct kee_ledger_item_t *item);
enum kee_initiator_e kee_ledger_item_initiator(struct kee_ledger_t *ledger, struct gpg_store *gpg, struct kee_ledger_item_t *item);

#endif
