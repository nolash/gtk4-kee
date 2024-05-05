#include <gtk/gtk.h>

#include "debug.h"

#define G_LOG_DOMAIN "Kee"


void debug_log(enum debugLevel level, const char *s) {
	int loglevel;

	switch(level) {
		case DEBUG_CRITICAL:
			loglevel = G_LOG_LEVEL_ERROR;
			break;
		case DEBUG_ERROR:
			loglevel = G_LOG_LEVEL_ERROR;
			break;
		default:
			loglevel = G_LOG_LEVEL_DEBUG;
	}
	g_log(G_LOG_DOMAIN, loglevel, s);
}
