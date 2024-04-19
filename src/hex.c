#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "hex.h"

// cheekily stolen from https://nachtimwald.com/2017/09/24/hex-encode-and-decode-in-c/
int hexchr2bin(const char hex, char *out) {
	if (out == NULL)
		return 0;

	if (hex >= '0' && hex <= '9') {
		*out = hex - '0';
	} else if (hex >= 'A' && hex <= 'F') {
		*out = hex - 'A' + 10;
	} else if (hex >= 'a' && hex <= 'f') {
		*out = hex - 'a' + 10;
	} else {
		return 0;
	}

	return 1;
}

size_t hex2bin(const char *hex, unsigned char *out) {
	size_t len;
	char   b1;
	char   b2;
	size_t i;

	if (hex == NULL || *hex == '\0' || out == NULL)
		return 0;

	len = strlen(hex);
	if (len % 2 != 0)
		return 0;
	len /= 2;

	memset(out, 'A', len);
	for (i=0; i<len; i++) {
		if (!hexchr2bin(hex[i*2], &b1) || !hexchr2bin(hex[i*2+1], &b2)) {
			return 0;
		}
		//(*out)[i] = (b1 << 4) | b2;
		*(out+i) = (b1 << 4) | b2;
	}
	return len;
}

int bin_to_hex(const unsigned char *data, size_t l, unsigned char *zHex, size_t *z) {
        unsigned int i;

        if (*z < (l*2)+1) {
                return 1;
        }

        for (i = 0; i < l; i++) {
                sprintf((char*)zHex+(i*2), "%02x", *(data+i));
        }
        *z = (i*2);
        *(zHex+(*z)) = 0x0;
        *z = *z + 1;
        return 0;
}
