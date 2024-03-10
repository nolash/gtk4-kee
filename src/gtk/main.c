#include <string.h>
#include <gtk/gtk.h>
#include <glib-object.h>

#include "kee-uicontext.h"
#include "ui.h"
#include "context.h"
#include "menu.h"
#include "settings.h"


static void startup(GtkApplication *app, gpointer user_data) {
	menu_setup(user_data);
}

static void activate(GtkApplication *app, gpointer user_data) {
	ui_build(app, user_data);
}

static void deactivate(GtkApplication *app, gpointer user_data) {
	g_object_unref(user_data);
	ui_free(user_data);
}

int main(int argc, char **argv) {
	int r;
	KeeUicontext *uctx;
	struct kee_context ctx;
	struct ui_container ui;

	r = ui_init(&ui);
	if (r) {
		return r;
	}

	kee_context_new(&ctx, &ui);
	uctx = g_object_new(KEE_TYPE_UICONTEXT, "ui_container", &ui, "core_context", &ctx, NULL);

	settings_new_from_xdg(&ctx.settings);
	settings_init(&ctx.settings);
	db_connect(&ctx.db, "./testdata_mdb");

	g_signal_connect (ui.gapp, "startup", G_CALLBACK (startup), uctx);
	g_signal_connect (ui.gapp, "activate", G_CALLBACK (activate), &ui);
	g_signal_connect (ui.gapp, "shutdown", G_CALLBACK (deactivate), uctx);
	g_signal_connect (uctx, "scan_want", G_CALLBACK( ui_handle_scan) , &ui);

	r = g_application_run (G_APPLICATION (ui.gapp), argc, argv);

	g_object_unref(ui.gapp);
	return r;
}
