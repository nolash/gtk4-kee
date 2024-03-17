#include <string.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

#include "ui.h"
#include "err.h"
#include "scan.h"
#include "settings.h"
#include "context.h"
#include "state.h"


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

//static void new_item(GtkListItemFactory *factory, GtkListItem *item, gpointer user_data) {
//}

static void ui_handle_unlock_click(GtkWidget *button, gpointer user_data) {
	struct ui_container *ui;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "click");

	ui = (struct ui_container*)user_data;
	gtk_stack_set_visible_child(ui->stack, GTK_WIDGET(ui->front_view));
}

static void ui_handle_camera_change(GtkDropDown *chooser, GParamSpec *spec, struct kee_context *ctx) {
	GtkLabel *label;
	char *s;
	struct ui_container *ui;

	ui = (struct ui_container*)ctx->front;
	
	label = gtk_drop_down_get_selected_item(chooser);
	s = g_object_get_data(G_OBJECT(label), "devpath");
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "dropdown changed: %s -> %s", spec->name, s);
	settings_set(ctx->settings, SETTINGS_VIDEO, (unsigned char*)s);
	ui_handle_scan(ui->gapp, ctx);
}


GtkWidget* ui_build_unlock(KeeUicontext *uctx) {
	GtkWidget *box;
	GtkWidget *entry;
	GtkWidget *button;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	entry = gtk_entry_new();
	gtk_box_append(GTK_BOX(box), entry);

	button = gtk_button_new_with_label("create");
	gtk_box_append(GTK_BOX(box), button);
	g_signal_connect (button, "clicked", G_CALLBACK (ui_handle_unlock_click), uctx);

	return GTK_WIDGET(box);
}

static GtkWidget* ui_build_scan_videochooser(struct kee_context *ctx) {
	GtkWidget *chooser;
	GtkWidget *label;
	GtkExpression *exp_label;
	//GtkExpression *exp_item;
	//GClosure *gclosure;
	struct kee_camera_devices *camera_device;
	struct ui_container *ui;

	ui = (struct ui_container*)ctx->front;
	ui->camera_list = G_LIST_MODEL(g_list_store_new(GTK_TYPE_LABEL));

	exp_label = gtk_property_expression_new(GTK_TYPE_LABEL, NULL, "label");
	//exp_item = gtk_closure_expression_new(G_TYPE_STRING, gclosure, 1, &exp_label);

	chooser = gtk_drop_down_new(G_LIST_MODEL(ui->camera_list), exp_label);
	camera_device = &ctx->camera_devices;
	while(1) {
		label = gtk_label_new(camera_device->label);
		g_object_set_data(G_OBJECT(label), "devpath", camera_device->path);
		g_list_store_append(G_LIST_STORE(ui->camera_list), GTK_LABEL(label));
		if (camera_device->next == NULL) {
			break;
		}
		camera_device = camera_device->next;	
	}

	g_signal_connect(chooser, "notify::selected-item", G_CALLBACK (ui_handle_camera_change), ctx);
	return chooser;
}

////static GtkWidget* ui_build_scan(struct ui_container *ui) {
////static GtkWidget* ui_build_scan(struct kee_context *ctx) {
//static GtkWidget* ui_build_scan(KeeUicontext *uctx) {
//	GtkWidget *chooser;
//	struct ui_container *ui;
//
//	ui = (struct ui_container*)ctx->front;
//	ui->front_scan = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 10));
//
//	chooser = ui_build_scan_videochooser(ctx);
//	gtk_box_append(GTK_BOX(ui->front_scan), chooser);
//
//	return GTK_WIDGET(ui->front_scan);
//}
//
//
////static GtkWidget* ui_build_view(struct ui_container *ui) {
//static GtkWidget* ui_build_view(KeeUicontext *uctx) {
//	GtkSelectionModel *sel;
//	GtkListItemFactory *factory;
//
//	ui->front_list = G_LIST_MODEL(gtk_string_list_new(NULL));
//	sel = GTK_SELECTION_MODEL(gtk_single_selection_new(ui->front_list));
//	factory = gtk_signal_list_item_factory_new();
//	g_signal_connect(factory, "setup", G_CALLBACK(new_item), NULL);
//	ui->front_view = GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(sel), factory));
//
//	return GTK_WIDGET(ui->front_view);
//}


void ui_build(GtkApplication *app, KeeUicontext *uctx) {
	GtkWidget *widget;
	GtkWidget *win;
	GtkWidget *stack;

	win = gtk_application_window_new (app);
	stack = gtk_stack_new();

	gtk_window_set_title (GTK_WINDOW (win), "kee");
	gtk_window_set_default_size (GTK_WINDOW (win), 800, 600);
	
	widget = ui_build_unlock(uctx);
	gtk_stack_add_child(GTK_STACK(stack), widget);
	//widget = ui_build_view(uctx);
	//gtk_stack_add_child(GTK_STACK(stack), widget);
	//widget = ui_build_scan(uctx);
	//gtk_stack_add_child(stack, widget);

	//g_object_get(uctx, "ui_window", win, NULL);
	gtk_stack_set_visible_child(GTK_STACK(stack), widget);
	gtk_window_set_child(GTK_WINDOW(win), GTK_WIDGET(stack));
	g_object_set(uctx, "ui_window", GTK_WINDOW(win), NULL);

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

void ui_handle_scan(GtkApplication *app, struct kee_context *ctx) {
	struct ui_container *ui;
	unsigned char *s;
	struct kee_scanner *scan;

	ui = (struct ui_container*)ctx->front;
	s = settings_get(ctx->settings, SETTINGS_VIDEO);
	scan = &ui->scan;
	
	if (ui->state & KEE_ST_SCAN_INIT) {
		gtk_box_remove(ui->front_scan, GTK_WIDGET(scan->video_view));
		scan_free(scan);
	}

	ui_build_scan_attach(ui, (const char*)s);
	ui_state_change(ui, KEE_ST_SCAN_INIT, 0);
	gtk_stack_set_visible_child(GTK_STACK(ui->stack), GTK_WIDGET(ui->front_scan));
}
