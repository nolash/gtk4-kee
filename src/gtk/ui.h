#ifndef _UI_H
#define _UI_H

#include <gtk/gtk.h>

#include "scan.h"

struct ui_container {
	GtkApplication *gapp;
	GtkApplicationWindow *win;
	GtkStack *stack;
	GListModel *front_list;
	GtkListView *front_view;
	struct kee_scanner scan;
};

int ui_init(struct ui_container *ui);
void ui_build(GtkApplication *app, struct ui_container *ui);
void ui_free(struct ui_container *ui);

#endif // _UI_H
