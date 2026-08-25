#include "play.h"

#include <stddef.h>
#include <stdint.h>

#include <fcntl.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <unistd.h>

int console_fd = -1;

void callBeep(double freq, double time) {
  int freqnum = (int)(1193180 / freq);
  ioctl(console_fd, KIOCSOUND, freqnum);
  usleep(1000 * time);
  ioctl(console_fd, KIOCSOUND, 0);
}

void callSleep(double time) { usleep(1000 * time); }
