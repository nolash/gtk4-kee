#include <string.h>
#include <gtk/gtk.h>
#include "ui.h"
#include "db.h"


static void act_scan(GSimpleAction *act, GVariant *param, GApplication *app) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "scan clicked");
}

static void act_quit(GSimpleAction *act, GVariant *param, GApplication *app) {
	g_application_quit(app);
}

static void startup(GtkApplication *app, gpointer user_data) {
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
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(act_quit), app);

	act = g_simple_action_new("scan", NULL);
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(act_scan), app);

	g_menu_item_set_submenu(menu_item_menu, G_MENU_MODEL(menu));
	g_object_unref(menu);

	g_menu_append_item(menu_bar, menu_item_menu);
	g_object_unref(menu_item_menu);

	gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(menu_bar));
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "set up menus");
}

static void activate(GtkApplication *app, gpointer user_data) {
	struct ui_container *ui;

	ui = (struct ui_container*)user_data;
	ui_build(app, ui);
}

static void deactivate(GtkApplication *app, gpointer user_data) {
	struct ui_container *ui;

	ui = (struct ui_container*)user_data;
	ui_free(ui);
}

int main(int argc, char **argv) {
	struct ui_container ui;
	int r;
	GtkApplication *app;
	struct db_ctx db;

	db_connect(&db, "./testdata_mdb");

	app = gtk_application_new ("no.holbrook.example.Buidler", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "startup", G_CALLBACK (startup), &ui);
	g_signal_connect (app, "activate", G_CALLBACK (activate), &ui);
	g_signal_connect (app, "shutdown", G_CALLBACK (deactivate), &ui);
	r = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref(app);
	return r;
}
