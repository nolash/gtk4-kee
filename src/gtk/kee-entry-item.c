#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-entry-item.h"
#include "ledger.h"
#include "db.h"
#include "err.h"


typedef struct {
} KeeEntryItemPrivate;

struct _KeeEntryItemClass {
	GtkWidget parent_class;
};

struct _KeeEntryItem {
	GtkWidget parent;
	int state;
	char header[1024];
	struct kee_ledger_t ledger;
	struct Cadiz *resolver;
	int alice_credit_delta;
	int bob_credit_delta;
	int alice_collateral_delta;
	int bob_collateral_delta;
	struct db_ctx *db;
};

G_DEFINE_TYPE(KeeEntryItem, kee_entry_item, GTK_TYPE_BOX);

void kee_entry_item_handle_setup(GtkListItemFactory* o, GtkListItem *item) {
	GtkWidget *label;

	label = gtk_label_new(NULL);
	gtk_list_item_set_child(item, label);
}

void kee_entry_item_handle_bind(GtkListItemFactory *o,  GtkListItem *item) {
	GtkWidget *label;
	GtkStringObject *s;

	label = gtk_list_item_get_child(item);
	s = gtk_list_item_get_item(item);
	gtk_label_set_label(GTK_LABEL(label), gtk_string_object_get_string(s));
}


static void kee_entry_item_dispose(GObject *o) {
}

static void kee_entry_item_finalize(GObject *o) {
}

static void kee_entry_item_class_init(KeeEntryItemClass *kls) {
	GObjectClass *object_class = G_OBJECT_CLASS(kls);
	object_class->finalize = kee_entry_item_finalize;
	object_class->dispose = kee_entry_item_dispose;
}

static void kee_entry_item_init(KeeEntryItem *o) {
	o->state = 2;
	o->resolver = NULL;
}

void kee_entry_item_set_resolver(KeeEntryItem *o,  struct Cadiz *resolver) {
	o->resolver = resolver;	
}

static int kee_entry_item_deserialize(KeeEntryItem *o, const char *data, size_t data_len, char *out, size_t *out_len) {
	struct kee_ledger_item_t *item;

	item = kee_ledger_parse_item(&o->ledger, data, data_len);
	if (item == NULL) {
		return ERR_FAIL;
	}
	kee_content_resolve(&item->content, o->resolver);
	
	if (item->content.flags & KEE_CONTENT_RESOLVED_SUBJECT) {
		strcpy(out, item->content.subject);
	} else {
		strcpy(out, "(no subject)");
	}

	return ERR_OK;
}

KeeEntryItem* kee_entry_item_new(struct db_ctx *db) {
	KeeEntryItem *o;
	o = KEE_ENTRY_ITEM(g_object_new(KEE_TYPE_ENTRY_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL));
	o->db = db;
	return o;
}
