#ifndef _GTK_KEE_UICONTEXT_H
#define _GTK_KEE_UICONTEXT_H

#include <glib-object.h>

#include "context.h"
#include "state.h"

G_BEGIN_DECLS


enum KEE_SIGS {
	KEE_S_STATE_CHANGE,
	KEE_S_KEY_UNLOCKED,
	KEE_N_SIGS,
};

#define KEE_TYPE_UICONTEXT kee_uicontext_get_type()
G_DECLARE_FINAL_TYPE(KeeUicontext, kee_uicontext, KEE, UICONTEXT, GObject)

KeeUicontext* kee_uicontext_new(void);
kee_state_t kee_uicontext_state(KeeUicontext *o);
void kee_uicontext_unlock(KeeUicontext *o);

G_END_DECLS

#endif
