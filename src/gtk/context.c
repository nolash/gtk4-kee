#include <string.h>

#include "context.h"
#include "settings.h"
#include "camera.h"
#include "err.h"
#include "gpg.h"
#include "db.h"

#define G_LOG_DOMAIN "Kee"


int kee_context_init(struct kee_context *ctx, struct kee_settings *settings) {
	unsigned char *v;

	memset(ctx, 0, sizeof(struct kee_context));
	ctx->state = 1;
	ctx->settings = settings;
	db_connect(&ctx->db, "./testdata_mdb");
	v = settings_get(ctx->settings, SETTINGS_KEY);
	gpg_store_init(&ctx->gpg, (char*)v);
	ctx->entry_store = kee_entry_store_new(&ctx->db);
	kee_entry_store_set_resolve(ctx->entry_store, "./testdata_resource");
	return ERR_OK;
}

int kee_context_state(struct kee_context *ctx) {
	return ctx->state;
}

void kee_context_free(struct kee_context *ctx) {
}
