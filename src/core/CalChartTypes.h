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
#include <functional>

enum class PSFONT {
    SYMBOL,
    NORM,
    BOLD,
    ITAL,
    BOLDITAL,
    TAB
};

enum SYMBOL_TYPE {
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
std::string GetLongNameForSymbol(SYMBOL_TYPE which);
SYMBOL_TYPE GetSymbolForName(const std::string& name);

// error occurred on parsing.  First arg is what went wrong, second is the values that need to be fixed.
// return a string of what to try parsing.
using ContinuityParseCorrection_t = std::function<std::string(std::string const&, std::string const&, int line, int column)>;

struct ParseErrorHandlers {
    ContinuityParseCorrection_t mContinuityParseCorrectionHandler;
};

}

typedef std::set<int> SelectionList;
