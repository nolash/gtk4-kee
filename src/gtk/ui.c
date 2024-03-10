#include <string.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

#include "ui.h"
#include "err.h"
#include "scan.h"
#include "settings.h"
#include "context.h"


int ui_init(struct ui_container *ui) {
	gtk_init();
	gst_init(0, NULL);
	ui->gapp = gtk_application_new ("org.defalsify.Kee", G_APPLICATION_DEFAULT_FLAGS);
	if (ui->gapp == NULL) {
		return ERR_FAIL;
	}
	ui->state = 0;
	return ERR_OK;
}

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

static GtkWidget* ui_build_scan(struct ui_container *ui) {
	GtkWidget *label;

	ui->front_scan = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 10));

	label = gtk_label_new("please scan qr code");
	gtk_box_append(GTK_BOX(ui->front_scan), label);

	return GTK_WIDGET(ui->front_scan);
}


static GtkWidget* ui_build_view(struct ui_container *ui) {
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
	GtkWidget *widget;

	ui->win = GTK_APPLICATION_WINDOW(gtk_application_window_new (app));
	ui->stack = GTK_STACK(gtk_stack_new());

	gtk_window_set_title (GTK_WINDOW (ui->win), "kee");
	gtk_window_set_default_size (GTK_WINDOW (ui->win), 800, 600);
	
	gtk_window_set_child(GTK_WINDOW(ui->win), GTK_WIDGET(ui->stack));
	gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(ui->win), TRUE);

	widget = ui_build_unlock(ui);
	gtk_stack_add_child(ui->stack, widget);
	gtk_stack_set_visible_child(GTK_STACK(ui->stack), widget);
	widget = ui_build_view(ui);
	gtk_stack_add_child(ui->stack, widget);
	widget = ui_build_scan(ui);
	gtk_stack_add_child(ui->stack, widget);

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

int ui_state_change(struct ui_container *ui, int set, int reset) {
	ui->state |= set;
	return ui->state;	
}

gboolean ui_scan_code_handler(GstBus *bus, GstMessage *msg, gpointer user_data) {
	GError *err;
	gchar *debug_info;
	GstState oldstate;
	GstState newstate;
	GstState pendingstate;
	const gchar *src;
	const gchar *code;
	const GstStructure *strctr;
	//struct _gst_data *data;
	//GstStateChangeReturn rsc;

	//data = (struct _gst_data*)user_data;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "scan msg got");

	switch (GST_MESSAGE_TYPE (msg)) {
		case GST_MESSAGE_ERROR:
			gst_message_parse_error(msg, &err, &debug_info);
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "logg %s: %s", GST_OBJECT_NAME(msg->src), err->message);
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "debug %s", debug_info ? debug_info : "none");
			g_clear_error(&err);
			g_free(debug_info);
			break;
		case GST_MESSAGE_EOS:
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "eos");
			break;
		case GST_MESSAGE_STATE_CHANGED:
			gst_message_parse_state_changed(msg, &oldstate, &newstate, &pendingstate);
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "state change: %s -> %s", gst_element_state_get_name(oldstate), gst_element_state_get_name(newstate));
			break;
		case GST_MESSAGE_ELEMENT:
			src = gst_object_get_name(msg->src);
			if (strcmp(src, "zbar")) {
				break;
			}
			strctr = gst_message_get_structure(msg);
			code = gst_structure_get_string(strctr, "symbol");
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "message %s: %d (%s) - decoded: %s", src, msg->type, gst_message_type_get_name(msg->type), code);
			break;
		default:
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "unknown message (ext %d): %s", GST_MESSAGE_TYPE_IS_EXTENDED(msg), GST_MESSAGE_TYPE_NAME(msg));
			break;
	}

	return true;
}

GtkWidget* ui_build_scan_attach(struct ui_container *ui, const char *device) {
	int r;
	struct kee_scanner *scan;
	GtkWidget *view;

	scan = &ui->scan;
	scan_init(scan, device);
	r = scan_begin(scan);
	if (r) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail scan setup");
		return NULL;
	}
	view = GTK_WIDGET(scan->video_view);
	gtk_box_append(GTK_BOX(ui->front_scan), view);
	scan_set_handler(scan, ui_scan_code_handler);
	return view;
}

//void ui_handle_scan(GtkApplication *app, struct ui_container *ui) {
void ui_handle_scan(GtkApplication *app, struct kee_context *ctx) {
	struct ui_container *ui;
	unsigned char *s;

	ui = (struct ui_container*)ctx->front;
	s = settings_get(ctx->settings, SETTINGS_VIDEO);

	if (!(ui->state & KEE_UI_STATE_SCAN_INIT)) {
		ui_build_scan_attach(ui, (const char*)s);

	}
	ui_state_change(ui, KEE_UI_STATE_SCANNING | KEE_UI_STATE_SCAN_INIT, 0);
	gtk_stack_set_visible_child(GTK_STACK(ui->stack), GTK_WIDGET(ui->front_scan));
}
