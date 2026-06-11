#ifndef CAMERA_H
#define CAMERA_H

#define CAM_WIDTH    320
#define CAM_HEIGHT   240
#define CAM_FPS      30
#define CAM_DEVICE   "/dev/video7"


typedef void (*FrameCallback)(const unsigned char *data,
                              int width, int height,
                              void *user);

int  camera_start(FrameCallback cb, void *user);


void camera_stop(void);

#endif 
