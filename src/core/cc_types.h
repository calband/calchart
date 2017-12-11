#pragma once
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

#include <set>
#include <string>

enum PSFONT_TYPE {
    PSFONT_SYMBOL,
    PSFONT_NORM,
    PSFONT_BOLD,
    PSFONT_ITAL,
    PSFONT_BOLDITAL,
    PSFONT_TAB
};

enum SYMBOL_TYPE {
    SYMBOLS_START = 0,
    SYMBOL_PLAIN = 0,
    SYMBOL_SOL,
    SYMBOL_BKSL,
    SYMBOL_SL,
    SYMBOL_X,
    SYMBOL_SOLBKSL,
    SYMBOL_SOLSL,
    SYMBOL_SOLX,
    MAX_NUM_SYMBOLS
};

static const SYMBOL_TYPE k_symbols[] = {
    SYMBOL_PLAIN, SYMBOL_SOL, SYMBOL_BKSL, SYMBOL_SL,
    SYMBOL_X, SYMBOL_SOLBKSL, SYMBOL_SOLSL, SYMBOL_SOLX
};

namespace CalChart {
std::string GetNameForSymbol(SYMBOL_TYPE which);
SYMBOL_TYPE GetSymbolForName(const std::string& name);
}

enum CC_DRAG_TYPES {
    CC_DRAG_NONE,
    CC_DRAG_BOX,
    CC_DRAG_POLY,
    CC_DRAG_LASSO,
    CC_DRAG_LINE,
    CC_DRAG_CROSSHAIRS,
    CC_DRAG_SHAPE_ELLIPSE,
    CC_DRAG_SHAPE_X,
    CC_DRAG_SHAPE_CROSS,
    CC_DRAG_SWAP,
};

enum CC_MOVE_MODES {
    CC_MOVE_NORMAL,
    CC_MOVE_SHAPE_LINE,
    CC_MOVE_SHAPE_X,
    CC_MOVE_SHAPE_CROSS,
    CC_MOVE_SHAPE_RECTANGLE,
    CC_MOVE_SHAPE_ELLIPSE,
    CC_MOVE_SHAPE_DRAW,
    CC_MOVE_LINE,
    CC_MOVE_ROTATE,
    CC_MOVE_SHEAR,
    CC_MOVE_REFL,
    CC_MOVE_SIZE,
    CC_MOVE_GENIUS,
};

typedef std::set<int> SelectionList;
