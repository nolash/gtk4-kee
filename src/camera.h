#ifndef _KEE_CAMERA_H
#define _KEE_CAMERA_H

struct kee_camera_devices {
	char label[128];
	char path[128];
	struct kee_camera_devices *next;
};

int kee_camera_scan(struct kee_camera_devices *devices);
void kee_camera_free(struct kee_camera_devices *devices);

#endif // _KEE_CAMERA_H
