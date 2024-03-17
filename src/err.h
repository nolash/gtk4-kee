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

};
