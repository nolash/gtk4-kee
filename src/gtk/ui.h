#ifndef _UI_H
#define _UI_H

#include <gtk/gtk.h>

#include "scan.h"


typedef enum {
	KEE_UI_INIT = 1,
	KEE_UI_SCANNING = 2,
} UiState;

#define KEE_UI_STATE_IS_SCANNING(c) c->state & KEE_UI_SCANNING

struct ui_container {
	GtkApplication *gapp;
	GtkApplicationWindow *win;
	GtkStack *stack;
	GListModel *front_list;
	GtkListView *front_view;
	struct kee_scanner scan;
	int state;
};

int ui_init(struct ui_container *ui);
void ui_build(GtkApplication *app, struct ui_container *ui);
int ui_state_change(struct ui_container *ui, int set, int reset);
void ui_free(struct ui_container *ui);

#endif // _UI_H
