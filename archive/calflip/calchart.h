/*
   Copyright (C) 1994  Garrick Brian Meeker

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
