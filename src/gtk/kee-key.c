#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-key.h"

typedef struct {
} KeeKeyPrivate;

struct _KeeKEyClass {
	GtkWidget parent_class;
};

struct _KeeKey {
	GtkWidget parent;
};

G_DEFINE_TYPE(KeeKey, kee_key, GTK_TYPE_BOX);

//static GParamSpec *kee_props[KEE_N_IMPORT_PROPS] = {NULL,};
//static guint kee_sigs[KEE_N_KEY_SIGS] = {0,};

static void kee_key_class_init(KeeKeyClass *kls) {
}

static void kee_key_init(KeeKey *o) {
}

static void kee_key_handle_unlock_click(GtkWidget *button, GObject *o) {
	GtkEntryBuffer *buf;
	//const char *passphrase;

	buf = g_object_get_data(o, "passphrase");
	//passphrase = gtk_entry_buffer_get_text(buf);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "click");

	//kee_uicontext_unlock(uctx);

	gtk_entry_buffer_delete_text(buf, 0, gtk_entry_buffer_get_length(buf));
}

KeeKey* kee_key_new() {
	KeeKey *o;
	GtkWidget *entry;
	GtkWidget *button;
	GtkEntryBuffer *buf;

	o = g_object_new(KEE_TYPE_KEY, "orientation", GTK_ORIENTATION_VERTICAL, NULL);

	entry = gtk_entry_new();
	gtk_box_append(GTK_BOX(o), entry);
	buf = gtk_entry_get_buffer(GTK_ENTRY(entry));
	gtk_entry_set_input_purpose(GTK_ENTRY(entry), GTK_INPUT_PURPOSE_PASSWORD);
	gtk_entry_set_visibility(GTK_ENTRY(entry), false);
	g_object_set_data(G_OBJECT(o), "passphrase", buf);

	button = gtk_button_new_with_label("create");
	gtk_box_append(GTK_BOX(o), button);
	g_signal_connect (button, "clicked", G_CALLBACK (kee_key_handle_unlock_click), o);

	return o;
}
