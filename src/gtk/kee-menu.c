#include <string.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "kee-menu.h"
#include "kee-entry.h"
#include "menu.h"
#include "nav.h"
#include "err.h"
#include "context.h"
#include "debug.h"


typedef struct {
} KeeMenuPrivate;

struct _KeeMenu {
	GtkApplicationWindow parent;
	GtkHeaderBar *head;
	GtkStack *stack;
	//struct KeeNav nav;
	struct kee_context *ctx;
};

struct _KeeMenuClass {
	GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE(KeeMenu, kee_menu, GTK_TYPE_APPLICATION_WINDOW);

static void kee_menu_act_back(GAction *act, GVariant *param, KeeMenu *menu) {
	kee_menu_prev(menu, 0);
}

static void kee_menu_act_import(GAction *act, GVariant *param, KeeMenu *menu) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "act import");
	//gtk_stack_set_visible_child_name(stack, "import");
	//kee_menu_next(menu, "import");
	kee_menu_next(menu, BEAMENU_DST_IMPORT);
}

static void kee_menu_act_new_entry(GAction *act, GVariant *param, KeeMenu *menu) {
	KeeEntry *o;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "act new entry");
	//gtk_stack_set_visible_child_name(stack, "import");
	//kee_menu_next(menu, "entry");
	kee_menu_next(menu, BEAMENU_DST_NEW);

	o = g_object_new(KEE_TYPE_ENTRY, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	kee_entry_set_signer(o, &menu->ctx->gpg);
	kee_menu_set(menu, GTK_WIDGET(o));
	kee_entry_modeswitch(o, KEE_ENTRY_VIEWMODE_EDIT);
}

static void kee_menu_act_import_entry(GAction *act, GVariant *param, KeeMenu *menu) {
	int r;
	struct kee_ledger_t ledger;
	const char *b;
	enum kee_ledger_state_e item_state;
	size_t c;
	KeeEntry *entry;

	c = (size_t)g_variant_n_children(param);
	b = (const char*)g_variant_get_data(param);
	r = kee_ledger_parse_open(&ledger, &menu->ctx->gpg, b, c);
	if (r) {
		debug_log(DEBUG_WARNING, "failed ledger import");
		return;
	}

	item_state = kee_ledger_item_state(ledger.last_item);
	
	switch (item_state) {
		case KEE_LEDGER_STATE_FINAL:
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "detected final state");
			r = kee_ledger_put(&ledger, &menu->ctx->db);
			if (r) {
				g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail put entry");
				return;
			}
			break;
		case KEE_LEDGER_STATE_RESPONSE:
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "detected response state");
			entry = kee_entry_new(&menu->ctx->db);
			if (entry == NULL) {
				g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail create entry");
				return;
			}
			kee_entry_set_signer(entry, &menu->ctx->gpg);
			kee_entry_set_resolver(entry, &menu->ctx->resolver);
			r = kee_entry_from_ledger(entry, &ledger);
			if (r) {
				g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail load entry from ledger");
				return;
			}
			r = kee_entry_modeswitch(entry, KEE_ENTRY_VIEWMODE_SIGN);
			if (!r) {
				g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail set entry widget view mode");
				return;
			}
			//kee_menu_next(menu, "entry");
			kee_menu_next(menu, BEAMENU_DST_NEW);
			r = kee_menu_set(menu, GTK_WIDGET(entry));
			if (r) {
				g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail replace menu entry content");
				return;
			}
			break;
		case KEE_LEDGER_STATE_REQUEST:
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "detected request state, ignoring");
			break;
		default:
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "invalid item state after parse");
			return;
	}
}

//static GParamSpec *kee_props[KEE_N_MENU_PROPS] = {NULL,};
//static guint kee_sigs[KEE_N_MENU_SIGS] = {0,};

static void kee_menu_class_init(KeeMenuClass *kls) {
//	GObjectClass *o = G_OBJECT_CLASS(kls);
}

static void kee_menu_init(KeeMenu *o) {
	//memset(&o->nav, 0, sizeof(struct KeeNav));
	//o->nav.c = 0;
	//o->nav.widgets[0] = 0;
	//o->nav.now = 0;
	o->head = GTK_HEADER_BAR(gtk_header_bar_new());
	o->stack = GTK_STACK(gtk_stack_new());
}

