#include <gcrypt.h>
#include <stddef.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include <libtasn1.h>

#include "cmime.h"

#include "kee-entry.h"
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
	int state;
	char header[1024];
	struct kee_dn_t bob_dn;
	char current_id[128];
	struct kee_ledger_t ledger;
	struct Cadiz *resolver;
	struct db_ctx *db;
};

G_DEFINE_TYPE(KeeEntry, kee_entry, GTK_TYPE_BOX);

static void kee_entry_handle_item_setup(GtkListItemFactory* o, GtkListItem *item) {
	GtkWidget *label;

	label = gtk_label_new(NULL);
	gtk_list_item_set_child(item, label);
}

static void kee_entry_handle_item_bind(GtkListItemFactory *o,  GtkListItem *item) {
	GtkWidget *label;
	GtkStringObject *s;

	label = gtk_list_item_get_child(item);
	s = gtk_list_item_get_item(item);
	gtk_label_set_label(GTK_LABEL(label), gtk_string_object_get_string(s));
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
	key_len = 33;
	r = db_next(o->db, DbKeyDN, &last_key, &key_len, &last_value, &last_value_length);
	if (r) {
		return ERR_FAIL;
	}
	db_rewind(o->db);
	r = kee_dn_from_str(&o->bob_dn, last_value, last_value_length);
	if (r) {
		return ERR_FAIL;
	}
	
	r = calculate_digest_algo(data, data_len, o->current_id, GCRY_MD_SHA512);	
	if (r) {
		return ERR_DIGESTFAIL;
	}

	o->state = 0;

	return ERR_OK;
}

static int kee_entry_deserialize_item(KeeEntry *o, const char *data, size_t data_len, char *out, size_t *out_len) {
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

void kee_entry_apply_list_item_widget(KeeEntry *o) {
	int r;
	GtkWidget *widget;
	size_t l;
	unsigned char alice_hex[129];
	unsigned char bob_hex[129];
	char *bob;

	if (o->state)  {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "entry must be loaded first");
		return;
	}

//	bob = NULL;
//	r = ldap_rdn2str(*o->bob_dn, &bob, LDAP_DN_FORMAT_LDAPV3);
//	if (r) {
//		return;
//	}

	l = 129;
	bin_to_hex((unsigned char*)o->ledger.pubkey_alice, 64, alice_hex, &l);
	l = 129;
	bin_to_hex((unsigned char*)o->ledger.pubkey_bob, 64, bob_hex, &l);
	sprintf(o->header, "[%s] %s -> %s", o->ledger.uoa, alice_hex, bob_hex);
	widget = gtk_label_new(o->header);
	gtk_box_append(GTK_BOX(o), widget);
	widget = gtk_label_new(o->bob_dn.cn);
	gtk_box_append(GTK_BOX(o), widget);
	return;
}

static int kee_entry_load_items(KeeEntry *o, GtkStringList *list) {
	int r;
	size_t key_len;
	size_t entry_key_len;
	char *mem = malloc(4096);
	char *last_key;
	char *entry_key;
	char *last_value;
	size_t last_value_length;
	char out[1024];
	size_t out_len;

	entry_key_len = 65;
	key_len = entry_key_len + 8;
	last_key = (char*)mem;
	entry_key = last_key + 128;
	last_value = entry_key + 128;
	*last_key = DbKeyLedgerEntry;
	memcpy(last_key+1, o->current_id, key_len - 1);
	memcpy(entry_key, last_key, entry_key_len);
	while (1) {
		last_value_length = 2048;
		r = db_next(o->db, DbKeyLedgerEntry, &last_key, &key_len, &last_value, &last_value_length);
		if (r) {
			break;
		}
		if (memcmp(last_key, entry_key, entry_key_len)) {
			break;
		}
		out_len = 1024;
		r = kee_entry_deserialize_item(o, last_value, last_value_length, out, &out_len);
		if (r) {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "corrupt entry!");
		} else {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "adding entry: %s", out);
			gtk_string_list_append(list, out);
		}
	}
	db_rewind(o->db);
	free(mem);
	return ERR_OK;
}

void kee_entry_apply_display_widget(KeeEntry *o) {
	GtkWidget *widget;
	GtkNoSelection *sel;
	GtkListItemFactory *factory;
	GtkStringList *list;

	list = gtk_string_list_new(NULL);
	kee_entry_load_items(o, list);

	widget = gtk_label_new(o->ledger.content.subject);
	gtk_box_append(GTK_BOX(o), widget);

	factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(kee_entry_handle_item_setup), NULL);
	g_signal_connect(factory, "bind", G_CALLBACK(kee_entry_handle_item_bind), NULL);

	sel = gtk_no_selection_new(G_LIST_MODEL(list));

	widget = gtk_list_view_new(GTK_SELECTION_MODEL(sel), GTK_LIST_ITEM_FACTORY(factory));
	gtk_box_append(GTK_BOX(o), widget);
	return;
}


void kee_entry_apply_entry(KeeEntry *target, KeeEntry *orig) {
	target->db = orig->db;
	memcpy(target->current_id, orig->current_id, 128);
	target->resolver = orig->resolver;
	target->ledger = orig->ledger;
	target->state = orig->state;
	target->bob_dn = orig->bob_dn;
	return;
}

