#ifndef _KEE_CONTEXT
#define _KEE_CONTEXT

#include "settings.h"
#include "db.h"
#include "camera.h"

struct kee_context {
	void *front;
	struct kee_settings *settings;
	struct kee_camera_devices camera_devices;
	struct db_ctx db;
	int state;
};

int kee_context_new(struct kee_context *ctx, void *front, struct kee_settings *settings);
int kee_context_state(struct kee_context *ctx);
void kee_context_free(struct kee_context *ctx);

#endif // _KEE_CONTEXT
