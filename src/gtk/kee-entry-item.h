#ifndef _GTK_KEE_ENTRY_ITEM_H
#define _GTK_KEE_ENTRY_ITEM_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "db.h"
#include "cadiz.h"
#include "ledger.h"

G_BEGIN_DECLS

enum KEE_ENTRY_ITEM_PROPS {
	KEE_P_ENTRY_ITEM_ALICE_CREDIT_DELTA = 1,
	KEE_P_ENTRY_ITEM_BOB_CREDIT_DELTA,
	KEE_P_ENTRY_ITEM_ALICE_COLLATERAL_DELTA,
	KEE_P_ENTRY_ITEM_BOB_COLLATERAL_DELTA,
	KEE_N_ENTRY_ITEM_PROPS,
};

#define KEE_TYPE_ENTRY_ITEM kee_entry_item_get_type()
G_DECLARE_FINAL_TYPE(KeeEntryItem, kee_entry_item, KEE, ENTRY_ITEM, GtkBox);

KeeEntryItem* kee_entry_item_new(struct db_ctx *db, struct kee_ledger_t *ledger);
void kee_entry_item_handle_setup(GtkListItemFactory* o, GtkListItem *item);
void kee_entry_item_handle_bind(GtkListItemFactory *o,  GtkListItem *item);
void kee_entry_item_set_resolver(KeeEntryItem *o,  struct Cadiz *resolver);
int kee_entry_item_deserialize(KeeEntryItem *o, const char *data, size_t data_len);
void kee_entry_item_apply_list_item_widget(KeeEntryItem *o);

G_END_DECLS

#endif //_GTK_KEE_ENTRY_ITEM_H
