#include <stddef.h>
#include <stdio.h>

#include "hex.h"


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
