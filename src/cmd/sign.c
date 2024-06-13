#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "transport.h"
#include "settings.h"
#include "ledger.h"
#include "debug.h"
#include "err.h"
#include "llog.h"

#include "cli.h"


void debug_log(int lvl, const char *s) {
	//char *e;

	//e = llog_new(lvl, (char*)s);
	fprintf(stderr, "%s\n", s);
}

void cli_init(struct kee_cli_t *cli) {
	memset(cli, 0, sizeof(struct kee_cli_t));
	err_init();
}

void cli_set_passphrase(struct kee_cli_t *cli, struct gpg_store *keystore, const char *passphrase) {
	cli->passphrase = malloc(keystore->passphrase_digest_len);
	gpg_store_digest(keystore, cli->passphrase, passphrase);
}

static void cli_free(struct kee_cli_t *cli) {
	if (cli->passphrase) {
		free(cli->passphrase);
	}
}

int cli_exit(struct kee_cli_t *cli, int err) {
	cli_free(cli);
	return err;
}

char* unlock(struct gpg_store *keystore, struct kee_settings *settings, char *passphrase) {
	int r;

	if (passphrase == NULL) {
		passphrase = getenv("KEE_PASSPHRASE");
	}
	if (passphrase == NULL || strlen(passphrase) == 0) {
		return NULL;
	}
	gpg_store_init(keystore, (const char*)settings->key);
	r = gpg_store_check(keystore, passphrase);
	if (r) {
		return NULL;
	}

	return passphrase;
}

int main(int argc, char **argv) {
	struct kee_settings settings;
	struct gpg_store keystore;
	struct kee_ledger_t ledger;
	struct kee_transport_t trans;
	struct kee_cli_t cli;
	char *passphrase;
	char dbg[4096];
	char b[KEE_CLI_BUFMAX];
	char *p;
	int r;
	int f;
	long unsigned int c;
	int l;

	cli_init(&cli);

	settings_new_from_xdg(&settings);
	settings_init(&settings);
	passphrase = unlock(&keystore, &settings, NULL);
	if (passphrase == NULL) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "keyunlock fail");
		return ERR_FAIL;
	}
	cli_set_passphrase(&cli, &keystore, passphrase);

	if (argc < 2) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "usage: kee-sign <file>");
		return cli_exit(&cli, ERR_FAIL);
	}

	f = open(*(argv+1), O_RDONLY);
	if (f < 0) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "argument is not a file that can be opened");
		return cli_exit(&cli, ERR_FAIL);
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
			return cli_exit(&cli, ERR_FAIL);
		}
	}
	close(f);

	c = KEE_CLI_BUFMAX - l;
	sprintf(dbg, "Read %lu bytes from %s", c, *(argv+1));
	debug_log(DEBUG_INFO, dbg);

	r = kee_transport_single(&trans, KEE_TRANSPORT_BASE64, KEE_CMD_IMPORT, 0);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "transport init fail");
		return cli_exit(&cli, ERR_FAIL);
	}

	r = kee_transport_write(&trans, b, c);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "parse transport fail");
		return cli_exit(&cli, ERR_FAIL);
	}

	c = KEE_CLI_BUFMAX;
	r = kee_transport_read(&trans, b, &c);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "unwrap transport fail");
		return cli_exit(&cli, ERR_FAIL);
	}

	r = kee_ledger_parse_open(&ledger, &keystore, b, c);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "not valid ledger data");
		return cli_exit(&cli, ERR_FAIL);
	}

	sprintf(dbg, "parsed ledger: %s", ledger.content.subject);
	debug_log(DEBUG_INFO, dbg);

	r = kee_ledger_sign(&ledger, ledger.last_item, &keystore, cli.passphrase);
	if (r) {
		debug_logerr(LLOG_CRITICAL, r, "ledger sign fail");
		return cli_exit(&cli, ERR_FAIL);
	}

	return cli_exit(&cli, 0);
}
