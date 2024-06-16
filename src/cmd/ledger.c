#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

#include "ledger.h"
#include "debug.h"
#include "transport.h"
//#include "llog.h"

#include "cli.h"


void debug_log(int lvl, const char *s) {
	//char *e;

	//e = llog_new(lvl, (char*)s);
	fprintf(stderr, "%s\n", s);
}


int act_sign(struct kee_cli_t *cli, struct kee_ledger_t *ledger, char *buf) {
	int r;
	long unsigned int c;

	r = kee_ledger_sign(ledger, ledger->last_item, &cli->gpg, cli->passphrase);
	if (r) {
		debug_logerr(LLOG_CRITICAL, r, "ledger sign fail");
		return ERR_FAIL;
	}

	c = KEE_CLI_BUFMAX;
	r = kee_ledger_serialize_open(ledger, buf, &c, KEE_LEDGER_STATE_FINAL);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "cannot serialize ledger");
		return ERR_FAIL;
	}

	r = cli_encode(cli, buf, &c);
	if (r) {
		return r;
	}

	cli->result = buf;
	cli->result_len = (size_t)c;

	return ERR_OK;
}

/// \todo buf (and length) should be in internal cli struct
int act_print(struct kee_cli_t *cli, struct kee_ledger_t *ledger, char *buf) {
	int r;
	size_t c;

	c = KEE_CLI_BUFMAX;
	r = kee_ledger_sprint(ledger, buf, &c, 1);

	cli->result = buf;
	cli->result_len = r;

	return ERR_OK;
}


int main(int argc, char **argv) {
	struct kee_ledger_t ledger;
	struct kee_cli_t cli;
	char dbg[4096];
	char b[KEE_CLI_BUFMAX];
	char *p;
	int r;
	int f;
	long unsigned int c;
	int l;

	r = cli_init(&cli, NULL);
	if (r) {
		return cli_exit(&cli, ERR_FAIL);
	} 

	r = cli_args(&cli, argc, argv);
	if (r) {
		return cli_exit(&cli, ERR_FAIL);
	}

	if (cli.poslen != 1) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "usage: kee-sign [opts] <file>");
		return cli_exit(&cli, ERR_FAIL);
	}

	f = open(*(cli.posarg), O_RDONLY);
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
	sprintf(dbg, "Read %lu bytes from %s", c, *(cli.posarg));
	debug_log(DEBUG_INFO, dbg);

	r = cli_decode(&cli, b, &c);
	if (r) {
		return cli_exit(&cli, r);
	}
	
	r = kee_ledger_parse_open(&ledger, &cli.gpg, b, c);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "not valid ledger data");
		return cli_exit(&cli, ERR_FAIL);
	}

	sprintf(dbg, "parsed ledger: %s", ledger.content.subject);
	debug_log(DEBUG_INFO, dbg);

	switch(cli.act) {
		case ACT_SIGN:
			r = act_sign(&cli, &ledger, b);
			if (r) {
				debug_logerr(LLOG_CRITICAL, ERR_FAIL, "sign command fail");
			}
			break;
		case ACT_PRINT:
			r = act_print(&cli, &ledger, b);
			if (r) {
				debug_logerr(LLOG_CRITICAL, ERR_FAIL, "print command fail");
			}
			break;
		default:
			debug_logerr(LLOG_CRITICAL, ERR_FAIL, "invalid command");
			r = ERR_FAIL;
	}

	return cli_exit(&cli, r);
}
