#include <stdlib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-entry-item-store.h"
#include "kee-entry-item.h"
#include "cadiz.h"
#include "db.h"
#include "err.h"

#define G_LOG_DOMAIN "Kee"

const size_t entry_ref_len = 65;
const size_t entry_key_len = 73;


typedef struct {
} KeeEntryItemStorePrivate;

struct _KeeEntryItemStore {
	GObject parent;
	struct db_ctx *db;
	int last_count;
	char **ref;
	char *ref_mem;
	struct Cadiz *resolver;
	struct kee_ledger_t *ledger;
};


static void kee_entry_item_store_iface_init(GListModelInterface *ifc);
G_DEFINE_TYPE_WITH_CODE(KeeEntryItemStore, kee_entry_item_store, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, kee_entry_item_store_iface_init));

static int kee_entry_item_store_scan(KeeEntryItemStore *o);

static void kee_entry_item_store_finalize(GObject *o);

static void kee_entry_item_store_class_init(KeeEntryItemStoreClass *kls) {
	GObjectClass *oc = G_OBJECT_CLASS(kls);
	oc->finalize = kee_entry_item_store_finalize;
}

static void kee_entry_item_store_init(KeeEntryItemStore *o) {
}

static GType kee_entry_item_store_get_item_type(GListModel *list) {
	return KEE_TYPE_ENTRY_ITEM;
}

static guint kee_entry_item_store_get_n_items(GListModel *list) {
	return KEE_ENTRY_ITEM_STORE(list)->last_count;
}


static gpointer kee_entry_item_store_get_item(GListModel *list, guint index) {
	KeeEntryItem *o;
	KeeEntryItemStore *store;

	store = KEE_ENTRY_ITEM_STORE(list);
	o = kee_entry_item_new(store->db, store->ledger, (int)index);
	if (o != NULL) {
		kee_entry_item_set_resolver(o, store->resolver);
		kee_entry_item_apply_list_item_widget(o);
	}

	return o;
}

static void kee_entry_item_store_iface_init(GListModelInterface *ifc) {
	ifc->get_item_type = kee_entry_item_store_get_item_type;
	ifc->get_n_items = kee_entry_item_store_get_n_items;
	ifc->get_item = kee_entry_item_store_get_item;
}

KeeEntryItemStore* kee_entry_item_store_new(struct db_ctx *db, struct kee_ledger_t *ledger, Cadiz *resolver) {
	KeeEntryItemStore *o;

	o = g_object_new(KEE_TYPE_ENTRY_ITEM_STORE, NULL);
	o->db = db;
	o->ledger = ledger;
	o->resolver = resolver;
	
	o->last_count = kee_entry_item_store_scan(o);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "max key index is: %d", o->last_count - 1);
	return o;
}

/// \todo always scans from 0, inefficient
/// \todo enum lookup states
static int kee_entry_item_store_scan(KeeEntryItemStore *o) {
	struct kee_ledger_item_t *item;
	int r;
	int i;
	size_t key_len;
	char *mem[4096];
	char *last_key;
	char *entry_key;
	char *last_value;
	size_t last_value_length;

	key_len = entry_ref_len;
	last_key = (char*)mem;
	entry_key = last_key + 128;
	last_value = entry_key + 128;
	*last_key = DbKeyLedgerEntry;
	memcpy(last_key+1, o->ledger->digest, key_len - 1);
	memcpy(entry_key, last_key, entry_ref_len);

	i = 0;
	while (i <= INT_MAX) {
		last_value_length = 2048;
		r = db_next(o->db, DbKeyLedgerEntry, &last_key, &key_len, &last_value, &last_value_length);
		if (r) {
			break;
		}
		if (memcmp(entry_key, last_key, entry_ref_len)) {
			break;
		}
		item = kee_ledger_parse_item_db(o->ledger, last_value, last_value_length);
		if (item == NULL) {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "corrupt entry!");
		} else {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "adding entry: %d", i);
			i++;
		}
	}
	
	db_rewind(o->db);
	return i;
}

void kee_entry_item_store_finalize(GObject *go) {
	//KeeEntryItemStore *o = KEE_ENTRY_ITEM_STORE(go);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "freeing entry item store");
}
