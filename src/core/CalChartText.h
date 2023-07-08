#pragma once
/*
 * CalChartText.h
 * textchunk and textline
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartConstants.h"
#include "CalChartDrawCommand.h"
#include "CalChartPrintContinuityLayout.h"

#include <string>
#include <vector>

namespace CalChart {

struct TextChunk {
    std::string text;
    PSFONT font = PSFONT::NORM;
};

enum class SymbolChunk {
    Plain,
    Backslash,
    Slash,
    Cross,
    Solid,
    SolidBackslash,
    SolidSlash,
    SolidCross,
};

struct TabChunk {
    friend auto operator==(TabChunk const&, TabChunk const&) -> bool = default;
};

// helper for comparisions
[[nodiscard]] static inline auto operator==(TextChunk const& a, TextChunk const& b)
{
    return (a.text == b.text)
        && (a.font == b.font);
}

struct Textline {
    std::vector<std::variant<TextChunk, TabChunk, SymbolChunk>> chunks;
    bool center = false;
    bool on_main = true;
    bool on_sheet = true;
};

[[nodiscard]] auto ParseTextLine(std::string line) -> Textline;

using Textline_list = std::vector<Textline>;

class PrintContinuity {
public:
    explicit PrintContinuity(std::string const& number = "", std::string const& data = "");
    [[nodiscard]] auto GetChunks() const { return mPrintChunks; }
    [[nodiscard]] auto GetOriginalLine() const { return mOriginalLine; }
    [[nodiscard]] auto GetPrintNumber() const { return mNumber; }
    [[nodiscard]] auto GetDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>;

private:
    Textline_list mPrintChunks;
    std::string mOriginalLine;
    std::string mNumber;
};
}
