#ifndef IMG_PROCESSING_H
#define IMG_PROCESSING_H

#define IMG_MIN_PIXELS  200

#define IMG_HFOV_DEG    60.0
#define IMG_VFOV_DEG    45.0

#define IMG_Y_MIN    40
#define IMG_Y_MAX   180
#define IMG_U_MIN    60
#define IMG_U_MAX   110
#define IMG_V_MIN    60
#define IMG_V_MAX   110

typedef struct {
    double yaw_error_rad;    // positive = ball to the right 
    double pitch_error_rad;  // positive = ball above centre 
    int    detected;         
    long   pixel_count;
} ImgResult;

void      img_init(int width, int height);
void      img_process_frame(const unsigned char *data, int width, int height);
ImgResult img_get_result(void);
void      img_destroy(void);

#endif 