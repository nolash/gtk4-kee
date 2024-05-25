#include <gtk/gtk.h>

#include "debug.h"


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

extern int debug_logerr(enum lloglvl_e lvl, int err, char *msg);
