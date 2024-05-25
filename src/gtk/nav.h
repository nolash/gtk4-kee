#ifndef _KEE_NAV_H
#define _KEE_NAV_H

#include <gtk/gtk.h>

#ifndef KEE_NAV_N_DST
#define KEE_NAV_N_DST 0
#endif

struct KeeNav {
//	GtkWidget *now;
//	struct KeeNav *prev;
	GtkWidget *now;
	GtkWidget *widgets[128];
	int c;
	int flags;
	void *dst[KEE_NAV_N_DST + 1];
};

int kee_nav_push(struct KeeNav *nav, GtkWidget *page);
GtkWidget* kee_nav_pop(struct KeeNav *nav);
int kee_nav_is_top(struct KeeNav *nav);

#endif // _KEE_NAV_H
