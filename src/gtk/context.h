#ifndef _KEE_CONTEXT
#define _KEE_CONTEXT

#include "kee-entry-store.h"
#include "settings.h"
#include "db.h"
#include "camera.h"
#include "gpg.h"
#include "cadiz.h"


struct kee_context {
	void *front;
	struct kee_settings *settings;
	struct kee_camera_devices camera_devices;
	struct db_ctx db;
	struct gpg_store gpg;
	KeeEntryStore *entry_store;
	struct Cadiz resolver;
	int state;
};

int kee_context_init(struct kee_context *ctx, struct kee_settings *settings);
int kee_context_state(struct kee_context *ctx);
void kee_context_free(struct kee_context *ctx);

#endif // _KEE_CONTEXT
