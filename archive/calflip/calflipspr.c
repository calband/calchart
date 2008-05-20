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

