#pragma once
/*
 * CalChartText.h
 * textchunk and textline
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartTypes.h"

#include <string>
#include <vector>

namespace CalChart {

struct Textchunk {
    std::string text;
    PSFONT font = PSFONT::NORM;
};

// helper for comparisions
static inline bool operator==(Textchunk const& a, Textchunk const& b)
{
    return (a.text == b.text)
        && (a.font == b.font);
}

struct Textline {
    std::vector<Textchunk> chunks;
    bool center = false;
    bool on_main = true;
    bool on_sheet = true;
};

Textline ParseTextLine(std::string line);

using Textline_list = std::vector<Textline>;

class PrintContinuity {
public:
    PrintContinuity(std::string const& number = "", std::string const& data = "");
    auto GetChunks() const { return mPrintChunks; }
    auto GetOriginalLine() const { return mOriginalLine; }
    auto GetPrintNumber() const { return mNumber; }

private:
    Textline_list mPrintChunks;
    std::string mOriginalLine;
    std::string mNumber;
};
}
