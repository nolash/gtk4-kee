#ifndef _KEE_DIGEST_H
#define _KEE_DIGEST_H

enum DigestErr {
	ERR_DIGESTFAIL = 1,
};

#ifdef __cplusplus
extern "C" {
#endif
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

#ifdef __cplusplus
}
#endif

#endif // _KEE_DIGEST_H
