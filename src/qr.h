#ifndef KEE_QR_H_
#define KEE_QR_H_

#include <stddef.h>

#ifndef QR_MODULE_SIZE
#define QR_MODULE_SIZE 7
#endif

#ifndef QR_CAP
#define QR_CAP 125 // ~1024 bytes
#endif

#define QR_VERSION 20


int qr_encode(char *in, char *out, size_t *out_len);

#endif // KEE_QR_H_
