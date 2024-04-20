#include <stdlib.h>
#include <string.h>

#include "cmime.h"

#include "content.h"
#include "err.h"


struct kee_content_t* kee_content_new(const char *key, size_t size_hint) {
	struct kee_content_t *content = malloc(sizeof(struct kee_content_t));
	if (content == NULL) {
		return NULL;
	}

	if (size_hint) {
		content->mem_size = size_hint;
	} else {
		content->mem_size = KEE_CONTENT_DEFAULT_SIZE;
	}

	content->mem = malloc(content->mem_size);
	if (content->mem == NULL) {
		free(content);
		return NULL;
	}
	content->body = content->mem;
	content->flags = 0;

	memcpy(content->key, key, KEE_CONTENT_KEY_SIZE);
	return content;
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
		content->flags |= KEE_CONTENT_RESOLVED_BODY;
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
		}
		cmime_message_free(msg);
	}
	content->flags |= KEE_CONTENT_RESOLVED;
	return ERR_OK;		
}

void kee_content_free(struct kee_content_t *content) {
	free(content->mem);
	free(content);
}
