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

#define MAXPOINTS 300
struct cc_point points[MAXPOINTS];

unsigned short flipx(unsigned short v);
unsigned short flipy(unsigned short v);

void main(int argc, char **argv) {
  int i, j, k, len;
  FILE *fp;
  unsigned short v;

  for (i=1; i<argc; i++) {
    fp = fopen(argv[i], "rb");
    if (fp == NULL) {
      printf("Unable to open '%s'\n", argv[i]);
      return;
    }
    len = fread(points, sizeof(struct cc_point), MAXPOINTS, fp);
    fclose(fp);

    for (j=0; j<len; j++) {
      v = get_word(&points[j].pos.x);
      v = flipx(v);
      put_word(&points[j].pos.x, v);
      v = get_word(&points[j].pos.y);
      v = flipy(v);
      put_word(&points[j].pos.y, v);
      for (k=0; k<3; k++) {
	v = get_word(&points[j].ref[k].x);
	if (v != 0xFFFF) {
	  v = flipx(v);
	  put_word(&points[j].ref[k].x, v);
	}
	v = get_word(&points[j].ref[k].y);
	if (v != 0xFFFF) {
	  v = flipy(v);
	  put_word(&points[j].ref[k].y, v);
	}
      }
    }

    fp = fopen(argv[i], "wb");
    if (fp == NULL) {
      printf("Unable to open '%s'\n", argv[i]);
      return;
    }
    if (fwrite(points, sizeof(struct cc_point), len, fp) != len) {
      printf("Error writing '%s'\n", argv[i]);
      fclose(fp);
      return;
    }
    fclose(fp);
  }
}

unsigned short flipx(unsigned short v) {
  return 1404-v;
}

unsigned short flipy(unsigned short v) {
  return 596-(v+96)-96;
}

