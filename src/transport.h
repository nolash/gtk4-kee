#ifndef _TRANSPORT_H
#define _TRANSPORT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif // _TRANSPORT_H
