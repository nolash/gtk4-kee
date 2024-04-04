#ifndef _GTK_KEE_ENTRY_H
#define _GTK_KEE_ENTRY_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "db.h"
#include "cadiz.h"

G_BEGIN_DECLS

enum KEE_ENTRY_PROPS {
	KEE_P_ENTRY_UNIT_OF_ACCOUNT = 1,
	KEE_N_ENTRY_PROPS,
};

enum KEE_ENTRY_SIGNS {
	ALICE_CREDIT_NEGATIVE = 1,
	BOB_CREDIT_NEGATIVE = 2,
	ALICE_COLLATERAL_NEGATIVE = 4,
	BOB_COLLATERAL_NEGATIVE = 8,
};

#define KEE_TYPE_ENTRY kee_entry_get_type()
G_DECLARE_FINAL_TYPE(KeeEntry, kee_entry, KEE, ENTRY, GtkBox);

int kee_entry_load(KeeEntry *o, struct db_ctx *db, const char *id);
int kee_entry_deserialize(KeeEntry *o, const char *key, size_t key_len, const char *data, size_t data_len);
void kee_entry_apply_list_item_widget(KeeEntry *o);
void kee_entry_apply_display_widget(KeeEntry *o);
KeeEntry* kee_entry_new(struct db_ctx *db);
void kee_entry_set_resolver(KeeEntry *o, struct Cadiz *resolver);
void kee_entry_apply_entry(KeeEntry *target, KeeEntry *orig);

G_END_DECLS


#endif //_GTK_KEE_ENTRY_H
