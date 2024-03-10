#include <gtk/gtk.h>
#include <gst/gst.h>

#include "scan.h"
#include "err.h"

//gboolean msg_func(GstBus *bus, GstMessage *msg, gpointer user_data) {
//	GError *err;
//	gchar *debug_info;
//	GstState oldstate;
//	GstState newstate;
//	GstState pendingstate;
//	//struct _gst_data *data;
//	//GstStateChangeReturn rsc;
//
//	//data = (struct _gst_data*)user_data;
//
//	switch (GST_MESSAGE_TYPE (msg)) {
//		case GST_MESSAGE_ERROR:
//			gst_message_parse_error(msg, &err, &debug_info);
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "logg %s: %s", GST_OBJECT_NAME(msg->src), err->message);
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "debug %s", debug_info ? debug_info : "none");
//			g_clear_error(&err);
//			g_free(debug_info);
//			break;
//		case GST_MESSAGE_EOS:
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "eos");
//			break;
//		case GST_MESSAGE_STATE_CHANGED:
//			gst_message_parse_state_changed(msg, &oldstate, &newstate, &pendingstate);
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "state change: %s -> %s", gst_element_state_get_name(oldstate), gst_element_state_get_name(newstate));
//			break;
//		default:
//			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "unknown message (ext %d): %s", GST_MESSAGE_TYPE_IS_EXTENDED(msg), GST_MESSAGE_TYPE_NAME(msg));
//	}
//
//	return false;
//}

//int scan_init(struct kee_scanner *scan) {
//	GdkPaintable *img;
//	GstCaps *caps;
//	GstElement *convert;
//	GstElement *filter;
//	GstStateChangeReturn rsc;
//	gchar *txt;
//
//	memset(scan, 0, sizeof(struct kee_scanner));
//
//	scan->pipeline = gst_pipeline_new("kee-qr-scan");
//	scan->source = gst_element_factory_make("v4l2src", "v4l2src");
//	scan->video_sink = gst_element_factory_make("gtk4paintablesink", "gtk4paintablesink");
//	if (scan->video_sink == NULL) {
//		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "cant create gl sinks");
//		return 1;
//	}
//	//g_object_set(G_OBJECT(scan->source), "device", "/dev/video0", NULL);
//	g_object_set(G_OBJECT(scan->source), "device", "/dev/video4", NULL);
//
//	g_object_get(scan->video_sink, "paintable", &img, NULL);
//	if (!img) {
//		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "failed getting paintable");
//		return 1;
//	}
//
//	g_object_get(scan->source, "device", &txt, NULL);
//	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "using video device: %s", txt);
//
//	filter = gst_element_factory_make("capsfilter", "capsfilter");
//	convert = gst_element_factory_make("videoconvert", "videoconvert");
//	caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "RGB", "width", G_TYPE_INT, 640, "height", G_TYPE_INT, 480,  "framerate", GST_TYPE_FRACTION, 30, 1,  "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, NULL);
//	g_object_set(G_OBJECT(filter), "caps", caps, NULL);
//	scan->video_view = GTK_PICTURE(gtk_picture_new_for_paintable(img));
//	gtk_picture_set_content_fit(scan->video_view, GTK_CONTENT_FIT_CONTAIN);
//	gst_bin_add_many(GST_BIN(scan->pipeline), scan->source, convert, filter, scan->video_sink, NULL);
//
//	if (gst_element_link_many(scan->source, convert, filter, scan->video_sink, NULL) != TRUE) {
//		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "gst pipeline link fail");
//		gst_object_unref(scan->pipeline);
//		return 1;
//	}
//
//	rsc = gst_element_set_state(scan->pipeline, GST_STATE_PLAYING);
//	if (rsc == GST_STATE_CHANGE_FAILURE) {
//		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fail set pipeline to pause");
//		gst_object_unref(scan->pipeline);
//		return 1;
//	}
//
//	return ERR_OK;
//}

int scan_init(struct kee_scanner *scan) {
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

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "camera setup");
	memset(scan, 0, sizeof(struct kee_scanner));

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

	g_object_set(G_OBJECT(scan->source), "device", "/dev/video4", NULL);
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
