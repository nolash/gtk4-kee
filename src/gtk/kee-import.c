#include <glib-object.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

#include "kee-import.h"
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
	GListModel *camera_list;
	struct kee_camera_devices camera_device;
	struct kee_scanner scan;
};

G_DEFINE_TYPE(KeeImport, kee_import, GTK_TYPE_BOX);

static guint kee_sigs[KEE_N_IMPORT_SIGS] = {0,};

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
}

static void kee_import_init(KeeImport *o) {
	o->camera_list = G_LIST_MODEL(g_list_store_new(GTK_TYPE_LABEL));
	kee_import_refresh(o);
	memset(&o->scan, 0, sizeof(struct kee_scanner));
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
	GtkWidget *label;

	p = gtk_widget_get_first_child(GTK_WIDGET(o));
	if (p) {
		gtk_box_remove(GTK_BOX(o), p);
	}

	p = GTK_WIDGET(o->scan.video_view);
	gtk_box_append(GTK_BOX(o), p);
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
	//scan_set_handler(&o->scan, kee_import_scan_code_handler);
	return ERR_OK;
}

GListModel* kee_import_get_camera_list(KeeImport *o) {
	return o->camera_list;	
}

void kee_import_free(KeeImport *o) {
	kee_camera_free(&o->camera_device);
	scan_free(&o->scan);
}
