/*
 * CC_DrawCommand.h
 * Class for how to draw
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

#include "CalChartDrawCommand.h"
#include "CalChartCoord.h"
#include "CalChartPoint.h"
#include <vector>

namespace CalChart::Draw {

namespace Field {
    // Construct commands for the field outline
    auto CreateOutline(Coord const& fieldsize) -> std::vector<CalChart::DrawCommand>
    {
        return std::vector<CalChart::DrawCommand>{
            CalChart::Draw::Line{ CalChart::Coord(0, 0), CalChart::Coord(fieldsize.x, 0) },
            CalChart::Draw::Line{ CalChart::Coord(fieldsize.x, 0), CalChart::Coord(fieldsize.x, fieldsize.y) },
            CalChart::Draw::Line{ CalChart::Coord(fieldsize.x, fieldsize.y), CalChart::Coord(0, fieldsize.y) },
            CalChart::Draw::Line{ CalChart::Coord(0, fieldsize.y), CalChart::Coord(0, 0) },
        };
    }

    // Construct commands for the vertical solid line down from the top
    auto CreateVerticalSolidLine(CalChart::Coord const& fieldsize, int step1) -> std::vector<CalChart::DrawCommand>
    {
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        for (auto j = 0; j <= fieldsize.x; j += step8) {
            result.emplace_back(CalChart::Draw::Line{ CalChart::Coord(j, 0), CalChart::Coord(j, fieldsize.y) });
        }
        return result;
    }

    // Construct commands for the vertical solid line down from the top
    auto CreateVerticalDottedLine(CalChart::Coord const& fieldsize, int step1) -> std::vector<CalChart::DrawCommand>
    {
        auto step2 = 2 * step1;
        auto step4 = 4 * step1;
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        for (auto j = step4; j < fieldsize.x; j += step8) {
            // draw mid-dotted lines
            for (auto k = 0; k < fieldsize.y; k += step2) {
                result.emplace_back(CalChart::Draw::Line{ CalChart::Coord(j, k), CalChart::Coord(j, k + step1) });
            }
        }
        return result;
    }

    auto CreateHorizontalDottedLine(CalChart::Coord const& fieldsize, int mode_HashW, int mode_HashE, int step1) -> std::vector<CalChart::DrawCommand>
    {
        auto step2 = 2 * step1;
        auto step4 = 4 * step1;
        std::vector<CalChart::DrawCommand> result;
        for (auto j = step4; j < fieldsize.y; j += step4) {
            if ((j == CalChart::Int2CoordUnits(mode_HashW)) || j == CalChart::Int2CoordUnits(mode_HashE))
                continue;
            for (auto k = 0; k < fieldsize.x; k += step2) {
                result.emplace_back(CalChart::Draw::Line{ CalChart::Coord(k, j), CalChart::Coord(k + step1, j) });
            }
        }
        return result;
    }

    // Draw the hashes
    auto CreateHashes(CalChart::Coord const& fieldsize, int mode_HashW, int mode_HashE, int step1) -> std::vector<CalChart::DrawCommand>
    {
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        auto hashW = CalChart::Int2CoordUnits(mode_HashW);
        auto hashE = CalChart::Int2CoordUnits(mode_HashE);
        for (auto j = CalChart::Int2CoordUnits(0); j < fieldsize.x; j += step8) {
            result.emplace_back(CalChart::Draw::Line{
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.0 * 8), hashW),
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.1 * 8), hashW) });
            result.emplace_back(CalChart::Draw::Line{
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.9 * 8), hashW),
                CalChart::Coord(j + CalChart::Float2CoordUnits(1.0 * 8), hashW) });

            result.emplace_back(CalChart::Draw::Line{
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.0 * 8), hashE),
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.1 * 8), hashE) });
            result.emplace_back(CalChart::Draw::Line{
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.9 * 8), hashE),
                CalChart::Coord(j + CalChart::Float2CoordUnits(1.0 * 8), hashE) });
        }
        return result;
    }

    // Draw the hashes
    auto CreateHashTicks(CalChart::Coord const& fieldsize, int mode_HashW, int mode_HashE, int step1) -> std::vector<CalChart::DrawCommand>
    {
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        auto hashW = CalChart::Int2CoordUnits(mode_HashW);
        auto hashE = CalChart::Int2CoordUnits(mode_HashE);
        for (auto j = CalChart::Int2CoordUnits(0); j < fieldsize.x; j += step8) {
            for (size_t midhash = 1; midhash < 5; ++midhash) {
                result.emplace_back(CalChart::Draw::Line{
                    CalChart::Coord(j + CalChart::Float2CoordUnits(midhash / 5.0 * 8), hashW),
                    CalChart::Coord(j + CalChart::Float2CoordUnits(midhash / 5.0 * 8), CalChart::Float2CoordUnits(mode_HashW - (0.2 * 8))) });
                result.emplace_back(CalChart::Draw::Line{
                    CalChart::Coord(j + CalChart::Float2CoordUnits(midhash / 5.0 * 8), hashE),
                    CalChart::Coord(j + CalChart::Float2CoordUnits(midhash / 5.0 * 8), CalChart::Float2CoordUnits(mode_HashE + (0.2 * 8))) });
            }
        }
        return result;
    }

    auto CreateYardlineLabels(std::vector<std::string> const& yard_text, CalChart::Coord const& fieldsize, int offset, int step1) -> std::vector<CalChart::DrawCommand>
    {
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        for (int i = 0; i < CalChart::CoordUnits2Int(fieldsize.x) / 8 + 1; i++) {
            auto text = yard_text[i];

            using TextAnchor = CalChart::Draw::Text::TextAnchor;
            result.emplace_back(CalChart::Draw::Text{ CalChart::Coord(i * step8, offset), text, TextAnchor::Bottom | TextAnchor::HorizontalCenter | TextAnchor::ScreenTop, true });
            result.emplace_back(CalChart::Draw::Text{ CalChart::Coord(i * step8, fieldsize.y - offset), text, TextAnchor::Top | TextAnchor::HorizontalCenter, true });
        }
        return result;
    }

}

}
