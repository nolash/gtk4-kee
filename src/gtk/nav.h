#ifndef _KEE_NAV_H
#define _KEE_NAV_H

#include <gtk/gtk.h>

struct KeeNav {
	GtkWidget *now;
	struct KeeNav *prev;
};

void kee_nav_push(struct KeeNav *nav, GtkWidget *page);
GtkWidget* kee_nav_pop(struct KeeNav *nav);

#endif // _KEE_NAV_H
