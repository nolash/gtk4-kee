#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-key.h"
#include "gpg.h"
#include "err.h"

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

static GParamSpec *kee_props[KEE_N_KEY_PROPS] = {NULL,};
static guint kee_sigs[KEE_N_KEY_SIGS] = {0,};

static void kee_key_set_property(GObject *oo, guint property_id, const GValue *value, GParamSpec *pspec) {
	KeeKey *o = KEE_KEY(oo);
	const gchar *s;

	switch((enum KEE_KEY_PROPS)property_id) {
		case KEE_P_KEY_STORE_PATH:
			s = g_value_get_string(value);
			gpg_store_init(o->gpg, (char*)s);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(oo, property_id, pspec);
	}
}

static void kee_key_get_property(GObject *oo, guint property_id, GValue *value, GParamSpec *pspec) {
	KeeKey *o = KEE_KEY(oo);

	switch((enum KEE_KEY_PROPS)property_id) {
		case KEE_P_KEY_STORE:
			g_value_set_pointer(value, o->gpg);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(oo, property_id, pspec);
	}
}

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

	o->set_property = kee_key_set_property;
	o->get_property = kee_key_get_property;

	kee_props[KEE_P_KEY_STORE_PATH] = g_param_spec_string(
			"keystore_path",
			"Keystore path",
			"GPG keystore interface path initializer",
			".",
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);


	kee_props[KEE_P_KEY_STORE] = g_param_spec_pointer(
			"keystore",
			"Keystore",
			"GPG keystore interface",
			G_PARAM_READABLE);

	g_object_class_install_properties(o, KEE_N_KEY_PROPS, kee_props);
}

static void kee_key_init(KeeKey *o) {
	o->gpg = malloc(sizeof(struct gpg_store));
}

static void kee_key_finalize(KeeKey *o) {
	free(o->gpg);
}

static void kee_key_handle_unlock_click(GtkWidget *button, KeeKey *o) {
	int r;
	GtkEntryBuffer *buf;
	struct gpg_store *gpg;
	GValue v = G_VALUE_INIT;
	char passphrase[1024];

	g_value_init(&v, G_TYPE_POINTER);
	buf = g_object_get_data(G_OBJECT(o), "passphrase");
	strcpy(passphrase, gtk_entry_buffer_get_text(buf));
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "click");

	g_object_get_property(G_OBJECT(o), "keystore", &v);
	gpg = g_value_get_pointer(&v);
	r = gpg_store_check(gpg, passphrase);
	if (r) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "wrong passphrase");
		return;
	}

	g_signal_emit(o, kee_sigs[KEE_S_KEY_UNLOCKED], 0);
	gtk_entry_buffer_delete_text(buf, 0, gtk_entry_buffer_get_length(buf));
}

KeeKey* kee_key_new(const char *key_path) {
	KeeKey *o;
	GtkWidget *entry;
	GtkWidget *button;
	GtkEntryBuffer *buf;

	o = g_object_new(KEE_TYPE_KEY, "keystore_path", key_path, "orientation", GTK_ORIENTATION_VERTICAL, NULL);

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
	strcpy(fingerprint, o->gpg->fingerprint);
	return fingerprint;
}
