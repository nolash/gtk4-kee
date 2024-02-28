#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <varint.h>

#include "export.h"
#include "common.h"
#include "err.h"


int export_init(struct kee_export *ex, size_t count) {
	size_t unit;

	ex->count = count;

	// 10 is max varint
	unit = sizeof(ex->lenlens) + sizeof(ex->data) + sizeof(ex->datalens) + 10; 
	ex->lens = malloc(unit * count);
	if (ex->lens == NULL) {
		return 1;
	}
	unit += sizeof(ex->lens) * count;
	ex->lenlens = (size_t*)ex->lens + unit;
	unit += sizeof(ex->lenlens) * count;
	ex->data = (char**)ex->lens + unit;
	unit += sizeof(ex->data) * count;
	ex->datalens = (size_t*)ex->lens + unit;
	return ERR_OK;
}

int export_add_item(struct kee_export *ex, char *in, size_t in_len) {
	int r;
	char b[10];

	if (in_len == 0) {
		return ERR_FAIL;
	}

	r = 0;
	r += varint_write_u(b, in_len);
	memcpy(ex->lens+(ex->count*10), b, 10);
	*(ex->lenlens + sizeof(ex->lenlens) * ex->count) = (size_t)r;
	*(ex->datalens + sizeof(ex->datalens) * ex->count) = in_len;
	*(ex->data + sizeof(ex->data) * ex->count) = in;

	ex->size += r;
	ex->count++;
	return r;

}

int export_write(struct kee_export *ex, char *out, size_t out_len) {
	char *p;
	int i;
	size_t l;
	size_t c;

	if (out_len < ex->size) {
		return -1;
	}

	p = out;
	c = 0;
	for (i = 0; i < ex->count; i++) {
		l = ex->lenlens[i];
		memcpy(p, ex->lens+(10*i), l);
		p += l;
		c += l;
		l = ex->datalens[i];
		memcpy(p, ex->data[i], l);
		p += l;
		c += l;
	}
	return c;
}


void export_free(struct kee_export *ex) {
	free(ex->lens);
}

// data not copied, must be in scope for duration
int import_init(struct kee_import *im, const char *in, size_t in_len) {
	im->data = (char*)in;
	im->size = in_len;
	im->crsr = 0;
	im->eof = 0;
	return ERR_OK;
}

kee_boolean done(struct kee_import *im) {
	return im->eof;
}

int import_read(struct kee_import *im, char *out, size_t out_len) {
	int r;
	uint64_t l;

	if (im->size - im->crsr <= 0) {
		im->eof = 1;
		return 0;
	}

	r = varint_read_u(im->data + im->crsr, out_len, &l);
	im->crsr += r;

	memcpy(out, im->data + im->crsr, (size_t)l);
	im->crsr += l;

	return l;
}

void import_free(struct kee_import *im) {
}
