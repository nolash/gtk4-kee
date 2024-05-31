#ifndef _GTK_KEE_MENU_H
#define _GTK_KEE_MENU_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "beamenu_defs.h"
#include "context.h"

/// \todo rename to kee-win

enum KEE_MENU_PROPS {
	KEE_N_MENU_PROPS,
};

enum KEE_MENU_SIGS {
	KEE_N_MENU_CHANGE,
	KEE_N_MENU_SIGS,
};

G_BEGIN_DECLS

#define KEE_TYPE_MENU kee_menu_get_type()
G_DECLARE_FINAL_TYPE(KeeMenu, kee_menu, KEE, MENU, GtkApplicationWindow);

KeeMenu* kee_menu_new(GtkApplication *app, struct kee_context *ctx);
int kee_menu_add(KeeMenu *o, const char *k, GtkWidget *v);
//GtkWidget* kee_menu_next(KeeMenu *o, const char *k);
GtkWidget* kee_menu_next(KeeMenu *o, int menu_id);
int kee_menu_prev(KeeMenu *o, int force);
int kee_menu_set(KeeMenu *o, GtkWidget *widget);

G_END_DECLS

#endif //_GTK_KEE_MENU_H
