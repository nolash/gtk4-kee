#include <glib-object.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

#include "kee-import.h"
#include "kee-menu.h"
#include "camera.h"
#include "scan.h"
#include "err.h"

typedef struct {
} KeeImportPrivate;

struct _KeeImportClass {
	GtkWidget parent_class;
};

struct _KeeImport {
	GtkWidget parent;
	KeeMenu *win;
	GListModel *camera_list;
	struct kee_camera_devices camera_device;
	struct kee_scanner scan;
	GtkStack *stack;
	GtkBox *viewbox;
	GtkWidget *toggler_text;
};

G_DEFINE_TYPE(KeeImport, kee_import, GTK_TYPE_BOX);


static GParamSpec *kee_props[KEE_N_IMPORT_PROPS] = {NULL,};
static guint kee_sigs[KEE_N_IMPORT_SIGS] = {0,};

static void kee_import_set_property(GObject *oo, guint property_id, const GValue *value, GParamSpec *pspec) {
	KeeImport *o = KEE_IMPORT(oo);

	switch((enum KEE_IMPORT_PROPS)property_id) {
		case KEE_P_IMPORT_WIN:
			o->win = g_value_get_object(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(oo, property_id, pspec);
	}

}

static void kee_import_class_init(KeeImportClass *kls) {
	GObjectClass *o = G_OBJECT_CLASS(kls);

	kee_sigs[KEE_S_IMPORT_SCAN_CHANGE] = g_signal_newv("scan", 
			G_TYPE_FROM_CLASS(o),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			NULL,
			NULL,
			NULL,
			NULL,
			G_TYPE_NONE,
			0,
			NULL
	);

	kee_sigs[KEE_S_IMPORT_DATA] = g_signal_new("data_available", 
			G_TYPE_FROM_CLASS(o),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			0,
			NULL,
			NULL,
			NULL,
			G_TYPE_NONE,
			1,
			G_TYPE_STRING
	);

	o->set_property = kee_import_set_property;

	kee_props[KEE_P_IMPORT_WIN] = g_param_spec_object(
			"window",
			"Window",
			"Application window",
			KEE_TYPE_MENU,
			G_PARAM_WRITABLE);

	g_object_class_install_properties(o, KEE_N_IMPORT_PROPS, kee_props);
}

static void kee_import_init(KeeImport *o) {
	o->camera_list = G_LIST_MODEL(g_list_store_new(GTK_TYPE_LABEL));
	kee_import_refresh(o);
	memset(&o->scan, 0, sizeof(struct kee_scanner));
	o->stack = GTK_STACK(gtk_stack_new());
	o->viewbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
}

static void kee_import_handle_camera_change(GtkDropDown *chooser, GParamSpec *spec, KeeImport *import) {
	GtkLabel *label;
	char *s;
	
	label = gtk_drop_down_get_selected_item(chooser);
	s = g_object_get_data(G_OBJECT(label), "devpath");
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "dropdown changed: %s -> %s", spec->name, s);

	kee_import_scanchange(import, s);
}

static void kee_import_handle_import_data_focus(KeeImport *o, const char *data, GtkStack *stack) {
	gtk_widget_activate(o->toggler_text);
}

static void kee_import_handle_import_data_text(KeeImport *o, const char *data, GtkTextBuffer *buf) {
	gtk_text_buffer_set_text(buf, data, strlen(data));
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "import data %s", data);
}

static void kee_import_handle_import_data_accept(KeeImport *o, const char *data) {
	GAction *act;
	GActionMap *am;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "import accept");

	am = G_ACTION_MAP(gtk_window_get_application(GTK_WINDOW(o->win)));

	act = g_action_map_lookup_action(am, "import_data_accept");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
}

static void kee_import_handle_import_data_check(KeeImport *o, const char *data, GtkActionable *act) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "checking import data");
}

static void kee_import_handle_scan_select(GActionGroup *act, char *action_name, gpointer user_data, GtkStack *stack) {
	GVariant *v;
	const char *s;

	v = g_action_group_get_action_state(act, action_name);
	s = g_variant_get_string(v, NULL);
	gtk_stack_set_visible_child_name(stack, s);
}

