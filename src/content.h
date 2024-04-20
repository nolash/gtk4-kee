#ifndef KEE_CONTENT_H_
#define KEE_CONTENT_H_

#include <stddef.h>

#include "cadiz.h"

#define KEE_CONTENT_DEFAULT_SIZE 4096
#define KEE_CONTENT_KEY_SIZE 64

#define KEE_CONTENT_RESOLVED 1
#define KEE_CONTENT_RESOLVED_DATA 2
#define KEE_CONTENT_RESOLVED_BODY 4
#define KEE_CONTENT_RESOLVED_SUBJECT 8

struct kee_content_t {
	char key[KEE_CONTENT_KEY_SIZE];
	size_t mem_size;
	char *mem;
	char *subject;
	char *body;
	char flags;
};

int kee_content_init(struct kee_content_t *content, const char *key, size_t size_hint);
int kee_content_resolve(struct kee_content_t *content, Cadiz *cadiz);
void kee_content_free(struct kee_content_t *content);

#endif
