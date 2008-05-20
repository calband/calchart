/* calchart.h
 * by Garrick Meeker 1994
 */

#ifndef CALCHART_H
#define CALCHART_H

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define get_word(a) ((*(char *)a & 0x000000FF) + ((*((char *)a + 1) & 0x000000FF) << 8))
#define put_word(a,v) {*(unsigned char *)(a)=(v)&0xFF;*(((unsigned char *)(a)) + 1)=(v)>>8;}

struct cc_coord {
  unsigned short x;
  unsigned short y;
};

struct cc_point {
  unsigned short type;
  struct cc_coord pos;
  unsigned short color;
  unsigned char code[2];
  unsigned short cont;
  struct cc_coord ref[3];
};

#endif
