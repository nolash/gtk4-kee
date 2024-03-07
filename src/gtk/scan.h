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

#endif // _KEE_GTK_SCAN_H
