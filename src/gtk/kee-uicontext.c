#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-uicontext.h"
#include "ui.h"
#include "context.h"
#include "state.h"


typedef struct {
} KeeUicontextPrivate;

/** 
 * \todo  parent can be gapplication object?
 */
struct _KeeUicontext {
	GObject parent;
	struct ui_container *ui;
	struct kee_context *ctx;
	GApplication *app;
};

G_DEFINE_TYPE(KeeUicontext, kee_uicontext, G_TYPE_OBJECT)

static GParamSpec *kee_props[KEE_N_PROPS] = {NULL,};
static guint kee_sigs[KEE_N_SIGS] = {0,};

static void kee_uicontext_set_property(GObject *oo, guint property_id, const GValue *value, GParamSpec *pspec) {
	KeeUicontext *o = KEE_UICONTEXT(oo);
	struct ui_container *ui;

	switch ((enum KEE_PROPS) property_id) {
		case CORE_CONTEXT:
			o->ctx = g_value_get_pointer(value);
			break;
		case UI_CONTAINER:
			ui = g_value_get_pointer(value);
			o->app = (GApplication*)ui->gapp;
			o->ctx->front = ui;
			break;
		case UI_HEADER:
			ui = (struct ui_container*)o->ctx->front;
			ui->head = g_value_get_object(value);
			break;
		case UI_WINDOW:
			ui = (struct ui_container*)o->ctx->front;
			ui->win = g_value_get_object(value);
			gtk_window_set_titlebar(GTK_WINDOW(ui->win), GTK_WIDGET(ui->head));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(oo, property_id, pspec);
			break;
	}

}

static void kee_uicontext_get_property(GObject *oo, guint property_id, GValue *value, GParamSpec *pspec) {
	KeeUicontext *o = KEE_UICONTEXT(oo);
	struct ui_container *ui;

	switch ((enum KEE_PROPS) property_id) {
		case GAPP:
			g_value_set_pointer(value, o->app);
			break;
		case UI_WINDOW:
			ui = (struct ui_container*)o->ctx->front;
			g_value_set_pointer(value, ui->win);
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(oo, property_id, pspec);
			break;
	}
}

static void kee_uicontext_class_init(KeeUicontextClass *kls) {
	GObjectClass *o = G_OBJECT_CLASS(kls);

	kee_sigs[SCAN_WANT] = g_signal_newv("scan_want", 
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

	o->set_property = kee_uicontext_set_property;
	o->get_property = kee_uicontext_get_property;
	
	kee_props[CORE_CONTEXT] = g_param_spec_pointer(
			"core_context",
			"Core context",
			"backend context to connect",
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
	kee_props[UI_CONTAINER] = g_param_spec_pointer(
			"ui_container",
			"Ui Container",
			"UI container to connect",
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
	kee_props[UI_HEADER] =  g_param_spec_object(
			"ui_header",
			"UI header",
			"UI header bar",
			GTK_TYPE_HEADER_BAR,
			G_PARAM_WRITABLE);
	kee_props[UI_WINDOW] = g_param_spec_object(
			"ui_window",
			"UI window",
			"UI application window",
			GTK_TYPE_WINDOW,
			G_PARAM_WRITABLE | G_PARAM_READABLE);
	kee_props[GAPP] = g_param_spec_pointer(
			"app",
			"Gapplication object",
			"Gapplication object attached to ui",
			G_PARAM_READABLE);

	g_object_class_install_properties(o, KEE_N_PROPS, kee_props);
}

static void kee_uicontext_init(KeeUicontext *self) {
	//KeeUicontextPrivate *o = kee_uicontext_get_instance_private(self);
}

void kee_uicontext_scanstart(KeeUicontext *o) {
	if (KEE_IS_SCANNING(o->ui)) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "already in scanning state");
		return;
	}
	ui_state_change(o->ui, KEE_ST_SCAN_SEARCH, 0);
	g_signal_emit(o, kee_sigs[SCAN_WANT], 0);
}

KeeState kee_uicontext_state(KeeUicontext *o) {
	KeeState state;

	state.ui = o->ui->state;
	state.ctx = o->ctx->state;

	return state;
}
