#define _CRT_SECURE_NO_WARNINGS

#include "decode.h"

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_COUNT 32

static void getnum(
    const char *strnote, size_t *startpointer,
    char *outstr) { // 开始时i指向数字的第一位，结束后指向第一位不是数字的字符
  size_t numcount = 0;
  bool reading = true;
  do {
    switch (strnote[*startpointer]) {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '0':
    case '.':
    case '-':
      outstr[numcount++] =
          strnote[(*startpointer)++]; // numcount指向下一个写入位置
      break;
    default:
      reading = false;
    }
  } while (numcount < 15 && reading);
  outstr[numcount] = '\0';
}

static char getnoteheight(char notechr, uint8_t octave, int8_t noteheight) {
  const char heighttable[8] = {0, 10, 12, 1, 3, 5, 6, 8};
  notechr |= 0x20; // to lower case
  assert(notechr >= 'a');
  if (notechr & 0x98) { //-> notechr > 'g'
    assert(notechr == 'p');
    return 0;
  }
  char dheight = heighttable[notechr & 0x7] + 12 * octave + noteheight;
  if (dheight > 84)
    dheight = 84;
  if (dheight < 1)
    dheight = 1;
  return dheight;
}

void decodenote(const char *file, note_t **notes) {
  size_t i = 0; // file position
  size_t notecount = 0;
  uint8_t octave = 3;
  uint8_t length = 4;
  int8_t noteheight = 0;
  float tempo = 120, staccato = 10;
  note_t thisnote = {0, 0, 0};
  size_t chord = SIZE_MAX;

  *notes = (note_t *)malloc(BUFFER_COUNT * sizeof(note_t));

  while (file[i] != '\0') {
    // printf("%c\n", file[i]);

    switch (file[i]) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'p':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'P':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.': {
      char strthislength[16];
      double thislength;

      getnum(file, &i, strthislength);

      if (sscanf(strthislength, "%lf", &thislength) <= 0 || thislength == 0)
        thislength = length;

      thisnote.time =
          (double)240 / tempo / (thislength > 0 ? thislength : -thislength);
      thisnote.height = getnoteheight(file[i], octave, noteheight);

      i++;
      bool reading = true;
      do {
        switch (file[i]) {
        case '#':
          if (thisnote.height > 0 && thisnote.height < 84)
            thisnote.height++;
          i++;
          break;
        case 'b':
          if (thisnote.height > 1)
            thisnote.height--;
          i++;
          break;
        case '.':
          thisnote.time *= 1.5;
          i++;
          break;
        default:
          reading = false;
        }
      } while (reading);

      (*notes)[notecount] = thisnote;
      (*notes)[notecount].staccato = staccato;

      notecount++;

      if (notecount % BUFFER_COUNT == 0) {
        void *pv = realloc(*notes, (notecount + BUFFER_COUNT) * sizeof(note_t));
        if (pv == NULL) {
          fprintf(stderr, "Run out of memory!");
          exit(1);
        }
        *notes = (note_t *)pv;
      }

      break;
    }
    case 'o':
    case 'O': // octave
    {
      i++;
      switch (file[i]) {
      case '#':
        if (octave < 7)
          octave++;
        i++;
        break;
      case 'b':
        if (octave > 0)
          octave--;
        i++;
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
        octave = file[i] - '0';
        i++;
        break;
      default:;
      }
      break;
    }
    case 't':
    case 'T': // tempo
    {
      char strtempo[16];
      float newtempo;
      i++;
      getnum(file, &i, strtempo);
      // tempo = (float)atof(strtempo);
      if (sscanf(strtempo, "%f", &newtempo) > 0) {
        if (newtempo > 1000) {
          tempo = 1000;
        } else if (newtempo < 30) {
          tempo = 30;
        } else {
          tempo = newtempo;
        }
      }
      break;
    }
    case 's':
    case 'S': {
      char strstaccato[16];
      float newstaccato;
      i++;
      getnum(file, &i, strstaccato);

      if (sscanf(strstaccato, "%f", &newstaccato) > 0) {
        if (newstaccato >= 100) {
          staccato = 1;
        } else if (newstaccato <= 0) {
          staccato = 0;
        } else {
          staccato = newstaccato / 100;
        }
      }
      break;
    }
    case 'l':
    case 'L': {
      char strlength[16];
      int newlength;
      i++;
      getnum(file, &i, strlength);

      if (sscanf(strlength, "%i", &newlength) > 0) {
        if (newlength >= 128) {
          length = 128;
        } else if (newlength <= 1) {
          length = 1;
        } else {
          length = (uint8_t)newlength;
        }
      }
      break;
    }
    case 'h':
    case 'H': {
      char strheight[16];
      int newheight;
      i++;
      getnum(file, &i, strheight);
      if (strheight[0] == '\0') {
        switch (file[i]) {
        case '#':
          newheight = noteheight + 1;
          i++;
          break;
        case 'b':
          newheight = noteheight - 1;
          i++;
          break;
        default:
          newheight = noteheight;
        }
      } else {
        if (sscanf(strheight, "%i", &newheight) <= 0)
          break;
      }

      if (newheight >= 12) {
        noteheight = 12;
      } else if (newheight <= -12) {
        noteheight = -12;
      } else {
        noteheight = (uint8_t)newheight;
      }

      break;
    }
    case '[': {
      i++;
      chord = notecount;
      break;
    }
    case ']': {
      i++;
      if (chord == SIZE_MAX)
        break;
      for (size_t n = chord; n < notecount - 1; n++) {
        (*notes)[n].time = CHORD_LENGTH;
      }
      (*notes)[notecount - 1].time -= CHORD_LENGTH * (notecount - chord - 1);
      if ((*notes)[notecount - 1].time < CHORD_LENGTH)
        (*notes)[notecount - 1].time = CHORD_LENGTH;
      chord = SIZE_MAX;
      break;
    }
    case '/': {
      if (file[++i] == '/') {
        do {
          i++;
        } while (file[i] != '\n' && file[i] != '\r');
      }
      // i++;
      break;
    }
    case 'r':
    case 'R': {
      octave = 3;
      length = 4;
      noteheight = 0;
      tempo = 120;
      staccato = 10;
      i++;
      break;
    }
    default:
      i++;
    }
  }
  (*notes)[notecount].time = 0;
}
