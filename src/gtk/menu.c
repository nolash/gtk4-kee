#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-uicontext.h"
#include "kee-menu.h"
//#include "ui.h"
#include "scan.h"
#include "context.h"
#include "state.h"
#include "menu.h"


static void act_quit(GAction *act, GVariant *param, GApplication *gapp) {
	g_application_quit(gapp);
}


static void menu_handle_state(KeeUicontext *uctx, char state_hint, kee_state_t *new_state, kee_state_t *old_state, GObject *head) {
}


GtkWidget* menu_button_setup(GObject *head, GtkApplication *gapp) {
	GMenu *menu;
	GMenuItem *menu_item;
	GtkWidget *butt;
	GSimpleAction *act;

	menu = g_menu_new();
	menu_item = g_menu_item_new("Import", "app.import");
	g_menu_append_item(menu, menu_item);
	g_object_unref(menu_item);

	menu_item = g_menu_item_new("Quit", "app.quit");
	g_menu_append_item(menu, menu_item);
	g_object_unref(menu_item);

	act = g_simple_action_new("quit", NULL);
	g_action_map_add_action(G_ACTION_MAP(gapp), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(act_quit), gapp);
	
	butt = gtk_menu_button_new();
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(butt), G_MENU_MODEL(menu));
	gtk_menu_button_set_primary(GTK_MENU_BUTTON(butt), true);
	gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(butt), "preferences-system");
	
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "set up menus");
	return butt;
}
