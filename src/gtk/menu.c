#include <gtk/gtk.h>

#include "kee-uicontext.h"
#include "ui.h"
#include "scan.h"
#include "context.h"


static void act_quit(GSimpleAction *act, GVariant *param, KeeUicontext *ui) {
	GApplication *gapp;
	g_object_get(ui, "app", &gapp, NULL);

	g_application_quit(gapp);
}

void menu_setup(KeeUicontext *ctx_ui) {
	GMenu *menu;
	GMenuItem *menu_item;
	GtkWidget *butt;
	GtkWidget *butt_back;
	GSimpleAction *act;
	GApplication *gapp;
	GtkWidget *head;

	g_object_get(ctx_ui, "app", &gapp, NULL);

	menu = g_menu_new();
	menu_item = g_menu_item_new("Scan", "app.scan");
	g_menu_append_item(menu, menu_item);
	g_object_unref(menu_item);

	menu_item = g_menu_item_new("Quit", "app.quit");
	g_menu_append_item(menu, menu_item);
	g_object_unref(menu_item);

	act = g_simple_action_new("quit", NULL);
	g_action_map_add_action(G_ACTION_MAP(gapp), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(act_quit), ctx_ui);

	act = g_simple_action_new("scan", NULL);
	g_action_map_add_action(G_ACTION_MAP(gapp), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(scan_act), ctx_ui);

	head = gtk_header_bar_new();

	butt = gtk_menu_button_new();
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(butt), G_MENU_MODEL(menu));
	gtk_menu_button_set_primary(GTK_MENU_BUTTON(butt), true);
	gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(butt), "preferences-system");
	gtk_header_bar_pack_end(GTK_HEADER_BAR(head), butt);

	butt_back = gtk_button_new_from_icon_name("go-previous");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(head), butt_back);
	gtk_widget_set_visible(butt_back, false);

	g_object_set(ctx_ui, "ui_header", GTK_HEADER_BAR(head), NULL);

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "set up menus");
}
