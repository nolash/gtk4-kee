#include "debug.h"

char *_rerr[2] = {
	"Epic Foo Fail",
	"Epic Bar Fail",
};


int main() {
	rerr_init();
	rerr_register(0x100, "debugtest", _rerr);
	debug_logerr(LLOG_INFO, "foo", 0x101);

	return ERR_OK;
}