static GtkWidget* kee_import_build_scan_footer(KeeImport *import, GtkStack *stack) {
	GtkWidget *foot;
	GtkWidget *butt;
	GtkToggleButton *butt_prev;
	GActionGroup *ag;
	GAction *act;
	GVariant *v;

	foot = gtk_action_bar_new();

	v = g_variant_new_string("");
	ag = G_ACTION_GROUP(g_simple_action_group_new());
	act = G_ACTION(g_simple_action_new_stateful("src", G_VARIANT_TYPE_STRING, v));
	g_action_map_add_action(G_ACTION_MAP(ag), act);

	v = g_variant_new_string(KEE_ACT_SCAN_QR);
	butt = gtk_toggle_button_new();
	gtk_button_set_icon_name(GTK_BUTTON(butt), "insert-image");
	gtk_action_bar_pack_start(GTK_ACTION_BAR(foot), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "import.src");
	gtk_actionable_set_action_target_value(GTK_ACTIONABLE(butt), v);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(butt), true);

	butt_prev = GTK_TOGGLE_BUTTON(butt);
	v = g_variant_new_string(KEE_ACT_SCAN_TEXT);
	butt = gtk_toggle_button_new();
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(butt), butt_prev);
	gtk_button_set_icon_name(GTK_BUTTON(butt), "document-new");
	gtk_action_bar_pack_start(GTK_ACTION_BAR(foot), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "import.src");
	gtk_actionable_set_action_target_value(GTK_ACTIONABLE(butt), v);
	import->toggler_text = butt;

	butt_prev = GTK_TOGGLE_BUTTON(butt);
	v = g_variant_new_string(KEE_ACT_SCAN_FILE);
	butt = gtk_toggle_button_new();
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(butt), butt_prev);
	gtk_button_set_icon_name(GTK_BUTTON(butt), "document-save");
	gtk_action_bar_pack_start(GTK_ACTION_BAR(foot), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "import.src");
	gtk_actionable_set_action_target_value(GTK_ACTIONABLE(butt), v);

	g_signal_connect(ag, "action-state-changed", G_CALLBACK(kee_import_handle_scan_select), stack);

	gtk_widget_insert_action_group(foot, "import", ag);

	return foot;
}
static GtkWidget* kee_import_build_scan_videochooser(KeeImport *o) {
	GtkWidget *chooser;
	GtkExpression *exp_label;

	exp_label = gtk_property_expression_new(GTK_TYPE_LABEL, NULL, "label");

	chooser = gtk_drop_down_new(o->camera_list, exp_label);

	g_signal_connect(chooser, "notify::selected-item", G_CALLBACK (kee_import_handle_camera_change), o);
	return chooser;
}

static GtkWidget* kee_import_build_import_text(KeeImport *o, GtkStack *stack) {
	GtkWidget *box;
	GtkTextView *txt;
	GtkWidget *butt;
	GAction *act;
	GtkApplication *gapp;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	gapp = gtk_window_get_application(GTK_WINDOW(o->win));

	txt = GTK_TEXT_VIEW(gtk_text_view_new());
	gtk_widget_set_vexpand(GTK_WIDGET(txt), true);
	gtk_box_append(GTK_BOX(box), GTK_WIDGET(txt));

	act = G_ACTION(g_simple_action_new("import_data_accept", NULL));
	g_simple_action_set_enabled(G_SIMPLE_ACTION(act), false);
	g_action_map_add_action(G_ACTION_MAP(gapp), act);

	butt = gtk_button_new_with_label("import");
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "app.import_data_accept");
	gtk_box_append(GTK_BOX(box), butt);

	g_signal_connect(o, "data_available", G_CALLBACK(kee_import_handle_import_data_text), gtk_text_view_get_buffer(txt));
	g_signal_connect(o, "data_available", G_CALLBACK(kee_import_handle_import_data_accept), NULL);
	g_signal_connect(o, "data_available", G_CALLBACK(kee_import_handle_import_data_focus), stack);
	g_signal_connect(o, "data_available", G_CALLBACK(kee_import_handle_import_data_check), butt);
	g_signal_connect(butt, "clicked", G_CALLBACK(kee_import_handle_import_data_check), butt);

	return box;
}

