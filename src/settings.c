#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <basedir.h>
#include <sys/stat.h>

#include "err.h"
#include "debug.h"
#include "settings.h"

#define KEE_SETTINGS_NAME "kee"
#define KEE_SETTINGS_CAP 4096
#define KEE_SETTINGS_ITEM_CAP 512

static void settings_get_paths(struct kee_settings *z) {
	char *p;

	p = getenv("KEE_PATH");
	if (p != NULL) {
		memcpy(z->data, p, strlen(p));
	}

	p = getenv("KEE_KEY_PATH");
	if (p != NULL) {
		memcpy(z->key, p, strlen(p));
	}
}

/**
 * \todo make xdg optional
 */
int settings_new_from_xdg(struct kee_settings *z) {
	xdgHandle xdg;
	const char *s;
	unsigned char *p;
	
	xdgInitHandle(&xdg);

	memset(z, 0, sizeof(struct kee_settings));

	//z->key = (unsigned char*)"./testdata_crypt";

	z->data = calloc(KEE_SETTINGS_CAP, 1);
	p = z->data;
	p += KEE_SETTINGS_ITEM_CAP;
	z->db = p;
	p += KEE_SETTINGS_ITEM_CAP;
	z->resource = p;
	p += KEE_SETTINGS_ITEM_CAP;
	z->run = p;
	p += KEE_SETTINGS_ITEM_CAP;
	z->key = p;
	p += KEE_SETTINGS_ITEM_CAP;
	z->locktime = p;
	p += KEE_SETTINGS_ITEM_CAP;
	z->video_device = p;

	settings_get_paths(z);
	if (*z->data == 0x0) {
		s = xdgDataHome(&xdg);
		sprintf((char*)z->data, "%s/%s", s, KEE_SETTINGS_NAME);
	}
	if (*z->key == 0x0) {
		sprintf((char*)z->key, "%s/crypt", z->data);
	}
	sprintf((char*)z->db, "%s/mdb", z->data);
	sprintf((char*)z->resource, "%s/resource", z->data);

	s = xdgRuntimeDirectory(&xdg);
	sprintf((char*)z->run, "%s/%s", s, KEE_SETTINGS_NAME);

	return ERR_OK;
}


/***
 * \todo verify default video exists
 */
int settings_init(struct kee_settings *z) {
	int r;
	char s[1024];
	char *ss;
	
	r = mkdir((char*)z->data, S_IRUSR | S_IWUSR);
	if (r) {
		if (errno != EEXIST) {
			debug_log(DEBUG_ERROR, strerror(errno));
			return r;
		}
	}
	sprintf(s, "datadir: %s\nrundir: %s", z->data, z->run);
	debug_log(DEBUG_DEBUG, s);

	ss = getenv("KEE_VIDEO");
	if (!ss) {
		ss = "/dev/video0";
	}
	strcpy((char*)z->video_device, ss);

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
		case SETTINGS_KEY:
			return z->key;
			break;
		case SETTINGS_LOCKTIME:
			return z->locktime;
			break;
		case SETTINGS_VIDEO:
			return z->video_device;
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
		case SETTINGS_VIDEO:
			z->video_device = v;
			break;

		default:
			return ERR_FAIL;
	}
	return ERR_OK;
}

void settings_free(struct kee_settings *z) {
	free(z->data);
}
