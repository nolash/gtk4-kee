#include <string.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-menu.h"
#include "kee-entry.h"
#include "menu.h"
#include "nav.h"
#include "err.h"

typedef struct {
} KeeMenuPrivate;

struct _KeeMenu {
	GtkApplicationWindow parent;
	GtkHeaderBar *head;
	GtkStack *stack;
	struct KeeNav nav;
};

struct _KeeMenuClass {
	GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE(KeeMenu, kee_menu, GTK_TYPE_APPLICATION_WINDOW);

static void kee_menu_act_back(GAction *act, GVariant *param, KeeMenu *menu) {
	kee_menu_prev(menu);
}

static void kee_menu_act_import(GAction *act, GVariant *param, KeeMenu *menu) {
	//gtk_stack_set_visible_child_name(stack, "import");
	kee_menu_next(menu, "import");
}

static void kee_menu_act_new_entry(GAction *act, GVariant *param, KeeMenu *menu) {
	KeeEntry *o;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "act new entry");
	//gtk_stack_set_visible_child_name(stack, "import");
	kee_menu_next(menu, "entry");

	o = g_object_new(KEE_TYPE_ENTRY, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	kee_menu_set(menu, GTK_WIDGET(o));
	kee_entry_modeswitch(o, KEE_ENTRY_VIEWMODE_EDIT);
}

//static GParamSpec *kee_props[KEE_N_MENU_PROPS] = {NULL,};
//static guint kee_sigs[KEE_N_MENU_SIGS] = {0,};

static void kee_menu_class_init(KeeMenuClass *kls) {
//	GObjectClass *o = G_OBJECT_CLASS(kls);
}

static void kee_menu_init(KeeMenu *o) {
	//memset(&o->nav, 0, sizeof(struct KeeNav));
	o->nav.c = 0;
	o->nav.widgets[0] = 0;
	o->nav.now = 0;
	o->head = GTK_HEADER_BAR(gtk_header_bar_new());
	o->stack = GTK_STACK(gtk_stack_new());
}

KeeMenu* kee_menu_new(GtkApplication *gapp) {
	KeeMenu *o;
	GtkWidget *butt;
	GSimpleAction *act;

	o = g_object_new(KEE_TYPE_MENU, "application", gapp, NULL);
	gtk_widget_set_vexpand(GTK_WIDGET(o->stack), true);

	gapp = gtk_window_get_application(GTK_WINDOW(o));

	butt = menu_button_setup(G_OBJECT(o->head), gapp);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(o->head), butt);

	act = g_simple_action_new("back", NULL);
	g_action_map_add_action(G_ACTION_MAP(o), G_ACTION(act));
	g_simple_action_set_enabled(act, false);
	butt = gtk_button_new_from_icon_name("go-previous");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(o->head), butt);
	//gtk_widget_set_visible(butt, false);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "win.back");
	g_signal_connect(act, "activate", G_CALLBACK(kee_menu_act_back), o);

	act = g_simple_action_new("import", NULL);
	g_action_map_add_action(G_ACTION_MAP(o), G_ACTION(act));
	g_simple_action_set_enabled(act, false);
	butt = gtk_button_new_from_icon_name("insert-object");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(o->head), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "win.import");
	g_signal_connect(act, "activate", G_CALLBACK(kee_menu_act_import), o);

	act = g_simple_action_new("new_entry", NULL);
	g_action_map_add_action(G_ACTION_MAP(o), G_ACTION(act));
	g_simple_action_set_enabled(act, false);
	butt = gtk_button_new_from_icon_name("document-new");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(o->head), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "win.new_entry");
	g_signal_connect(act, "activate", G_CALLBACK(kee_menu_act_new_entry), o);

	gtk_window_set_titlebar(GTK_WINDOW(o), GTK_WIDGET(o->head));

	gtk_window_set_title (GTK_WINDOW (o), "kee");
	gtk_window_set_default_size (GTK_WINDOW (o), 720, 1440);

	gtk_window_set_child(GTK_WINDOW(o), GTK_WIDGET(o->stack));

	return o;
}


/// \todo less strcmp
static void kee_menu_header_update(KeeMenu *o, const char *label) {
	GAction *act;

	if (!(strcmp(label, "unlock"))) {
	} else if (!(strcmp(label, "view"))) {
		act = g_action_map_lookup_action(G_ACTION_MAP(o), "import");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
		act = g_action_map_lookup_action(G_ACTION_MAP(o), "back");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), false);
		act = g_action_map_lookup_action(G_ACTION_MAP(o), "new_entry");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
	} else if (!(strcmp(label, "entry"))) {
		act = g_action_map_lookup_action(G_ACTION_MAP(o), "back");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
	} else if (!(strcmp(label, "import"))) {
	} else {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "unknown nav label: %s", label);
	}
}

int kee_menu_add(KeeMenu *o, const char *label, GtkWidget *widget) {
	gtk_stack_add_named(o->stack, widget, label);
	return ERR_OK;
}

GtkWidget* kee_menu_next(KeeMenu *o, const char *label) {
	GtkWidget *widget;

	widget = gtk_stack_get_child_by_name(o->stack, label);
	kee_nav_push(&o->nav, widget);
	gtk_stack_set_visible_child(o->stack, widget);
	kee_menu_header_update(o, label);
	return widget;
}

int kee_menu_set(KeeMenu *o, GtkWidget *widget) {
	GtkBox *container;
	GtkWidget *widget_old;

	container = GTK_BOX(o->nav.now);

	widget_old = gtk_widget_get_first_child(GTK_WIDGET(container));
	if (widget_old) {
		gtk_box_remove(container, widget_old);
	}
	gtk_box_append(container, widget);
	return 0;
}

int kee_menu_prev(KeeMenu *o) {
	const char *label;

	kee_nav_pop(&o->nav);
	gtk_stack_set_visible_child(o->stack, o->nav.now);
	label = gtk_stack_get_visible_child_name(o->stack);
	kee_menu_header_update(o, label);

	return ERR_OK;
}
