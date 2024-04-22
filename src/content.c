#include <stdlib.h>
#include <string.h>

#include "cmime.h"

#include "content.h"
#include "err.h"


int kee_content_init(struct kee_content_t *content, const char *key, size_t size_hint) {
	if (size_hint) {
		content->mem_size = size_hint;
	} else {
		content->mem_size = KEE_CONTENT_DEFAULT_SIZE;
	}

	content->mem = malloc(content->mem_size);
	if (content->mem == NULL) {
		return 1;
	}
	content->body = content->mem;
	content->flags = 0;
	content->subject = 0;

	memcpy(content->key, key, KEE_CONTENT_KEY_SIZE);
	return ERR_OK;
}

int kee_content_resolve(struct kee_content_t *content, Cadiz *cadiz) {
	int r;
	CMimeMessage_T *msg;
	char *subject;
	size_t subject_len;
	size_t c;

	content->flags = 0;
	c = content->mem_size;
	r = cadiz_resolve(cadiz, content->key, content->body, &c);
	if (!r) {
		content->mem_size -= c;
		content->flags |= KEE_CONTENT_RESOLVED_DATA;
		msg = cmime_message_new();
		r = cmime_message_from_string(&msg, content->body, 0);
		if (!r) {
			subject = cmime_message_get_subject(msg);
			subject = cmime_string_strip(subject);
			subject_len = strlen(subject) + 1;
			if (subject_len <= content->mem_size) {
				content->subject = content->body + c;
				strcpy(content->subject, subject);
				content->flags |= KEE_CONTENT_RESOLVED_SUBJECT;
				content->mem_size -= subject_len;
			}
			content->flags |= KEE_CONTENT_RESOLVED_BODY;
		}
		cmime_message_free(msg);
	}
	content->flags |= KEE_CONTENT_RESOLVED;
	return ERR_OK;		
}

void kee_content_free(struct kee_content_t *content) {
	free(content->mem);
}
