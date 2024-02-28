#ifndef _KEE_GPG_H
#define _KEE_GPG_H
#include <string>
#include <stddef.h>

#define GPG_MIN_VERSION "1.10.2"
#define CHACHA20_KEY_LENGTH_BYTES 32
#define CHACHA20_NONCE_LENGTH_BYTES 12

#ifndef ENCRYPT_BLOCKSIZE
#define ENCRYPT_BLOCKSIZE 4096
#endif

/**
 *
 * \brief Encrypt the given string data with the provided encryption key and nonce.
 *
 * \sa encryptb
 * 
 */
int encrypt(char *ciphertext, size_t ciphertext_len, std::string indata, const char *key, const char *nonce);

/**
 *
 * Encrypt the given binary data with the provided encryption key and nonce, using the CHACHA20-POLY1305 algorithm.
 *
 * If successful, \c ciphertext and \c ciphertext_len hold the encrypted data and its length respectively.
 * 
 * \param ciphertext holds resulting ciphertext
 * \param ciphertext_len holds the length of the resulting ciphertext
 * \param indata input data to encrypt
 * \param indata_len length of input data to encrypt
 * \param key 256-bit key to use for encryption
 * \param nonce 256-bit nonce to use as salt for encryption
 * \return 0 on successful encryption.
 *
 */
int encryptb (char *ciphertext, size_t ciphertext_len, const char *indata, size_t indata_len, const char *key, const char *nonce);

/**
 *
 * Decrypt the given string ciphertext with the provided encryption key and nonce.
 *
 * \sa decryptb
 *
 */
int decrypt(std::string *outdata, const char *ciphertext, size_t ciphertext_len, const char *key, const char *nonce);

/**
 *
 * Decrypt the given binary ciphertext with the provided encryption key and nonce, using the CHACHA20-POLY1305 algorithm.
 *
 * If successful, the decrypted (plaintext) data will be stored in \c outdata.
 *
 * \param outdata holdes the resulting plaintext
 * \param ciphertext encrypted data to decrypt
 * \param ciphertext_len length of encrypted data
 * \param key 256-bit key to use for decryption
 * \param nonce 256-bit nonce to use as salt for decryption
 * \return 0 on successful decryption.
 *
 */
int decryptb(char *outdata, const char *ciphertext, size_t ciphertext_len, const char *key, const char *nonce);

/**
 *
 * Calculate number of bytes a given byte count when rounded up to the given block size.
 *
 * For example, if \c blocksize is \c 5 and \c insize is \c 11, then \c 15 will be returned.
 *
 * \param insize size of data
 * \param blocksize block size to round up to
 *
 */
size_t get_padsize(size_t insize, size_t blocksize);

/**
 *
 * \brief Interface to the encrypted key storage for both identity public key and the key used for encryption of the identity public key.
 *
 */
class GpgStore {

	public:
		/// Sets correct context values for underlying \c gcrypt operations.
		GpgStore();
		/**
		 *
		 * Attempts to decrypt the identity public key with the given passphrase.
		 *
		 * If no public key exists, one will be created and encrypted using the passphrase.
		 *
		 * \param p path to key store
		 * \param passphrase passphrase for public key encryption
		 * \return 0 if successful, any other value indicates an error
		 *
		 */
		int check(std::string p, std::string passphrase);
		/**
		 *
		 * Returns the fingerprint of the identity public key.
		 *
		 * \return 160-bit fingerprint value
		 */
		char *get_fingerprint();
	
	private:
		/// calculates sha256 digest for the given string value, using secure memory
		int digest(char *out, std::string in);
		//const char *m_version;
		//char *m_seckey;
		/// cached fingerprint value, in string format with zero terminator
		char m_fingerprint[41];
		/// cached digest length of sha256 
		unsigned int m_passphrase_digest_len;
};
#endif
