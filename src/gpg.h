#ifndef _KEE_GPG_H
#define _KEE_GPG_H

//#include <string>
#include <stddef.h>
#include <gcrypt.h>
#include "digest.h"

#define GPG_MIN_VERSION "1.10.2"
#define CHACHA20_KEY_LENGTH_BYTES 32
#define CHACHA20_NONCE_LENGTH_BYTES 12
#define PUBKEY_LENGTH 32
#define FINGERPRINT_LENGTH 20
#define SIGNATURE_LENGTH 64
#define POINT_LENGTH 32

#ifndef ENCRYPT_BLOCKSIZE
#define ENCRYPT_BLOCKSIZE 4096
#endif

#define RERR_PFX_GPG 0x100
/// Crypto backend unavailable
#define ERR_NOCRYPTO 0x101
/// Crypto authentication fail
#define ERR_KEYFAIL 0x102
/// Last attempt to unlock key failed
#define ERR_KEY_UNLOCK 0x103
/// Usage of key for signature has been rejected (by user)
#define ERR_KEY_REJECT 0x104
/// Crypto resource fail
#define ERR_NOKEY 0x105


enum gpg_find_mode_e {
	KEE_GPG_FIND_MAIN,
	KEE_GPG_FIND_FINGERPRINT,	
};

struct gpg_store {
	gcry_sexp_t k;
	size_t passphrase_digest_len;
	char fingerprint[FINGERPRINT_LENGTH];
	char public_key[PUBKEY_LENGTH];
	char path[1024];
	char last_signature[SIGNATURE_LENGTH];
	char last_data[DIGEST_LENGTH];
};

/**
 *
 * \brief Encrypt the given string data with the provided encryption key and nonce.
 *
 * \sa encryptb
 * 
 */
int encrypt(char *ciphertext, size_t ciphertext_len, const char *indata, const char *key, const char *nonce);

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
int decrypt(char *outdata, const char *ciphertext, size_t ciphertext_len, const char *key, const char *nonce);

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


void gpg_store_init(struct gpg_store *gpg, const char *path);
int gpg_store_check(struct gpg_store *gpg, const char *passphrase);
int gpg_store_digest(struct gpg_store *gpg, char *out, const char *in);
char *gpg_store_get_fingerprint(struct gpg_store *gpg);
//int gpg_key_create(gcry_sexp_t *key);
int gpg_key_create(struct gpg_store *gpg, const char *passphrase);
//int gpg_sign(gcry_sexp_t *out, gcry_sexp_t *key, const char *v);
int gpg_key_load(struct gpg_store *gpg, const char *passphrase, enum gpg_find_mode_e mode, const void *criteria);
int gpg_store_sign(struct gpg_store *gpg, char *data, size_t data_len, const char *passphrase);
int gpg_store_sign_with(struct gpg_store *gpg, char *data, size_t data_len, const char *passphrase, const char *fingerprint);
int gpg_store_verify(const char *sig_bytes, const char *digest, const char *pubkey_bytes);

#endif
