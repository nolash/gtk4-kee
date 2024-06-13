#ifndef KEE_CLI_H_
#define KEE_CLI_H_

#define KEE_CLI_BUFMAX 4096

struct kee_cli_t {
	char *passphrase;
};

void cli_init(struct kee_cli_t *cli);
void cli_set_passphrase(struct kee_cli_t *cli, struct gpg_store *keystore, const char *passphrase);
int cli_exit(struct kee_cli_t *cli, int err);

#endif // KEE_CLI_H_
