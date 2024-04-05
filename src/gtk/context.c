#include <string.h>

#include "context.h"
#include "settings.h"
#include "camera.h"
#include "err.h"
#include "gpg.h"


int kee_context_new(struct kee_context *ctx, struct kee_settings *settings) {
	memset(ctx, 0, sizeof(struct kee_context));
	ctx->state = 1;
	ctx->settings = settings;

	return ERR_OK;
}

int kee_context_state(struct kee_context *ctx) {
	return ctx->state;
}

void kee_context_free(struct kee_context *ctx) {
}
