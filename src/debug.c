#include "debug.h"

void debug_log(enum debugLevel level, const char *s);

int debug_logerr(enum lloglvl_e lvl, int err, char *msg) {
	char *e;
	char *s;

	if (msg == 0) {
		msg = "debug logerr";
	}
	s = rerrpfx(err);
	e = llog_new_ns(lvl, msg, s);
	e = llog_add_x("errcode", err);
	s = rerrstrv(err);
	e = llog_add_s("err", s);
	debug_log(lvl, e);
	return err;
}
