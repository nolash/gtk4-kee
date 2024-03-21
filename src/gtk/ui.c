#include <string.h>

#include <gtk/gtk.h>
#include <gst/gst.h>

#include "ui.h"
#include "err.h"
#include "scan.h"
#include "settings.h"
#include "context.h"
#include "state.h"
#include "view.h"
#include "menu.h"
#include "kee-import.h"


static void new_item(GtkListItemFactory *factory, GtkListItem *item, gpointer user_data) {
}

void ui_handle_import_data_focus(KeeImport *o, const char *data, GtkStack *stack) {
	gtk_stack_set_visible_child_name(stack, KEE_ACT_SCAN_TEXT);
}

void ui_handle_import_data_text(KeeImport *o, const char *data, GtkTextBuffer *buf) {
	gtk_text_buffer_set_text(buf, data, strlen(data));
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "import data %s", data);
}

void ui_handle_import_data_accept(KeeImport *o, const char *data, GActionMap *am) {
	GAction *act;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "import accept %s", data);

	act = g_action_map_lookup_action(am, "import_data_accept");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
}

static void win_handle_state(KeeUicontext *uctx, char state_hint, kee_state_t *new_state, kee_state_t *old_state, GtkWindow *win) {
	GAction *act;

	if (!(state_hint & KEE_ST_HINT_UI_MENU)) {
		return;
	}

	if (new_state->ui_menu & KEE_ST_UI_HEAD_ADD) {
		act = g_action_map_lookup_action(G_ACTION_MAP(win), "import");
		g_simple_action_set_enabled(G_SIMPLE_ACTION(act), true);
	}
}


static void act_import(GAction *act, GVariant *param, GtkStack *stack) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "act impot");
	gtk_stack_set_visible_child_name(stack, "import");
}


//static void act_scan_select(GActionGroup *act, GtkActionBar *foot) {
// \todo why is there user_data in addition to pointer
static void act_scan_select(GActionGroup *act, char *action_name, gpointer user_data, GtkStack *stack) {
	GVariant *v;
	const char *s;

	v = g_action_group_get_action_state(act, action_name);
	s = g_variant_get_string(v, NULL);
	//g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "have act select: %d", g_variant_get_string(v));
	gtk_stack_set_visible_child_name(stack, s);
}


void ui_handle_unlock(KeeUicontext *uctx, gpointer user_data) {
	kee_state_t state_delta;

	kee_state_zero(&state_delta);

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "key is unlocked");
	kee_view_prev();

	state_delta.ui_menu |= KEE_ST_UI_HEAD_ADD;
	kee_uicontext_state_change(uctx, &state_delta, NULL);
}


static void ui_handle_unlock_click(GtkWidget *button, KeeUicontext *uctx) {
	GtkEntryBuffer *buf;
	const char *passphrase;

	buf = g_object_get_data(G_OBJECT(uctx), "passphrase");
	passphrase = gtk_entry_buffer_get_text(buf);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "click");

	kee_uicontext_unlock(uctx);

	gtk_entry_buffer_delete_text(buf, 0, gtk_entry_buffer_get_length(buf));
}

static void ui_handle_camera_change(GtkDropDown *chooser, GParamSpec *spec, KeeImport *import) {
	GtkLabel *label;
	char *s;
	
	label = gtk_drop_down_get_selected_item(chooser);
	s = g_object_get_data(G_OBJECT(label), "devpath");
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "dropdown changed: %s -> %s", spec->name, s);

	kee_import_scanchange(import, s);
	
}


GtkWidget* ui_build_unlock(KeeUicontext *uctx) {
	GtkWidget *box;
	GtkWidget *entry;
	GtkWidget *button;
	GtkEntryBuffer *buf;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	entry = gtk_entry_new();
	gtk_box_append(GTK_BOX(box), entry);
	buf = gtk_entry_get_buffer(GTK_ENTRY(entry));
	gtk_entry_set_input_purpose(GTK_ENTRY(entry), GTK_INPUT_PURPOSE_PASSWORD);
	gtk_entry_set_visibility(GTK_ENTRY(entry), false);
	g_object_set_data(G_OBJECT(uctx), "passphrase", buf);

	button = gtk_button_new_with_label("create");
	gtk_box_append(GTK_BOX(box), button);
	g_signal_connect (button, "clicked", G_CALLBACK (ui_handle_unlock_click), uctx);

	return GTK_WIDGET(box);
}


