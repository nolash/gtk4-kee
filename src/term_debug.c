#include <stdio.h>
#include "debug.h"


void debugLog(enum debugLevel level, const char *s) {
	fprintf(stderr, "%d: %s\n", level, s);
}
