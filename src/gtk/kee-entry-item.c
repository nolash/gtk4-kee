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
	struct kee_ledger_t *ledger;
	struct kee_ledger_item_t *item;
	struct Cadiz *resolver;
	int alice_credit_delta;
	int bob_credit_delta;
	int alice_collateral_delta;
	int bob_collateral_delta;
	struct db_ctx *db;
};

G_DEFINE_TYPE(KeeEntryItem, kee_entry_item, GTK_TYPE_BOX);


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

KeeEntryItem* kee_entry_item_new(struct db_ctx *db, struct kee_ledger_t *ledger, int idx) {
	int i;
	KeeEntryItem *o;

	o = KEE_ENTRY_ITEM(g_object_new(KEE_TYPE_ENTRY_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL));
	o->db = db;
	o->ledger = ledger;
	o->item = ledger->last_item;
	for (i = 0; i < idx; i++) {
		o->item = o->item->prev_item;
	}
	return o;
}

//int kee_entry_item_deserialize(KeeEntryItem *o, const char *data, size_t data_len) {
//	o->item = kee_ledger_parse_item(o->ledger, data, data_len);
//	if (o->item == NULL) {
//		return ERR_FAIL;
//	}
//	kee_content_resolve(&o->item->content, o->resolver);
//	
//	if (o->item->content.flags & KEE_CONTENT_RESOLVED_SUBJECT) {
//		strcpy(o->header, o->item->content.subject);
//	} else {
//		strcpy(o->header, "(no subject)");
//	}
//
//	return ERR_OK;
//}

void kee_entry_item_apply_list_item_widget(KeeEntryItem *o) {
	GtkWidget *widget;

	//if (o->state)  {
	//	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "entry must be loaded first");
	//	return;
	//}

	//sprintf(o->header, "%s [%s]\n%s (%s)", o->ledger.content.subject, o->ledger.uoa, o->bob_dn.cn, o->bob_dn.uid);
	kee_content_resolve(&o->item->content, o->resolver);
	widget = gtk_label_new(o->item->content.subject);
	gtk_box_append(GTK_BOX(o), widget);
	return;
}
