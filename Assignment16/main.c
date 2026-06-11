#include "camera.h"
#include "img_processing.h"
#include "fpga.h"

#include "xxsubmod.h"
#include "tiltsubmod.h"

#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <gst/gst.h>

#define LOOP_PERIOD_NS  10000000L 

static volatile int g_running = 1;
static void handle_sigint(int sig) { (void)sig; g_running = 0; }

static void on_frame(const unsigned char *data,
                     int width, int height, void *user)
{
    (void)user;
    img_process_frame(data, width, height);
}

int main(int argc, char **argv)
{
    signal(SIGINT, handle_sigint);

    gst_init(&argc, &argv);

    img_init(CAM_WIDTH, CAM_HEIGHT);

    if (fpga_init() < 0) return -1;

    printf("\nStarting homing procedure\n");
    fpga_home_axis(AXIS_YAW);
    fpga_home_axis(AXIS_PITCH);
    printf("Homing complete\n\n");

    XXDouble   u[3]       = {0.0, 0.0, 0.0};
    XXDouble   y[3]       = {0.0, 0.0, 0.0};
    tiltDouble u_pitch[4] = {0.0, 0.0, 0.0, 0.0};
    tiltDouble y_pitch[2] = {0.0, 0.0};

    XXInitializeSubmodel(u, y, xx_time);
    tiltInitializeSubmodel(u_pitch, y_pitch, tilt_time);

    xx_P[1]   = 1.3;    // yaw  Kp  
    xx_P[2]   = 0.025;  // yaw  tauD 
    tilt_P[1] = 1.3;    // pitch Kp  
    tilt_P[2] = 0.025;  // pitch tauD 

    if (camera_start(on_frame, NULL) < 0) {
        fpga_destroy();
        return -1;
    }
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    while (g_running) {

        int32_t yaw_counts   = fpga_encoder_read(AXIS_YAW);
        int32_t pitch_counts = fpga_encoder_read(AXIS_PITCH);

        double yaw_rad   = fpga_counts_to_rad(yaw_counts,   AXIS_YAW);
        double pitch_rad = fpga_counts_to_rad(pitch_counts, AXIS_PITCH);

        ImgResult vis = img_get_result();

        if (vis.detected) {
            double yaw_setpoint   = yaw_rad   + vis.yaw_error_rad;
            double pitch_setpoint = -(pitch_rad + vis.pitch_error_rad);
            u[0] = yaw_setpoint;
            u[1] = yaw_rad;
            XXCalculateSubmodel(u, y, xx_time);

            u_pitch[0] = y[0];          
            u_pitch[1] = pitch_setpoint;
            u_pitch[2] = pitch_rad;
            tiltCalculateSubmodel(u_pitch, y_pitch, tilt_time);

            fpga_motor_set(AXIS_YAW,   -y[1]);
            fpga_motor_set(AXIS_PITCH, -y_pitch[0]);

            printf("yaw=%+6.3f rad  pitch=%+6.3f rad  "
                   "out_yaw=%+6.3f  out_pitch=%+6.3f\n",
                   yaw_rad, pitch_rad, y[1], y_pitch[0]);

        } else {
            fpga_motor_stop(AXIS_YAW);
            fpga_motor_stop(AXIS_PITCH);
        }
        next.tv_nsec += LOOP_PERIOD_NS;
        if (next.tv_nsec >= 1000000000L) {
            next.tv_nsec -= 1000000000L;
            next.tv_sec  += 1;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
    fpga_motor_stop(AXIS_YAW);
    fpga_motor_stop(AXIS_PITCH);

    camera_stop();

    XXTerminateSubmodel(u, y, xx_time);
    tiltTerminateSubmodel(u_pitch, y_pitch, tilt_time);

    fpga_destroy();
    img_destroy();
    return 0;
}