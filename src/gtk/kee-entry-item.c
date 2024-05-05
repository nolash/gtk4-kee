#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-entry-item.h"
#include "ledger.h"
#include "db.h"
#include "err.h"


typedef struct {
} KeeEntryItemPrivate;

struct _KeeEntryItemClass {
	GtkWidget parent_class;
};

struct _KeeEntryItem {
	GtkWidget parent;
	int state;
	char header[1024];
	struct kee_ledger_t *ledger;
	struct kee_ledger_item_t *item;
	struct Cadiz *resolver;
	int alice_credit_delta;
	int bob_credit_delta;
	int alice_collateral_delta;
	int bob_collateral_delta;
	struct db_ctx *db;
};

G_DEFINE_TYPE(KeeEntryItem, kee_entry_item, GTK_TYPE_BOX);



static void kee_entry_item_dispose(GObject *o) {
}

static void kee_entry_item_finalize(GObject *o) {
}

static void kee_entry_item_class_init(KeeEntryItemClass *kls) {
	GObjectClass *object_class = G_OBJECT_CLASS(kls);
	object_class->finalize = kee_entry_item_finalize;
	object_class->dispose = kee_entry_item_dispose;
}

static void kee_entry_item_init(KeeEntryItem *o) {
	o->state = 2;
	o->resolver = NULL;
}

void kee_entry_item_set_resolver(KeeEntryItem *o,  struct Cadiz *resolver) {
	o->resolver = resolver;	
}

KeeEntryItem* kee_entry_item_new(struct db_ctx *db, struct kee_ledger_t *ledger, int idx) {
	int i;
	KeeEntryItem *o;

	o = KEE_ENTRY_ITEM(g_object_new(KEE_TYPE_ENTRY_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL));
	o->db = db;
	o->item = ledger->last_item;
	for (i = 0; i < idx; i++) {
		o->item = o->item->prev_item;
	}
	return o;
}

void kee_entry_item_set(KeeEntryItem *o, struct kee_ledger_item_t *item) {
	o->item = item;
}

GtkWidget *list_item_widget(KeeEntryItem *o) {
	char *subject;
	GtkWidget *widget;

	kee_content_resolve(&o->item->content, o->resolver);
	subject = o->item->content.subject;
	if (subject == NULL) {
		subject = "(no description)";
	}

	sprintf(o->header, "%s\nalice: %i\nbob: %i\n", subject, o->item->alice_credit_delta, o->item->bob_credit_delta);

	widget = gtk_label_new(o->header);

	return widget;
}

void kee_entry_item_apply_list_item_widget(KeeEntryItem *o) {
	GtkWidget *widget;

	widget = gtk_widget_get_first_child(GTK_WIDGET(o));
	if (widget) {
		gtk_box_remove(GTK_BOX(o), widget);
	}

	widget = list_item_widget(o);

	gtk_box_append(GTK_BOX(o), widget);
}

/// \todo make interface and function name more intuitive to reflect that this is not operating on the KeeEntryItem object.
void kee_entry_item_apply_edit_widget(GtkBox *box, struct kee_entry_item_form_t *form, int first) {
	GtkWidget *widget;

	if (first) {
		widget = gtk_label_new("Initial credit");
	} else {
		widget = gtk_label_new("Credit change");
	}
	gtk_box_append(box, widget);
	widget = gtk_entry_new();
	form->alice_credit_delta = GTK_ENTRY(widget);
	gtk_entry_set_input_purpose(form->alice_credit_delta, GTK_INPUT_PURPOSE_NUMBER);
	gtk_box_append(box, widget);

	if (first) {
		widget = gtk_label_new("Initial collateral");
	} else {
		widget = gtk_label_new("Collateral change");
	}
	gtk_box_append(box, widget);
	widget = gtk_entry_new();
	form->alice_collateral_delta = GTK_ENTRY(widget);
	gtk_entry_set_input_purpose(form->alice_collateral_delta, GTK_INPUT_PURPOSE_NAME);
	gtk_box_append(box, widget);

}

void kee_entry_item_apply_summary_widget(KeeEntryItem *o, GtkBox *box) {
	GtkWidget *widget;
	widget = list_item_widget(o);
	gtk_box_append(box, widget);
}

//KeeEntryItem* kee_entry_item_import(const char *data, size_t data_len) {
//	KeeEntryItem *item;
//
//	item = g_object_new(KEE_TYPE_ENTRY_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
//
//	return item;
//}
