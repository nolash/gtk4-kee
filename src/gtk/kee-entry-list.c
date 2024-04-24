#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-entry-list.h"
#include "kee-entry.h"
#include "kee-menu.h"
#include "err.h"

typedef struct {
} KeeEntryListPrivate;

struct _KeeEntryList {
	GtkWidget parent;
	GListModel *list;
	GtkListItemFactory *factory;
};

G_DEFINE_TYPE(KeeEntryList, kee_entry_list, GTK_TYPE_BOX);

static void kee_entry_list_handle_select(GtkListView *view, guint i, KeeMenu *menu) {
	GtkSingleSelection *sel;
	KeeEntry *o;

	sel = GTK_SINGLE_SELECTION(gtk_list_view_get_model(view));

	o = KEE_ENTRY(gtk_single_selection_get_selected_item(sel));
	g_object_take_ref(G_OBJECT(o));
	kee_menu_next(menu, "entry");
	if (kee_entry_modeswitch(o, KEE_ENTRY_VIEWMODE_FULL)) {
		kee_menu_set(menu, GTK_WIDGET(o));
	}

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "list item selected %d", i);
}


/// \todo first member is probably not entry list
static void kee_entry_list_handle_setup(GtkListItemFactory* o, GtkListItem *item) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "entry list setup");
}

static void kee_entry_list_handle_bind(GtkListItemFactory *o,  GtkListItem *item) {
	KeeEntry *go;
	//GtkGesture *ctrl;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "entry list bind");
	go = gtk_list_item_get_item(item);
	g_object_take_ref(G_OBJECT(go));
	gtk_list_item_set_child(item, GTK_WIDGET(go));
	//ctrl = gtk_gesture_long_press_new();
	//gtk_widget_add_controller(item, GtkEventController(ctrl));
}

static void kee_entry_list_handle_unbind(GtkListItemFactory* o,  GtkListItem *item) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "entry list unbind");
	//GObject *go;
	//go = gtk_list_item_get_child(item);
	gtk_list_item_set_child(item, NULL);
	//g_object_unref(go);
}

static void kee_entry_list_handle_teardown(GtkListItemFactory* o,  GtkListItem *item) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "entry list teardown");
}

static void kee_entry_list_class_init(KeeEntryListClass *kls) {
}

static void kee_entry_list_init(KeeEntryList *o) {
	o->factory = gtk_signal_list_item_factory_new();
	g_signal_connect(o->factory, "setup", G_CALLBACK(kee_entry_list_handle_setup), NULL);
	g_signal_connect(o->factory, "bind", G_CALLBACK(kee_entry_list_handle_bind), NULL);
	g_signal_connect(o->factory, "unbind", G_CALLBACK(kee_entry_list_handle_unbind), NULL);
	g_signal_connect(o->factory, "teardown", G_CALLBACK(kee_entry_list_handle_teardown), NULL);
}

GtkWidget* kee_entry_list_new(GListModel *model, KeeMenu *win) {
	KeeEntryList *o;
	GtkSingleSelection *sel;
	GtkWidget *view;

	o = g_object_new(KEE_TYPE_ENTRY_LIST, "orientation", GTK_ORIENTATION_VERTICAL, NULL);

	sel = gtk_single_selection_new(model);

	view = gtk_list_view_new(GTK_SELECTION_MODEL(sel), o->factory);
	g_signal_connect(view, "activate", G_CALLBACK(kee_entry_list_handle_select), win);

	gtk_box_append(GTK_BOX(o), view);

	return GTK_WIDGET(o);
}
