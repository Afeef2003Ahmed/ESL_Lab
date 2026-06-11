#ifndef FPGA_H
#define FPGA_H
#include <stdint.h>

#define CPR_YAW   21754
#define CPR_PITCH 26634


#define HOMING_DUTY        0.30  
#define HOMING_TIMEOUT_MS  8000
#define HOMING_STILL_MS    400
#define HOMING_POLL_MS     10

typedef enum { AXIS_YAW = 0, AXIS_PITCH = 1 } Axis;

int  fpga_init(void);
void fpga_destroy(void);
void fpga_home_axis(Axis axis);

int32_t fpga_encoder_read(Axis axis);

double  fpga_counts_to_rad(int32_t counts, Axis axis);

void fpga_motor_set(Axis axis, double output);

void fpga_motor_stop(Axis axis);

#endif 
