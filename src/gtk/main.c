#include <string.h>
#include <gtk/gtk.h>
#include <glib-object.h>

#include "kee-uicontext.h"
#include "ui.h"
//#include "context.h"
#include "menu.h"
#include "settings.h"
//#include "camera.h"


static void startup(GtkApplication *app, KeeUicontext *uctx) {
	kee_uicontext_scaninit(uctx);
	header_setup(app, uctx);
	//ui_setup(uctx);
}


static void activate(GtkApplication *app, KeeUicontext *uctx) {
	ui_build(app, uctx);
}

static void deactivate(GtkApplication *app, gpointer user_data) {
	g_object_unref(user_data);
	//ui_free(user_data);
}

int main(int argc, char **argv) {
	int r;
	KeeUicontext *uctx;
	struct kee_settings settings;
	struct kee_context ctx;
	GtkApplication *gapp;
	//struct ui_container ui;

	//r = ui_init(&ui);
	//if (r) {
	//	return r;
	//}

	gapp = gtk_application_new ("org.defalsify.Kee", G_APPLICATION_DEFAULT_FLAGS);

	//settings_new_from_xdg(&ctx.settings);
	settings_new_from_xdg(&settings);
	//settings_init(&ctx.settings);
	settings_init(&settings);

	//kee_context_new(&ctx, &ui, &settings);
	//uctx = g_object_new(KEE_TYPE_UICONTEXT, "ui_container", &ui, "core_context", &ctx, NULL);
	uctx = g_object_new(KEE_TYPE_UICONTEXT, "gtk_application", gapp, "core_context", &ctx, NULL);
	//db_connect(&ctx.db, "./testdata_mdb");

	g_signal_connect (gapp, "startup", G_CALLBACK (startup), uctx);
	g_signal_connect (gapp, "activate", G_CALLBACK (activate), uctx);
	g_signal_connect (gapp, "shutdown", G_CALLBACK (deactivate), uctx);
//	//g_signal_connect (uctx, "scan_want", G_CALLBACK( ui_handle_scan) , &ctx);
//	g_signal_connect (uctx, "scan_want", G_CALLBACK( ui_handle_scan) , uctx);

//	r = g_application_run (G_APPLICATION (ui.gapp), argc, argv);
	r = g_application_run (G_APPLICATION (gapp), argc, argv);

	g_object_unref(gapp);
	kee_context_free(&ctx);
	return r;
}
