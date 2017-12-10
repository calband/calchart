#pragma once
/*
 * cc_text.h
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

#include "cc_types.h"

#include <string>
#include <vector>

namespace CalChart {

struct Textchunk {
    Textchunk()
        : text()
        , font(PSFONT_NORM)
    {
    }
    std::string text;
    enum PSFONT_TYPE font;
};

using Textchunk_list = std::vector<Textchunk>;

class Textline {
private:
    friend class Print_continuity;
    Textline(std::string line, PSFONT_TYPE& currfontnum);

public:
    Textchunk_list GetChunks() const { return chunks; }
    bool GetCenter() const { return center; }
    bool GetOnMain() const { return on_main; }
    bool GetOnSheet() const { return on_sheet; }

private:
    Textchunk_list chunks;
    bool center;
    bool on_main;
    bool on_sheet;
};

using Textline_list = std::vector<Textline>;

class Print_continuity {
public:
    Print_continuity();
    Print_continuity(const std::string& data);
    Print_continuity(const std::string& number, const std::string& data);
    auto GetChunks() const { return mPrintChunks; }
    auto GetOriginalLine() const { return mOriginalLine; }
    auto GetPrintNumber() const { return mNumber; }

private:
    Textline_list mPrintChunks;
    std::string mOriginalLine;
    std::string mNumber;
};
}
