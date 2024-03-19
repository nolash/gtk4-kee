#include <gtk/gtk.h>

#include "kee-uicontext.h"
//#include "ui.h"
#include "scan.h"
#include "context.h"
#include "state.h"
#include "menu.h"


static void act_quit(GSimpleAction *act, GVariant *param, KeeUicontext *uctx) {
	GApplication *gapp;
	g_object_get(uctx, "gtk_application", &gapp, NULL);

	g_application_quit(gapp);
}

static void act_import(GSimpleAction *act, GVariant *param, KeeUicontext *uctx) {
	scan_act(act, param, uctx);
}

static void menu_handle_state(KeeUicontext *uctx, char state_hint, kee_state_t *new_state, kee_state_t *old_state, GObject *head) {
	GtkApplication *gapp;
	GtkWidget *widget;
	GAction *act;

	if (!(state_hint & KEE_ST_HINT_UI_MENU)) {
		return;
	}

	if (new_state->ui_menu & KEE_ST_UI_HEAD_ADD) {
		widget = g_object_get_data(head, KEE_W_UI_MENU_QUICK_ADD);
		gtk_widget_set_sensitive(widget, true);
		g_object_get(uctx, "gtk_application", &gapp, NULL);
		act = g_action_map_lookup_action(G_ACTION_MAP(gapp), "import");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
	}
}


static GtkWidget* menu_button_setup(GObject *head, GtkApplication *gapp, KeeUicontext *uctx) {
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
	g_signal_connect(act, "activate", G_CALLBACK(act_quit), uctx);

	act = g_simple_action_new("import", NULL);
	g_simple_action_set_enabled(act, false);
	g_action_map_add_action(G_ACTION_MAP(gapp), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(act_import), uctx);

	butt = gtk_menu_button_new();
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(butt), G_MENU_MODEL(menu));
	gtk_menu_button_set_primary(GTK_MENU_BUTTON(butt), true);
	gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(butt), "preferences-system");
	
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "set up menus");
	return butt;
}


void header_setup(GtkApplication *gapp, KeeUicontext *uctx) {
	GtkWidget *head;
	GtkWidget *butt;
	GtkToggleButton *butt_prev;
	head = gtk_header_bar_new();

	butt = menu_button_setup(G_OBJECT(head), gapp, uctx);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(head), butt);

	butt = gtk_button_new_from_icon_name("go-previous");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(head), butt);
	gtk_widget_set_visible(butt, false);

	butt = gtk_button_new_from_icon_name("insert-object");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(head), butt);
	gtk_widget_set_sensitive(butt, false);
	g_object_set_data(G_OBJECT(head), KEE_W_UI_MENU_QUICK_ADD, butt);

	butt = gtk_toggle_button_new();
	gtk_button_set_icon_name(GTK_BUTTON(butt), "insert-image");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(head), butt);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(butt), true);
	gtk_widget_set_visible(butt, false);

	butt_prev = GTK_TOGGLE_BUTTON(butt);
	butt = gtk_toggle_button_new();
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(butt), butt_prev);
	gtk_button_set_icon_name(GTK_BUTTON(butt), "document-new");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(head), butt);
	gtk_widget_set_visible(butt, false);

	butt_prev = GTK_TOGGLE_BUTTON(butt);
	butt = gtk_toggle_button_new();
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(butt), butt_prev);
	gtk_button_set_icon_name(GTK_BUTTON(butt), "document-save");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(head), butt);
	gtk_widget_set_visible(butt, false);

	g_object_set_data(G_OBJECT(uctx), KEE_W_HEADER, GTK_HEADER_BAR(head));

	g_signal_connect (uctx, "state", G_CALLBACK(menu_handle_state), head);


}


void menu_setup(GtkApplication *gapp, KeeUicontext *uctx) {
	header_setup(gapp, uctx);
}
