#ifndef _GTK_KEE_ENTRY_ITEM_STORE_H
#define _GTK_KEE_ENTRY_ITEM_STORE_H

#include <glib-object.h>
#include "db.h"
#include "ledger.h"

G_BEGIN_DECLS

#define KEE_TYPE_ENTRY_ITEM_STORE kee_entry_item_store_get_type()
G_DECLARE_FINAL_TYPE(KeeEntryItemStore, kee_entry_item_store, KEE, ENTRY_ITEM_STORE, GObject);

KeeEntryItemStore* kee_entry_item_store_new(struct db_ctx *db, struct kee_ledger_t *ledger);
void kee_entry_item_store_set_resolve(KeeEntryItemStore *o, const char *locator);

G_END_DECLS

#endif // _GTK_KEE_ENTRY_ITEM_STORE_H
