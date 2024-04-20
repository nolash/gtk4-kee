#ifndef _GTK_KEE_ENTRY_STORE_H
#define _GTK_KEE_ENTRY_STORE_H

#include <glib-object.h>
#include "db.h"

G_BEGIN_DECLS

#define KEE_TYPE_ENTRY_STORE kee_entry_store_get_type()
G_DECLARE_FINAL_TYPE(KeeEntryStore, kee_entry_store, KEE, ENTRY_STORE, GObject);

KeeEntryStore* kee_entry_store_new(struct db_ctx *db);
void kee_entry_store_set_resolve(KeeEntryStore *o, const char *locator);

G_END_DECLS

#endif // _GTK_KEE_ENTRY_STORE_H
