#include <string.h>
#include <stddef.h>
#include <qrencode.h>

#include "qr.h"
#include "err.h"


int qr_encode(char *in, char *out, size_t *out_len) {
	QRcode *qr;

	//qr = QRcode_encodeString8bit((const char*)in, QRSPEC_VERSION_MAX, QR_ECLEVEL_M);
	qr = QRcode_encodeString8bit((const char*)in, QR_VERSION, QR_ECLEVEL_L);
	if (qr == NULL) {
		return 1;
	}
	//qr_render((char*)qr->data, qr->width, out, out_len);
	memcpy(out, (char*)qr->data, qr->width * qr->width);

	*out_len = qr->width;

	QRcode_free(qr);
	
	return 0;	
}
