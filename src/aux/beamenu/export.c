#include <string.h>

#include "beamenu.h"


union export_width {
	char c;
	short s;
	int i;
};

static int beamenu_export_exits(struct beamenu_node *o, char *b, int width) {
	int c;
	char *p;
	union export_width w;
	int i;

	c = 0;
	p = b;
	for (i = 0; i < BEAMENU_N_EXITS; i++) {
		w.i = o->dst[i];
		switch (width) {
			case 4:
				memcpy(p, &w.i, 4);
				break;
			case 2:
				memcpy(p, &w.s, 2);
				break;
			case 1:
				*p = w.c;
				break;
			default:
				return 0;
		}
		c += width;
		p += width;
	}

	return c;
}

int beamenu_export(char *out, int width) {
	char *p;
	int l;
	int c;
	int i;
	struct beamenu_node *o;

	if (width == 0) {
		width = 1;
	}

	l = 0;
	c = 0;
	p = out;
	for (i = 0; i < BEAMENU_N_DST; i++) {
		o = beamenu_get(i);
		p = strcpy(p, o->cn);
		c = strlen(o->cn) + 1;
		l += c;
		p += c;
		c = beamenu_export_exits(o, p, width);
		if (!c) {
			return 0;
		}
		l += c;
		p += c;
	}

	return l;
}
