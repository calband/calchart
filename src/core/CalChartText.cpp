/*
 * CalChartText.cpp
 * textchunk and textline
 */

/*
   Copyright (C) 1995-2013  Garrick Brian Meeker, Richard Michael Powell

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

#include <sstream>

namespace CalChart {

Textline::Textline(std::string line, PSFONT currfontnum)
    : center(false)
    , on_main(true)
    , on_sheet(true)
{
    // peel off the '<>~'
    if (!line.empty() && line.at(0) == '<') {
        on_sheet = false;
        line.erase(0, 1);
    }
    if (!line.empty() && line.at(0) == '>') {
        on_main = false;
        line.erase(0, 1);
    }
    if (!line.empty() && line.at(0) == '~') {
        center = true;
        line.erase(0, 1);
    }
    // break the line into substrings
    while (!line.empty()) {
        // first take care of any tabs
        if (line.at(0) == '\t') {
            if (line.length() > 1) {
                Textchunk new_text;
                new_text.font = PSFONT::TAB;
                chunks.push_back(new_text);
            }
            line.erase(0, 1);
            continue;
        }
        // now check to see if we have any special person marks
        if ((line.length() >= 3) && (line.at(0) == '\\') && ((tolower(line.at(1)) == 'p') || (tolower(line.at(1)) == 's'))) {
            auto new_text = Textchunk{ "", PSFONT::SYMBOL };
            if (tolower(line.at(1)) == 'p') {
                switch (tolower(line.at(2))) {
                case 'o':
                    new_text.text.push_back('A');
                    break;
                case 'b':
                    new_text.text.push_back('C');
                    break;
                case 's':
                    new_text.text.push_back('D');
                    break;
                case 'x':
                    new_text.text.push_back('E');
                    break;
                default:
                    // code not recognized
                    throw std::runtime_error("Print continuity not recognized");
                }
            }
            if (tolower(line.at(1)) == 's') {
                switch (tolower(line.at(2))) {
                case 'o':
                    new_text.text.push_back('B');
                    break;
                case 'b':
                    new_text.text.push_back('F');
                    break;
                case 's':
                    new_text.text.push_back('G');
                    break;
                case 'x':
                    new_text.text.push_back('H');
                    break;
                default:
                    // code not recognized
                    throw std::runtime_error("Print continuity not recognized");
                }
            }
            chunks.push_back(new_text);
            line.erase(0, 3);
            continue;
        }
        // now check to see if we have any font
        if ((line.length() >= 3) && (line.at(0) == '\\') && ((tolower(line.at(1)) == 'b') || (tolower(line.at(1)) == 'i'))) {
            if (tolower(line.at(2)) == 'e') {
                currfontnum = PSFONT::NORM;
            }
            if (tolower(line.at(2)) == 's') {
                currfontnum = (tolower(line.at(1)) == 'b') ? PSFONT::BOLD : PSFONT::ITAL;
            }
            line.erase(0, 3);
            continue;
        }
        auto pos = line.find_first_of("\\\t", 1);

        chunks.push_back({ line.substr(0, pos), currfontnum });
        line.erase(0, pos);
    }
}

PrintContinuity::PrintContinuity(std::string const& number, std::string const& data)
    : mOriginalLine(data)
    , mNumber(number)
{
    std::istringstream reader(data);
    std::string line;
    while (std::getline(reader, line, '\n')) {
        mPrintChunks.emplace_back(line, PSFONT::NORM);
    }
}
}
