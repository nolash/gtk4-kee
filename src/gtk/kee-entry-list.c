#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-entry-list.h"
#include "kee-entry.h"
#include "err.h"

typedef struct {
} KeeEntryListPrivate;

struct _KeeEntryList {
	GtkWidget parent;
	GListModel *list;
	GtkListItemFactory *factory;
};

G_DEFINE_TYPE(KeeEntryList, kee_entry_list, GTK_TYPE_BOX);

static void kee_entry_handle_setup(KeeEntryList* o, GtkListItem *item) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "entry list setup");
}

static void kee_entry_handle_bind(KeeEntryList *o,  GtkListItem *item) {
	GtkWidget *widget;
	char *s;
	KeeEntry *go;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "entry list bind");
	go = gtk_list_item_get_item(item);
	g_object_take_ref(G_OBJECT(go));
	gtk_list_item_set_child(item, GTK_WIDGET(go));
}

static void kee_entry_handle_unbind(KeeEntryList* o,  GtkListItem *item) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "entry list unbind");
	//GObject *go;
	//go = gtk_list_item_get_child(item);
	gtk_list_item_set_child(item, NULL);
	//g_object_unref(go);
}

static void kee_entry_handle_teardown(KeeEntryList* o,  GtkListItem *item) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "entry list teardown");
}

static void kee_entry_list_class_init(KeeEntryListClass *kls) {
}

static void kee_entry_list_init(KeeEntryList *o) {
	o->factory = gtk_signal_list_item_factory_new();
	g_signal_connect(o->factory, "setup", G_CALLBACK(kee_entry_handle_setup), NULL);
	g_signal_connect(o->factory, "bind", G_CALLBACK(kee_entry_handle_bind), NULL);
	g_signal_connect(o->factory, "unbind", G_CALLBACK(kee_entry_handle_unbind), NULL);
	g_signal_connect(o->factory, "teardown", G_CALLBACK(kee_entry_handle_teardown), NULL);
}

GtkWidget* kee_entry_list_new(GListModel *model) {
	KeeEntryList *o;
	GtkSingleSelection *sel;
	GtkWidget *view;

	o = g_object_new(KEE_TYPE_ENTRY_LIST, "orientation", GTK_ORIENTATION_VERTICAL, NULL);

	sel = gtk_single_selection_new(model);

	view = gtk_list_view_new(GTK_SELECTION_MODEL(sel), o->factory);

	gtk_box_append(GTK_BOX(o), view);

	return GTK_WIDGET(o);
}
