#include <string.h>

#include "content.h"

#include "hex.h"
#include "cadiz.h"

const char hash_of_foo[] = "1382ea21c44e0352916e17267351fcc2d890816a254caba354b2da23c2b33a9c50c5b4151ddab876d199e3c0bca26fcf75302427db5ca05e5f049f2a9f8dc9d4";


int main() {
	int r;
	Cadiz cadiz;
	struct kee_content_t *content;
	char digest[64];
	size_t l;

	hex2bin(hash_of_foo, (unsigned char*)digest);

	cadiz.locator = "./testdata_resource";

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
	content = kee_content_new(digest, l);
	if (content == NULL) {
		return 1;
	}
	r = kee_content_resolve(content, &cadiz);
	if (r) {
		return 1;
	}

	if (content->flags != (KEE_CONTENT_RESOLVED_SUBJECT | KEE_CONTENT_RESOLVED_BODY | KEE_CONTENT_RESOLVED)) {
		return 1;
	}

	if (strcmp(content->subject, "Federal back single democratic growth fly image.")) {
		kee_content_free(content);
		return 1;
	}

	kee_content_free(content);

	return 0;
}
