#ifndef _CADIR_H
#define _CADIR_H

enum CadirKeyType {
	CADIR_KEYTYPE_ANY,
};

int cadir_get(const char *dirpath, enum CadirKeyType key_type, const char *key, char *out, size_t *out_len);

#endif // _CADIR_H
