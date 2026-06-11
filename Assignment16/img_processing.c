#include "img_processing.h"
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static ImgResult        g_result;
static pthread_mutex_t  g_lock;
static double           g_rad_per_px_x;
static double           g_rad_per_px_y;

void img_init(int width, int height)
{
    memset(&g_result, 0, sizeof(g_result));
    pthread_mutex_init(&g_lock, NULL);

    g_rad_per_px_x = (IMG_HFOV_DEG * M_PI / 180.0) / width;
    g_rad_per_px_y = (IMG_VFOV_DEG * M_PI / 180.0) / height;
}

void img_process_frame(const unsigned char *data, int width, int height)
{
    const unsigned char *Y = data;
    const unsigned char *U = data + width * height;
    const unsigned char *V = data + width * height * 5 / 4;

    long sum_x = 0, sum_y = 0, count = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            unsigned char y_val = Y[y * width + x];
            unsigned char u_val = U[(y/2) * (width/2) + (x/2)];
            unsigned char v_val = V[(y/2) * (width/2) + (x/2)];

            if (y_val >= IMG_Y_MIN && y_val <= IMG_Y_MAX &&
                u_val >= IMG_U_MIN && u_val <= IMG_U_MAX &&
                v_val >= IMG_V_MIN && v_val <= IMG_V_MAX) {
                sum_x += x;
                sum_y += y;
                count++;
            }
        }
    }

    ImgResult result;
    result.pixel_count = count;

    if (count >= IMG_MIN_PIXELS) {
        double cx = (double)sum_x / count;
        double cy = (double)sum_y / count;

        result.yaw_error_rad   =  (cx - width  / 2.0) * g_rad_per_px_x;
        result.pitch_error_rad = -(cy - height / 2.0) * g_rad_per_px_y;
        result.detected = 1;

        printf("ball at (%.0f, %.0f)  yaw=%.3f  pitch=%.3f  px=%ld\n",
               cx, cy, result.yaw_error_rad, result.pitch_error_rad, count);
    } else {
        result.yaw_error_rad   = 0.0;
        result.pitch_error_rad = 0.0;
        result.detected = 0;
        printf("no ball (px=%ld)\n", count);
    }

    pthread_mutex_lock(&g_lock);
    g_result = result;
    pthread_mutex_unlock(&g_lock);
}

ImgResult img_get_result(void)
{
    ImgResult r;
    pthread_mutex_lock(&g_lock);
    r = g_result;
    pthread_mutex_unlock(&g_lock);
    return r;
}

void img_destroy(void)
{
    pthread_mutex_destroy(&g_lock);
}