#include <gtk/gtk.h>

#include "nav.h"


void kee_nav_push(struct KeeNav *nav, GtkWidget *page) {
	struct KeeNav *nav_old;

	nav_old = malloc(sizeof(struct KeeNav));
	nav_old->prev = nav->prev;
	nav_old->now = nav->now;
	nav->prev = nav_old;
	nav->now = page;
}

GtkWidget* kee_nav_pop(struct KeeNav *nav) {
	struct KeeNav *nav_old;
	GtkWidget *r;

	if (!nav->prev) {
		return NULL;
	}
	
	r = nav->now;
	nav_old = nav->prev->prev;
	nav->now = nav->prev->now;
	free(nav->prev);
	nav->prev = nav_old;
	return r;
}
