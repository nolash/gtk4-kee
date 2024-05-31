#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-key.h"
#include "gpg.h"
#include "err.h"
#include "hex.h"
#include "debug.h"


typedef struct {
} KeeKeyPrivate;

struct _KeeKeyClass {
	GtkWidget parent_class;
};

struct _KeeKey {
	GtkWidget parent;
	struct gpg_store *gpg;
};

G_DEFINE_TYPE(KeeKey, kee_key, GTK_TYPE_BOX);

static guint kee_sigs[KEE_N_KEY_SIGS] = {0,};

static void kee_key_class_init(KeeKeyClass *kls) {
	GObjectClass *o = G_OBJECT_CLASS(kls);

	kee_sigs[KEE_S_KEY_UNLOCKED] = g_signal_new("unlock", 
			G_TYPE_FROM_CLASS(o),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			0,
			NULL,
			NULL,
			NULL,
			G_TYPE_NONE,
			0,
			NULL);
}

static void kee_key_init(KeeKey *o) {
}

//static void kee_key_finalize(KeeKey *o) {
//}

static void kee_key_handle_unlock_click(GtkWidget *button, KeeKey *o) {
	int r;
	GtkEntryBuffer *buf;
	GValue v = G_VALUE_INIT;
	char passphrase[1024];

	g_value_init(&v, G_TYPE_POINTER);
	buf = g_object_get_data(G_OBJECT(o), "passphrase");
	strcpy(passphrase, gtk_entry_buffer_get_text(buf));
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "click");

	r = gpg_store_check(o->gpg, passphrase);
	if (r) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "wrong passphrase");
		return;
	}

	g_signal_emit(o, kee_sigs[KEE_S_KEY_UNLOCKED], 0);
	gtk_entry_buffer_delete_text(buf, 0, gtk_entry_buffer_get_length(buf));
}

KeeKey* kee_key_new(struct gpg_store *gpg) {
	KeeKey *o;
	GtkWidget *entry;
	GtkWidget *button;
	GtkEntryBuffer *buf;

	o = g_object_new(KEE_TYPE_KEY, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	o->gpg = gpg;

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

const char *kee_key_get_fingerprint(KeeKey *o, char *fingerprint) {
	b2h((unsigned char*)o->gpg->fingerprint, 20, (unsigned char*)fingerprint);
	//strcpy(fingerprint, o->gpg->fingerprint);
	return fingerprint;
}
