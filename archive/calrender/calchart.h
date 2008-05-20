/* calchart.h
 * Definitions for the show structures and functions
 *
 * Modification history:
 * 1-2-95     Garrick Meeker              Created from previous CalPrint
 *
 */

#ifndef _CALCHART_H_
#define _CALCHART_H_

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define get_word(a) ((*(char *)a & 0xFF) + ((*((char *)a + 1) & 0xFF) << 8))
#define put_word(a,v) {*(unsigned char *)(a)=v&0xFF;*(unsigned char *)((a) + 1)=v>>8;}

typedef unsigned short coord;

struct Coord {
  coord x;
  coord y;
};

#endif
