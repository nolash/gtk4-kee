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

void kee_entry_item_apply_list_item_widget(KeeEntryItem *o) {
	GtkWidget *widget;

	kee_content_resolve(&o->item->content, o->resolver);
	sprintf(o->header, "%s\nalice: %i\nbob: %i\n", o->item->content.subject, o->item->alice_credit_delta, o->item->bob_credit_delta);

	widget = gtk_widget_get_first_child(GTK_WIDGET(o));
	if (widget) {
		gtk_box_remove(GTK_BOX(o), widget);
	}
	widget = gtk_label_new(o->header);
	gtk_box_append(GTK_BOX(o), widget);
}

void kee_entry_item_apply_sign_widget(KeeEntryItem *o) {
	GtkBox *box;
	GtkWidget *widget;

	widget = gtk_widget_get_first_child(GTK_WIDGET(o));
	if (widget) {
		gtk_box_remove(GTK_BOX(o), widget);
	}

	widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_append(GTK_BOX(o), widget);
	box = GTK_BOX(widget);

	widget = gtk_label_new(o->header);
	gtk_box_append(box, widget);

	widget = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(widget), "passphrase");
	gtk_box_append(box, widget);
}

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

