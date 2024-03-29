#ifndef _CADIZ_H
#define _CADIZ_H


enum CadizKeyType {
	CADIZ_KEY_TYPE_ANY,
};


typedef struct Cadiz {
	char *locator;
	enum CadizKeyType key_type;
} Cadiz;

typedef int (*cadiz_resolve_fn)(Cadiz *cadiz, const char *key, char *out, size_t *out_len);
int cadiz_resolve(Cadiz *cadiz, const char *key, char *out, size_t *out_len);

#endif // _CADIZ_H
