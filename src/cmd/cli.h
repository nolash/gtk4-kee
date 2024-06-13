#ifndef KEE_CLI_H_
#define KEE_CLI_H_

#define KEE_CLI_BUFMAX 4096

#include "settings.h"
#include "gpg.h"

struct kee_cli_t {
	struct kee_settings settings;
	struct gpg_store gpg;
	char *passphrase;
};

int cli_init(struct kee_cli_t *cli, const char *passphrase);
int cli_exit(struct kee_cli_t *cli, int err);

#endif // KEE_CLI_H_
