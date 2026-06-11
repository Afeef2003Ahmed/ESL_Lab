#include "fpga.h"
#include "soc_system.h"

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

static struct {
    int fd;

    volatile uint32_t *pwm[2];   
    volatile uint32_t *enc[2];

    int32_t home[2];             
} hw;

static void motor_write(volatile uint32_t *pwm, double output)
{
    uint32_t direction = 0;
    if (output < 0.0) { direction = 1; output = -output; }

    uint32_t duty = (uint32_t)(output * 100.0 * 0.2);
    if (duty > 99) duty = 99;

    *pwm = (1u << 31) | (direction << 30) | (duty & 0x7F);
}

static void motor_stop(volatile uint32_t *pwm)
{
    *pwm = 0;
}

int fpga_init(void)
{
    hw.fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (hw.fd < 0) {
        perror("[FPGA] open /dev/mem");
        return -1;
    }
    hw.pwm[AXIS_YAW] = (volatile uint32_t *)mmap(NULL,
        HPS_0_ARM_A9_0_PWM_MOD_WRAPPER_1_SPAN,
        PROT_READ | PROT_WRITE, MAP_SHARED,
        hw.fd, HPS_0_ARM_A9_0_PWM_MOD_WRAPPER_1_BASE);

    hw.pwm[AXIS_PITCH] = (volatile uint32_t *)mmap(NULL,
        HPS_0_ARM_A9_0_PWM_MOD_WRAPPER_0_SPAN,
        PROT_READ | PROT_WRITE, MAP_SHARED,
        hw.fd, HPS_0_ARM_A9_0_PWM_MOD_WRAPPER_0_BASE);

    hw.enc[AXIS_YAW] = (volatile uint32_t *)mmap(NULL,
        HPS_0_ARM_A9_0_ENCODER_IP_DEMO_1_SPAN,
        PROT_READ | PROT_WRITE, MAP_SHARED,
        hw.fd, HPS_0_ARM_A9_0_ENCODER_IP_DEMO_1_BASE);

    hw.enc[AXIS_PITCH] = (volatile uint32_t *)mmap(NULL,
        HPS_0_ARM_A9_0_ENCODER_IP_DEMO_0_SPAN,
        PROT_READ | PROT_WRITE, MAP_SHARED,
        hw.fd, HPS_0_ARM_A9_0_ENCODER_IP_DEMO_0_BASE);

    if (hw.pwm[AXIS_YAW]   == MAP_FAILED ||
        hw.pwm[AXIS_PITCH]  == MAP_FAILED ||
        hw.enc[AXIS_YAW]   == MAP_FAILED ||
        hw.enc[AXIS_PITCH]  == MAP_FAILED) {
        perror("[FPGA] mmap");
        return -1;
    }
    printf("All the Peripheral are mapped")

    hw.home[AXIS_YAW]   = 0;
    hw.home[AXIS_PITCH] = 0;
    return 0;
}

void fpga_destroy(void)
{
    motor_stop(hw.pwm[AXIS_YAW]);
    motor_stop(hw.pwm[AXIS_PITCH]);

    munmap((void *)hw.pwm[AXIS_YAW],   HPS_0_ARM_A9_0_PWM_MOD_WRAPPER_1_SPAN);
    munmap((void *)hw.pwm[AXIS_PITCH], HPS_0_ARM_A9_0_PWM_MOD_WRAPPER_0_SPAN);
    munmap((void *)hw.enc[AXIS_YAW],   HPS_0_ARM_A9_0_ENCODER_IP_DEMO_1_SPAN);
    munmap((void *)hw.enc[AXIS_PITCH], HPS_0_ARM_A9_0_ENCODER_IP_DEMO_0_SPAN);

    close(hw.fd);
}

void fpga_home_axis(Axis axis)
{
    const char *name = (axis == AXIS_YAW) ? "YAW" : "PITCH";
    printf("Homing %s\n", name);

    volatile uint32_t *pwm = hw.pwm[axis];
    volatile uint32_t *enc = hw.enc[axis];

    int32_t prev     = (int32_t)enc[0];
    int     still_ms = 0;
    int     elapsed  = 0;

    motor_write(pwm, -HOMING_DUTY);

    while (still_ms < HOMING_STILL_MS && elapsed < HOMING_TIMEOUT_MS) {
        usleep(HOMING_POLL_MS * 1000);
        elapsed += HOMING_POLL_MS;

        int32_t cur = (int32_t)enc[0];
        if (cur == prev) still_ms += HOMING_POLL_MS;
        else             still_ms  = 0;
        prev = cur;
    }

    motor_stop(pwm);

    if (elapsed >= HOMING_TIMEOUT_MS)
        printf("%s homing timed out\n", name);
    else
        printf("%s end-stop found at %d counts\n", name, prev);

    hw.home[axis] = prev;
    usleep(300000);
}

int32_t fpga_encoder_read(Axis axis)
{
    return (int32_t)hw.enc[axis][0] - hw.home[axis];
}

double fpga_counts_to_rad(int32_t counts, Axis axis)
{
    double cpr = (axis == AXIS_YAW) ? CPR_YAW : CPR_PITCH;
    return (2.0 * M_PI * counts) / cpr;
}

void fpga_motor_set(Axis axis, double output)
{
    motor_write(hw.pwm[axis], output);
}

void fpga_motor_stop(Axis axis)
{
    motor_stop(hw.pwm[axis]);
}
