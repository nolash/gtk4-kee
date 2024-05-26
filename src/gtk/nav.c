#include <string.h>

#include <gtk/gtk.h>
#include <beamenu.h>

#include "nav.h"
#include "beamenu_defs.h"


static GtkWidget* widgets[KEE_NAV_N_DST];

int kee_nav_init(const char *path) {
	char *p;
	char fullpath[1024];

	p = stpcpy(fullpath, path);
	if (*(p-1) != '/') {
		*p = '/';
		p++;
	}
	p = strcpy(p, "beamenu.dat");

	return beamenu_load_file(fullpath, 1);
}

int kee_nav_set(GtkWidget *widget, int menu_id) {
	if (widgets[menu_id]) {
		return 1;
	}
	widgets[menu_id] = widget;
	beamenu_jump(menu_id);
	return 0;
}

int kee_nav_unset(int menu_id) {
	if (widgets[menu_id] == NULL) {
		return 1;
	}
	widgets[menu_id] = 0x0;
	return 0;
}

GtkWidget* kee_nav_back() {
	GtkWidget *widget;
	int r;

	r = beamenu_use_exit(KEE_NAV_EXIT_BACK);
	if (r < 0) {
		return NULL;
	}
	if (widgets[r] == NULL) {
		return NULL;
	}
	widget = widgets[r];
	widgets[r] = 0x0;
	return widget;
}

GtkWidget* kee_nav_get() {
	struct beamenu_node *o;

	o = beamenu_get(-1);
	return widgets[o->i];
}

char *kee_nav_get_label() {
	struct beamenu_node *o;

	o = beamenu_get(-1);
	return beamenu_dst_r[o->i];
}
