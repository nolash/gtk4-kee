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
#include "kee-menu.h"


static void new_item(GtkListItemFactory *factory, GtkListItem *item, gpointer user_data) {
}


//
//static void win_handle_state(KeeUicontext *uctx, char state_hint, kee_state_t *new_state, kee_state_t *old_state, GtkWindow *win) {
//	GAction *act;
//
//	if (!(state_hint & KEE_ST_HINT_UI_MENU)) {
//		return;
//	}
//
//	if (new_state->ui_menu & KEE_ST_UI_HEAD_ADD) {
//		act = g_action_map_lookup_action(G_ACTION_MAP(win), "import");
//		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
//	}
//}




// \todo why is there user_data in addition to pointer



void ui_handle_unlock(KeeUicontext *uctx, gpointer user_data) {
	kee_state_t state_delta;

	kee_state_zero(&state_delta);

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "key is unlocked");
	kee_view_prev();

	//state_delta.ui_menu |= KEE_ST_UI_HEAD_ADD;
	//kee_uicontext_state_change(uctx, &state_delta, NULL);
}


static void ui_handle_unlock_click(GtkWidget *button, KeeUicontext *uctx) {
	GtkEntryBuffer *buf;
	const char *passphrase;

	buf = g_object_get_data(G_OBJECT(uctx), "passphrase");
	passphrase = gtk_entry_buffer_get_text(buf);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "click");

	kee_uicontext_unlock(uctx);

	gtk_entry_buffer_delete_text(buf, 0, gtk_entry_buffer_get_length(buf));
}



GtkWidget* ui_build_unlock(KeeUicontext *uctx) {
	GtkWidget *box;
	GtkWidget *entry;
	GtkWidget *button;
	GtkEntryBuffer *buf;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	entry = gtk_entry_new();
	gtk_box_append(GTK_BOX(box), entry);
	buf = gtk_entry_get_buffer(GTK_ENTRY(entry));
	gtk_entry_set_input_purpose(GTK_ENTRY(entry), GTK_INPUT_PURPOSE_PASSWORD);
	gtk_entry_set_visibility(GTK_ENTRY(entry), false);
	//g_object_set_data(G_OBJECT(uctx), "passphrase", buf);

	button = gtk_button_new_with_label("create");
	gtk_box_append(GTK_BOX(box), button);
	g_signal_connect (button, "clicked", G_CALLBACK (ui_handle_unlock_click), uctx);

	return GTK_WIDGET(box);
}





static GtkWidget* ui_build_view(KeeUicontext *uctx) {
	GtkListItemFactory *factory;
	GtkSelectionModel *sel;
	GListModel *front_list;
	GtkListView *front_view;

	factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(new_item), NULL);
	
	front_list = G_LIST_MODEL(gtk_string_list_new(NULL));
	//g_object_set_data(G_OBJECT(uctx), KEE_W_FRONTLIST, front_list);

	sel = GTK_SELECTION_MODEL(gtk_single_selection_new(front_list));
	front_view = GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(sel), factory));

	return GTK_WIDGET(front_view);
}


void ui_build(GtkApplication *app) {
	GtkWidget *widget;
	GtkWidget *win;
	GtkWidget *import;

	win = kee_menu_new(app); //g_object_new(KEE_TYPE_MENU, "application", app, NULL);

	widget = ui_build_unlock(NULL);
	kee_menu_add(KEE_MENU(win), "unlock", widget);

	widget = ui_build_view(NULL);
	kee_menu_add(KEE_MENU(win), "view", widget);
	
	//widget = ui_build_scan(uctx);
	//kee_view_add(widget, "import");

	kee_menu_next(KEE_MENU(win), "view");
	kee_menu_next(KEE_MENU(win), "unlock");
	
	//g_object_set_data(G_OBJECT(uctx), KEE_W_WINDOW, GTK_WINDOW(win));

	//widget = g_object_get_data(G_OBJECT(uctx), KEE_W_UI_MENU_QUICK_ADD);

	//g_signal_connect (uctx, "state", G_CALLBACK(win_handle_state), win);

	import = kee_import_new(win);
	//import = g_object_new(KEE_TYPE_IMPORT, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	kee_menu_add(win, "import", import);

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
