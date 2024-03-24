#ifndef _GTK_KEE_MENU_H
#define _GTK_KEE_MENU_H

#include <glib-object.h>
#include <gtk/gtk.h>

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

KeeMenu* kee_menu_new(GtkApplication *app);
int kee_menu_add(KeeMenu *o, const char *k, GtkWidget *v);
int kee_menu_next(KeeMenu *o, const char *k);
int kee_menu_prev(KeeMenu *o);

G_END_DECLS

#endif //_GTK_KEE_MENU_H
