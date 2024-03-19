#ifndef _GTK_KEE_UICONTEXT_H
#define _GTK_KEE_UICONTEXT_H

#include <glib-object.h>

#include "context.h"
#include "state.h"

#define KEE_W_FRONTLIST "frontlist"
#define KEE_W_CAMERA_VIEWFINDER "camera_view"
#define KEE_W_WINDOW "win"
#define KEE_W_PASSPHRASE "passphrase"

#define KEE_ST_UI_HEAD_ADD 0x1

G_BEGIN_DECLS


enum KEE_SIGS {
	KEE_S_STATE_CHANGE,
	KEE_S_SCAN_CHANGE,
	KEE_S_KEY_UNLOCKED,
	KEE_N_SIGS,
};

enum KEE_PROPS {
	CORE_CONTEXT = 1,
	CAMERA_LIST,
	GAPP,
	KEE_N_PROPS,
};

#define KEE_TYPE_UICONTEXT kee_uicontext_get_type()
G_DECLARE_FINAL_TYPE(KeeUicontext, kee_uicontext, KEE, UICONTEXT, GObject)

KeeUicontext* kee_uicontext_new(void);
void kee_uicontext_scaninit(KeeUicontext *o);
void kee_uicontext_scanstart(KeeUicontext *o);
void kee_uicontext_scanadd(KeeUicontext *o, GtkLabel *label);
void kee_uicontext_scanchange(KeeUicontext *o, const char *devices);
kee_state_t kee_uicontext_state(KeeUicontext *o);
void kee_uicontext_unlock(KeeUicontext *o);
void kee_uicontext_state_change(KeeUicontext *o, kee_state_t *add, kee_state_t *sub);

G_END_DECLS

#endif