static GtkWidget* ui_build_scan_videochooser(KeeImport *import) {
	GtkWidget *chooser;
	GtkExpression *exp_label;
	GListModel *camera_list;

	exp_label = gtk_property_expression_new(GTK_TYPE_LABEL, NULL, "label");

	camera_list = kee_import_get_camera_list(import); //, "camera_list", &camera_list, NULL);
	chooser = gtk_drop_down_new(camera_list, exp_label);

	g_signal_connect(chooser, "notify::selected-item", G_CALLBACK (ui_handle_camera_change), import);
	return chooser;
}


static GtkWidget* ui_build_scan_footer(KeeImport *import, GtkStack *stack) {
	GtkWidget *foot;
	GtkWidget *butt;
	GtkToggleButton *butt_prev;
	GActionGroup *ag;
	GAction *act;
	GVariant *v;

	foot = gtk_action_bar_new();

	v = g_variant_new_string("");
	ag = G_ACTION_GROUP(g_simple_action_group_new());
	act = G_ACTION(g_simple_action_new_stateful("src", G_VARIANT_TYPE_STRING, v));
	g_action_map_add_action(G_ACTION_MAP(ag), act);

	v = g_variant_new_string(KEE_ACT_SCAN_QR);
	butt = gtk_toggle_button_new();
	gtk_button_set_icon_name(GTK_BUTTON(butt), "insert-image");
	gtk_action_bar_pack_start(GTK_ACTION_BAR(foot), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "import.src");
	gtk_actionable_set_action_target_value(GTK_ACTIONABLE(butt), v);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(butt), true);

	butt_prev = GTK_TOGGLE_BUTTON(butt);
	v = g_variant_new_string(KEE_ACT_SCAN_TEXT);
	butt = gtk_toggle_button_new();
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(butt), butt_prev);
	gtk_button_set_icon_name(GTK_BUTTON(butt), "document-new");
	gtk_action_bar_pack_start(GTK_ACTION_BAR(foot), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "import.src");
	gtk_actionable_set_action_target_value(GTK_ACTIONABLE(butt), v);

	butt_prev = GTK_TOGGLE_BUTTON(butt);
	v = g_variant_new_string(KEE_ACT_SCAN_FILE);
	butt = gtk_toggle_button_new();
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(butt), butt_prev);
	gtk_button_set_icon_name(GTK_BUTTON(butt), "document-save");
	gtk_action_bar_pack_start(GTK_ACTION_BAR(foot), butt);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "import.src");
	gtk_actionable_set_action_target_value(GTK_ACTIONABLE(butt), v);

	g_signal_connect(ag, "action-state-changed", G_CALLBACK(act_scan_select), stack);

	gtk_widget_insert_action_group(foot, "import", ag);

	return foot;
}


static GtkWidget* ui_build_import_text(GtkApplication *app, KeeImport *import, GtkStack *stack) {
	GtkWidget *box;
	GtkTextView *txt;
	GtkWidget *butt;
	GAction *act;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	txt = GTK_TEXT_VIEW(gtk_text_view_new());
	gtk_widget_set_vexpand(GTK_WIDGET(txt), true);
	gtk_box_append(GTK_BOX(box), GTK_WIDGET(txt));

	act = G_ACTION(g_simple_action_new("import_data_accept", NULL));
	g_simple_action_set_enabled(G_SIMPLE_ACTION(act), false);
	g_action_map_add_action(G_ACTION_MAP(app), act);

	butt = gtk_button_new_with_label("import");
	gtk_actionable_set_action_name(GTK_ACTIONABLE(butt), "app.import_data_accept");
	gtk_box_append(GTK_BOX(box), butt);

	g_signal_connect(import, "data_available", G_CALLBACK(ui_handle_import_data_text), gtk_text_view_get_buffer(txt));
	g_signal_connect(import, "data_available", G_CALLBACK(ui_handle_import_data_accept), app); // replace with import
	g_signal_connect(import, "data_available", G_CALLBACK(ui_handle_import_data_focus), stack);

	return box;
}


