#ifndef _GTK_KEE_TRANSPORT_H
#define _GTK_KEE_TRANSPORT_H

#include "qr.h"

#define QR_IMAGE_COMPONENTS 3
#define QR_IMAGE_BIT_DEPTH 8
#define QR_IMAGE_WIDTH  QR_CAP * QR_MODULE_SIZE
#define QR_IMAGE_SIZE QR_IMAGE_WIDTH * QR_IMAGE_WIDTH
#define QR_IMAGE_BYTES QR_IMAGE_SIZE * QR_IMAGE_COMPONENTS

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KEE_TYPE_TRANSPORT kee_transport_get_type()
G_DECLARE_FINAL_TYPE(KeeTransport, kee_transport, KEE, TRANSPORT, GtkBox);

void kee_transport_handle_qr(GAction *Act, GVariant *v, KeeTransport *o);

G_END_DECLS

#endif //_GTK_KEE_TRANSPORT_H
