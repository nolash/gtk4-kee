#ifndef KEE_LEDGER_H_
#define KEE_LEDGER_H_

#include <time.h>

#include "content.h"

enum kee_initiator {
	ALICE,
	BOB,
};

struct kee_ledger_item_t {
	struct kee_ledger_item_t *prev_item;
	int alice_credit_delta;
	int bob_credit_delta;
	int alice_collateral_delta;
	int bob_collateral_delta;
	time_t time;
	enum kee_initiator initiator;
	char response;
	struct kee_content_t content;
};

struct kee_ledger_t {
	struct kee_ledger_item_t *last_item;
	char pubkey_alice[32];
	char pubkey_bob[32];
	char uoa_decimals;
	char uoa[64];
	struct kee_content_t content;
};

struct kee_ledger_item_t *kee_ledger_parse_item(struct kee_ledger_t *ledger, const char *data, size_t data_len);
int kee_ledger_parse(struct kee_ledger_t *ledger, const char *data, size_t data_len);
void kee_ledger_free(struct kee_ledger_t *ledger);
void kee_ledger_item_free(struct kee_ledger_item_t *item);

#endif
