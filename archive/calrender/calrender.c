/*
   Copyright (C) 1995  Garrick Brian Meeker

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include "calchart.h"

struct Coord* loadanim(FILE*, int, int*);

char buf[256];

#define SCALE 16.0

void usage(char **argv) {
  printf("Usage: %s nummen numsheets inbase outbase [e]\n", argv[0]);
  exit(1);
}

int main(int argc, char** argv) {
  int nummen, numsheets, numbeats;
  int sheet, man, beat, totalbeats;
  int i;
  int east;
  FILE* fp;
  struct Coord* pts;
  float x,y;

  if ((argc < 5) || (argc > 6)) {
    usage(argv);
  }
  if (argc == 6) {
    east = 1;
  } else {
    east = 0;
  }
  nummen = atoi(argv[1]);
  numsheets = atoi(argv[2]);
  totalbeats = 0;
  for (sheet = 0; sheet < numsheets; sheet++) {
    sprintf(buf, "%s.a%d", argv[3], sheet+1);
    fp = fopen(buf, "rb");
    if (fp == NULL) {
      printf("Error reading %s\n", buf);
      exit(1);
    }
    pts = loadanim(fp, nummen, &numbeats);
    if (pts == NULL) {
      printf("Error reading points\n");
      exit(1);
    }
    fclose(fp);
    i=0;
    for (beat = 0; beat < numbeats; beat++) {
      sprintf(buf, "%s%05d.pov", argv[4], totalbeats+1);
      fp = fopen(buf, "w");
      if (fp == NULL) {
	printf("Error writing %s\n", buf);
	exit(1);
      }
      for (man = 0; man < nummen; man++) {
	x = (((float)pts[i].x) / 2.0 - 88.0) * SCALE;
	y = (((float)(pts[i].y>>4)) / 2.0 - 50.0) * SCALE;
	/* x is already flipped because of the different axises */
	if (east == 1) {
	  x = (-x);
	} else {
	  y = (-y);
	}
	fprintf(fp, "cylinder { <%f, 0.0, %f>, <%f, %f, %f> %f\n",
		x, y,
		x, 4.0 * SCALE, y,
		0.75 * SCALE);
	switch (pts[i].y & 0xF) {
	case 0xE:
	  /* heading west */
	  if (east == 1) {
	    fprintf(fp, "pigment { colour red 1.0 green 1.0 blue 0.25 }\n");
	  } else {
	    fprintf(fp, "pigment { colour red 1.0 green 1.0 blue 1.0 }\n");
	  }
	  break;
	case 0xF:
	  /* heading east */
	  if (east == 1) {
	    fprintf(fp, "pigment { colour red 1.0 green 1.0 blue 1.0 }\n");
	  } else {
	    fprintf(fp, "pigment { colour red 1.0 green 1.0 blue 0.25 }\n");
	  }
	  break;
	case 0xB:
	  /* heading north/south */
	  fprintf(fp, "pigment { colour red 0.25 green 0.25 blue 1.0 }\n");
	  break;
	default:
	  printf("Unknown direction %d\n", pts[i].y & 0xF);
	  break;
	}
	fprintf(fp, "}\n");
	i++;
      }
	fclose(fp);
      totalbeats++;
    }
    free(pts);
  }
  printf("Handled %d beats\n", totalbeats);
  return 0;
}

struct Coord* loadanim(FILE* fp, int nummen, int* beats) {
  coord num;
  struct Coord* mem;
  int i;

  fread(&num, sizeof(coord), 1, fp);
  *beats = get_word(&num);

  mem = malloc(sizeof(struct Coord)*nummen*(*beats));
  fread(mem, sizeof(struct Coord), nummen*(*beats), fp);

  for (i=0; i < (*beats)*nummen; i++) {
    mem[i].x = get_word(&mem[i].x);
    mem[i].y = get_word(&mem[i].y);
  }
  return mem;
}
