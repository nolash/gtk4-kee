#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "err.h"
#include "debug.h"
#include "camera.h"

//#define KEE_VIDEO_DEVICE_TEMPLATE "/dev/video%d"


int kee_camera_scan(struct kee_camera_devices *devices) {
	int r;
	int fd;
	int devnum;
	struct kee_camera_devices *p;
	struct v4l2_capability video_cap;
	char s[1024];

	p = devices;
	memset(p, 0, sizeof(struct kee_camera_devices));
	devnum = 0;
	while (1) {
		//sprintf(dev.path, KEE_VIDEO_DEVICE_TEMPLATE, devnum);
		sprintf(p->path, "/dev/video%d", devnum);
		fd = open(p->path, O_RDONLY);
		if (fd < 0) {
			p->path[0] = 0;
			break;
		}
		r = ioctl(fd, VIDIOC_QUERYCAP, &video_cap);
		if (r == -1) {
			debug_log(DEBUG_ERROR, "could not get video cap");
			kee_camera_free(devices);
			return -1;
		}
		if ((video_cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) == 0) {
			sprintf(s, "%s is not a capture video device", p->path);
			debug_log(DEBUG_DEBUG, s);
			devnum++;
			continue;
		}
		strcpy(p->label, (char*)video_cap.card);
		p->next = calloc(1, sizeof(struct kee_camera_devices));
		if (p->next == NULL) {
			debug_log(DEBUG_ERROR, "could not allocate video cap struct");
			kee_camera_free(devices);
			return -1;
		}
		sprintf(s, "found camera: %s (%s)", p->label, p->path);
		debug_log(DEBUG_INFO, s);
		p = p->next;
		devnum++;
	}
	return ERR_OK;
}

void kee_camera_free(struct kee_camera_devices *devices) {
	if (devices->next != NULL) {
		kee_camera_free(devices->next);
	}
	free(devices->next);
}
