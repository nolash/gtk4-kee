#include <string.h>
#include "context.h"
#include "settings.h"

void kee_context_new(struct kee_context *ctx, void *front, struct kee_settings *settings) {
	memset(ctx, 0, sizeof(struct kee_context));
	ctx->front = front;
	ctx->state = 1;
	ctx->settings = settings;
}

int kee_context_state(struct kee_context *ctx) {
	return ctx->state;
}
