#ifndef _GTK_KEE_UICONTEXT_H
#define _GTK_KEE_UICONTEXT_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct {
	int ui;
	int ctx;
} KeeState;

enum KEE_SIGS {
	SCAN_WANT,
	KEE_N_SIGS,
};

enum KEE_PROPS {
	CORE_CONTEXT = 1,
	UI_CONTAINER,
	GAPP,
	KEE_N_PROPS,
};

#define KEE_TYPE_UICONTEXT kee_uicontext_get_type()
G_DECLARE_FINAL_TYPE(KeeUicontext, kee_uicontext, KEE, UICONTEXT, GObject)

KeeUicontext* kee_uicontext_new(void);
void kee_uicontext_scanstart(KeeUicontext *o);
KeeState kee_uicontext_state(KeeUicontext *o);

G_END_DECLS

#endif
