#ifndef KEE_CLI_H_
#define KEE_CLI_H_

#define KEE_CLI_BUFMAX 4096

#include "settings.h"
#include "gpg.h"

struct kee_cli_t {
	struct kee_settings settings;
	struct gpg_store gpg;
	struct kee_transport_t trans;
	char *passphrase;
	char *result;
	size_t result_len;
};

int cli_init(struct kee_cli_t *cli, const char *passphrase);
int cli_exit(struct kee_cli_t *cli, int err);
int cli_decode(struct kee_cli_t *cli, char *in, long unsigned int *in_size);
int cli_encode(struct kee_cli_t *cli, char *out, long unsigned int *out_size);

#endif // KEE_CLI_H_
