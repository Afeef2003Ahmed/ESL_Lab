#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#include "soc_system.h"

int main(int argc, char** argv) {
    int fd;
    uint32_t* encoder_map;
    uint32_t counter;
    uint32_t direction;
    
    // Open /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Couldn't open /dev/mem\n");
        return -1;
    }
    
    // Map the encoder registers (same base as esl_bus_demo, different offset? check soc_system.h)
    encoder_map = (uint32_t*)mmap(NULL, HPS_0_ARM_A9_0_ESL_BUS_DEMO_0_SPAN, 
                                   PROT_READ | PROT_WRITE, MAP_SHARED, 
                                   fd, HPS_0_ARM_A9_0_ESL_BUS_DEMO_0_BASE);
    if (encoder_map == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return -1;
    }
    
    // Read and print encoder values
    while (1) {
        counter = encoder_map[0];      // Read counter at offset 0
        direction = encoder_map[1];    // Read direction at offset 1
        
        printf("Counter: %u, Direction: %s\n", counter, direction ? "CW" : "CCW");
        usleep(100000);  // Update every 100ms
    }
    
    close(fd);
    return 0;
}