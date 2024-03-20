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


static void new_item(GtkListItemFactory *factory, GtkListItem *item, gpointer user_data) {
}

static void win_handle_state(KeeUicontext *uctx, char state_hint, kee_state_t *new_state, kee_state_t *old_state, GtkWindow *win) {
	GAction *act;

	if (!(state_hint & KEE_ST_HINT_UI_MENU)) {
		return;
	}

	if (new_state->ui_menu & KEE_ST_UI_HEAD_ADD) {
		act = g_action_map_lookup_action(G_ACTION_MAP(win), "import");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
	}
}


static void scan_menu_handle_state(KeeUicontext *uctx, char state_hint, kee_state_t *new_state, kee_state_t *old_state, GObject *head) {
}

static void act_import(GAction *act, GVariant *param, GtkStack *stack) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "act impot");
	gtk_stack_set_visible_child_name(stack, "import");
}

static void act_scan_select(GActionGroup *act, GtkActionBar *foot) {
	GVariant *v;

	v = g_action_group_get_action_state(act, "src");
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "have act select: %d", g_variant_get_uint32(v));
}

void ui_handle_unlock(KeeUicontext *uctx, gpointer user_data) {
	kee_state_t state_delta;

	kee_state_zero(&state_delta);

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "key is unlocked");
	kee_view_prev();

	state_delta.ui_menu |= KEE_ST_UI_HEAD_ADD;
	kee_uicontext_state_change(uctx, &state_delta, NULL);
}


//static void ui_handle_unlock_click(GtkWidget *button, gpointer user_data) {
static void ui_handle_unlock_click(GtkWidget *button, KeeUicontext *uctx) {
	GtkEntryBuffer *buf;
	const char *passphrase;

	//ui = (struct ui_container*)user_data;
	//gtk_stack_set_visible_child(ui->stack, GTK_WIDGET(ui->front_view));

	buf = g_object_get_data(G_OBJECT(uctx), "passphrase");
	passphrase = gtk_entry_buffer_get_text(buf);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "click");

	kee_uicontext_unlock(uctx);

	gtk_entry_buffer_delete_text(buf, 0, gtk_entry_buffer_get_length(buf));
}

//static void ui_handle_camera_change(GtkDropDown *chooser, GParamSpec *spec, struct kee_context *ctx) {
static void ui_handle_camera_change(GtkDropDown *chooser, GParamSpec *spec, KeeUicontext *uctx) {
	GtkLabel *label;
	char *s;
	//struct ui_container *ui;

	//ui = (struct ui_container*)ctx->front;
	
	label = gtk_drop_down_get_selected_item(chooser);
	s = g_object_get_data(G_OBJECT(label), "devpath");
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "dropdown changed: %s -> %s", spec->name, s);

	kee_uicontext_scanchange(uctx, s);
	
	//ui_handle_scan(ui->gapp, ctx);
	ui_handle_scan(uctx);
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
	g_object_set_data(G_OBJECT(uctx), "passphrase", buf);

	button = gtk_button_new_with_label("create");
	gtk_box_append(GTK_BOX(box), button);
	g_signal_connect (button, "clicked", G_CALLBACK (ui_handle_unlock_click), uctx);

	return GTK_WIDGET(box);
}

//static GtkWidget* ui_build_scan_videochooser(struct kee_context *ctx) {
static GtkWidget* ui_build_scan_videochooser(KeeUicontext *uctx) {
	GtkWidget *chooser;
	GtkExpression *exp_label;
	GListModel *camera_list;
	//GtkExpression *exp_item;
	//GClosure *gclosure;

	//ui = (struct ui_container*)ctx->front;

	exp_label = gtk_property_expression_new(GTK_TYPE_LABEL, NULL, "label");
	//exp_item = gtk_closure_expression_new(G_TYPE_STRING, gclosure, 1, &exp_label);

	g_object_get(uctx, "camera_list", &camera_list, NULL);
	chooser = gtk_drop_down_new(camera_list, exp_label);

	
	g_signal_connect(chooser, "notify::selected-item", G_CALLBACK (ui_handle_camera_change), uctx);
	return chooser;
}