KeeMenu* kee_menu_new(GtkApplication *gapp, struct kee_context *ctx) {
	int r;
	KeeMenu *o;
	GtkWidget *butt;
	GSimpleAction *act;

	r = kee_nav_init((char*)settings_get(ctx->settings, SETTINGS_DATA));
	if (r) {
		return NULL;
	}

	o = g_object_new(KEE_TYPE_MENU, "application", gapp, NULL);
	o->ctx = ctx;
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
	gtk_widget_set_tooltip_text(butt, "back");

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

	act = g_simple_action_new("ledger", g_variant_type_new_array(G_VARIANT_TYPE_BYTE));
	g_action_map_add_action(G_ACTION_MAP(o), G_ACTION(act));
	g_signal_connect(act, "activate", G_CALLBACK(kee_menu_act_import_entry), o);

	gtk_window_set_titlebar(GTK_WINDOW(o), GTK_WIDGET(o->head));

	gtk_window_set_title (GTK_WINDOW (o), "kee");
	gtk_window_set_default_size (GTK_WINDOW (o), 720, 1440);

	gtk_window_set_child(GTK_WINDOW(o), GTK_WIDGET(o->stack));

	return o;
}


//static void kee_menu_header_update(KeeMenu *o, const char *label) {
static void kee_menu_header_update_explicit(KeeMenu *o, int menu_id) {
	GAction *act;

	switch (menu_id) {
		case BEAMENU_DST_KEY:
			break;
		case BEAMENU_DST_LIST:
			act = g_action_map_lookup_action(G_ACTION_MAP(o), "import");
			g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
			act = g_action_map_lookup_action(G_ACTION_MAP(o), "back");
			g_simple_action_set_enabled(G_SIMPLE_ACTION(act), false);
			act = g_action_map_lookup_action(G_ACTION_MAP(o), "new_entry");
			g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
			break;
		case BEAMENU_DST_NEW:
			act = g_action_map_lookup_action(G_ACTION_MAP(o), "back");
			g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
		case BEAMENU_DST_IMPORT:
			break;
		case BEAMENU_DST_TRANSPORT:
			break;
		default:
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "unknown nav label: %s", beamenu_dst_r[menu_id]);
	}
}

static void kee_menu_header_update(KeeMenu *o) {
	GAction *act;
	int v;

	v = kee_nav_get_exit(KEE_NAV_EXIT_BACK);
	act = g_action_map_lookup_action(G_ACTION_MAP(o), "back");
	if (v) {
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
	} else {
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), false);
	}

	v = kee_nav_get_exit(KEE_NAV_EXIT_NEW);
	act = g_action_map_lookup_action(G_ACTION_MAP(o), "new_entry");
	if (v) {
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
	} else {
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), false);
	}

	v = kee_nav_get_exit(KEE_NAV_EXIT_IMPORT);
	act = g_action_map_lookup_action(G_ACTION_MAP(o), "import");
	if (v) {
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
	} else {
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), false);
	}
}

int kee_menu_add(KeeMenu *o, const char *label, GtkWidget *widget) {
	gtk_stack_add_named(o->stack, widget, label);
	return ERR_OK;
}

//GtkWidget* kee_menu_next(KeeMenu *o, const char *label) {
GtkWidget* kee_menu_next(KeeMenu *o, int menu_id) {
	GtkWidget *widget;
	char *label;

	label = beamenu_dst_r[menu_id];
	widget = gtk_stack_get_child_by_name(o->stack, label);
	//kee_nav_push(&o->nav, widget);
	kee_nav_set(widget, menu_id);
	gtk_stack_set_visible_child(o->stack, widget);
	//kee_menu_header_update(o, label);
	//kee_menu_header_update(o, menu_id);
	kee_menu_header_update(o);
	return widget;
}

int kee_menu_set(KeeMenu *o, GtkWidget *widget) {
	GtkBox *container;
	GtkWidget *widget_old;

	//container = GTK_BOX(o->nav.now);
	container = GTK_BOX(KEE_NAV_NOW);

	widget_old = gtk_widget_get_first_child(GTK_WIDGET(container));
	if (widget_old) {
		gtk_box_remove(container, widget_old);
	}
	gtk_box_append(container, widget);
	return 0;
}

int kee_menu_prev(KeeMenu *o, int force) {
	GtkWidget *widget;

	widget = kee_nav_back(force);
	gtk_stack_set_visible_child(o->stack, widget);

	//kee_menu_header_update(o, KEE_NAV_IDX);
	kee_menu_header_update(o);

	return ERR_OK;
}

int kee_menu_peek(int ridx) {
	return kee_nav_get_stack_idx(ridx+1);
}
