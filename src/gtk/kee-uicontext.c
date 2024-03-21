#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-uicontext.h"
//#include "ui.h"
#include "context.h"
#include "state.h"
#include "settings.h"


typedef struct {
} KeeUicontextPrivate;

struct _KeeUicontext {
	GObject parent;
	kee_state_t state;
};

G_DEFINE_TYPE(KeeUicontext, kee_uicontext, G_TYPE_OBJECT)

static guint kee_sigs[KEE_N_SIGS] = {0,};

static void kee_uicontext_class_init(KeeUicontextClass *kls) {
	GObjectClass *o = G_OBJECT_CLASS(kls);

	kee_sigs[KEE_S_STATE_CHANGE] = g_signal_new("state", 
			G_TYPE_FROM_CLASS(o),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			0,
			NULL,
			NULL,
			NULL,
			G_TYPE_NONE,
			3,
			G_TYPE_CHAR,
			G_TYPE_POINTER,
			G_TYPE_POINTER
	);

	kee_sigs[KEE_S_KEY_UNLOCKED] = g_signal_new("unlock",
			G_TYPE_FROM_CLASS(o),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			0,
			NULL,
			NULL,
			NULL,
			G_TYPE_NONE,
			0,
			NULL
	);
}

static void kee_uicontext_init(KeeUicontext *o) {
	//KeeUicontextPrivate *o = kee_uicontext_get_instance_private(self);
}

void kee_uicontext_unlock(KeeUicontext *o) {
       g_signal_emit(o, kee_sigs[KEE_S_KEY_UNLOCKED], 0);
}

void kee_uicontext_state_change(KeeUicontext *o, kee_state_t *add, kee_state_t *sub) {
	kee_state_t old_state;
	char hint;

	hint = 0;

	if (add) {
		hint = kee_state_add(&o->state, add);
	}
	memcpy(&old_state, &o->state, sizeof(kee_state_t));
	g_signal_emit(o, kee_sigs[KEE_S_STATE_CHANGE], 0, hint, &o->state, &old_state);
}

