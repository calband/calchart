#pragma once
/*
 * CalChartPostScript.h
 * PostScript utilities
 */

/*
   Copyright (C) 2024  Richard Michael Powell

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

#include "CalChartText.h"
#include <format>
#include <sstream>

namespace CalChart::PostScript {

constexpr auto GenerateContinuityLine = [](auto const& line, auto currfontnum, auto fontsize) -> std::string {
    static auto fontnames = std::array{
        "CalChart",
        "contfont",
        "boldfont",
        "italfont",
        "bolditalfont",
    };
    std::ostringstream buffer;

    buffer << "/x lmargin def\n";
    short tabstop = 0;
    for (auto const& part : line.chunks) {
        std::visit(
            CalChart::overloaded{
                [&buffer, &tabstop]([[maybe_unused]] TabChunk const& arg) {
                    if (++tabstop > 3) {
                        buffer << "space_over\n";
                    } else {
                        buffer << "tab" << tabstop << " do_tab\n";
                    }
                },
                []([[maybe_unused]] SymbolChunk const& arg) {
                },
                [&buffer, &currfontnum, fontsize, &line](auto const& arg) {
                    if (arg.font != currfontnum) {
                        buffer << std::format("/{} findfont {:.2f} scalefont setfont\n", fontnames[static_cast<int>(arg.font)], fontsize);
                        currfontnum = arg.font;
                    }
                    std::string textstr(arg.text);
                    const char* text = textstr.c_str();
                    while (*text != 0) {
                        std::string temp_buf = "";
                        while (*text != 0) {
                            // Need backslash before parenthesis
                            if ((*text == '(') || (*text == ')')) {
                                temp_buf += "\\";
                            }
                            temp_buf += *(text++);
                        }

                        if (!temp_buf.empty()) {
                            if (line.center) {
                                buffer << "(" << temp_buf << ") centerText\n";
                            } else {
                                buffer << "(" << temp_buf << ") leftText\n";
                            }
                        }
                    }
                },
            },
            part);
    }
    buffer << "/y y h sub def\n";
    return buffer.str();
};
}