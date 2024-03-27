#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-entry.h"
#include "db.h"
#include "err.h"

typedef struct {
} KeeEntryPrivate;

struct _KeeEntry {
	GtkWidget parent;
	char *current_id;
};

G_DEFINE_TYPE(KeeEntry, kee_entry, GTK_TYPE_BOX);

static void kee_entry_class_init(KeeEntryClass *kls) {
}

static void kee_entry_init(KeeEntry *o) {
}

int kee_entry_load(KeeEntry *o, struct db_ctx *db, const char *id) {
	return ERR_OK;
}
