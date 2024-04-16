#include <gcrypt.h>
#include <stddef.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include <libtasn1.h>

#include "cmime.h"
#include "varint.h"

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
	long long timestamp;
	char mem[4096];
	char header[1024];
	char *current_id;
	char *unit_of_account;
	char *alice;
	char *bob;
	char *body;
	char *subject;
	int decimals;
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
	o->unit_of_account = (char*)((o->mem)+128);
	o->state = 2;
	o->resolver = NULL;
	o->subject = NULL;
}

KeeEntry* kee_entry_new(struct db_ctx *db) {
	KeeEntry *o;
	o = KEE_ENTRY(g_object_new(KEE_TYPE_ENTRY, "orientation", GTK_ORIENTATION_VERTICAL, NULL));
	o->db = db;
	return o;
}

void kee_entry_set_resolver(KeeEntry *o,  struct Cadiz *resolver) {
	o->resolver = resolver;	
}

//int kee_entry_load(KeeEntry *o, struct db_ctx *db, const char *id) {
//	return ERR_OK;
//}

static int kee_entry_deserialize_item(KeeEntry *o, const char *data, size_t data_len, char *out, size_t *out_len) {
	int r;
	char err[1024];
	asn1_node root;
	asn1_node item;
	int alice;
	int bob;
	int c;
	int v;
	char *p;

	memset(&root, 0, sizeof(root));
	memset(&item, 0, sizeof(item));
	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return ERR_FAIL;
	}

	r = asn1_create_element(root, "Kee.KeeEntry", &item);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}
	
	c = (int)data_len;
	r = asn1_der_decoding(&item, data, c, err);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}

	c = sizeof(v);
	v = 0;
	alice = 0;
	p = (char*)&v;
	r = asn1_read_value(item, "aliceCreditDelta", p, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}

	strap_be(p, c, (char*)&alice, sizeof(alice));
	if (is_le()) {
		flip_endian(sizeof(int), (void*)&alice);
	}

	c = sizeof(bob);
	v = 0;
	bob = 0;
	p = (char*)&v;
	r = asn1_read_value(item, "bobCreditDelta", p, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}

	strap_be(p, c, (char*)&bob, sizeof(bob));
	if (is_le()) {
		flip_endian(sizeof(int), (void*)&bob);
	}
	
	sprintf(out, "alice: %i, bob %i", alice, bob);
	*out_len = strlen(out);
	return ERR_OK;
}

int kee_entry_deserialize(KeeEntry *o, const char *key, size_t key_len, const char *data, size_t data_len) {
	int r;
	char err[1024];
	size_t out_len;
	size_t remaining;
	asn1_node root;
	asn1_node item;
	int c;
	char *p;
	CMimeMessage_T *msg;

	memset(&root, 0, sizeof(root));
	memset(&item, 0, sizeof(item));
	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return ERR_FAIL;
	}

	r = asn1_create_element(root, "Kee.KeeEntryHead", &item);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}
	
	c = (int)data_len;
	r = asn1_der_decoding(&item, data, c, err);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}

	r = calculate_digest_algo(data, data_len, o->current_id, GCRY_MD_SHA512);	
	if (r) {
		return ERR_DIGESTFAIL;
	}

	o->state = 1;

	out_len = 4096;
	remaining = out_len;
	c = remaining;
	p = o->unit_of_account;
	r = asn1_read_value(item, "uoa", p, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}
	p = o->unit_of_account + c;
	*p = 0;
	remaining -= (c + 1);
	p += 1;

	c = remaining;
	o->alice = p;
	r = asn1_read_value(item, "alicePubKey", o->alice, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}

	remaining -= c;
	p += c;

	c = remaining;
	o->bob = p;
	r = asn1_read_value(item, "bobPubKey", o->bob, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}
	if (is_le()) {
		flip_endian(c, (void*)o->bob);
	}
	p += c;

	c = remaining;
	o->body = p;
	r = asn1_read_value(item, "body", p, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}
	p += c;

	if (o->resolver) {
		r = cadiz_resolve(o->resolver, o->body, o->body, &out_len);
		if (!r) {
			msg = cmime_message_new();
			o->subject = o->body + out_len;
			r = cmime_message_from_string(&msg, o->body, 0);
			if (!r) {
				o->subject = cmime_message_get_subject(msg);
				o->subject = cmime_string_strip(o->subject);
				out_len += strlen(o->subject) + 1;
			}
			remaining -= out_len;
		} else {
			remaining -= c;
		}
	}

	o->state = 0;

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
	//widget = gtk_label_new(o->header);
	//gtk_box_append(GTK_BOX(o), widget);
	widget = gtk_label_new(o->subject);
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

	widget = gtk_label_new(o->subject);
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
	target->current_id = orig->current_id;
	target->resolver = orig->resolver;
	target->subject = orig->subject;
	target->unit_of_account = orig->unit_of_account;
	target->alice = orig->alice;
	target->bob = orig->bob;
	return;
}

