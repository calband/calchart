#pragma once
/*
 * CalChartTypes.h
 * Primative types for CalChart.  All types in the CalChart namespace (and
 * sub namespaces) should be put here.  This should be a "leaf" header,
 * meaning it should depend on no other headers (except standard headers).
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

#include <functional>
#include <map>
#include <set>
#include <string>

namespace CalChart {

// error occurred on parsing.  First arg is what went wrong, second is the values that need to be fixed.
// return a string of what to try parsing.
using ContinuityParseCorrection_t = std::function<std::string(std::string const&, std::string const&, int line, int column)>;
using VersionMismatchNotice_t = std::function<bool(int, int)>;

struct ParseErrorHandlers {
    ContinuityParseCorrection_t mContinuityParseCorrectionHandler{};
    VersionMismatchNotice_t mVersionMismatchHandler{};
};

using MarcherIndex = unsigned;
using SelectionList = std::set<MarcherIndex>;

using Beats = unsigned;

enum class MoveMode : uint8_t {
    Normal,
    ShapeLine,
    ShapeX,
    ShapeCross,
    ShapeRectange,
    ShapeEllipse,
    ShapeDraw,
    MoveLine,
    MoveRotate,
    MoveShear,
    MoveReflect,
    MoveSize,
    MoveGenius,
};

}
