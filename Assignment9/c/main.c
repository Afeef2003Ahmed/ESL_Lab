#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "soc_system.h"

int main(int argc, char **argv) {
    uint32_t enable     = 1;
    uint32_t duty_cycle = 75;   // 0-100 %
    uint32_t direction  = 0;    // 0 = forward, 1 = reverse

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Couldn't open /dev/mem");
        return -1;
    }

    volatile uint32_t *pwm_map =
        (volatile uint32_t *)mmap(NULL, HPS_0_ARM_A9_0_ESL_BUS_DEMO_0_SPAN,
                                  PROT_READ | PROT_WRITE, MAP_SHARED,
                                  fd, HPS_0_ARM_A9_0_ESL_BUS_DEMO_0_BASE);
    if (pwm_map == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return -1;
    }

    
    *pwm_map = (enable    << 31)
             | (direction << 30)
             | (duty_cycle & 0x7F);

    munmap((void *)pwm_map, HPS_0_ARM_A9_0_ESL_BUS_DEMO_0_SPAN);
    close(fd);
    return 0;
}