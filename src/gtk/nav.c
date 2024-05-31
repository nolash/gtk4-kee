#include <string.h>

#include <gtk/gtk.h>
#include <beamenu.h>

#include "nav.h"
#include "beamenu_defs.h"
#include "debug.h"


static GtkWidget* widgets[KEE_NAV_N_DST];
static int stack[KEE_NAV_STACK_SIZE];
static int stack_crsr;

int kee_nav_init(const char *path) {
	char *p;
	char fullpath[1024];

	p = stpcpy(fullpath, path);
	if (*(p-1) != '/') {
		*p = '/';
		p++;
	}
	p = strcpy(p, "beamenu.dat");

	stack_crsr = 0;
	return beamenu_load_file(fullpath, 1);
}

int kee_nav_set(GtkWidget *widget, int menu_id) {
	if (widgets[menu_id]) {
		return 1;
	}
	widgets[menu_id] = widget;
	beamenu_jump(menu_id);
	stack[stack_crsr] = menu_id;
	stack_crsr++;
	return 0;
}

int kee_nav_unset(int menu_id) {
	if (widgets[menu_id] == NULL) {
		return 1;
	}
	widgets[menu_id] = 0x0;
	return 0;
}

GtkWidget* kee_nav_back(int force) {
	GtkWidget *widget;
	int r;

	if (stack_crsr == 0) {
		return NULL;
	}

	r = beamenu_use_exit(KEE_NAV_EXIT_BACK);
	if (r < 0) {
		if (!force) {
			debug_log(DEBUG_WARNING, "no back action found");
			return NULL;
		}
		r = 0;
	}
	if (r == 0 || r == BEAMENU_DEFAULT) {
		if (stack_crsr < 2) {
			debug_log(DEBUG_WARNING, "menu stack underrun");
			return NULL;
		}
		r = stack[stack_crsr-1];
		kee_nav_unset(r);
		stack_crsr--;
		r = stack[stack_crsr-1];
	}

	if (widgets[r] == NULL) {
		debug_log(DEBUG_WARNING, "no widget found");
		return NULL;
	}
	widget = widgets[r];
	beamenu_jump(r);

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

int kee_nav_get_idx() {
	struct beamenu_node *o;
	o = beamenu_get(-1);
	return o->i;
}

int kee_nav_get_exit(int exit_id) {
	struct beamenu_node *o;
	o = beamenu_get(-1);
	return o->dst[exit_id];
}
