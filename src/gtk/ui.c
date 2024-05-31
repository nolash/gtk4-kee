#include <string.h>

#include <gtk/gtk.h>
#include <gst/gst.h>

#include "ui.h"
#include "err.h"
#include "scan.h"
#include "settings.h"
#include "context.h"
#include "state.h"
//#include "view.h"
#include "menu.h"
#include "kee-import.h"
#include "kee-entry-list.h"
#include "kee-entry-store.h"
#include "kee-menu.h"
#include "kee-key.h"
#include "kee-transport.h"
#include "transport.h"
#include "debug.h"


static void ui_handle_unlock(KeeKey *o, KeeMenu *menu) {
	kee_state_t state_delta;
	char fingerprint[41];

	kee_state_zero(&state_delta);
	kee_key_get_fingerprint(o, fingerprint);
	gtk_window_set_title(GTK_WINDOW(menu), fingerprint);
	
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "key is unlocked");
	kee_menu_prev(menu, 1);
}

//static void ui_handle_import(KeeImport *import, GString *v, KeeMenu *menu) {
//	//GtkWidget *widget;
//	int r;
//	struct kee_transport_t trans;
//	char *s;
//
//	s = (char*)v->str;
//	r = kee_transport_import(&trans, KEE_TRANSPORT_BASE64, s, strlen(s) + 1);
//	if (r) {
//		debug_log(DEBUG_INFO, "invalid input for transport");
//		return;
//	}
//	//kee_transport_read(&trans);
//
//	//switch(kee_transport_cmd(&trans)) {
//	//	case KEE_CMD_DELTA:
//	//		widget = kee_menu_next("item");
//	//}
//
//}

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


void ui_build(GtkApplication *gapp, struct kee_context *ctx) {
	KeeTransport *trans;
	GSimpleAction *act;
	GtkWidget *widget;
	KeeMenu *win;
	KeeImport *import;

	win = kee_menu_new(gapp, ctx);
	if (!win) {
		debug_log(DEBUG_CRITICAL, "menu init fail");
	}

	widget = GTK_WIDGET(kee_key_new(&ctx->gpg));
	//kee_menu_add(win, "unlock", widget);
	kee_menu_add(win, beamenu_dst_r[BEAMENU_DST_KEY], widget);
	g_signal_connect (widget, "unlock", G_CALLBACK(ui_handle_unlock), win);

	widget = kee_entry_list_new(G_LIST_MODEL(ctx->entry_store), win);
	//kee_menu_add(win, "view", widget);
	kee_menu_add(win, beamenu_dst_r[BEAMENU_DST_LIST], widget);

	//kee_menu_next(win, "view");
	kee_menu_next(win, BEAMENU_DST_LIST);
	kee_menu_next(win, BEAMENU_DST_KEY);

	import = kee_import_new(win);
	//kee_menu_add(win, "import", GTK_WIDGET(import));
	kee_menu_add(win, beamenu_dst_r[BEAMENU_DST_IMPORT], GTK_WIDGET(import));
	//g_signal_connect(import, "data_available", G_CALLBACK(ui_handle_import), win);

	widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	//kee_menu_add(win, "entry", widget);
	kee_menu_add(win, beamenu_dst_r[BEAMENU_DST_NEW], widget);

	trans = g_object_new(KEE_TYPE_TRANSPORT, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	//kee_menu_add(win, "transport", GTK_WIDGET(trans));
	kee_menu_add(win, beamenu_dst_r[BEAMENU_DST_TRANSPORT], GTK_WIDGET(trans));

	/// \todo make kee-entry action map/group?
	act = g_simple_action_new("qr", G_VARIANT_TYPE_STRING);
	g_action_map_add_action(G_ACTION_MAP(gapp), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(kee_transport_handle_qr), trans);

	act = g_simple_action_new("commit", G_VARIANT_TYPE_STRING);
	g_action_map_add_action(G_ACTION_MAP(gapp), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(kee_transport_handle_qr), trans);

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
