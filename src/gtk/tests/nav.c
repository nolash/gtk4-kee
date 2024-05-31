#include <gtk/gtk.h>

#include "nav.h"


int test_stack() {
	//struct KeeNav nav;
	int r;
	GtkWidget *a;
	GtkWidget *b;
	GtkWidget *c;
	GtkWidget *o;

	gtk_init();
	a = gtk_label_new("foo");
	b = gtk_label_new("bar");
	c = gtk_label_new("baz");

	r = kee_nav_init("..");
	if (r) {
		return 1;
	}
	r = kee_nav_set(a, 0);
	if (r) {
		return 1;
	}
	o = kee_nav_get();
	if (!o) {
		return 1;
	}
	r = kee_nav_set(b, 1);
	if (r) {
		return 1;
	}
	o = kee_nav_get();
	if (!o) {
		return 1;
	}
	o = kee_nav_back(1);
	if (o != a) {
		return 1;
	}
	kee_nav_set(b, 1);
	o = kee_nav_back(1);
	if (o != a) {
		return 1;
	}

	o = kee_nav_back(1);
	if (o) {
		return 1;
	}

	kee_nav_set(c, 2);
	o = kee_nav_back(1);
	if (o == NULL) {
		return 1;
	}

	o = kee_nav_back(1);
	if (o) {
		return 1;
	}

	o = kee_nav_back(1);
	if (o) {
		return 1;
	}

	return 0;
}

int main(int argc, char **argv) {
	int r;

	r = test_stack();
	if (r) {
		return 1;
	}

	return 0;
}
