#include <stdlib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-entry-item-store.h"
#include "kee-entry-item.h"
#include "cadiz.h"
#include "db.h"
#include "err.h"

const size_t entry_ref_len = 65;
const size_t entry_key_len = 73;


typedef struct {
} KeeEntryItemStorePrivate;

struct _KeeEntryItemStore {
	GObject parent;
	struct db_ctx *db;
//	int last_idx;
//	int last_state;
	int last_count;
//	char *last;
//	char *last_key;
//	char *last_digest;
//	char *last_value;
//	size_t last_value_length;
	char **ref;
	char *ref_mem;
	struct Cadiz *resolver;
	struct kee_ledger_t *ledger;
};


static void kee_entry_item_store_iface_init(GListModelInterface *ifc);
G_DEFINE_TYPE_WITH_CODE(KeeEntryItemStore, kee_entry_item_store, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, kee_entry_item_store_iface_init));


static void kee_entry_item_store_finalize(GObject *o);

static int kee_entry_item_store_seek(KeeEntryItemStore *o, int idx);


static void kee_entry_item_store_class_init(KeeEntryItemStoreClass *kls) {
	GObjectClass *oc = G_OBJECT_CLASS(kls);
	oc->finalize = kee_entry_item_store_finalize;
}

static void kee_entry_item_store_init(KeeEntryItemStore *o) {
//	o->resolver.key_type = CADIZ_KEY_TYPE_ANY;
//	o->resolver.locator = malloc(1024);
}

//void kee_entry_item_store_set_resolve(KeeEntryItemStore *o, const char *locator) {
//	strcpy(o->resolver.locator, locator);
//}

static GType kee_entry_item_store_get_item_type(GListModel *list) {
	return KEE_TYPE_ENTRY_ITEM;
}

static guint kee_entry_item_store_get_n_items(GListModel *list) {
	return KEE_ENTRY_ITEM_STORE(list)->last_count;
}


static gpointer kee_entry_item_store_get_item(GListModel *list, guint index) {
	KeeEntryItem *o;
	KeeEntryItemStore *store;
//
//	//kee_entry_load(o, list->db);
	store = KEE_ENTRY_ITEM_STORE(list);
	o = kee_entry_item_new(store->db, store->ledger, (int)index);
	kee_entry_item_set_resolver(o, store->resolver);
	//kee_entry_item_store_seek(store, index);
	//kee_entry_deserialize(o, store->last_key, 9, store->last_value, store->last_value_length);
	//r = kee_entry_item_deserialize(o, store->last_value, store->last_value_length);
	//if (r) {
	//	return NULL;
	//}

//	//return o;
	kee_entry_item_apply_list_item_widget(o);
//
//	return o;
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
//	o->last = calloc(4096, 1);
//	o->last_digest = o->last + DB_KEY_SIZE_LIMIT;
//	o->last_key = o->last;
//	o->last_value = o->last_digest + 64;
//	o->last_value_length = 1024;
	o->ledger = ledger;
	o->resolver = resolver;
	
	o->last_count = kee_entry_item_store_seek(o, INT_MAX);
//	o->ref_mem = malloc(o->last_count * entry_key_len);
//	o->ref = &o->ref_mem;
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "max key index is: %d", o->last_count - 1);
	return o;
}


/// \todo always scans from 0, inefficient
/// \todo enum lookup states
static int kee_entry_item_store_seek(KeeEntryItemStore *o, int idx) {
	struct kee_ledger_item_t *item;
	int r;
	int i;
	size_t key_len;
	//size_t entry_key_len;
	//size_t cmp_key_len;
	char *mem[4096];
	char *last_key;
	//char mem[DB_KEY_SIZE_LIMIT];
	//char *cmp_key = (char*)mem;
	char *entry_key;
	char *last_value;
	size_t last_value_length;
	//char out[1024];
	//size_t out_len;

	//cmp_key_len = 65;
	//entry_key_len = 65;
	key_len = entry_ref_len; //entry_key_len + 8;
	last_key = (char*)mem;
	entry_key = last_key + 128;
	last_value = entry_key + 128;
	*last_key = DbKeyLedgerEntry;
	//*cmp_key = DbKeyLedgerEntry;
	memcpy(last_key+1, o->ledger->digest, key_len - 1);
	//memcpy(cmp_key+1, o->ledger->digest, key_len - 1);
	memcpy(entry_key, last_key, entry_ref_len);
	//memcpy(o->last_key, cmp_key, cmp_key_len);

	i = 0;
	while (i <= idx) {
		last_value_length = 2048;
		r = db_next(o->db, DbKeyLedgerEntry, &last_key, &key_len, &last_value, &last_value_length);
		//r = db_next(o->db, DbKeyLedgerEntry, &cmp_key, &key_len, &o->last_value, &o->last_value_length);
		if (r) {
			break;
		}
		if (memcmp(entry_key, last_key, entry_ref_len)) {
		//if (memcmp(cmp_key, o->last_key, cmp_key_len)) {
			break;
		}
		//out_len = 1024;
		//r = kee_entry_item_deserialize(o, last_value, last_value_length);
		item = kee_ledger_parse_item(o->ledger, last_value, last_value_length);
		if (item == NULL) {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "corrupt entry!");
		} else {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "adding entry: %d", i);
			i++;
		}
		//memcpy(entry_key, last_key, entry_key_len);
		//o->alice_credit_balance += o->
	}
	db_rewind(o->db);
	return i;
}

void kee_entry_item_store_finalize(GObject *go) {
	//KeeEntryItemStore *o = KEE_ENTRY_ITEM_STORE(go);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "freeing entry item store");
}
