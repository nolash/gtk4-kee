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
	GstBus *bus;
	char *device;
};

void scan_init(struct kee_scanner *scan, const char *device);
int scan_begin(struct kee_scanner *scan);
void scan_free(struct kee_scanner *scan);
void scan_set_handler(struct kee_scanner *scan, gboolean(*fn)(GstBus *bus, GstMessage *msg, gpointer user_data));
//void scan_act(GSimpleAction *act, GVariant *param, KeeUicontext *ui);

#endif // _KEE_GTK_SCAN_H
