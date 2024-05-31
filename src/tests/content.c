#include <string.h>

#include "content.h"

#include "hex.h"
#include "cadiz.h"

const char hash_of_foo[] = "184f2c1505c76d3c7b22116d9227a33c95be0a18e0ca15a4cbb983f68c31e9ce0e1a6d365a26b40c5884654c4d38778090185e5cbfad1efc16f30dc9a2231a84";


int main() {
	int r;
	Cadiz cadiz;
	//struct kee_content_t *content;
	struct kee_content_t content;
	char digest[64];
	size_t l;

	h2b(hash_of_foo, (unsigned char*)digest);

	cadiz.locator = "./testdata/resource";

/// \todo this causes a character to be emitted to console after main
//	l = 256;
//	content = kee_content_new(digest, l);
//	if (content == NULL) {
//		return 1;
//	}
//	r = kee_content_resolve(content, &cadiz);
//	if (r) {
//		return 1;
//	}
//	kee_content_free(content);

	l = 4096;
	r = kee_content_init(&content, digest, l);
	if (r) {
		return 1;
	}
	r = kee_content_resolve(&content, &cadiz);
	if (r) {
		return 1;
	}

	if (content.flags != (KEE_CONTENT_RESOLVED_SUBJECT | KEE_CONTENT_RESOLVED_BODY | KEE_CONTENT_RESOLVED_DATA | KEE_CONTENT_RESOLVED)) {
		return 1;
	}

	if (strcmp(content.subject, "foo")) {
		kee_content_free(&content);
		return 1;
	}

	kee_content_free(&content);

	return 0;
}