static GtkWidget* ui_build_scan_footer(KeeUicontext *uctx) {
	GtkWidget *foot;
	GtkWidget *butt;
	GtkToggleButton *butt_prev;
	GActionGroup *ag;
	GAction *act;
	GVariant *v;

	foot = gtk_action_bar_new();

	v = g_variant_new_uint32(0);
	ag = G_ACTION_GROUP(g_simple_action_group_new());
	act = G_ACTION(g_simple_action_new_stateful("src", G_VARIANT_TYPE_UINT32, v));
	g_action_map_add_action(G_ACTION_MAP(ag), act);

	v = g_variant_new_uint32(KEE_ACT_SCAN_QR);
	butt = gtk_toggle_button_new();
	gtk_button_set_icon_name(GTK_BUTTON(butt), "insert-image");
	gtk_action_bar_pack_start(GTK_ACTION_BAR(foot), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "import.src");
	gtk_actionable_set_action_target_value(GTK_ACTIONABLE(butt), v);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(butt), true);

	butt_prev = GTK_TOGGLE_BUTTON(butt);
	v = g_variant_new_uint32(KEE_ACT_SCAN_FILE);
	butt = gtk_toggle_button_new();
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(butt), butt_prev);
	gtk_button_set_icon_name(GTK_BUTTON(butt), "document-new");
	gtk_action_bar_pack_start(GTK_ACTION_BAR(foot), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "import.src");
	gtk_actionable_set_action_target_value(GTK_ACTIONABLE(butt), v);

	butt_prev = GTK_TOGGLE_BUTTON(butt);
	v = g_variant_new_uint32(KEE_ACT_SCAN_TEXT);
	butt = gtk_toggle_button_new();
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(butt), butt_prev);
	gtk_button_set_icon_name(GTK_BUTTON(butt), "document-save");
	gtk_action_bar_pack_start(GTK_ACTION_BAR(foot), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "import.src");
	gtk_actionable_set_action_target_value(GTK_ACTIONABLE(butt), v);

	g_signal_connect(ag, "action-state-changed", G_CALLBACK(act_scan_select), ag);

	g_object_set_data(G_OBJECT(uctx), KEE_W_FOOTER, GTK_ACTION_BAR(foot));

	gtk_widget_insert_action_group(foot, "import", ag);

	g_signal_connect (uctx, "state", G_CALLBACK(scan_menu_handle_state), foot);
	return foot;
}

//static GtkWidget* ui_build_scan(struct ui_container *ui) {
//static GtkWidget* ui_build_scan(struct kee_context *ctx) {
static GtkWidget* ui_build_scan(KeeUicontext *uctx) {
	GtkWidget *chooser;
	GtkWidget *box;
	GtkWidget *widget;
	GtkWidget *stack;
	//struct ui_container *ui;

	stack = gtk_stack_new();

	//ui = (struct ui_container*)ctx->front;
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	chooser = ui_build_scan_videochooser(uctx);
	gtk_box_append(GTK_BOX(box), chooser);

	widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	g_object_set_data(G_OBJECT(uctx), KEE_W_CAMERA_VIEWFINDER, widget);

	//widget = g_object_get_data(G_OBJECT(uctx), KEE_W_FOOTER);
	widget = ui_build_scan_footer(uctx);
	gtk_box_append(GTK_BOX(box), widget);
	gtk_stack_add_child(GTK_STACK(stack), box);

	//g_object_set(uctx, "camera_view", box, NULL);
	g_object_set_data(G_OBJECT(uctx), KEE_W_CAMERA_SCAN, box); // replace with state listen

	return GTK_WIDGET(stack);
}


//static GtkWidget* ui_build_view(struct ui_container *ui) {
static GtkWidget* ui_build_view(KeeUicontext *uctx) {
	GtkListItemFactory *factory;
	GtkSelectionModel *sel;
	GListModel *front_list;
	GtkListView *front_view;

	factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(new_item), NULL);
	
	front_list = G_LIST_MODEL(gtk_string_list_new(NULL));
	g_object_set_data(G_OBJECT(uctx), KEE_W_FRONTLIST, front_list);

	sel = GTK_SELECTION_MODEL(gtk_single_selection_new(front_list));
	front_view = GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(sel), factory));

	return GTK_WIDGET(front_view);
}


void ui_build(GtkApplication *app, KeeUicontext *uctx) {
	GtkWidget *widget;
	GtkWidget *win;
	GtkWidget *stack;
	GtkWidget *head;
	GSimpleAction *act;

	win = gtk_application_window_new (app);

	head = header_setup(app, uctx);

	gtk_window_set_title (GTK_WINDOW (win), "kee");
	gtk_window_set_default_size (GTK_WINDOW (win), 720, 1440);

	stack = gtk_stack_new();
	kee_view_init(GTK_STACK(stack));

	widget = ui_build_unlock(uctx);
	kee_view_add(widget, "unlock");

	widget = ui_build_view(uctx);
	kee_view_add(widget, "view");
	
	widget = ui_build_scan(uctx);
	kee_view_add(widget, "import");

	kee_view_next("view");
	kee_view_next("unlock");

	act = g_simple_action_new("import", NULL);
	g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(act));
	g_simple_action_set_enabled(act, false);
	g_signal_connect(act, "activate", G_CALLBACK(act_import), stack);
	
	gtk_window_set_titlebar(GTK_WINDOW(win), head);
	g_object_set_data(G_OBJECT(uctx), KEE_W_WINDOW, GTK_WINDOW(win));

	widget = g_object_get_data(G_OBJECT(uctx), KEE_W_UI_MENU_QUICK_ADD);

	g_signal_connect (uctx, "state", G_CALLBACK(win_handle_state), win);

	gtk_widget_set_vexpand(stack, true);

	gtk_window_set_child(GTK_WINDOW(win), stack);

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


