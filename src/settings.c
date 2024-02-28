#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <basedir.h>
#include <sys/stat.h>

#include "err.h"
#include "debug.h"
#include "settings.h"

#define KEE_SETTINGS_DATANAME "kee"
#define KEE_SETTINGS_CAP 4096

/**
 * \todo make xdg optional
 */
int settings_new_from_xdg(struct kee_settings *z) {
	xdgHandle xdg;
	const char *s;
	
	xdgInitHandle(&xdg);

	memset(z, 0, sizeof(struct kee_settings));

	z->data = malloc(KEE_SETTINGS_CAP);

	s = xdgDataHome(&xdg);
	sprintf((char*)z->data, "%s/%s", s, KEE_SETTINGS_DATANAME);

	s = xdgRuntimeDirectory(&xdg);
	sprintf((char*)z->run, "%s/%s", s, KEE_SETTINGS_DATANAME);

	return ERR_OK;
}

int settings_init(struct kee_settings *z) {
	int r;
	char s[1024];
	
	r = mkdir((char*)z->data, S_IRUSR | S_IWUSR);
	if (r) {
		if (errno != EEXIST) {
			debug_log(DEBUG_ERROR, strerror(errno));
			return r;
		}
	}
	sprintf(s, "datadir: %s\nrundir: %s", z->data, z->run);
	debug_log(DEBUG_DEBUG, s);

	return ERR_OK;
}

unsigned char *settings_get(struct kee_settings *z, enum SettingsType typ) {
	switch(typ) {
		case SETTINGS_DATA:
			return z->data;
			break;
		case SETTINGS_RUN:
			return z->run;
			break;
		case SETTINGS_LOCKTIME:
			return z->locktime;
			break;
		default:
			return (unsigned char*)"";
	}
}

int settings_set(struct kee_settings *z, enum SettingsType typ, unsigned char* v) {
	switch(typ) {
		case SETTINGS_DATA:
			z->data = v;
			break;
		case SETTINGS_RUN:
			z->run = v;
			break;
		case SETTINGS_LOCKTIME:
			z->locktime = v;
			break;
		default:
			return ERR_FAIL;
	}
	return ERR_OK;
}

void settings_free(struct kee_settings *z) {
	free(z->data);
}
