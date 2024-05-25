#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "beamenu.h"

int beamenu_load_file(const char *path, int msize) {
	int r;
	int f;
	int v;
	int i;
	int ii;
	size_t c;
	char p[BEAMENU_CN_MAXLEN + 1];
	struct beamenu_node *o;

	f = open(path, O_RDONLY);
	if (f < 0) {
		return 1;
	}

	for (i = 0; i < BEAMENU_N_DST; i++) {
		v = 1;
		ii = 0;
		while (v) {
			if (ii > BEAMENU_CN_MAXLEN) {
				return 1;
			}
			c = read(f, &v, 1);
			if (c == 0) {
				return 1;
			}
			p[ii] = v;
			ii++;
		}
		p[ii] = 0;
		r = beamenu_register(i, p);
		if (r) {
			return 1;
		}

		for (ii = 0; ii < BEAMENU_N_EXITS; ii++) {
			c = read(f, &v, 1);
			if (c == 0) {
				return 1;
			}
			beamenu_set(i, ii, v);
		}
	}
	return 0;
}
