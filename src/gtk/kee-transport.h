#ifndef _GTK_KEE_TRANSPORT_H
#define _GTK_KEE_TRANSPORT_H

#include "qr.h"

#ifndef QR_IMAGE_MODULE_SIZE
#define QR_IMAGE_MODULE_SIZE 7
#endif

#define QR_IMAGE_COMPONENTS 3
#define QR_IMAGE_BIT_DEPTH 8
#define QR_IMAGE_WIDTH QR_CAP * QR_IMAGE_MODULE_SIZE
#define QR_IMAGE_SIZE (QR_IMAGE_WIDTH * QR_IMAGE_COMPONENTS) * (QR_IMAGE_WIDTH * QR_IMAGE_COMPONENTS)
#define QR_IMAGE_PIXELS QR_IMAGE_SIZE

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KEE_TYPE_TRANSPORT kee_transport_get_type()
G_DECLARE_FINAL_TYPE(KeeTransport, kee_transport, KEE, TRANSPORT, GtkBox);

void kee_transport_handle_qr(GAction *Act, GVariant *v, KeeTransport *o);

G_END_DECLS

#endif //_GTK_KEE_TRANSPORT_H
