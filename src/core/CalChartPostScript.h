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
#include <iostream>

namespace CalChart::PostScript {
static inline void PageHeader(std::ostream& buffer, double dots_x, double dots_y)
{
    buffer << "0 setgray\n";
    buffer << "0.25 setlinewidth\n";
    buffer << (dots_x) << " " << (dots_y) << " translate\n";
}

static inline void PrintContinuityPreamble(std::ostream& buffer,
    double y_def,
    double h_def,
    double rmargin,
    double tab1,
    double tab2,
    double tab3,
    double font_size)
{
    buffer << "/y " << (y_def) << " def\n";
    buffer << "/h " << (h_def) << " def\n";
    buffer << "/lmargin 0 def /rmargin " << (rmargin) << " def\n";
    buffer << "/tab1 " << (tab1) << " def /tab2 " << (tab2) << " def /tab3 "
           << (tab3) << " def\n";
    buffer << "/contfont findfont " << (font_size) << " scalefont setfont\n";
}

static inline void PageBreak(std::ostream& buffer) { buffer << "showpage\n"; }

static inline void PrintFontHeader(std::ostream& buffer, std::array<std::string, 7> const& fonts)
{
    buffer << "%%IncludeResources: font";
    for (auto const& i : fonts) {
        buffer << " " << i;
    }
    buffer << "\n";
    auto fontname = std::array{
        "head",
        "main",
        "number",
        "cont",
        "bold",
        "ital",
        "boldital",
    };
    for (auto i = static_cast<size_t>(0); i < fonts.size(); ++i) {
        buffer << "/" << fontname[i] << "font0 /" << fonts[i] << " def\n";
    }
}

static inline void GenerateContinuityLine(
    std::ostream& buffer,
    const Textline& line,
    PSFONT currfontnum,
    float fontsize)
{
    static auto fontnames = std::array{
        "CalChart",
        "contfont",
        "boldfont",
        "italfont",
        "bolditalfont",
    };

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
                        buffer << "/" << fontnames[static_cast<int>(arg.font)];
                        buffer << " findfont " << fontsize << " scalefont setfont\n";
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
}
}