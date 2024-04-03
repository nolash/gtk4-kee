#include <gcrypt.h>
#include <stddef.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include "cmime.h"

#include "kee-entry.h"
#include "db.h"
#include "err.h"
#include "export.h"
#include "hex.h"
#include "cadiz.h"
#include "db.h"
#include "digest.h"


typedef struct {
} KeeEntryPrivate;

struct _KeeEntryClass {
	GtkWidget parent_class;
};

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
	char decimals;
	struct Cadiz *resolver;
	struct db_ctx *db;
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
	o->unit_of_account = (char*)((o->mem)+128);
	o->state = 2;
	o->resolver = NULL;
	o->subject = NULL;
}

KeeEntry* kee_entry_new(struct db_ctx *db) {
	KeeEntry *o;
	o = KEE_ENTRY(g_object_new(KEE_TYPE_ENTRY, NULL));
	o->db = db;
	return o;
}

void kee_entry_set_resolver(KeeEntry *o,  struct Cadiz *resolver) {
	o->resolver = resolver;	
}

//int kee_entry_load(KeeEntry *o, struct db_ctx *db, const char *id) {
//	return ERR_OK;
//}


/// \todo replace with struct
static int kee_entry_deserialize_item(KeeEntry *o, const char *data, size_t data_len) {
	GtkWidget *item;
	int remaining;
	int r;
	uint64_t alice_u;
	uint64_t bob_u;
	long long alice;
	long long bob;
	char mem[1024];
	size_t out_len;
	char *s = (char*)mem;
	char *flags = s + 512;
	char *parent = flags + 1;
	char *ts = parent + 64;
	char *signs = ts + 8;
	char *alice_delta = signs + 1;
	char *bob_delta = alice_delta + 10;
	struct kee_import im;

	alice = 0;
	bob = 0;

	import_init(&im, data, data_len);

	remaining = 1024;
	out_len = remaining;
	r = import_read(&im, flags, out_len);
	if (!r) {
		return ERR_FAIL;
	}

	remaining -= r;
	out_len = remaining;
	r = import_read(&im, parent, out_len);
	if (!r) {
		return ERR_FAIL;
	}

	remaining -= r;
	out_len = remaining;
	r = import_read(&im, ts, out_len);
	if (!r) {
		return ERR_FAIL;
	}

	remaining -= r;
	out_len = remaining;
	r = import_read(&im, signs, out_len);
	if (!r) {
		return ERR_FAIL;
	}

	remaining -= r;
	out_len = remaining;
	r = import_read(&im, alice_delta, out_len);
	if (!r) {
		return ERR_FAIL;
	}
	if (r > 7) {
		return ERR_FAIL;
	}

	remaining -= r;
	out_len = remaining;
	r = varint_read_u(alice_delta, r, &alice_u);
	if (!r) {
		return ERR_FAIL;
	}
	alice = (long long)alice_u;
	if (alice > 0 && *signs & ALICE_CREDIT_NEGATIVE) {
		alice *= -1;
	}

	remaining -= r;
	out_len = remaining;
	r = import_read(&im, bob_delta, out_len);
	if (!r) {
		return ERR_FAIL;
	}
	if (r > 7) {
		return ERR_FAIL;
	}

	remaining -= r;
	out_len = remaining;
	r = varint_read_u(bob_delta, r, &bob_u);
	if (!r) {
		return ERR_FAIL;
	}
	bob = (long long)bob_u;
	if (bob > 0 && *signs & BOB_CREDIT_NEGATIVE) {
		bob *= -1;
	}

//	remaining -= r;
//	out_len = remaining;
//	r = varint_read_u(alice_delta, r, &alice);
//	if (!r) {
//		return ERR_FAIL;
//	}

	sprintf(s, "alice %i bob %i", alice, bob);

	item = gtk_label_new(s);
	gtk_widget_set_hexpand(item, true);
	gtk_box_append(GTK_BOX(o), item);

	return ERR_OK;
}

/// \todo enum state
/// \todo separate message rsolve and parse in separate function
int kee_entry_deserialize(KeeEntry *o, const char *key, size_t key_len, const char *data, size_t data_len) {
	int r;
	struct kee_import im;
	size_t out_len;
	size_t t;
	size_t remaining;
	char *p;
	CMimeMessage_T *msg;

	// copy entry hash
	r = calculate_digest_algo(data, data_len, o->current_id, GCRY_MD_SHA512);	
	if (r) {
		return ERR_DIGESTFAIL;
	}

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

	if (o->resolver) {
		t = out_len;
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
			remaining -= t;
		}
	}

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
	//widget = gtk_label_new(o->header);
	//gtk_box_append(GTK_BOX(o), widget);
	widget = gtk_label_new(o->subject);
	gtk_box_append(GTK_BOX(o), widget);
	return;
}

static int kee_entry_load_items(KeeEntry *o) {
	GtkWidget *widget;
	int r;
	size_t key_len;
	char *mem = malloc(4096);
	char *last_key;
	char *last_value;
	size_t last_value_length;

	key_len = 73;
	last_key = (char*)mem;
	last_value = last_key + 128;
	*last_key = DbKeyLedgerEntry;
	memcpy(last_key+1, o->current_id, key_len - 1);
	while (1) {
		last_value_length = 2048;
		r = db_next(o->db, DbKeyLedgerEntry, &last_key, &key_len, &last_value, &last_value_length);
		if (r) {
			break;
		}
		r = kee_entry_deserialize_item(o, last_value, last_value_length);
		if (r) {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "corrupt entry!");
		}
	}
	free(mem);
}

void kee_entry_apply_display_widget(KeeEntry *o) {
	GtkWidget *widget;

	kee_entry_load_items(o);
	widget = gtk_label_new(o->subject);
	gtk_box_append(GTK_BOX(o), widget);
	return;
}


void kee_entry_apply_entry(KeeEntry *target, KeeEntry *orig) {
	KeeEntry *o;

	target->db = orig->db;
	target->current_id = orig->current_id;
	target->resolver = orig->resolver;
	target->subject = orig->subject;
	target->unit_of_account = orig->unit_of_account;
	target->alice = orig->alice;
	target->bob = orig->bob;
	return target;
}
