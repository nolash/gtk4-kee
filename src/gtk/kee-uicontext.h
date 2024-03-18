#ifndef _GTK_KEE_UICONTEXT_H
#define _GTK_KEE_UICONTEXT_H

#include <glib-object.h>
#include "context.h"

#define KEE_W_FRONTLIST "frontlist"
#define KEE_W_CAMERA_VIEWFINDER "camera_view"
#define KEE_W_WINDOW "win"
#define KEE_W_HEADER "header"

G_BEGIN_DECLS

typedef struct {
	int ui;
	int ctx;
} KeeState;

enum KEE_SIGS {
	SCAN_WANT,
	SCAN_CHANGE,
	KEE_N_SIGS,
};

enum KEE_PROPS {
	CORE_CONTEXT = 1,
//	UI_CONTAINER,
//	UI_HEADER,
//	UI_LIST,
//	UI_WINDOW,
//	UI_PUSH,
	CAMERA_LIST,
//	CAMERA_SCAN,
//	CAMERA_DEVICE,
//	CAMERA_VIEW,
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
KeeState kee_uicontext_state(KeeUicontext *o);

G_END_DECLS

#endif
