#define _CRT_SECURE_NO_WARNINGS

#include <malloc.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <linux/kd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "decode.h"
#include "play.h"

bool playing = false, stopped = false;

void CtrlHandler(int signal) {

  if (signal != 18) {
    if (console_fd >= 0) {
      ioctl(console_fd, KIOCSOUND, 0);
      close(console_fd);
    }
    playing = false;
    fprintf(stderr, "\nStopped!\n");
    exit(1);
  }
  // TODO when resume, hide cursor
}

void printHelp(void) {
  fprintf(stderr,
          "BeepPlayer - A program to play music through PC speaker\n"
          "Modified for Afterglow OS\n\n"
          "Usage: beepplayer [options] file\n"
          "-h\t\tshow help\n"
          "\n"
          "Project website: <https://github.com/AlexGuo1998/BeepPlayer>\n");
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Please open a file!\ntype: \"beepplayer -h\" for help\n");
    return 1;
  }
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      printHelp();
      return 0;
    }
  }

  char *notestr;

  {
    FILE *pf = fopen(argv[argc - 1], "rb");
    if (pf == NULL) {
      fprintf(stderr, "Can't open file \"%s\"!\n", argv[argc - 1]);
      return 1;
    }

    puts("file open OK");

    fseek(pf, 0, SEEK_END);
    int filelen;
    filelen = ftell(pf);
    notestr = (char *)malloc(filelen + 1);
    puts("memory allocate OK");
    if (notestr == NULL) {
      fprintf(stderr, "Run out of memory!\n");
      return 1;
    }
    rewind(pf);
    size_t ret = fread(notestr, 1, filelen, pf);
    notestr[ret] = '\0';
    fclose(pf);
  }

  note_t *notelist;

  decodenote(notestr, &notelist);
  puts("decode OK");
  free(notestr);

  console_fd = open("/dev/console", O_WRONLY);
  if (console_fd == -1) {
    perror("open /dev/console");
    return 1;
  }
  printf("open /dev/console OK, console_fd = %d\n", console_fd);

  int signalsToCatch[] = {2, 3, 18};

  for (size_t i = 0; i < 3; i++) {
    signal(signalsToCatch[i], CtrlHandler);
  }

  playing = true;
  stopped = false;

  size_t i = 0;
  puts("start playing");

  while (notelist[i].time != 0 && playing) {
    printf("playing note #%u: height %d\n", i + 1, notelist[i].height);
    if (notelist[i].height > 0) {
      callBeep(440 * pow(2, ((float)(notelist[i].height - 34) / 12)),
               notelist[i].time * 10 * (100 - notelist[i].staccato));
      callSleep(notelist[i].time * 10 * notelist[i].staccato); // staccato
    } else {
      callSleep(notelist[i].time * 1000);
      // height=0 -> pause
    }
    i++;
  }

  playing = false;
  stopped = true;

  // free mem
  free(notelist);

  close(console_fd);

  return 0;
}
