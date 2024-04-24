#include <gtk/gtk.h>

#include "nav.h"

int main(int argc, char **argv) {
	struct KeeNav nav;
	GtkLabel *a;
	GtkLabel *b;
	GtkLabel *c;
	GtkLabel *r;

	gtk_init();
	a = gtk_label_new("foo");
	b = gtk_label_new("bar");
	c = gtk_label_new("baz");

	memset(&nav, 0, sizeof(struct KeeNav));
	r = kee_nav_push(&nav, a);
	if (r) {
		return 1;
	}
	r = kee_nav_push(&nav, b);
	if (r) {
		return 1;
	}
	r = kee_nav_pop(&nav);
	if (r != a) {
		return 1;
	}
	kee_nav_push(&nav, b);
	r = kee_nav_pop(&nav);
	if (r != a) {
		return 1;
	}
	r = kee_nav_pop(&nav);
	if (r != NULL) {
		return 1;
	}

	kee_nav_push(&nav, c);
	r = kee_nav_pop(&nav);
	if (r != NULL) {
		return 1;
	}

	r = kee_nav_pop(&nav);
	if (r) {
		return 1;
	}

	r = kee_nav_pop(&nav);
	if (r) {
		return 1;
	}

	return 0;
}
