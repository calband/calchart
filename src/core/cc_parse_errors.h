#pragma once
/*
 * cc_parse_errors.cpp
 * A way to coordinate parse errors to the caller.
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

#include <string>
#include <functional>

namespace CalChart {

// error occurred on parsing.  First arg is what went wrong, second is the values that need to be fixed.
// return a string of what to try parsing.
using ContinuityParseCorrection_t = std::function<std::string(std::string const&, std::string const&, int line, int column)>;

struct ParseErrorHandlers
{
    ContinuityParseCorrection_t mContinuityParseCorrectionHandler;
};

}
