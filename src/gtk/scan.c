#include <gtk/gtk.h>
#include <gst/gst.h>

#include "scan.h"
#include "err.h"

gboolean msg_func(GstBus *bus, GstMessage *msg, gpointer user_data) {
	return false;
}

int scan_init(struct kee_scanner *scan) {
	GdkPaintable *img;
	GstCaps *caps;
	GstElement *convert;
	GstElement *filter;
	GstStateChangeReturn rsc;
	GstBus *bus;
	gchar *txt;

	memset(scan, 0, sizeof(struct kee_scanner));

	scan->pipeline = gst_pipeline_new("kee-qr-scan");
	scan->source = gst_element_factory_make("v4l2src", "v4l2src");
	scan->video_sink = gst_element_factory_make("gtk4paintablesink", "gtk4paintablesink");
	if (scan->video_sink == NULL) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "cant create gl sinks");
		return 1;
	}
	g_object_set(G_OBJECT(scan->source), "device", "/dev/video0", NULL);

	g_object_get(scan->video_sink, "paintable", &img, NULL);
	if (!img) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "failed getting paintable");
		return 1;
	}

	g_object_get(scan->source, "device", &txt, NULL);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "using video device: %s", txt);

	filter = gst_element_factory_make("capsfilter", "capsfilter");
	convert = gst_element_factory_make("videoconvert", "videoconvert");
	caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "RGB", "width", G_TYPE_INT, 640, "height", G_TYPE_INT, 480,  "framerate", GST_TYPE_FRACTION, 30, 1,  "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, NULL);
	g_object_set(G_OBJECT(filter), "caps", caps, NULL);
	scan->video_view = GTK_PICTURE(gtk_picture_new_for_paintable(img));
	gst_bin_add_many(GST_BIN(scan->pipeline), scan->source, convert, filter, scan->video_sink, NULL);

	if (gst_element_link_many(scan->source, convert, filter, scan->video_sink, NULL) != TRUE) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "gst pipeline link fail");
		gst_object_unref(scan->pipeline);
		return 1;
	}

	rsc = gst_element_set_state(scan->pipeline, GST_STATE_PAUSED);
	if (rsc == GST_STATE_CHANGE_FAILURE) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail set pipeline to pause");
		gst_object_unref(scan->pipeline);
		return 1;
	}

	bus = gst_element_get_bus(scan->pipeline);
	gst_bus_add_watch(bus, msg_func, &scan);

	return ERR_OK;
}