//gboolean ui_scan_code_handler(GstBus *bus, GstMessage *msg, gpointer user_data) {
//	GError *err;
//	gchar *debug_info;
//	GstState oldstate;
//	GstState newstate;
//	GstState pendingstate;
//	const gchar *src;
//	const gchar *code;
//	const GstStructure *strctr;
//	//struct _gst_data *data;
//	//GstStateChangeReturn rsc;
//
//	//data = (struct _gst_data*)user_data;
//
//	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "scan msg got");
//
//	switch (GST_MESSAGE_TYPE (msg)) {
//		case GST_MESSAGE_ERROR:
//			gst_message_parse_error(msg, &err, &debug_info);
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "logg %s: %s", GST_OBJECT_NAME(msg->src), err->message);
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "debug %s", debug_info ? debug_info : "none");
//			g_clear_error(&err);
//			g_free(debug_info);
//			break;
//		case GST_MESSAGE_EOS:
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "eos");
//			break;
//		case GST_MESSAGE_STATE_CHANGED:
//			gst_message_parse_state_changed(msg, &oldstate, &newstate, &pendingstate);
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "state change: %s -> %s", gst_element_state_get_name(oldstate), gst_element_state_get_name(newstate));
//			break;
//		case GST_MESSAGE_ELEMENT:
//			src = gst_object_get_name(msg->src);
//			if (strcmp(src, "zbar")) {
//				break;
//			}
//			strctr = gst_message_get_structure(msg);
//			code = gst_structure_get_string(strctr, "symbol");
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "message %s: %d (%s) - decoded: %s", src, msg->type, gst_message_type_get_name(msg->type), code);
//			break;
//		default:
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "unknown message (ext %d): %s", GST_MESSAGE_TYPE_IS_EXTENDED(msg), GST_MESSAGE_TYPE_NAME(msg));
//			break;
//	}
//
//	return true;
//}
//
////GtkWidget* ui_build_scan_attach(GtkWidget *front_scan, const char *device) {
////	int r;
////	struct kee_scanner scan;
////	GtkWidget *view;
////
////	//scan = &ui->scan;
////	scan_init(&scan, device);
////	r = scan_begin(&scan);
////	if (r) {
////		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail scan setup");
////		return NULL;
////	}
////	view = GTK_WIDGET(scan.video_view);
////	gtk_box_append(GTK_BOX(front_scan), view);
////	scan_set_handler(&scan, ui_scan_code_handler);
////	return view;
////}
//
////void ui_handle_scan(GtkApplication *app, struct kee_context *ctx) {
//void ui_handle_scan(GtkApplication *app, KeeUicontext *uctx) {
void ui_handle_scan(KeeUicontext *uctx) {
//	int r;
//	GtkWidget *front_scan;
//	struct kee_scanner scan;
//	char *device;
//	GtkWidget *view;
//	//struct kee_scanner *scan;
//
//	//ui = (struct ui_container*)ctx->front;
//	//s = settings_get(ctx->settings, SETTINGS_VIDEO);
//	//scan = &ui->scan;
//
//	g_object_get(uctx, "camera_device", &device, NULL);	
//	//g_object_get(uctx, "camera_scan", &scan, NULL);	
//	g_object_get(uctx, "camera_view", &front_scan, NULL);	
//	
//	//if (ui->state & KEE_ST_SCAN_INIT) {
////	if (scan.video_view) {
////		gtk_box_remove(GTK_BOX(front_scan), GTK_WIDGET(scan.video_view));
////		scan_free(&scan);
////	}
//
//	scan_init(&scan, device);
//	r = scan_begin(&scan);
//	if (r) {
//		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail scan setup");
//		return;
//	}
//	view = GTK_WIDGET(scan.video_view);
//	gtk_box_append(GTK_BOX(front_scan), view);
//	scan_set_handler(&scan, ui_scan_code_handler);
//
////	ui_build_scan_attach(uctx, (const char*)s);
//	//ui_state_change(ui, KEE_ST_SCAN_INIT, 0);
//	//gtk_stack_set_visible_child(GTK_STACK(ui->stack), GTK_WIDGET(front_scan));
//	g_object_set(uctx, "ui_push", GTK_BOX(front_scan), NULL);
}
