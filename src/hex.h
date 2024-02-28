#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 
 * \brief Convert data to hex string
 *
 * \param data Input data.
 * \param l Input data length.
 * \param zHex If successful, contains hex string output.
 * \param z Pointer to available length of \c zHex. Will contain the string length after succesful conversion.
 * \todo \c z output is superfluous as zHex will (should) be zero-terminated.
 * \return 1 if Output buffer is insufficient to write the result string, otherwise 0 (success).
 */
int bin_to_hex(const unsigned char *data, size_t l, unsigned char *zHex, size_t *z);

#ifdef __cplusplus
}
#endif
