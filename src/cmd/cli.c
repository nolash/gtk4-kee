#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "gpg.h"
#include "err.h"
#include "debug.h"

#include "cli.h"


static char* unlock(struct gpg_store *keystore, struct kee_settings *settings, char *passphrase) {
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

static void cli_set_passphrase(struct kee_cli_t *cli, const char *passphrase) {
	cli->passphrase = malloc(cli->gpg.passphrase_digest_len);
	gpg_store_digest(&cli->gpg, cli->passphrase, passphrase);
}

static void cli_free(struct kee_cli_t *cli) {
	if (cli->passphrase) {
		free(cli->passphrase);
	}
}

int cli_init(struct kee_cli_t *cli, const char *passphrase) {
	memset(cli, 0, sizeof(struct kee_cli_t));
	err_init();
	settings_new_from_xdg(&cli->settings);
	settings_init(&cli->settings);
	passphrase = unlock(&cli->gpg, &cli->settings, NULL);
	if (passphrase == NULL) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "keyunlock fail");
		return ERR_FAIL;
	}
	cli_set_passphrase(cli, passphrase);
	return ERR_OK;
}

int cli_exit(struct kee_cli_t *cli, int err) {
	int f;
	int c;
	int r;

	if (cli->result != NULL) {
		if (err != ERR_OK) {
			f = STDERR_FILENO;
		} else {
			f = STDOUT_FILENO;
		}
		c = cli->result_len;
		while (c > 0) {
			r = write(f, cli->result, c);
			if (r == 0) {
				debug_logerr(LLOG_CRITICAL, ERR_FAIL, "cli result output fail");
				err = ERR_FAIL;
				break;
			}
			c -= r;
		}
	}
	cli_free(cli);
	return err;
}
