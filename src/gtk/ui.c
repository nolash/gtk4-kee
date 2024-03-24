#include <string.h>

#include <gtk/gtk.h>
#include <gst/gst.h>

#include "ui.h"
#include "err.h"
#include "scan.h"
#include "settings.h"
#include "context.h"
#include "state.h"
#include "view.h"
#include "menu.h"
#include "kee-import.h"
#include "kee-entry-list.h"
#include "kee-entry-store.h"
#include "kee-menu.h"
#include "kee-key.h"


static void ui_handle_unlock(GtkWidget *widget, KeeMenu *menu) {
	kee_state_t state_delta;

	kee_state_zero(&state_delta);

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "key is unlocked");
	kee_menu_prev(menu);
}

//
//static GtkWidget* ui_build_view(KeeMenu *menu) {
//	GtkListItemFactory *factory;
//	GtkSelectionModel *sel;
//	GListModel *front_list;
//	GtkListView *front_view;
//
//	factory = gtk_signal_list_item_factory_new();
//	g_signal_connect(factory, "setup", G_CALLBACK(new_item), NULL);
//	
//	front_list = G_LIST_MODEL(gtk_string_list_new(NULL));
//
//	sel = GTK_SELECTION_MODEL(gtk_single_selection_new(front_list));
//	front_view = GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(sel), factory));
//
//	return GTK_WIDGET(front_view);
//}


void ui_build(GtkApplication *app, struct kee_context *ctx) {
	GtkWidget *widget;
	KeeMenu *win;
	KeeImport *import;
	KeeEntryStore *store;

	win = kee_menu_new(app);

	widget = GTK_WIDGET(kee_key_new());
	kee_menu_add(win, "unlock", widget);
	g_signal_connect (widget, "unlock", G_CALLBACK(ui_handle_unlock), win);

//	widget = ui_build_view(NULL);
	store = kee_entry_store_new(&ctx->db);
	widget = kee_entry_list_new(G_LIST_MODEL(store));
	kee_menu_add(win, "view", widget);
	
	kee_menu_next(win, "view");
	kee_menu_next(win, "unlock");

	import = kee_import_new(win);
	kee_menu_add(win, "import", GTK_WIDGET(import));

	gtk_window_present(GTK_WINDOW (win));
}

//void ui_build_from_resource(GtkApplication *app, struct ui_container *ui) {
//	GtkBuilder *build;
//	GtkWidget *unlock;
//
//	build = gtk_builder_new_from_resource("/org/defalsify/Kee/main.ui");
//	//ui->view = GTK_WINDOW(gtk_builder_get_object(build, "keechoose"));
//	unlock = GTK_WINDOW(gtk_builder_get_object(build, "keeunlock"));
//	if (!unlock) {
//		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "unlock widget could not load");
//	}
//
//	ui->win = GTK_APPLICATION_WINDOW(gtk_application_window_new (app));
//	gtk_window_set_child(GTK_WINDOW(ui->win), GTK_WIDGET(unlock));
//
//	gtk_window_present(GTK_WINDOW (ui->win));
//}
