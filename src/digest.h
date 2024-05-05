#ifndef KEE_DIGEST_H_
#define KEE_DIGEST_H_

#include <gcrypt.h>

enum DigestErr {
	ERR_DIGESTFAIL = 1,
};

#define DIGEST_LENGTH 64

/**
 *
 * \brief Calculate SHA256 digest over the given input data.
 *
 * \param in Input data.
 * \param in_len Length of input data.
 * \param out Contains digest data on success. Must have at least 32 bytes available.
 * \return 0 on success, any other value indicates failure.
 *
 */
int calculate_digest(const char *in, size_t in_len, char *out);
int calculate_digest_algo(const char *in, size_t in_len, char *out, enum gcry_md_algos algo);

#endif // _KEE_DIGEST_H
