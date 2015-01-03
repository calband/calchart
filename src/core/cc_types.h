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

#pragma once

#include <stdint.h>
#include <set>
#include <string>

typedef int16_t Coord;

enum class PSFONT_TYPE
{
	SYMBOL, NORM, BOLD, ITAL, BOLDITAL, TAB
};

enum class SYMBOL_TYPE
{
	PLAIN, SOL, BKSL, SL, X, SOLBKSL, SOLSL, SOLX
	, MAX_NUM_SYMBOLS
};

static const SYMBOL_TYPE k_symbols[] = {
	SYMBOL_TYPE::PLAIN, SYMBOL_TYPE::SOL, SYMBOL_TYPE::BKSL, SYMBOL_TYPE::SL, SYMBOL_TYPE::X, SYMBOL_TYPE::SOLBKSL, SYMBOL_TYPE::SOLSL, SYMBOL_TYPE::SOLX
};

std::string GetNameForSymbol(SYMBOL_TYPE which);
SYMBOL_TYPE GetSymbolForName(const std::string& name);

enum class CC_DRAG_TYPES
{
	NONE, BOX, POLY, LASSO, LINE, CROSS
};

enum class CC_MOVE_MODES
{
	NORMAL, SWAP, LINE, ROTATE, SHEAR, REFL, SIZE, GENIUS
};

typedef std::set<unsigned> SelectionList;

// General utility for extracting the underlying type from an enum
// should be moved to a more generic place
template<typename E>
constexpr typename std::underlying_type<E>::type toUType(E enumerator) noexcept
{
	return static_cast<typename std::underlying_type<E>::type>(enumerator);
}

