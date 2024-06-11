#ifndef _KEE_NAV_H
#define _KEE_NAV_H

#include <gtk/gtk.h>

#include <beamenu_defs.h>

#define KEE_NAV_NOW kee_nav_get()
#define KEE_NAV_LABEL kee_nav_get_label()
#define KEE_NAV_IDX kee_nav_get_idx()

#ifndef KEE_NAV_N_DST
#define KEE_NAV_N_DST BEAMENU_N_DST + 1
#endif

#define KEE_NAV_EXIT_BACK 0
#define KEE_NAV_EXIT_NEW 1
#define KEE_NAV_EXIT_IMPORT 2

#define KEE_NAV_STACK_SIZE 8

int kee_nav_init(const char *path);
int kee_nav_set(GtkWidget *, int idx);
int kee_nav_unset(int idx);
GtkWidget* kee_nav_get();
int kee_nav_get_stack_idx(int idx);
int kee_nav_get_idx();
char* kee_nav_get_label();
GtkWidget* kee_nav_back(); // returns new current widget
int kee_nav_get_exit();

#endif // _KEE_NAV_H
