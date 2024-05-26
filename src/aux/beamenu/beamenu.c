#include <string.h>
#include <stdlib.h>

#include "beamenu.h"


static struct beamenu_node node[BEAMENU_N_DST + 1];

int beamenu_now;

int beamenu_register(int idx, char *cn) {
	int i;
	int l;

	l = strlen(cn) + 1;
	if (node[idx].cn) {
		free(node[idx].cn);
	}
	node[idx].cn = malloc(l);
	if (!node[idx].cn) {
		return 1;
	}
	memcpy(node[idx].cn, cn, l);
	for (i = 0; i < BEAMENU_N_EXITS; i++) {
		node[idx].dst[i] = 0;
	}
	node[idx].i = idx;
	return 0;
}

void beamenu_free() {
	int i;

	for (i = 0; i <= BEAMENU_N_DST; i++) {
		if (node[i].cn) {
			free(node[i].cn);
			node[i].cn = 0;
		}
	}
}

void beamenu_set(int idx_node, int idx_exit, int idx_dst) {
	struct beamenu_node *o;

	o = beamenu_get(idx_node);
	o->dst[idx_exit] = idx_dst;
}

int beamenu_use_exit(int idx_exit) {
	int idx;

	idx = beamenu_get_exit(idx_exit);
	switch(idx) {
		case BEAMENU_INACTIVE:
			return -1;
			break;
		case BEAMENU_ROOT:
			idx = 0;
			break;
		default:
			if (idx_exit < 0 || idx_exit >= BEAMENU_N_EXITS) {
				return -2;
			}
	}

	return beamenu_jump(idx);
}

int beamenu_get_exit(int idx_exit) {
	struct beamenu_node *o;

	if (idx_exit == BEAMENU_ROOT) {
		return idx_exit;
	}
	o = beamenu_get(beamenu_now);
	return o->dst[idx_exit];
}

int beamenu_jump(int idx) {
	beamenu_now = idx;
	return idx;
}

struct beamenu_node *beamenu_get(int idx_node) {
	if (idx_node < 0) {
		return &node[beamenu_now];
	}
	return &node[idx_node];
}
