#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "transport.h"
#include "settings.h"
#include "ledger.h"
#include "debug.h"
#include "err.h"
#include "llog.h"

#include "cmd.h"


void debug_log(int lvl, const char *s) {
	char *e;

	//e = llog_new(lvl, (char*)s);
	fprintf(stderr, "%s\n", s);
}

int unlock(struct gpg_store *keystore, struct kee_settings *settings, char *passphrase) {
	int r;

	if (passphrase == NULL) {
		passphrase = getenv("KEE_PASSPHRASE");
	}
	if (passphrase == NULL || strlen(passphrase) == 0) {
		return ERR_FAIL;
	}
	gpg_store_init(keystore, (const char*)settings->key);
	r = gpg_store_check(keystore, passphrase);
	if (r) {
		return ERR_FAIL;
	}

	return ERR_OK;
}

int main(int argc, char **argv) {
	struct kee_settings settings;
	struct gpg_store keystore;
	struct kee_ledger_t ledger;
	struct kee_transport_t trans;
	char dbg[4096];
	char b[KEE_CLI_BUFMAX];
	char *p;
	int r;
	int f;
	long unsigned int c;
	int l;

	err_init();

	settings_new_from_xdg(&settings);
	settings_init(&settings);
	r = unlock(&keystore, &settings, NULL);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "keyunlock fail");
		return 1;
	}

	if (argc < 2) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "usage: kee-sign <file>");
		return 1;
	}

	f = open(*(argv+1), O_RDONLY);
	if (f < 0) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "argument is not a file that can be opened");
		return 1;
	}

	l = KEE_CLI_BUFMAX;
	p = b;
	while (1) {
		c = read(f, b, l);
		if (c == 0) {
			break;
		}
		p += c;
		l -= c;
		if (l == 0) {
			debug_logerr(LLOG_CRITICAL, ERR_FAIL, "read buffer overrun");
			return 1;
		}
	}
	close(f);

	c = KEE_CLI_BUFMAX - l;
	sprintf(dbg, "Read %lu bytes from %s", c, *(argv+1));
	debug_log(DEBUG_INFO, dbg);

	r = kee_transport_single(&trans, KEE_TRANSPORT_BASE64, KEE_CMD_IMPORT, 0);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "transport init fail");
		return ERR_FAIL;
	}

	r = kee_transport_write(&trans, b, c);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "parse transport fail");
		return ERR_FAIL;
	}

	c = KEE_CLI_BUFMAX;
	r = kee_transport_read(&trans, b, &c);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "unwrap transport fail");
		return ERR_FAIL;
	}

	r = kee_ledger_parse_open(&ledger, &keystore, b, c);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "not valid ledger data");
		return 1;
	}

	sprintf(dbg, "parsed ledger: %s", ledger.content.subject);
	debug_log(DEBUG_INFO, dbg);

	return 0;

}
