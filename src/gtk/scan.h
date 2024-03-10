#ifndef _KEE_GTK_SCAN_H
#define _KEE_GTK_SCAN_H

#include <gst/gst.h>
#include <gtk/gtk.h>

struct kee_scanner {
	GtkPicture *video_view;
	GstElement *pipeline;
	GstElement *source;
	GstElement *video_sink;
	GtkImage *snap;
};

int scan_init(struct kee_scanner *scan);
void scan_set_handler(struct kee_scanner *scan, gboolean(*fn)(GstBus *bus, GstMessage *msg, gpointer user_data));

#endif // _KEE_GTK_SCAN_H
