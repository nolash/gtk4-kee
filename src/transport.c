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

/// \todo implement checksum
int kee_transport_single(struct kee_transport_t *trans, char cmd, size_t data_len) {

	memset(trans, 0, sizeof(struct kee_transport_header_t));
	if (cmd >= KEE_N_CMD) {
		return ERR_INVALID_CMD;	
	}

	trans->chunk.data_len(in_len + 1);
	trans->chunk.data = malloc(trans->chunk.data_len);
	trans->cmd = trans->chunk.data;

	return ERR_OK;
}

int kee_transport_write(struct kee_transport_t *trans, const char *in, size_t in_len) {
	if (trans.state) {
		return ERR_FAIL;
	}
	memcpy(trans->chunk.data + trans->chunk.crsr, in, in_len);
	trans->chunk.crsr += in_len;
	return ERR_OK;
}

/// \todo consider pass validation function
/// \todo implement chunking
int kee_transport_next(struct kee_transport_t *trans, enum kee_transport_mode_e mode, char *out, size_t *out_len) {
	int r;
	size_t l;

	if (trans.state) {
		if (trans.state > 1) {
			return ERR_FAIL;
		}
	else {
		trans->chunk.data_len = trans->chunk.crsr;
		trans->chunk.crsr = 0;
		trans->state = 1;
	}

	if (trans->remaining < KEE_TRANSPORT_CHUNK_SIZE - 1) {
		l = trans_remaining;
	} else {
		return 1; // unimplemented
		//l = KEE_TRANSPORT_CHUNK_SIZE;
	}

	*(trans->buf) = trans->cmd;
	memcpy(trans->buf + 1, trans->data, l - 1);
	switch (mode) {
		case KEE_TRANSPORT_BASE64:
			r = pack_encode(trans->data, trans->data_len + 1, out, out_len);
			if (r) {
				retrun ERR_FAIL;
			}
			break;
		case KEE_TRANSPORT_RAW:
			memcpy(out, trans->data, trans->data_len + 1);
			break;
		default:
			retrun ERR_FAIL;
	}
	return 0;	
}

void kee_transport_header_free() {
	free(trans->buf);
}
