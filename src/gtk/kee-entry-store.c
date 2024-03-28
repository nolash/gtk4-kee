#include <glib-object.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <limits.h>

#include "kee-entry-store.h"
#include "kee-entry.h"
#include "err.h"


typedef struct {
} KeeEntryStorePrivate;

struct _KeeEntryStore {
	GObject parent;
	struct db_ctx *db;
	int last_idx;
	int last_state;
	int last_count;
	char *last;
	char *last_key;
	char *last_digest;
	char *last_value;
	size_t last_value_length;
};


static void kee_entry_store_iface_init(GListModelInterface *ifc);
G_DEFINE_TYPE_WITH_CODE(KeeEntryStore, kee_entry_store, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, kee_entry_store_iface_init));

// \todo add construct pointer for db
static void kee_entry_store_class_init(KeeEntryStoreClass *kls) {
}


static int kee_entry_store_seek(KeeEntryStore *o, int idx);

static void kee_entry_store_init(KeeEntryStore *o) {
}


static GType kee_entry_store_get_item_type(GListModel *list) {
	return KEE_TYPE_ENTRY;
}


static guint kee_entry_store_get_n_items(GListModel *list) {
	return KEE_ENTRY_STORE(list)->last_count;
}


static gpointer kee_entry_store_get_item(GListModel *list, guint index) {
	KeeEntry *o;
	KeeEntryStore *store;
	char s;

	o = g_object_new(KEE_TYPE_ENTRY, NULL);

	//kee_entry_load(o, list->db);
	store = KEE_ENTRY_STORE(list);
	kee_entry_store_seek(store, index);
	kee_entry_deserialize(o, store->last_key, 9, store->last_value, store->last_value_length);

	//return o;
	kee_entry_apply_list_item_widget(o);

	return o;
}

static void kee_entry_store_iface_init(GListModelInterface *ifc) {
	ifc->get_item_type = kee_entry_store_get_item_type;
	ifc->get_n_items = kee_entry_store_get_n_items;
	ifc->get_item = kee_entry_store_get_item;
}

/// \todo always scans from 0, inefficient
/// \todo enum lookup states
static int kee_entry_store_seek(KeeEntryStore *o, int idx) {
	int c;
	int r;
	int i;
	size_t key_len;
	//int direction;

	//c = o->last_idx;
	//if (idx == c) {
	//	return;
	//}

	//direction = 0;
	key_len = 9;
	o->last_key = o->last;
	o->last_value = o->last_digest + 64;
	*o->last_key = DbKeyLedgerHead;
	i = 0;
	o->last_state = 2;
	while (i <= idx) {
		o->last_idx = i;
		o->last_value_length = 1024;
		r = db_next(o->db, DbKeyLedgerHead, &o->last_key, &key_len, &o->last_value, &o->last_value_length);
		if (r) {
			o->last_state = 0;
			return i;
		}
		o->last_state = 1;
		i++;
	}
	return i;
}

KeeEntryStore* kee_entry_store_new(struct db_ctx *db) {
	KeeEntryStore *o;

	o = g_object_new(KEE_TYPE_ENTRY_STORE, NULL);
	o->db = db;
	o->last_value_length = 1024;
	o->last = calloc(2048, 1);
	o->last_digest = o->last + DB_KEY_SIZE_LIMIT;
	
	o->last_count = kee_entry_store_seek(o, INT_MAX);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "max key index is: %d", o->last_idx - 1);
	return o;
}

void kee_entry_store_finalize(KeeEntryStore *o) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "freeing entry store");
	free(o->last);
}
