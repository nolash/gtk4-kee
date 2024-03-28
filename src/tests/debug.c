#include <stdio.h>

#include "debug.h"

void debug_log(enum debugLevel level, const char *s) {
	fprintf(stderr, s);
}
