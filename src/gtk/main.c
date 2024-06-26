#include <string.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include <gst/gst.h>

#include "ui.h"
//#include "menu.h"
#include "settings.h"
#include "context.h"
#include "kee-entry-store.h"
#include "cadir.h"


//static void state_log(KeeUicontext *uctx, char state_hint, kee_state_t *new_state, kee_state_t *old_state) {
//	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "new state hint: %d", state_hint);
//}

static void startup(GtkApplication *app, struct kee_context *ctx) {
//	kee_uicontext_scaninit(uctx);
}

//static void activate(GtkApplication *app, KeeEntryStore *store) { //struct kee_context *ctx) {
static void activate(GtkApplication *app, struct kee_context *ctx) {
	//ui_build(app, store);
	ui_build(app, ctx);
}

static void deactivate(GtkApplication *app, gpointer user_data) {
	//g_object_unref(user_data);
}

int main(int argc, char **argv) {
	int r;
	struct kee_settings settings;
	GtkApplication *gapp;
	struct kee_context ctx;

	gtk_init();
	gst_init(0, NULL);

	gapp = gtk_application_new ("org.defalsify.Kee", G_APPLICATION_DEFAULT_FLAGS);

	settings_new_from_xdg(&settings);
	r = settings_init(&settings);
	if (r) {
		return r;
	}

	r = kee_context_init(&ctx, &settings);
	if (r) {
		return r;
	}

	g_signal_connect (gapp, "startup", G_CALLBACK (startup), &ctx);
	g_signal_connect (gapp, "activate", G_CALLBACK (activate), &ctx);
	g_signal_connect (gapp, "shutdown", G_CALLBACK (deactivate), NULL);
	//g_signal_connect (uctx, "state", G_CALLBACK(state_log), NULL);

	r = g_application_run (G_APPLICATION (gapp), argc, argv);

	g_object_unref(gapp);
	return r;
}
