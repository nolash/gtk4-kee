#include <glib-object.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

#include "kee-entry-store.h"


typedef struct {
} KeeEntryStorePrivate;

struct _KeeEntryStore {
	GObject parent;
	struct db_ctx *db;
};


static void kee_entry_store_iface_init(GListModelInterface *ifc);
G_DEFINE_TYPE_WITH_CODE(KeeEntryStore, kee_entry_store, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, kee_entry_store_iface_init));

// \todo add construct pointer for db
static void kee_entry_store_class_init(KeeEntryStoreClass *kls) {
}


static void kee_entry_store_init(KeeEntryStore *o) {
}


static GType kee_entry_store_get_item_type(GListModel *list) {
	return G_TYPE_OBJECT;
}


static guint kee_entry_store_get_n_items(GListModel *list) {
	return 1;
}


static gpointer kee_entry_store_get_item(GListModel *list, guint index) {
	GObject *o;

	o = g_object_new(G_TYPE_OBJECT, NULL);

	g_object_set_data(o, "foo", "bar");
	return o;
}


static void kee_entry_store_iface_init(GListModelInterface *ifc) {
	ifc->get_item_type = kee_entry_store_get_item_type;
	ifc->get_n_items = kee_entry_store_get_n_items;
	ifc->get_item = kee_entry_store_get_item;
}

KeeEntryStore* kee_entry_store_new(struct db_ctx *db) {
	KeeEntryStore *o;

	o = g_object_new(KEE_TYPE_ENTRY_STORE, NULL);
	o->db = db;
	return o;
}
