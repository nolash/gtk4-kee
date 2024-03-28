#ifndef _GTK_KEE_ENTRY_H
#define _GTK_KEE_ENTRY_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "db.h"

G_BEGIN_DECLS

enum KEE_ENTRY_PROPS {
	KEE_P_ENTRY_UNIT_OF_ACCOUNT = 1,
	KEE_N_ENTRY_PROPS,
};

#define KEE_TYPE_ENTRY kee_entry_get_type()
G_DECLARE_FINAL_TYPE(KeeEntry, kee_entry, KEE, ENTRY, GtkBox);

int kee_entry_load(KeeEntry *o, struct db_ctx *db, const char *id);
int kee_entry_deserialize(KeeEntry *o, const char *key, size_t key_len, const char *data, size_t data_len);
void kee_entry_apply_unit_of_account(KeeEntry *o);

G_END_DECLS


#endif //_GTK_KEE_ENTRY_H
