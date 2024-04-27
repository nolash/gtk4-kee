#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "kee-transport.h"
#include "kee-menu.h"
#include "err.h"
#include "qr.h"


typedef struct {
} KeeTransportPrivate;

struct _KeeTransportClass {
	GtkWidget parent_class;
};

struct _KeeTransport {
	GtkWidget parent;
	char *image_data;
	size_t image_width;
	size_t image_size;
	GdkPixbuf *pixbuf;
};

G_DEFINE_TYPE(KeeTransport, kee_transport, GTK_TYPE_BOX);

static void kee_transport_finalize(GObject *o) {
	KeeTransport *trans = KEE_TRANSPORT(o);
	free(trans->image_data);
}

static void kee_transport_class_init(KeeTransportClass *kls) {
	GObjectClass *object_class = G_OBJECT_CLASS(kls);
	object_class->finalize = kee_transport_finalize;
}

static void kee_transport_init(KeeTransport *o) {
	o->image_data = malloc(QR_IMAGE_BYTES);
}

/// \todo find a way to modify underlying bytes and keep the stack from pixbuf to widget
static void kee_transport_render(KeeTransport *o) {
	KeeMenu *menu;
	GtkWidget *widget;
	GdkTexture *texture;
	GdkPixbuf *pixbuf;
	GBytes *bytes;
	size_t width_bytes;

	width_bytes = o->image_width * QR_IMAGE_COMPONENTS;

	bytes = g_bytes_new(o->image_data, o->image_size);
	pixbuf = gdk_pixbuf_new_from_bytes(bytes, GDK_COLORSPACE_RGB, false, QR_IMAGE_BIT_DEPTH, o->image_width, o->image_width, width_bytes);
	texture = gdk_texture_new_for_pixbuf(pixbuf);
	widget = gtk_widget_get_first_child(GTK_WIDGET(o));
	if (widget) {
		gtk_box_remove(GTK_BOX(o), widget);
	}
	widget = gtk_picture_new_for_paintable(GDK_PAINTABLE(texture));
	gtk_picture_set_content_fit(GTK_PICTURE(widget), GTK_CONTENT_FIT_SCALE_DOWN);
	gtk_box_append(GTK_BOX(o), widget);

	widget = gtk_widget_get_ancestor(GTK_WIDGET(o), KEE_TYPE_MENU);
	menu = KEE_MENU(widget);
	kee_menu_next(menu, "transport");
}

/// \todo share buffer with image data?
void kee_transport_handle_qr(GAction *Act, GVariant *v, KeeTransport *o) {
	size_t c;
	char *p;
	int i;
	char r;
	char *b;
	char out[QR_CAP * QR_CAP];

	b = (char*)g_variant_get_string(v, NULL);
	if (b == NULL) {
		return;
	}

	c = QR_CAP * QR_CAP;
	r = (char)qr_encode(b, out, &o->image_width);
	if (r) {
		return;
	}

	p = o->image_data;
	for (i = 0; i < (int)(o->image_width * o->image_width); i++) {
		if (*(out+i) & 0x01) {
			r = 0x00;
		} else {
			r = 0xff;
		}
		*p = r;
		p++;
		*p = r;
		p++;
		*p = r;
		p++;
	}
	o->image_size = (o->image_width * QR_IMAGE_COMPONENTS) * (o->image_width * QR_IMAGE_COMPONENTS);
	kee_transport_render(o);
}
