#ifndef _EXPORT_H
#define _EXPORT_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

struct kee_export {
	size_t cap;
	int count;
	size_t size;
	char *lens;
	size_t *lenlens;
	char **data;
	size_t *datalens;
};

struct kee_import {
	char *data;
	size_t size;
	unsigned int crsr;
	kee_boolean eof;
};

int export_init(struct kee_export *ex, size_t count);
void export_free(struct kee_export *ex);
int export_add_item(struct kee_export *ex, char *in, size_t in_len);
int export_write(struct kee_export *ex, char *out, size_t out_len);

int import_init(struct kee_import *im, const char *in, size_t in_len);
void import_free(struct kee_import *im);
int import_read(struct kee_import *im, char *out, size_t out_len);
kee_boolean done(struct kee_import *im);

#endif // _EXPORT_H
