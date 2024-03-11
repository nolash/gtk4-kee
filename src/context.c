#include <string.h>

#include "context.h"
#include "settings.h"
#include "camera.h"
#include "err.h"


int kee_context_new(struct kee_context *ctx, void *front, struct kee_settings *settings) {
	int r;

	memset(ctx, 0, sizeof(struct kee_context));
	ctx->front = front;
	ctx->state = 1;
	ctx->settings = settings;

	r = kee_camera_scan(&ctx->camera_devices);
	if (r) {
		return r;
	}
	return ERR_OK;
}

int kee_context_state(struct kee_context *ctx) {
	return ctx->state;
}

void kee_context_free(struct kee_context *ctx) {
	kee_camera_free(&ctx->camera_devices);
}
