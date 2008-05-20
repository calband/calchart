/* calchart.h
 * Definitions for the show structures and functions
 *
 * Modification history:
 * 1-2-95     Garrick Meeker              Created from previous CalPrint
 *
 */

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
