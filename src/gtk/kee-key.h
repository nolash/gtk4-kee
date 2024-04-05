#ifndef _GTK_KEE_KEY_H
#define _GTK_KEE_KEY_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KEE_TYPE_KEY kee_key_get_type()
G_DECLARE_FINAL_TYPE(KeeKey, kee_key, KEE, KEY, GtkBox)

enum KEE_KEY_PROPS {
	KEE_P_KEY_STORE_PATH = 1,
	KEE_P_KEY_STORE,
	KEE_N_KEY_PROPS,
};

enum KEE_KEY_SIGS {
	KEE_S_KEY_UNLOCKED,
	KEE_N_KEY_SIGS,
};

G_END_DECLS

KeeKey* kee_key_new();
const char *kee_key_get_fingerprint(KeeKey *o, char *fingerprint);

#endif // _GTK_KEE_KEY_H
