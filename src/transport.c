#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <b64/cencode.h>
#include <b64/cdecode.h>
#include <zlib.h>
#include "transport.h"
#include "err.h"


static int pack_compress(char *in, size_t in_len, char *out, size_t *out_len) {
	int r;
	z_stream z;

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	z.data_type = Z_TEXT;
	z.next_in = (unsigned char*)in;
	z.avail_in = in_len;
	z.next_out = (unsigned char*)out;
	z.avail_out = *out_len;

	r = deflateInit(&z, Z_BEST_COMPRESSION);
	if (r != Z_OK) {
		return 1;
	}
	r = deflate(&z, Z_FINISH);
	if (r != Z_STREAM_END) {
		return 2;
	}
	*out_len = z.total_out;

	return 0;
}

static int pack_encode(char *in, size_t in_len, char *out, size_t *out_len) {
	char *p;
	int r;
	base64_encodestate bst;

	base64_init_encodestate(&bst);

	p = out;
	r = base64_encode_block(in, in_len, p, &bst);
	if (r == 0) {
		return 1;
	}
	*out_len = r;
	p += r;
	r = base64_encode_blockend(p, &bst);
	if (r == 0) {
		return 1;
	}
	*out_len += r;

	return 0;

}

static int unpack_decompress(char *in, size_t in_len, char *out, size_t *out_len) {
	int r;
	z_stream z;

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	z.data_type = Z_TEXT;
	z.next_in = (unsigned char*)in;
	z.avail_in = in_len;
	z.next_out = (unsigned char*)out;
	z.avail_out = *out_len;

	r = inflateInit(&z);
	if (r != Z_OK) {
		return 1;
	}
	r = inflate(&z, Z_FINISH);
	if (r != Z_STREAM_END) {
		return 2;
	}
	*out_len = z.total_out;

	return 0;
}

static int unpack_decode(char *in, size_t in_len, char *out, size_t *out_len) {
	int r;
	base64_decodestate dst;

	base64_init_decodestate(&dst);
	r = base64_decode_block(in, in_len, out, &dst);
	if (r == 0) {
		return 1;	
	}
	*out_len = r;

	return 0;
}

int pack(char *in, size_t in_len, char *out, size_t *out_len) {
	int r;
	char *buf;

	buf = malloc(*out_len);

	r = pack_compress(in, in_len, buf, out_len);
	if (r) {
		free(buf);	
		return ERR_FAIL;
	}
	
	r = pack_encode(buf, *out_len, out, out_len);
	if (r) {
		free(buf);	
		return ERR_FAIL;
	}

	free(buf);
	return ERR_OK;
}

int unpack(char *in, size_t in_len, char *out, size_t *out_len) {
	int r;
	char *buf;

	buf = malloc(*out_len);

	r = unpack_decode(in, in_len, buf, out_len);
	if (r) {
		free(buf);	
		return ERR_FAIL;
	}

	r = unpack_decompress(buf, *out_len, out, out_len);
	if (r) {
		free(buf);	
		return ERR_FAIL;
	}

	free(buf);
	return ERR_OK;
}