void ui_build_scan(GtkApplication *app, KeeImport *import) {
	GtkWidget *chooser;
	GtkWidget *box_outer;
	GtkWidget *box;
	GtkWidget *widget;
	GtkStack *stack;

	box_outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	stack = kee_import_get_stack(import);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	chooser = ui_build_scan_videochooser(import);
	gtk_box_append(GTK_BOX(box), chooser);

	gtk_box_append(GTK_BOX(box), GTK_WIDGET(import));

	gtk_stack_add_named(stack, box, KEE_ACT_SCAN_QR);

	widget = ui_build_import_text(app, import, stack);
	gtk_stack_add_named(stack, widget, KEE_ACT_SCAN_TEXT);

	gtk_stack_set_visible_child_name(stack, KEE_ACT_SCAN_QR);

	widget = ui_build_scan_footer(import, stack);

	gtk_box_append(GTK_BOX(box_outer), GTK_WIDGET(stack));
	gtk_box_append(GTK_BOX(box_outer), widget);

	kee_view_add(box_outer, "import");
}


static GtkWidget* ui_build_view(KeeUicontext *uctx) {
	GtkListItemFactory *factory;
	GtkSelectionModel *sel;
	GListModel *front_list;
	GtkListView *front_view;

	factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(new_item), NULL);
	
	front_list = G_LIST_MODEL(gtk_string_list_new(NULL));
	g_object_set_data(G_OBJECT(uctx), KEE_W_FRONTLIST, front_list);

	sel = GTK_SELECTION_MODEL(gtk_single_selection_new(front_list));
	front_view = GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(sel), factory));

	return GTK_WIDGET(front_view);
}


void ui_build(GtkApplication *app, KeeUicontext *uctx) {
	GtkWidget *widget;
	GtkWidget *win;
	GtkWidget *stack;
	GtkWidget *head;
	GSimpleAction *act;

	win = gtk_application_window_new (app);

	head = header_setup(app, uctx);

	gtk_window_set_title (GTK_WINDOW (win), "kee");
	gtk_window_set_default_size (GTK_WINDOW (win), 720, 1440);

	stack = gtk_stack_new();
	kee_view_init(GTK_STACK(stack));

	widget = ui_build_unlock(uctx);
	kee_view_add(widget, "unlock");

	widget = ui_build_view(uctx);
	kee_view_add(widget, "view");
	
	//widget = ui_build_scan(uctx);
	//kee_view_add(widget, "import");

	kee_view_next("view");
	kee_view_next("unlock");

	act = g_simple_action_new("import", NULL);
	g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(act));
	g_simple_action_set_enabled(act, false);
	g_signal_connect(act, "activate", G_CALLBACK(act_import), stack);
	
	gtk_window_set_titlebar(GTK_WINDOW(win), head);
	g_object_set_data(G_OBJECT(uctx), KEE_W_WINDOW, GTK_WINDOW(win));

	widget = g_object_get_data(G_OBJECT(uctx), KEE_W_UI_MENU_QUICK_ADD);

	g_signal_connect (uctx, "state", G_CALLBACK(win_handle_state), win);

	gtk_widget_set_vexpand(stack, true);

	gtk_window_set_child(GTK_WINDOW(win), stack);

	gtk_window_present(GTK_WINDOW (win));
}

//void ui_build_from_resource(GtkApplication *app, struct ui_container *ui) {
//	GtkBuilder *build;
//	GtkWidget *unlock;
//
//	build = gtk_builder_new_from_resource("/org/defalsify/Kee/main.ui");
//	//ui->view = GTK_WINDOW(gtk_builder_get_object(build, "keechoose"));
//	unlock = GTK_WINDOW(gtk_builder_get_object(build, "keeunlock"));
//	if (!unlock) {
//		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "unlock widget could not load");
//	}
//
//	ui->win = GTK_APPLICATION_WINDOW(gtk_application_window_new (app));
//	gtk_window_set_child(GTK_WINDOW(ui->win), GTK_WIDGET(unlock));
//
//	gtk_window_present(GTK_WINDOW (ui->win));
//}
