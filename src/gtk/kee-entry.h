#ifndef _GTK_KEE_ENTRY_H
#define _GTK_KEE_ENTRY_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "db.h"

G_BEGIN_DECLS

#define KEE_TYPE_ENTRY kee_entry_get_type()
G_DECLARE_FINAL_TYPE(KeeEntry, kee_entry, KEE, ENTRY, GtkBox);

int kee_entry_load(KeeEntry *o, struct db_ctx *db, const char *id);

G_END_DECLS


#endif //_GTK_KEE_ENTRY_H