KeeImport* kee_import_new(KeeMenu *win) {
	KeeImport *o;
	GtkWidget *widget;
	GtkWidget *chooser;
	GValue v = G_VALUE_INIT;

	o = g_object_new(KEE_TYPE_IMPORT, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	g_value_init(&v, G_TYPE_OBJECT);
	g_value_set_object(&v, win);
	g_object_set_property(G_OBJECT(o), "window", &v);

	gtk_box_append(GTK_BOX(o), GTK_WIDGET(o->stack));

	widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	chooser = kee_import_build_scan_videochooser(o);
	gtk_box_append(GTK_BOX(widget), chooser);
	gtk_box_append(GTK_BOX(widget), GTK_WIDGET(o->viewbox));

	gtk_stack_add_named(o->stack, widget, KEE_ACT_SCAN_QR);

	widget = kee_import_build_import_text(o, o->stack);
	gtk_stack_add_named(o->stack, widget, KEE_ACT_SCAN_TEXT);

	gtk_stack_set_visible_child_name(o->stack, KEE_ACT_SCAN_QR);

	widget = kee_import_build_scan_footer(o, o->stack);
	gtk_box_append(GTK_BOX(o), widget);

	return o;
}

static void kee_import_scanadd(KeeImport *o, GtkLabel *label) {
	g_list_store_append(G_LIST_STORE(o->camera_list), label);
}

int kee_import_refresh(KeeImport *o) {
	int r;
	GtkWidget *label;
	struct kee_camera_devices *p;

	p = &o->camera_device;
	r = kee_camera_scan(p);
	if (r) {
		return ERR_FAIL;
	}

	while(strcmp(p->path, "")) {
		label = gtk_label_new(p->label);
		g_object_set_data(G_OBJECT(label), "devpath", p->path);
		kee_import_scanadd(o, GTK_LABEL(label));
		if (p->next == NULL) {
			break;
		}
		p = p->next;
	}

	return ERR_OK;
}

static void kee_import_apply_viewfinder(KeeImport *o) { 
	GtkWidget *p;

	p = gtk_widget_get_first_child(GTK_WIDGET(o->viewbox));
	if (p) {
		gtk_box_remove(GTK_BOX(o->viewbox), p);
	}

	p = GTK_WIDGET(o->scan.video_view);
	gtk_box_append(GTK_BOX(o->viewbox), p);
	gtk_widget_set_visible(GTK_WIDGET(o), true);
}

static gboolean kee_import_scan_code_handler(GstBus *bus, GstMessage *msg, gpointer user_data) {
	GError *err;
	gchar *debug_info;
	GstState oldstate;
	GstState newstate;
	GstState pendingstate;
	const gchar *src;
	const gchar *code;
	const GstStructure *strctr;
	KeeImport *import;

	import = KEE_IMPORT(user_data);

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
			g_signal_emit(import, kee_sigs[KEE_S_IMPORT_DATA], 0, code);
			return false;
		default:
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "unhandled message (ext %d): %s", GST_MESSAGE_TYPE_IS_EXTENDED(msg), GST_MESSAGE_TYPE_NAME(msg));
			break;
	}

	return true;
}

int kee_import_scanchange(KeeImport *o, const char *device) {
	if (!(strcmp(device, ""))) {
		return ERR_FAIL;
	}

	if (o->scan.pipeline) {
		scan_free(&o->scan);	
	}
	scan_init(&o->scan, device);
	scan_begin(&o->scan);
	
	kee_import_apply_viewfinder(o);

	o->scan.bus = gst_element_get_bus(o->scan.pipeline);
	gst_bus_add_watch(o->scan.bus, kee_import_scan_code_handler, o);
	return ERR_OK;
}

void kee_import_free(KeeImport *o) {
	kee_camera_free(&o->camera_device);
	scan_free(&o->scan);
}

GtkStack* kee_import_get_stack(KeeImport *o) {
	return o->stack;
}
