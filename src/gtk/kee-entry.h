#ifndef _GTK_KEE_ENTRY_H
#define _GTK_KEE_ENTRY_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "db.h"
#include "cadiz.h"
#include "gpg.h"

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

enum kee_entry_viewmode_e {
	KEE_ENTRY_VIEWMODE_SHORT,
	KEE_ENTRY_VIEWMODE_FULL,
	KEE_ENTRY_VIEWMODE_EDIT,
};

#define KEE_TYPE_ENTRY kee_entry_get_type()
G_DECLARE_FINAL_TYPE(KeeEntry, kee_entry, KEE, ENTRY, GtkBox);

int kee_entry_load(KeeEntry *o, struct db_ctx *db, const char *id);
int kee_entry_deserialize(KeeEntry *o, const char *data, size_t data_len);
KeeEntry* kee_entry_new(struct db_ctx *db);
void kee_entry_set_resolver(KeeEntry *o, struct Cadiz *resolver);
void kee_entry_set_signer(KeeEntry *o, struct gpg_store *gpg);
int kee_entry_modeswitch(KeeEntry *o, enum kee_entry_viewmode_e);

G_END_DECLS

#endif //_GTK_KEE_ENTRY_H
