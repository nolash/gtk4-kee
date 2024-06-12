#include "debug.h"

char *_rerr_test[2] = {
	"Epic Foo Fail",
	"Epic Bar Fail",
};


int main() {
	rerr_init("testcore");
	rerr_register(0x100, "debugtest", _rerr_test);
	debug_logerr(LLOG_INFO, 0x101, "foo");

	return ERR_OK;
}
