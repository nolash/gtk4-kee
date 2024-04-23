#include <gcrypt.h>
#include <stddef.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include <libtasn1.h>

#include "cmime.h"

#include "kee-entry.h"
#include "kee-entry-item.h"
#include "kee-entry-item-store.h"
#include "db.h"
#include "err.h"
#include "hex.h"
#include "cadiz.h"
#include "db.h"
#include "digest.h"
#include "debug.h"
#include "endian.h"
#include "strip.h"
#include "ledger.h"
#include "dn.h"


typedef struct {
} KeeEntryPrivate;

struct _KeeEntryClass {
	GtkWidget parent_class;
};

extern const asn1_static_node schema_entry_asn1_tab[];

/// \todo factor out separate struct for listitem
struct _KeeEntry {
	GtkWidget parent;
	GtkWidget *entry_list;
	int state;
	char header[1024];
	struct kee_dn_t bob_dn;
	char current_id[64];
	struct kee_ledger_t ledger;
	struct Cadiz *resolver;
	int is_displaying;
	struct db_ctx *db;
};

G_DEFINE_TYPE(KeeEntry, kee_entry, GTK_TYPE_BOX);

static void kee_entry_handle_item_setup(GtkListItemFactory* o, GtkListItem *item) {
	GtkWidget *box;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_list_item_set_child(item, box);
}

static void kee_entry_handle_item_bind(GtkListItemFactory *o,  GtkListItem *item) {
	GtkWidget *box;
	GtkWidget *box_item;

	box = gtk_list_item_get_child(item);
	box_item = gtk_list_item_get_item(item);
	g_object_take_ref(G_OBJECT(box_item));
	gtk_box_append(GTK_BOX(box), box_item);
	
}

/// \todo free reference to self from parent box necessary..?
static void kee_entry_dispose(GObject *o) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "disposing entry");
}

static void kee_entry_finalize(GObject *o) {
	KeeEntry *entry = KEE_ENTRY(o);

	kee_ledger_free(&entry->ledger);

	kee_dn_free(&entry->bob_dn);

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "tearing down entry");
	//G_OBJECT_CLASS(kee_entry_parent_class)->finalize(o);
}

static void kee_entry_class_init(KeeEntryClass *kls) {
	GObjectClass *object_class = G_OBJECT_CLASS(kls);
	object_class->finalize = kee_entry_finalize;
	object_class->dispose = kee_entry_dispose;
}

static void kee_entry_init(KeeEntry *o) {
	o->state = 2;
	o->resolver = NULL;
	o->is_displaying = 0;
	kee_ledger_init(&o->ledger);
	kee_ledger_reset_cache(&o->ledger);
}

KeeEntry* kee_entry_new(struct db_ctx *db) {
	KeeEntry *o;
	o = KEE_ENTRY(g_object_new(KEE_TYPE_ENTRY, "orientation", GTK_ORIENTATION_VERTICAL, NULL));
	o->db = db;
	kee_dn_init(&o->bob_dn, 0);
	return o;
}

void kee_entry_set_resolver(KeeEntry *o,  struct Cadiz *resolver) {
	o->resolver = resolver;	
}

static void kee_entry_init_list_widget(KeeEntry *o) {
	GtkSingleSelection *sel;
	GtkListItemFactory *factory;
	KeeEntryItemStore *model;

	factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(kee_entry_handle_item_setup), NULL);
	g_signal_connect(factory, "bind", G_CALLBACK(kee_entry_handle_item_bind), NULL);

	model = kee_entry_item_store_new(o->db, &o->ledger, o->resolver);
	sel = gtk_single_selection_new(G_LIST_MODEL(model));
	o->entry_list = gtk_list_view_new(GTK_SELECTION_MODEL(sel), GTK_LIST_ITEM_FACTORY(factory));
}

int kee_entry_deserialize(KeeEntry *o, const char *data, size_t data_len) {
	int r;
	size_t key_len;
	size_t last_value_length;
	char mem[33 + 1024];
	char *last_key;
	char *last_value = mem + 33;
    
       	last_key = mem;	
	key_len = 33;
	last_value_length = 1024;

	r = kee_ledger_parse(&o->ledger, data, data_len);
	if (r) {
		return ERR_FAIL;
	}
	kee_content_resolve(&o->ledger.content, o->resolver);

	last_value_length = 2048;
	*last_key = DbKeyDN;
	memcpy(last_key+1, o->ledger.pubkey_bob, 32);
	key_len = 32;
	r = db_next(o->db, DbKeyDN, &last_key, &key_len, &last_value, &last_value_length);
	if (r) {
		return ERR_FAIL;
	}
	r = kee_dn_from_str(&o->bob_dn, last_value, last_value_length);
	if (r) {
		return ERR_FAIL;
	}
	db_rewind(o->db);

	last_value_length = 129;
	strcpy(last_value, "uid=");
	if (o->bob_dn.uid == NULL) {
		r = bin_to_hex((unsigned char*)o->ledger.pubkey_bob, 32, (unsigned char*)last_value+4, &last_value_length);
		if (r) {
			return ERR_FAIL;
		}
		r = kee_dn_from_str(&o->bob_dn, last_value, last_value_length+4);
		if (r) {
			return ERR_FAIL;
		}
	}
	
	r = calculate_digest_algo(data, data_len, o->current_id, GCRY_MD_SHA512);	
	if (r) {
		return ERR_DIGESTFAIL;
	}

	o->state = 0;

	kee_entry_init_list_widget(o);

	return ERR_OK;
}

void kee_entry_apply_list_item_widget(KeeEntry *o) {
	GtkWidget *widget;

	if (o->state)  {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "entry must be loaded first");
		return;
	}

	sprintf(o->header, "%s [%s]\n%s (%s)\nalice: %d\nbob: %d", o->ledger.content.subject, o->ledger.uoa, o->bob_dn.cn, o->bob_dn.uid, o->ledger.cache->alice_credit_balance, o->ledger.cache->bob_credit_balance);
	widget = gtk_label_new(o->header);
	gtk_box_append(GTK_BOX(o), widget);
	o->is_displaying = 0;
	return;
}


int kee_entry_apply_display_widget(KeeEntry *o) {
	if (o->is_displaying) {
		return 1;
	}
	o->is_displaying = 1;

	gtk_box_append(GTK_BOX(o), o->entry_list);
	return 0;
}
