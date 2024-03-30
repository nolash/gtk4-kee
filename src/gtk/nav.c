#include <gtk/gtk.h>

#include "nav.h"

static void kee_nav_log(struct KeeNav *nav) {
	char s[128];
	char out[1024];
	int c;
	int i;

	c = 0;
	for (i = 0; i < nav->c + 1; i++) {
		sprintf(s, "[%d:%p] ", i, nav->widgets[i]);
		sprintf(out+c, s);
		c += strlen(s);
	}
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "nav now %p: %s", nav->now, out);
}

void kee_nav_push(struct KeeNav *nav, GtkWidget *page) {
	nav->c++;
	nav->widgets[nav->c] = page;
	nav->now = nav->widgets[nav->c];
	kee_nav_log(nav);
}

GtkWidget* kee_nav_pop(struct KeeNav *nav) {
	GtkWidget *r;

	if (nav->c == 0) {
		return NULL;
	}
	r = nav->widgets[nav->c];
	nav->c--;
	nav->now = nav->widgets[nav->c];
	kee_nav_log(nav);
}


int kee_nav_is_top(struct KeeNav *nav) {
	return nav->c == 0;
}


//
//void kee_nav_push(struct KeeNav *nav, GtkWidget *page) {
//	struct KeeNav *nav_old;
//
//	nav_old = malloc(sizeof(struct KeeNav));
//	nav_old->prev = nav->prev;
//	nav_old->now = nav->now;
//	nav->prev = nav_old;
//	nav->now = page;
//}
//
//
//GtkWidget* kee_nav_pop(struct KeeNav *nav) {
//	struct KeeNav *nav_old;
//	GtkWidget *r;
//
//	if (!nav->prev) {
//		return NULL;
//	}
//	
//	r = nav->now;
//	nav_old = nav->prev->prev;
//	nav->now = nav->prev->now;
//	free(nav->prev);
//	nav->prev = nav_old;
//	return r;
//}
//
//
//int kee_nav_is_top(struct KeeNav *nav) {
//	return !nav->prev;
//}
