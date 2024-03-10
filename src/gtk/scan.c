#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

#include "scan.h"
#include "err.h"


void scan_init(struct kee_scanner *scan, const char *device) {
	memset(scan, 0, sizeof(struct kee_scanner));
	scan->device = malloc(strlen(device) + 1);
	strcpy(scan->device, device);
}

void scan_free(struct kee_scanner *scan) {
	free(scan->device);
}

int scan_begin(struct kee_scanner *scan) {
	GstElement *tee;
	GstElement *zbar;
	GstElement *queue_display;
	GstElement *queue_scan;
	GstPad *queue_display_pad;
	GstPad *queue_scan_pad;
	GstPad *tee_display;
	GstPad *tee_scan;
	GstStateChangeReturn rsc;
	GstElement *convert_display;
	GstElement *convert_scan;
	GstElement *filter;
	GstElement *devnull;
	GstCaps *caps;
	GdkPaintable *img;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "starting scan with video device %s", scan->device);

	scan->pipeline = gst_pipeline_new("webcam-zbar");
	scan->source = gst_element_factory_make("v4l2src", "v4l2src");
	filter = gst_element_factory_make("capsfilter", "capsfilter");
	convert_display = gst_element_factory_make("videoconvert", "videoconvert_display");
	convert_scan = gst_element_factory_make("videoconvert", "videoconvert_scan");
	scan->video_sink = gst_element_factory_make("gtk4paintablesink", "gtk4paintablesink");
	tee = gst_element_factory_make("tee", "split-view-zbar");
	zbar = gst_element_factory_make("zbar", "zbar");
	devnull = gst_element_factory_make("fakesink", "fakesink");
	queue_display = gst_element_factory_make("queue", "queue-display");
	queue_scan = gst_element_factory_make("queue", "queue-scan");

	caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "RGB", "width", G_TYPE_INT, 640, "height", G_TYPE_INT, 360,  "framerate", GST_TYPE_FRACTION, 30, 1,  "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, NULL);
	g_object_set(G_OBJECT(filter), "caps", caps, NULL);

	gst_bin_add_many(GST_BIN(scan->pipeline), scan->source, scan->video_sink, convert_display, convert_scan, filter, tee, zbar, queue_display, queue_scan, devnull, NULL);
	if (gst_element_link_many(scan->source, tee, NULL) != TRUE) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail link src to muxer");
		gst_object_unref(scan->pipeline);
		return 1;
	}

	if (gst_element_link_many(queue_display, convert_display, filter, scan->video_sink, NULL) != TRUE) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail link muxer to display");
		gst_object_unref(scan->pipeline);
		return 1;
	}

	if (gst_element_link_many(queue_scan, convert_scan, zbar, devnull, NULL) != TRUE) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail link muxer to scanner");
		gst_object_unref(scan->pipeline);
		return 1;
	}

	tee_display = gst_element_request_pad_simple(tee, "src_%u");
	queue_display_pad = gst_element_get_static_pad(queue_display, "sink");
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "display queue linked with %s", gst_pad_get_name(tee_display));
	tee_scan = gst_element_request_pad_simple(tee, "src_%u");
	queue_scan_pad = gst_element_get_static_pad(queue_scan, "sink");
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "scan queue linked with %s", gst_pad_get_name(tee_scan));

	g_object_set(G_OBJECT(scan->source), "device", scan->device, NULL);
	g_object_get(scan->video_sink, "paintable", &img, NULL);
	if (!img) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "failed getting paintable");
		return 1;
	}
	scan->video_view = GTK_PICTURE(gtk_picture_new_for_paintable(img));
	gtk_picture_set_content_fit(scan->video_view, GTK_CONTENT_FIT_CONTAIN);

	if (gst_pad_link(tee_display, queue_display_pad) != GST_PAD_LINK_OK) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "mux link fail for display");
		gst_object_unref(scan->pipeline);
		return 1;
	}

	if (gst_pad_link(tee_scan, queue_scan_pad) != GST_PAD_LINK_OK) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "mux link fail for scan");
		gst_object_unref(scan->pipeline);
		return 1;
	}

	g_object_unref(queue_display_pad);
	g_object_unref(queue_scan_pad);

	rsc = gst_element_set_state(scan->pipeline, GST_STATE_PLAYING);
	if (rsc == GST_STATE_CHANGE_FAILURE) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail set pipeline to pause");
		gst_object_unref(scan->pipeline);
		return 1;
	}

	return ERR_OK;
}

void scan_set_handler(struct kee_scanner *scan, gboolean(*fn)(GstBus *bus, GstMessage *msg, gpointer user_data)) {
	GstBus *bus;

	bus = gst_element_get_bus(scan->pipeline);
	gst_bus_add_watch(bus, fn, scan);
}
