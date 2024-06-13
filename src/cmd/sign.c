#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

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



int main(int argc, char **argv) {
	struct kee_transport_t trans;
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

	r = kee_ledger_parse_open(&ledger, &cli.gpg, b, c);
	if (r) {
		debug_logerr(LLOG_CRITICAL, ERR_FAIL, "not valid ledger data");
		return cli_exit(&cli, ERR_FAIL);
	}

	sprintf(dbg, "parsed ledger: %s", ledger.content.subject);
	debug_log(DEBUG_INFO, dbg);

	r = kee_ledger_sign(&ledger, ledger.last_item, &cli.gpg, cli.passphrase);
	if (r) {
		debug_logerr(LLOG_CRITICAL, r, "ledger sign fail");
		return cli_exit(&cli, ERR_FAIL);
	}

	return cli_exit(&cli, 0);
}
