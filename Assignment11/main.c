#include <fcntl.h>
#include <getopt.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include<time.h>

#define LOOPS 10
#define SPEED 500000
#define BYTES 1



int spiOpen(unsigned spiChan, unsigned spiBaud, unsigned spiFlags) {
  int fd;
  char spiMode;
  char spiBits = 8;
  char dev[32];

  spiMode = spiFlags & 3;
  spiBits = 8;

  sprintf(dev, "/dev/spidev0.%d", spiChan);

  if ((fd = open(dev, O_RDWR)) < 0) {
    return -1;
  }

  if (ioctl(fd, SPI_IOC_WR_MODE, &spiMode) < 0) {
    close(fd);
    return -2;
  }

  if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBits) < 0) {
    close(fd);
    return -3;
  }

  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spiBaud) < 0) {
    close(fd);
    return -4;
  }

  return fd;
}

int spiClose(int fd) { return close(fd); }

int spiXfer(int fd, unsigned speed, char *txBuf, char *rxBuf, unsigned count) {
  int err;
  struct spi_ioc_transfer spi;

  memset(&spi, 0, sizeof(spi));

  spi.tx_buf = (unsigned long)txBuf;
  spi.rx_buf = (unsigned long)rxBuf;
  spi.len = count;
  spi.speed_hz = speed;
  spi.delay_usecs = 0;
  spi.bits_per_word = 8;
  spi.cs_change = 0;

  err = ioctl(fd, SPI_IOC_MESSAGE(1), &spi);

  return err;
}

#define MAX_SPI_BUFSIZ 8192

char RXBuf[MAX_SPI_BUFSIZ];
char TXBuf[MAX_SPI_BUFSIZ];

int bytes = BYTES;
int speed = SPEED;
int loops = LOOPS;

int main(int argc, char *argv[]) {
  int i, fd, errors;
  uint8_t prev_tx, expected;

  if (argc > 1)
    loops = atoi(argv[1]);

  if (argc > 2)
    speed = atoi(argv[2]);
  if ((speed < 32000) || (speed > 250000000))
    speed = SPEED;

  fd = spiOpen(1, speed, 0);
  if (fd < 0)
    return 1;

  printf("SPI speed: %d Hz  loops: %d  bytes: %d\n\n", speed, loops, bytes);
  long min_ns  =  999999999L;
  long max_ns  =  0L;
  long sum_ns  =  0L;

  
  errors = 0;
  for (i = 0; i < loops; i++) {
    TXBuf[0] = i * 17 + 1;
    expected = ~prev_tx;

    struct timespec t1,t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    spiXfer(fd, speed, TXBuf, RXBuf, bytes);

    clock_gettime(CLOCK_MONOTONIC, &t2);

    long ns = (t2.tv_sec - t1.tv_sec) * 1000000000L + (t2.tv_nsec - t1.tv_nsec);

    if (ns < min_ns) min_ns = ns;
    if (ns > max_ns) max_ns = ns;
    sum_ns += ns;

    if ((uint8_t)RXBuf[0] == expected)
      printf("loop %d: sent %d, got %d - correct\n", i, (uint8_t)TXBuf[0], (uint8_t)RXBuf[0]);
    else {
      printf("loop %d: sent %d, got %d, expected %d - wrong\n", i, (uint8_t)TXBuf[0], (uint8_t)RXBuf[0], expected);
      errors++;
    }

    prev_tx = TXBuf[0];
    usleep(10000);
  }

  printf("\n========== Timing Summary ==========\n");
    printf("Speed    : %d Hz\n",          speed);
    printf("Loops    : %d\n",             loops);
    printf("Errors   : %d\n",             errors);
    printf("Min      : %ld ns  (%.2f us)\n", min_ns, min_ns / 1000.0);
    printf("Max      : %ld ns  (%.2f us)\n", max_ns, max_ns / 1000.0);
    printf("Average  : %.0f ns  (%.2f us)\n",
           (double)sum_ns / loops, (double)sum_ns / loops / 1000.0);
  printf("=====================================\n");

  spiClose(fd);

  return 0;
}