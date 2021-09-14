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

/* This is the format text line:
 * normal ascii text possibly containing the following codes:
 * \bs \be \is \ie for bold start, bold end, italics start, italics end
 * \po plainman : A
 * \pb backslashman : C
 * \ps slashman : D
 * \px xman : E
 * \so solidman : B
 * \sb solidbackslashman :F
 * \ss solidslashman : G
 * \sx solidxman : H
 * a line may begin with these symbols in order: <>~
 * < don't print continuity on individual sheets
 * > don't print continuity on master sheet
 * ~ center this line
 * also, there are three tab stops set for standard continuity format
 */

Textline ParseTextLine(std::string line)
{
    auto result = Textline{};
    auto currfontnum = PSFONT::NORM;
    // peel off the '<>~'
    if (!line.empty() && line.at(0) == '<') {
        result.on_sheet = false;
        line.erase(0, 1);
    }
    if (!line.empty() && line.at(0) == '>') {
        result.on_main = false;
        line.erase(0, 1);
    }
    if (!line.empty() && line.at(0) == '~') {
        result.center = true;
        line.erase(0, 1);
    }
    // break the line into substrings
    while (!line.empty()) {
        // first take care of any tabs
        if (line.at(0) == '\t') {
            if (line.length() > 1) {
                Textchunk new_text;
                new_text.font = PSFONT::TAB;
                result.chunks.push_back(new_text);
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
            result.chunks.push_back(new_text);
            line.erase(0, 3);
            continue;
        }
        // now check to see if we have bold
        if ((line.length() >= 3) && (line.at(0) == '\\') && ((tolower(line.at(1)) == 'b'))) {
            if (tolower(line.at(2)) == 's') {
                currfontnum = (currfontnum == PSFONT::NORM || currfontnum == PSFONT::BOLD) ? PSFONT::BOLD : PSFONT::BOLDITAL;
            } else if (tolower(line.at(2)) == 'e') {
                currfontnum = (currfontnum == PSFONT::NORM || currfontnum == PSFONT::BOLD) ? PSFONT::NORM : PSFONT::ITAL;
            } else {
                // code not recognized
                throw std::runtime_error("Print continuity not recognized");
            }
            line.erase(0, 3);
            continue;
        }
        // now check to see if we have ital
        if ((line.length() >= 3) && (line.at(0) == '\\') && ((tolower(line.at(1)) == 'i'))) {
            if (tolower(line.at(2)) == 's') {
                currfontnum = (currfontnum == PSFONT::NORM || currfontnum == PSFONT::ITAL) ? PSFONT::ITAL : PSFONT::BOLDITAL;
            } else if (tolower(line.at(2)) == 'e') {
                currfontnum = (currfontnum == PSFONT::NORM || currfontnum == PSFONT::ITAL) ? PSFONT::NORM : PSFONT::BOLD;
            } else {
                // code not recognized
                throw std::runtime_error("Print continuity not recognized");
            }
            line.erase(0, 3);
            continue;
        }
        auto pos = line.find_first_of("\\\t", 1);

        result.chunks.push_back({ line.substr(0, pos), currfontnum });
        line.erase(0, pos);
    }
    return result;
}

PrintContinuity::PrintContinuity(std::string const& number, std::string const& data)
    : mOriginalLine(data)
    , mNumber(number)
{
    std::istringstream reader(data);
    std::string line;
    while (std::getline(reader, line, '\n')) {
        mPrintChunks.push_back(ParseTextLine(line));
    }
}
}
