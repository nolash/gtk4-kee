#include "debug.h"

void debug_log(enum debugLevel level, const char *s);

void debug_logerr(enum lloglvl_e lvl, char *msg, int err) {
	char *e;
	char *s;

	s = rerrpfx(err);
	e = llog_new_ns(lvl, msg, s);
	e = llog_add_n("errcode", err);
	s = rerrstrv(err);
	e = llog_add_s("err", s);
	debug_log(lvl, e);
}
