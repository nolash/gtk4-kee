#ifndef _GTK_KEE_ENTRY_LIST_H
#define _GTK_KEE_ENTRY_LIST_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-menu.h"

G_BEGIN_DECLS

#define KEE_TYPE_ENTRY_LIST kee_entry_list_get_type()
G_DECLARE_FINAL_TYPE(KeeEntryList, kee_entry_list, KEE, ENTRY_LIST, GtkBox);

GtkWidget* kee_entry_list_new(GListModel *model, KeeMenu *menu);

G_END_DECLS

#endif // _GTK_KEE_ENTRY_LIST_H
