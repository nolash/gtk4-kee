#include <string.h>
#include <stddef.h>

#include "dn.h"

int main() {
	int r;
	struct kee_dn_t dn;

	kee_dn_init(&dn, 0);

	r = kee_dn_from_str(&dn, "CN=Foo Bar,O=Baz", 16);
	if (r) {
		return 1;
	}

	if (strcmp(dn.cn, "Foo Bar")) {
		return 1;	
	}

	if (strcmp(dn.o, "Baz")) {
		return 1;	
	}

	kee_dn_free(&dn);

	return 0;
}
