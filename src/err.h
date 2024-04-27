#ifndef _KEE_ERR_H
#define _KEE_ERR_H

/**
 * 
 * Error codes within context of the kee application and backend.
 *
 */
enum keeError {
	/// No error so far within current context
	ERR_OK,
	/// General failure code
	ERR_FAIL,
	/// Last attempt to unlock key failed
	ERR_KEY_UNLOCK,
	/// Usage of key for signature has been rejected (by user)
	ERR_KEY_REJECT,
	/// Last input matches entry that already exists in storage
	ERR_INPUT_CORRUPT,
	/// Last input changes entry that already exists in storage, but was not allowed
	ERR_INPUT_DUP,
	/// Last input changes entry that already exists in storage, but was not allowed
	ERR_INPUT_PROTECT,
	/// Crypto backend unavailable
	ERR_NOCRYPTO,
	/// Crypto resource fail
	ERR_NOKEY,
	/// Crypto authentication fail
	ERR_KEYFAIL,
	ERR_ALREADY_SIGNED,
	ERR_INVALID_CMD,
	ERR_QR_MISSING,
	ERR_QR_INVALID,
	ERR_SPACE,
};

#endif // _KEE_ERR_H
