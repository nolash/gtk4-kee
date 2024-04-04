#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "cadiz.h"
#include "hex.h"


/// \todo replace with fadfada
//int cadiz_resolve(const char *locator, enum CadizKeyType key_type, const char *key, char *out, size_t *out_len) {
int cadiz_resolve(Cadiz *cadiz, const char *key, char *out, size_t *out_len) {
	int r;
	int c;
	int fd;
	char *p;
	char path[1024];
	size_t l;
	const char *locator = cadiz->locator;

	strcpy(path, locator);
	c = strlen(locator) - 1;
	p = path + c;
	if (*p != '/') {
		*(p+1) = '/';
		*(p+2) = 0;
		c++;
	}

	p = path + c + 1;
	l = 129; 
	r = bin_to_hex((unsigned char*)key, 64, (unsigned char*)p, &l);
	if (r) {
		return 1;
	}
	p += l;
	*p = 0;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		return 1;
	}
	l = read(fd, out, *out_len);
	close(fd);
	*out_len = l;
		
	return 0;
}
