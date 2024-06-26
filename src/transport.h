#ifndef _KEE_TRANSPORT_H
#define _KEE_TRANSPORT_H

#include <stddef.h>

#include "chunk.h"

#define ERRR_PFX_TRANSPORT 0x300
#define ERR_INVALID_CMD 0x301

enum kee_transport_mode_e {
	KEE_TRANSPORT_RAW,
	KEE_TRANSPORT_BASE64,
};

enum kee_cmd_e { // max number 31
	KEE_CMD_IMPORT = 0,
	KEE_CMD_ID,
	KEE_CMD_LEDGER,
	KEE_CMD_DELTA,
	KEE_CMD_CLEAR,
	KEE_CMD_PACKED,
	KEE_N_CMD,
	KEE_INVALID_CMD,
};

#define KEE_CMD_SIGN_RESPONSE 64
#define KEE_CMD_CHUNKED 128

struct kee_transport_t {
	enum kee_transport_mode_e mode;
	char *cmd;
	struct kee_chunk_t chunker;
	short checksum;
	char state;
	char *data;
};

/**
 * 
 * \brief Compress and encode serialized data for transport in text format.
 *
 * \param in Input data
 * \param in_len Length of input data
 * \param out If successful, contains encoded result string.
 * \param out_len Must contain available bytes in \c out. Will contain length of data written to \c out if successful.
 *
 * \return ERR_OK if successful, ERR_FAIL if not.
 */
int pack(char *in, size_t in_len, char *out, size_t *out_len);

/**
 *
 * \brief Decode and decompress data encoded in text format for transport.
 *
 * \param in Input data
 * \param in_len Length of input data
 * \param out If successful, contains encoded result string.
 * \param out_len Must contain available bytes in \c out. Will contain length of data written to \c out if successful.
 *
 * \return ERR_OK if successful, ERR_FAIL if not.
 */
int unpack(char *in, size_t in_len, char *out, size_t *out_len);

int kee_transport_single(struct kee_transport_t *trans, enum kee_transport_mode_e mode, char cmd, size_t data_len);
int kee_transport_write(struct kee_transport_t *trans, const char *in, size_t in_len);
int kee_transport_next(struct kee_transport_t *trans, char *out, size_t *out_len);
void kee_transport_set_response(struct kee_transport_t *trans);
//int kee_transport_import(struct kee_transport_t *trans, enum kee_transport_mode_e mode, size_t decoded_len);
int kee_transport_read(struct kee_transport_t *trans, char *out, size_t *out_len);

//int kee_transport_encode_ledger(struct kee_transport_t *trans_ledger, struct kee_transport_t *trans_item, struct kee_transport_t *trans_out, enum kee_transport_mode_e mode);
//int kee_transport_validate(struct kee_transport_t *trans);
#endif // _KEE_TRANSPORT_H
