#ifndef _KEE_CONTEXT
#define _KEE_CONTEXT

#include "settings.h"
#include "db.h"

struct kee_context {
	void *front;
	struct kee_settings settings;
	struct db_ctx db;
};

void kee_context_new(struct kee_context *ctx, void *front);

#endif // _KEE_CONTEXT
