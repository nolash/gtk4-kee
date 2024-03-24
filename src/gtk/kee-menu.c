#include <string.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-menu.h"
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

static void kee_menu_act_import(GAction *act, GVariant *param, GtkStack *stack) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "act impot");
	gtk_stack_set_visible_child_name(stack, "import");
}

//static GParamSpec *kee_props[KEE_N_MENU_PROPS] = {NULL,};
static guint kee_sigs[KEE_N_MENU_SIGS] = {0,};

static void kee_menu_class_init(KeeMenuClass *kls) {
//	GObjectClass *o = G_OBJECT_CLASS(kls);
}

static void kee_menu_init(KeeMenu *o) {
	memset(&o->nav, 0, sizeof(struct KeeNav));
	o->head = GTK_HEADER_BAR(gtk_header_bar_new());
	o->stack = GTK_STACK(gtk_stack_new());
}

KeeMenu* kee_menu_new(GtkApplication *gapp) {
	KeeMenu *o;
	GtkWidget *butt;
	GSimpleAction *act;

	o = g_object_new(KEE_TYPE_MENU, "application", gapp, NULL);
	gtk_widget_set_vexpand(o->stack, true);

	gapp = gtk_window_get_application(GTK_WINDOW(o));

	butt = menu_button_setup(G_OBJECT(o->head), gapp);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(o->head), butt);

	butt = gtk_button_new_from_icon_name("go-previous");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(o->head), butt);
	gtk_widget_set_visible(butt, false);

	act = g_simple_action_new("import", NULL);
	g_action_map_add_action(G_ACTION_MAP(o), G_ACTION(act));
	g_simple_action_set_enabled(act, false);

	butt = gtk_button_new_from_icon_name("insert-object");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(o->head), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "win.import");
	g_signal_connect(act, "activate", G_CALLBACK(kee_menu_act_import), o->stack);

	gtk_window_set_titlebar(GTK_WINDOW(o), GTK_WIDGET(o->head));

	gtk_window_set_title (GTK_WINDOW (o), "kee");
	gtk_window_set_default_size (GTK_WINDOW (o), 720, 1440);

	gtk_window_set_child(GTK_WINDOW(o), o->stack);

	return o;
}


int kee_menu_add(KeeMenu *o, const char *label, GtkWidget *widget) {
	gtk_stack_add_named(o->stack, widget, label);
	return ERR_OK;
}

int kee_menu_next(KeeMenu *o, const char *label) {
	GtkWidget *widget;

	widget = gtk_stack_get_child_by_name(o->stack, label);
	kee_nav_push(&o->nav, widget);
	gtk_stack_set_visible_child(o->stack, widget);
	return ERR_OK;
}

int kee_menu_prev(KeeMenu *o) {
	kee_nav_pop(&o->nav);
	gtk_stack_set_visible_child(o->stack, o->nav.now);
	return ERR_OK;
}

GtkWidget* kee_menu_get_stack(KeeMenu *o) {
	return o->stack;
}
