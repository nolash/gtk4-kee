#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-entry.h"
#include "db.h"
#include "err.h"
#include "export.h"

typedef struct {
} KeeEntryPrivate;

struct _KeeEntryClass {
	GtkWidget parent_class;
};

struct _KeeEntry {
	GtkWidget parent;
	char *current_id;
	int state;
	long long timestamp;
	char mem[4096];
	char *unit_of_account;
};

G_DEFINE_TYPE(KeeEntry, kee_entry, GTK_TYPE_BOX);

static void kee_entry_finalize(GObject *o) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "tearing down entry");
	//G_OBJECT_CLASS(kee_entry_parent_class)->finalize(o);
}

static void kee_entry_class_init(KeeEntryClass *kls) {
	GObjectClass *object_class = G_OBJECT_CLASS(kls);
	object_class->finalize = kee_entry_finalize;
}

static void kee_entry_init(KeeEntry *o) {
	o->current_id = (char*)o->mem;
	o->unit_of_account = (char*)((o->mem)+64);
	o->state = 2;
}


int kee_entry_load(KeeEntry *o, struct db_ctx *db, const char *id) {
	return ERR_OK;
}

/// \todo enum state
int kee_entry_deserialize(KeeEntry *o, const char *key, size_t key_len, const char *data, size_t data_len) {
	int r;
	struct kee_import im;
	char out[4096];
	size_t out_len;

	o->state = 1;
	import_init(&im, data, data_len);

	out_len = 4096;
	r = import_read(&im, o->unit_of_account, out_len);
	*(o->unit_of_account + r) = 0;

	o->state = 0;

	return ERR_OK;
}

void kee_entry_apply_list_item_widget(KeeEntry *o) {
	GtkWidget *widget;

	if (o->state)  {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "entry must be loaded first");
		return;
	}

	widget = gtk_label_new(o->unit_of_account);
	gtk_box_append(GTK_BOX(o), widget);
	return;
}
