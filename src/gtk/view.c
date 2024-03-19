#include <gtk/gtk.h>

#include "view.h"
#include "nav.h"
#include "err.h"

static struct KeeView view;

void kee_view_init(GtkStack *stack) {
	if (view.stack != NULL) {
		return;
	}
	view.stack = stack;
}

int kee_view_add(GtkWidget *widget, const char *label) {
	gtk_stack_add_named(view.stack, widget, label);
	return ERR_OK;
}

int kee_view_next(const char *label) {
	GtkWidget *widget;

	widget = gtk_stack_get_child_by_name(view.stack, label);
	kee_nav_push(&view.nav, widget);
	gtk_stack_set_visible_child(view.stack, widget);
	return ERR_OK;
}

int kee_view_prev() {
	kee_nav_pop(&view.nav);
	gtk_stack_set_visible_child(view.stack, view.nav.now);
	return ERR_OK;
}
