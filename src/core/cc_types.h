/*
 * cc_types.h
 * Definitions for the types
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#ifndef _CC_TYPES_H_
#define _CC_TYPES_H_

#include <stdint.h>
#include <set>

typedef int16_t Coord;

enum PSFONT_TYPE
{
	PSFONT_SYMBOL, PSFONT_NORM, PSFONT_BOLD, PSFONT_ITAL, PSFONT_BOLDITAL,
	PSFONT_TAB
};

enum SYMBOL_TYPE
{
	SYMBOL_PLAIN = 0, SYMBOL_SOL, SYMBOL_BKSL, SYMBOL_SL,
	SYMBOL_X, SYMBOL_SOLBKSL, SYMBOL_SOLSL, SYMBOL_SOLX
};

enum CC_DRAG_TYPES
{
	CC_DRAG_NONE, CC_DRAG_BOX, CC_DRAG_POLY,
	CC_DRAG_LASSO, CC_DRAG_LINE, CC_DRAG_CROSS
};
enum CC_MOVE_MODES
{
	CC_MOVE_NORMAL, CC_MOVE_LINE, CC_MOVE_ROTATE,
	CC_MOVE_SHEAR, CC_MOVE_REFL, CC_MOVE_SIZE,
	CC_MOVE_GENIUS
};

typedef std::set<unsigned> SelectionList;

#define Make4CharWord(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define INGL_INGL Make4CharWord('I','N','G','L')
#define INGL_GURK Make4CharWord('G','U','R','K')
#define INGL_SHOW Make4CharWord('S','H','O','W')
#define INGL_SHET Make4CharWord('S','H','E','T')
#define INGL_SIZE Make4CharWord('S','I','Z','E')
#define INGL_LABL Make4CharWord('L','A','B','L')
#define INGL_MODE Make4CharWord('M','O','D','E')
#define INGL_DESC Make4CharWord('D','E','S','C')
#define INGL_NAME Make4CharWord('N','A','M','E')
#define INGL_DURA Make4CharWord('D','U','R','A')
#define INGL_POS  Make4CharWord('P','O','S',' ')
#define INGL_SYMB Make4CharWord('S','Y','M','B')
#define INGL_TYPE Make4CharWord('T','Y','P','E')
#define INGL_REFP Make4CharWord('R','E','F','P')
#define INGL_CONT Make4CharWord('C','O','N','T')
#define INGL_ECNT Make4CharWord('E','C','N','T')
#define INGL_PCNT Make4CharWord('P','C','N','T')
#define INGL_PNTS Make4CharWord('P','N','T','S')
#define INGL_PONT Make4CharWord('P','O','N','T')
#define INGL_END  Make4CharWord('E','N','D',' ')

struct Version_3_3_and_earlier {};
struct Current_version_and_later {};

#endif
