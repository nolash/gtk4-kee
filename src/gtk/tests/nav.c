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

	kee_nav_push(&nav, a);
	kee_nav_push(&nav, b);
	r = kee_nav_pop(&nav);
	if (r != b) {
		return 1;
	}
	kee_nav_push(&nav, b);
	r = kee_nav_pop(&nav);
	if (r != b) {
		return 1;
	}
	r = kee_nav_pop(&nav);
	if (r != a) {
		return 1;
	}

	kee_nav_push(&nav, c);
	r = kee_nav_pop(&nav);
	if (r != c) {
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
