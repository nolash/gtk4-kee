#include <string.h>
#include <stdlib.h>

#include "beamenu.h"


static struct beamenu_node node[BEAMENU_N_DST + 1];

int beamenu_now;

int beamenu_register(int idx, char *cn) {
	int i;
	int l;
	char *p;

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
	return 0;
}

void beamenu_free() {
	int i;
	char *p;

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

int beamenu_move(int idx_exit) {
	struct beamenu_node *o;
	int r;
	int idx;

	o = beamenu_get(beamenu_now);
	idx = idx_exit;
	r = o->dst[idx_exit];
	switch(r) {
		case BEAMENU_INACTIVE:
			return 1;
			break;
		case BEAMENU_ROOT:
			idx = 0;
			break;
		default:
			idx = r;
	}

	beamenu_now = idx;
	return 0;
}

struct beamenu_node *beamenu_get(int idx_node) {
	if (idx_node < 0) {
		return &node[beamenu_now];
	}
	return &node[idx_node];
}

