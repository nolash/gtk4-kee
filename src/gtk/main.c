#include <string.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include <gst/gst.h>

#include "kee-uicontext.h"
#include "kee-import.h"
#include "ui.h"
//#include "context.h"
#include "menu.h"
#include "settings.h"
//#include "camera.h"


static void state_log(KeeUicontext *uctx, char state_hint, kee_state_t *new_state, kee_state_t *old_state) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "new state hint: %d", state_hint);
}

static void startup(GtkApplication *app, KeeUicontext *uctx) {
//	kee_uicontext_scaninit(uctx);
}

static void activate(GtkApplication *app, KeeUicontext *uctx) {
	ui_build(app, uctx);
}
static void activate_scan(GtkApplication *app, KeeImport *import) {
	ui_build_scan(app, import);
}

static void deactivate(GtkApplication *app, gpointer user_data) {
	g_object_unref(user_data);
}

int main(int argc, char **argv) {
	int r;
	KeeUicontext *uctx;
	struct kee_settings settings;
	struct kee_context ctx;
	GtkApplication *gapp;
	KeeImport *import;

	gtk_init();
	gst_init(0, NULL);

	gapp = gtk_application_new ("org.defalsify.Kee", G_APPLICATION_DEFAULT_FLAGS);

	settings_new_from_xdg(&settings);
	settings_init(&settings);

	//uctx = g_object_new(KEE_TYPE_UICONTEXT, "gtk_application", gapp, "core_context", &ctx, NULL);
	import = g_object_new(KEE_TYPE_IMPORT, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	//db_connect(&ctx.db, "./testdata_mdb");
	uctx = NULL;

	g_signal_connect (gapp, "startup", G_CALLBACK (startup), uctx);
	g_signal_connect (gapp, "activate", G_CALLBACK (activate), uctx);
	g_signal_connect (gapp, "activate", G_CALLBACK (activate_scan), import);
	g_signal_connect (gapp, "shutdown", G_CALLBACK (deactivate), uctx);
	g_signal_connect (uctx, "unlock", G_CALLBACK(ui_handle_unlock), uctx);
	g_signal_connect (uctx, "state", G_CALLBACK(state_log), NULL);

	r = g_application_run (G_APPLICATION (gapp), argc, argv);

	g_object_unref(gapp);
	kee_context_free(&ctx);
	return r;
}
