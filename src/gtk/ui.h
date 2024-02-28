#ifndef _UI_H
#define _UI_H

#include <gtk/gtk.h>

struct ui_container {
	GtkApplicationWindow *win;
	GtkStack *stack;
	GListModel *front_list;
	GtkListView *front_view;
};

void ui_build(GtkApplication *app, struct ui_container *ui);
void ui_free(struct ui_container *ui);

#endif // _UI_H
