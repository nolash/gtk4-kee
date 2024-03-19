#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-uicontext.h"
//#include "ui.h"
#include "context.h"
#include "state.h"
#include "settings.h"


typedef struct {
} KeeUicontextPrivate;

/** 
 * \todo  parent can be gapplication object?
 */
struct _KeeUicontext {
	GObject parent;
	//struct ui_container *ui;
	struct kee_context *ctx;
	GListModel *camera_list;
	GtkApplication *gapp;
	kee_state_t state;
};

G_DEFINE_TYPE(KeeUicontext, kee_uicontext, G_TYPE_OBJECT)

static GParamSpec *kee_props[KEE_N_PROPS] = {NULL,};
static guint kee_sigs[KEE_N_SIGS] = {0,};

static void kee_uicontext_set_property(GObject *oo, guint property_id, const GValue *value, GParamSpec *pspec) {
	KeeUicontext *o = KEE_UICONTEXT(oo);
	//struct ui_container *ui;
	//GtkWidget *widget;
	//GtkStack *stack;

	switch ((enum KEE_PROPS) property_id) {
		case CORE_CONTEXT:
			o->ctx = g_value_get_pointer(value);
			break;
//		case UI_CONTAINER:
//			ui = g_value_get_pointer(value);
//			o->app = (GApplication*)ui->gapp;
//			o->ctx->front = ui;
//			o->ui = (struct ui_container*)o->ctx->front;
//			break;
//		case UI_HEADER:
//			ui = (struct ui_container*)o->ctx->front;
//			ui->head = g_value_get_object(value);
//			break;
//		case UI_WINDOW:
//			ui = (struct ui_container*)o->ctx->front;
//			ui->win = g_value_get_object(value);
//			gtk_window_set_titlebar(GTK_WINDOW(ui->win), GTK_WIDGET(ui->head));
//			break;
//		case UI_PUSH:
//			ui = (struct ui_container*)o->ctx->front;
//			widget = g_value_get_object(value);
//			stack = GTK_STACK(gtk_window_get_child(GTK_WINDOW(ui->win)));
//			gtk_stack_set_visible_child(stack, widget);
//			break;
//		case CAMERA_VIEW:
//			ui = (struct ui_container*)o->ctx->front;
//			widget = g_value_get_object(value);
//			ui->front_scan = GTK_BOX(widget);
//			break;
		case GAPP:
			o->gapp = g_value_get_object(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(oo, property_id, pspec);
			break;
	}

}

static void kee_uicontext_get_property(GObject *oo, guint property_id, GValue *value, GParamSpec *pspec) {
	KeeUicontext *o = KEE_UICONTEXT(oo);
	//struct ui_container *ui;

	switch ((enum KEE_PROPS) property_id) {
		case GAPP:
			g_value_set_object(value, o->gapp);
			break;
//		case UI_WINDOW:
//			ui = (struct ui_container*)o->ctx->front;
//			g_value_set_pointer(value, ui->win);
//			break;
//		case UI_LIST:
//			ui = (struct ui_container*)o->ctx->front;
//			g_value_set_object(value, ui->front_list);
//			break;
		case CAMERA_LIST:
			g_value_set_object(value, o->camera_list);
			break;
//		case CAMERA_SCAN:
//			g_value_set_object(value, o->ui->camera_list);
//			break;
//		case CAMERA_DEVICE:
//			g_value_set_string(value, (char*)settings_get(o->ctx->settings, SETTINGS_VIDEO)); //;o->ui->scan);
//			break;
//		case CAMERA_VIEW:
//			g_value_set_object(value, o->ui->front_scan); //;o->ui->scan);
//			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(oo, property_id, pspec);
			break;
	}
}

static void kee_uicontext_class_init(KeeUicontextClass *kls) {
	GObjectClass *o = G_OBJECT_CLASS(kls);

	kee_sigs[KEE_S_STATE_CHANGE] = g_signal_new("state", 
			G_TYPE_FROM_CLASS(o),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			0,
			NULL,
			NULL,
			NULL,
			G_TYPE_NONE,
			3,
			G_TYPE_CHAR,
			G_TYPE_POINTER,
			G_TYPE_POINTER
	);

//	kee_sigs[KEE_S_IMPORT] = g_signal_new("state", 
//			G_TYPE_FROM_CLASS(o),
//			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
//			0,
//			NULL,
//			NULL,
//			NULL,
//			G_TYPE_NONE,
//			0,
//			NULL
//	);

	kee_sigs[KEE_S_SCAN_CHANGE] = g_signal_newv("scan", 
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

	kee_sigs[KEE_S_KEY_UNLOCKED] = g_signal_new("unlock",
			G_TYPE_FROM_CLASS(o),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			0,
			NULL,
			NULL,
			NULL,
			G_TYPE_NONE,
			0,
			NULL
	);

	o->set_property = kee_uicontext_set_property;
	o->get_property = kee_uicontext_get_property;
	
	kee_props[CORE_CONTEXT] = g_param_spec_pointer(
			"core_context",
			"Core context",
			"backend context to connect",
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
//	kee_props[UI_CONTAINER] = g_param_spec_pointer(
//			"ui_container",
//			"Ui Container",
//			"UI container to connect",
//			G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
	kee_props[CAMERA_LIST] = g_param_spec_object(
			"camera_list",
			"Camera device List",
			"List model containing current list of available camera",
			G_TYPE_LIST_MODEL,
			G_PARAM_READABLE);
//	kee_props[CAMERA_SCAN] = g_param_spec_pointer(
//			"camera_scan",
//			"Camera scan",
//			"Scan context object pointer",
//			G_PARAM_READABLE);
//	kee_props[CAMERA_DEVICE] = g_param_spec_string(
//			"camera_device",
//			"Camera Device",
//			"Path for current camera device",
//			"/dev/video0",
//			G_PARAM_READABLE);
//	kee_props[CAMERA_VIEW] = g_param_spec_object(
//			"camera_view",
//			"Camera view",
//			"Viewfinder widget for camera",
//			GTK_TYPE_BOX,
//			G_PARAM_READABLE | G_PARAM_WRITABLE);
//	kee_props[UI_HEADER] =  g_param_spec_object(
//			"ui_header",
//			"UI header",
//			"UI header bar",
//			GTK_TYPE_HEADER_BAR,
//			G_PARAM_WRITABLE);
//	kee_props[UI_WINDOW] = g_param_spec_object(
//			"ui_window",
//			"UI window",
//			"UI application window",
//			GTK_TYPE_WINDOW,
//			G_PARAM_WRITABLE | G_PARAM_READABLE);
//	kee_props[UI_LIST] = g_param_spec_object(
//			"ui_list",
//			"UI item list",
//			"UI item list",
//			G_TYPE_LIST_MODEL,
//			G_PARAM_READABLE);
//	kee_props[UI_PUSH] = g_param_spec_object(
//			"ui_push",
//			"UI push",
//			"Add UI element on top of stack",
//			GTK_TYPE_WIDGET,
//			G_PARAM_WRITABLE);
//
	kee_props[GAPP] = g_param_spec_object(
			"gtk_application",
			"Gtk application object",
			"Gtk application object attached to ui",
			GTK_TYPE_APPLICATION,
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE | G_PARAM_READABLE);

	g_object_class_install_properties(o, KEE_N_PROPS, kee_props);
}

static void kee_uicontext_init(KeeUicontext *o) {
	//KeeUicontextPrivate *o = kee_uicontext_get_instance_private(self);
	o->camera_list = G_LIST_MODEL(g_list_store_new(GTK_TYPE_LABEL));
}

void kee_uicontext_scaninit(KeeUicontext *o) {
	struct kee_camera_devices *camera_device;
	GtkWidget *label;

	camera_device = &o->ctx->camera_devices;
	while(1) {
		label = gtk_label_new(camera_device->label);
		g_object_set_data(G_OBJECT(label), "devpath", camera_device->path);
		kee_uicontext_scanadd(o, GTK_LABEL(label));
		if (camera_device->next == NULL) {
			break;
		}
		camera_device = camera_device->next;
	}
}


void kee_uicontext_scanchange(KeeUicontext *o, const char *device) {
	settings_set(o->ctx->settings, SETTINGS_VIDEO, (unsigned char*)device);
	//ui_state_change(o->ui, KEE_ST_SCAN_SEARCH, 0);
	g_signal_emit(o, kee_sigs[KEE_S_SCAN_CHANGE], 0);
}

void kee_uicontext_scanadd(KeeUicontext *o, GtkLabel *label) {
	g_list_store_append(G_LIST_STORE(o->camera_list), label);
}

void kee_uicontext_unlock(KeeUicontext *o) {
	g_signal_emit(o, kee_sigs[KEE_S_KEY_UNLOCKED], 0);
}

void kee_uicontext_state_change(KeeUicontext *o, kee_state_t *add, kee_state_t *sub) {
	kee_state_t old_state;
	char hint;

	hint = 0;
	if (add) {
		hint = kee_state_add(&o->state, add);
	}

	memcpy(&old_state, &o->state, sizeof(kee_state_t));

	g_signal_emit(o, kee_sigs[KEE_S_STATE_CHANGE], 0, hint, &o->state, &old_state);
}

void kee_uicontext_scanstart(KeeUicontext *o) {
//	if (KEE_IS_SCANNING(o->ui)) {
//		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "already in scanning state");
//		return;
//	}
//
//	ui_state_change(o->ui, KEE_ST_SCAN_SEARCH, 0);
	g_signal_emit(o, kee_sigs[KEE_S_SCAN_CHANGE], 0);
}

void kee_uicontext_import(KeeUicontext *o) {
	g_signal_emit(o, kee_sigs[KEE_S_IMPORT], 0);
}
//
//KeeState kee_uicontext_state(KeeUicontext *o) {
//	KeeState state;
//
//	state.ui = o->ui->state;
//	state.ctx = o->ctx->state;
//
//	return state;
//}
