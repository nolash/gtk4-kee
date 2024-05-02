#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <b64/cencode.h>
#include <b64/cdecode.h>
#include <zlib.h>
#include "transport.h"
#include "err.h"
#include "ledger.h"


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
	if (*(out+*out_len-1) == 0x0a) {
		*(out+*out_len-1) = 0;
		(*out_len)--;	
	}

	free(buf);
	return ERR_OK;
}

int unpack(char *in, size_t in_len, char *out, size_t *out_len) {
	int r;
	char *buf;
	size_t c;

	buf = malloc(*out_len);

	c = *out_len;
	r = unpack_decode(in, in_len, buf, &c);
	if (r) {
		free(buf);	
		return ERR_FAIL;
	}

	r = unpack_decompress(buf, c, out, out_len);
	if (r) {
		free(buf);	
		return ERR_FAIL;
	}

	free(buf);
	return ERR_OK;
}

/// \todo implement checksum
int kee_transport_single(struct kee_transport_t *trans, enum kee_transport_mode_e mode, char cmd, size_t data_len) {
	memset(trans, 0, sizeof(struct kee_transport_t));
	if (cmd >= KEE_N_CMD) {
		return ERR_INVALID_CMD;	
	}

	trans->chunker.data_len = data_len + 1;
	trans->cmd = (char*)trans->chunker.data;
	trans->mode = mode;
	*(trans->cmd) = cmd;
	trans->chunker.crsr = 1;

	return ERR_OK;
}

void kee_transport_set_response(struct kee_transport_t *trans) {
	if (trans->state == 0) {
		*trans->cmd |= KEE_CMD_SIGN_RESPONSE;
	}
}

int kee_transport_write(struct kee_transport_t *trans, const char *in, size_t in_len) {
	if (trans->state) {
		return ERR_FAIL;
	}
	memcpy(trans->chunker.data + trans->chunker.crsr, in, in_len);
	trans->chunker.crsr += in_len;
	return ERR_OK;
}

/// \todo consider pass validation function
/// \todo implement chunking
int kee_transport_next(struct kee_transport_t *trans, char *out, size_t *out_len) {
	int r;
	size_t l;
	size_t remaining;

	if (trans->state) {
		if (trans->state > 1) {
			return ERR_FAIL;
		}
	} else {
		trans->chunker.data_len = trans->chunker.crsr;
		trans->chunker.crsr = 0;
		trans->state = 1;
	}

	remaining = trans->chunker.data_len - trans->chunker.crsr;
	if (remaining < KEE_TRANSPORT_CHUNK_MAX_SIZE - 1) {
		l = remaining;
	} else {
		return 1; // unimplemented
		//l = KEE_TRANSPORT_CHUNK_SIZE;
	}

	/// \todo when chunking, dont forget this as a separate step before start to chunk
	switch (trans->mode) {
		case KEE_TRANSPORT_BASE64:
			r = pack(trans->chunker.data, l, out, out_len);
			if (r) {
				return ERR_FAIL;
			}
			break;
		case KEE_TRANSPORT_RAW:
			memcpy(out, trans->chunker.data, l);
			break;
		default:
			return ERR_FAIL;
	}
	return 0;	
}

int kee_transport_import(struct kee_transport_t *trans, enum kee_transport_mode_e mode, const char *data, size_t data_len) {
	memset(trans, 0, sizeof(struct kee_transport_t));
	memcpy(trans->chunker.data, data, data_len);
	if (*data & KEE_CMD_CHUNKED) {
		return ERR_UNSUPPORTED;
	}
	trans->chunker.crsr = 1;
	trans->chunker.data_len = data_len - 1;
	trans->mode = mode;

	return ERR_OK;
}

int kee_transport_read(struct kee_transport_t *trans, char *out, size_t *out_len) {
	int r;

	switch (trans->mode) {
		case KEE_TRANSPORT_BASE64:
			r = unpack(trans->chunker.data, trans->chunker.data_len, out, out_len);
			if (r) {
				return ERR_FAIL;
			}
			break;
		case KEE_TRANSPORT_RAW:
			memcpy(out, trans->chunker.data, trans->chunker.data_len);
			break;
		default:
			return ERR_FAIL;
	}

	return ERR_OK;
}

//
//// make sure reported sizes add up to data boundary
//static int validate_multi(char *data, size_t data_len) {
//	char *p;
//	size_t c;
//	unsigned short l;
//
//	c = 0;
//	p = data;
//	while (c < data_len - 1) {
//		memcpy(&l, p, sizeof(unsigned short));
//		if (l == 0) {
//			break;
//		}
//		if (is_le()) {
//			flip_endian(2, &l);
//		}
//		c += l + sizeof(unsigned short);
//		p += c;
//	}
//	if (p == data) {
//		return ERR_FAIL;
//	}
//	return c != data_len - 1;
//}
//
///// \todo check state
//int kee_transport_validate(struct kee_transport_t *trans) {
//	int r;
//	char cmd;
//
//	cmd = *trans->cmd & 0x1f;
//
//	if (cmd == 0) {
//		r = validate_multi(trans->chunker.data + 1, trans->chunker.data_len);	
//		if (r) {
//			return ERR_FAIL;
//		}	
//	}
//	return ERR_OK;
//}


//int kee_transport_encode_ledger(struct kee_transport_t *trans_ledger, struct kee_transport_t *trans_item, struct kee_transport_t *trans_out, enum kee_transport_mode_e mode) {
//	int r;
//	char *p;
//	unsigned short part_length;
//	size_t l;
//	size_t c;
//	char *out;
//
//	l = sizeof(unsigned short);
//	out = malloc(trans_ledger->chunker.data_len + trans_item->chunker.data_len + (l * 2) + 1);
//	// only use raw mode for this, since we are joining data
//	if (trans_ledger->mode != KEE_TRANSPORT_RAW) {
//		return ERR_FAIL;
//	}
//	if (trans_item->mode != KEE_TRANSPORT_RAW) {
//		return ERR_FAIL;
//	}
//	c = 0;
//	p = out;
//	part_length = (unsigned short)trans_ledger->chunker.data_len;
//	r = to_endian(TO_ENDIAN_BIG, 2, &part_length);
//	if (r) {
//		return ERR_FAIL;
//	}
//	memcpy(p, &part_length, l);
//	p += sizeof(unsigned short);
//	c += l;
//	memcpy(p, trans_ledger->chunker.data, trans_ledger->chunker.data_len);
//	p += trans_ledger->chunker.data_len;
//	c += trans_ledger->chunker.data_len;
//
//	part_length = (unsigned short)trans_item->chunker.data_len;
//	r = to_endian(TO_ENDIAN_BIG, 2, &part_length);
//	if (r) {
//		return ERR_FAIL;
//	}
//	memcpy(p, &part_length, l);
//	p += l;
//	c += l;
//	memcpy(p, trans_item->chunker.data, trans_item->chunker.data_len);
//	p += trans_item->chunker.data_len;
//	c += trans_item->chunker.data_len;
//
//	r = kee_transport_single(trans_out, mode, KEE_CMD_PACKED, c);
//	if (r) {
//		return ERR_FAIL;
//	}
//	r = kee_transport_write(trans_out, out, c);
//	if (r) {
//		return ERR_FAIL;
//	}
//
//	free(out);
//
//	return ERR_OK;
//}
