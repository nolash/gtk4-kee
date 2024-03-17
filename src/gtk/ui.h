#ifndef _UI_H
#define _UI_H

#include <gtk/gtk.h>

#include "scan.h"
#include "settings.h"
#include "kee-uicontext.h"

struct ui_container {
	GtkApplication *gapp;
	GtkApplicationWindow *win;
	GtkStack *stack;
	GListModel *front_list;
	GListModel *camera_list;
	GtkListView *front_view;
	GtkBox *front_scan;
	GtkHeaderBar *head;
	struct kee_scanner scan;
	struct kee_context *ctx;
	int state;
};

int ui_init(struct ui_container *ui);
void ui_build(GtkApplication *app, KeeUicontext *uctx);
int ui_state_change(struct ui_container *ui, int set, int reset);
void ui_free(struct ui_container *ui);

void ui_handle_scan(GtkApplication *app, struct kee_context *ctx);

#endif // _UI_H
