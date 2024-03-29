#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-entry.h"
#include "db.h"
#include "err.h"
#include "export.h"
#include "hex.h"
#include "cadiz.h"


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
	char header[1024];
	char *unit_of_account;
	char *alice;
	char *bob;
	char *body;
	char decimals;
	struct Cadiz *resolver;
};

G_DEFINE_TYPE(KeeEntry, kee_entry, GTK_TYPE_BOX);

/// \todo free reference to self from parent box necessary..?
static void kee_entry_dispose(GObject *o) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "disposing entry");
}

static void kee_entry_finalize(GObject *o) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "tearing down entry");
	//G_OBJECT_CLASS(kee_entry_parent_class)->finalize(o);
}

static void kee_entry_class_init(KeeEntryClass *kls) {
	GObjectClass *object_class = G_OBJECT_CLASS(kls);
	object_class->finalize = kee_entry_finalize;
	object_class->dispose = kee_entry_dispose;
}

static void kee_entry_init(KeeEntry *o) {
	o->current_id = (char*)o->mem;
	o->unit_of_account = (char*)((o->mem)+64);
	o->state = 2;
	o->resolver = NULL;
}

KeeEntry* kee_entry_new(struct Cadiz *resolver) {
	KeeEntry *o;
	o = KEE_ENTRY(g_object_new(KEE_TYPE_ENTRY, NULL));
	o->resolver = resolver;	
	return o;
}

int kee_entry_load(KeeEntry *o, struct db_ctx *db, const char *id) {
	return ERR_OK;
}

/// \todo enum state
int kee_entry_deserialize(KeeEntry *o, const char *key, size_t key_len, const char *data, size_t data_len) {
	int r;
	struct kee_import im;
	size_t out_len;
	size_t remaining;
	char *p;

	o->state = 1;
	import_init(&im, data, data_len);

	out_len = 4096;
	remaining = out_len;
	r = import_read(&im, o->unit_of_account, out_len);
	p = o->unit_of_account + r;
	*p = 0;
	remaining -= (r + 1);
	p += 1;

	out_len = 1;
	r = import_read(&im, &o->decimals, out_len);

	out_len = remaining;
	o->alice = p;
	r = import_read(&im, o->alice, out_len);
	remaining -= r;
	p += r;

	out_len = remaining;
	o->bob = p;
	r = import_read(&im, o->bob, out_len);

	out_len = remaining;
	o->body = p;
	r = import_read(&im, o->body, out_len);

	o->state = 0;

	import_free(&im);

	return ERR_OK;
}

void kee_entry_apply_list_item_widget(KeeEntry *o) {
	GtkWidget *widget;
	size_t l;
	unsigned char alice_hex[129];
	unsigned char bob_hex[129];

	if (o->state)  {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "entry must be loaded first");
		return;
	}

	//widget = gtk_label_new(o->unit_of_account);
	l = 129;
	bin_to_hex((unsigned char*)o->alice, 64, alice_hex, &l);
	l = 129;
	bin_to_hex((unsigned char*)o->bob, 64, bob_hex, &l);
	sprintf(o->header, "[%s] %s -> %s", o->unit_of_account, alice_hex, bob_hex);
	widget = gtk_label_new(o->header);
	gtk_box_append(GTK_BOX(o), widget);
	return;
}
