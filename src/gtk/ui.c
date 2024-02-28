#include <gtk/gtk.h>
#include "ui.h"

/***
 * \todo change file to resource
 */

static void new_item(GtkListItemFactory *factory, GtkListItem *item, gpointer user_data) {
}

static void unlock_click(GtkWidget *button, gpointer user_data) {
	struct ui_container *ui;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "click");

	ui = (struct ui_container*)user_data;
	gtk_stack_set_visible_child(ui->stack, GTK_WIDGET(ui->front_view));
}

GtkWidget* ui_build_unlock(struct ui_container *ui) {
	GtkWidget *box;
	GtkWidget *entry;
	GtkWidget *button;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	entry = gtk_entry_new();
	gtk_box_append(GTK_BOX(box), entry);

	button = gtk_button_new_with_label("create");
	gtk_box_append(GTK_BOX(box), button);
	g_signal_connect (button, "clicked", G_CALLBACK (unlock_click), ui);

	return GTK_WIDGET(box);
}

GtkWidget* ui_build_view(struct ui_container *ui) {
	GtkSelectionModel *sel;
	GtkListItemFactory *factory;

	ui->front_list = G_LIST_MODEL(gtk_string_list_new(NULL));
	sel = GTK_SELECTION_MODEL(gtk_single_selection_new(ui->front_list));
	factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(new_item), NULL);
	ui->front_view = GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(sel), factory));

	return GTK_WIDGET(ui->front_view);
}

void ui_build(GtkApplication *app, struct ui_container *ui) {
	GtkWidget *unlock;
	GtkWidget *view;

	ui->win = GTK_APPLICATION_WINDOW(gtk_application_window_new (app));
	ui->stack = GTK_STACK(gtk_stack_new());

	gtk_window_set_title (GTK_WINDOW (ui->win), "kee");
	gtk_window_set_default_size (GTK_WINDOW (ui->win), 800, 600);
	
	gtk_window_set_child(GTK_WINDOW(ui->win), GTK_WIDGET(ui->stack));
	gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(ui->win), TRUE);

	unlock = ui_build_unlock(ui);
	gtk_stack_add_child(ui->stack, unlock);
	view = ui_build_view(ui);
	gtk_stack_add_child(ui->stack, view);

	gtk_stack_set_visible_child(GTK_STACK(ui->stack), unlock);
	//gtk_stack_set_visible_child(GTK_STACK(ui->stack), view);

	gtk_window_present(GTK_WINDOW (ui->win));
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
//


void ui_free(struct ui_container *ui) {
	//g_object_unref(ui->view);
	//g_object_unref(ui->unlock);
	//g_object_unref(ui->win);
}
