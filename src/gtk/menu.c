#include <gtk/gtk.h>

#include "ui.h"


static void act_scan(GSimpleAction *act, GVariant *param, GApplication *app) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "scan clicked");
}

static void act_quit(GSimpleAction *act, GVariant *param, GApplication *app) {
	g_application_quit(app);
}

void menu_setup(struct ui_container *ui) {
	GMenu *menu_bar;
	GMenu *menu;
	GMenuItem *menu_item;
	GMenuItem *menu_item_menu;
	GSimpleAction *act;

	menu_bar = g_menu_new();
	menu_item_menu = g_menu_item_new("Menu", NULL);
	menu = g_menu_new();
	menu_item = g_menu_item_new("Scan", "app.scan");
	g_menu_append_item(menu, menu_item);
	g_object_unref(menu_item);

	menu_item = g_menu_item_new("Quit", "app.quit");
	g_menu_append_item(menu, menu_item);
	g_object_unref(menu_item);

	act = g_simple_action_new("quit", NULL);
	g_action_map_add_action(G_ACTION_MAP(ui->gapp), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(act_quit), ui->gapp);

	act = g_simple_action_new("scan", NULL);
	g_action_map_add_action(G_ACTION_MAP(ui->gapp), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(act_scan), ui->gapp);

	g_menu_item_set_submenu(menu_item_menu, G_MENU_MODEL(menu));
	g_object_unref(menu);

	g_menu_append_item(menu_bar, menu_item_menu);
	g_object_unref(menu_item_menu);

	gtk_application_set_menubar(GTK_APPLICATION(ui->gapp), G_MENU_MODEL(menu_bar));
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "set up menus");
}
