#include "img_processing.h"

#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define HSV_H_MIN   35
#define HSV_H_MAX   85
#define HSV_S_MIN   50
#define HSV_S_MAX  255
#define HSV_V_MIN   50
#define HSV_V_MAX  255

static ImgResult       g_result;
static pthread_mutex_t g_lock;
static double          g_rad_per_px_x;
static double          g_rad_per_px_y;

extern "C" void img_init(int width, int height)
{
    memset(&g_result, 0, sizeof(g_result));
    pthread_mutex_init(&g_lock, NULL);

    g_rad_per_px_x = (IMG_HFOV_DEG * M_PI / 180.0) / width;
    g_rad_per_px_y = (IMG_VFOV_DEG * M_PI / 180.0) / height;

    printf("[ImgCV] Initialised %dx%d  HSV threshold H:%d-%d S:%d-%d V:%d-%d\n",
           width, height,
           HSV_H_MIN, HSV_H_MAX,
           HSV_S_MIN, HSV_S_MAX,
           HSV_V_MIN, HSV_V_MAX);
}

extern "C" void img_process_frame(const unsigned char *data, int width, int height)
{

    cv::Mat i420(height + height / 2, width, CV_8UC1, (void *)data);

  
    cv::Mat bgr;
    cv::cvtColor(i420, bgr, cv::COLOR_YUV2BGR_I420);


    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    /* Threshold to isolate green ball */
    cv::Mat mask;
    cv::inRange(hsv,
        cv::Scalar(HSV_H_MIN, HSV_S_MIN, HSV_V_MIN),
        cv::Scalar(HSV_H_MAX, HSV_S_MAX, HSV_V_MAX),
        mask);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                               cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN,  kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    cv::Moments m  = cv::moments(mask, true);
    long pixel_count = (long)cv::countNonZero(mask);

    ImgResult result;
    result.pixel_count = pixel_count;

    if (pixel_count >= IMG_MIN_PIXELS && m.m00 > 0) {
        double cx = m.m10 / m.m00;
        double cy = m.m01 / m.m00;

        result.yaw_error_rad   =  (cx - width  / 2.0) * g_rad_per_px_x;
        result.pitch_error_rad = -(cy - height / 2.0) * g_rad_per_px_y;
        result.detected = 1;

        printf("ball at (%.0f, %.0f)  yaw=%.3f  pitch=%.3f  px=%ld\n",
               cx, cy, result.yaw_error_rad, result.pitch_error_rad,
               pixel_count);
    } else {
        result.yaw_error_rad   = 0.0;
        result.pitch_error_rad = 0.0;
        result.detected = 0;
        printf("no ball (px=%ld)\n", pixel_count);
    }

    pthread_mutex_lock(&g_lock);
    g_result = result;
    pthread_mutex_unlock(&g_lock);
}

extern "C" ImgResult img_get_result(void)
{
    ImgResult r;
    pthread_mutex_lock(&g_lock);
    r = g_result;
    pthread_mutex_unlock(&g_lock);
    return r;
}

extern "C" void img_destroy(void)
{
    pthread_mutex_destroy(&g_lock);
}
