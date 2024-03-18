#ifndef _KEE_VIEW_H
#define _KEE_VIEW_H

#include <gtk/gtk.h>

#include "nav.h"

struct KeeView {
	GtkStack *stack;
	struct KeeNav nav;
};

void kee_view_init(GtkStack *stack);
int kee_view_add(GtkWidget *widget, const char *label);
int kee_view_next(const char *label);
int kee_view_prev();

#endif // _KEE_VIEW_H
