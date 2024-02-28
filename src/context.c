#include <string.h>
#include "context.h"
#include "settings.h"

void kee_context_new(struct kee_context *ctx, void *front) {
	memset(ctx, 0, sizeof(struct kee_context));
	ctx->front = front;
}
