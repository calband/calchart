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

namespace CalChart::DrawCommands {

namespace Point {

    std::vector<CalChart::DrawCommand> CreatePoint(CalChart::Point const& point, CalChart::Coord const& pos, std::string const& label, CalChart::SYMBOL_TYPE symbol, double dotRatio, double pLineRatio, double sLineRatio)
    {
        auto filled = [](auto symbol) {
            switch (symbol) {
            case CalChart::SYMBOL_SOL:
            case CalChart::SYMBOL_SOLBKSL:
            case CalChart::SYMBOL_SOLSL:
            case CalChart::SYMBOL_SOLX:
                return true;
                break;
            default:
                return false;
            }
        }(symbol);

        auto const circ_r = CalChart::Float2CoordUnits(dotRatio) / 2.0;
        auto const plineoff = circ_r * pLineRatio;
        auto const slineoff = circ_r * sLineRatio;

        auto drawCmds = std::vector<CalChart::DrawCommand>{};
        drawCmds.push_back(CalChart::DrawCommands::Circle(pos, CalChart::Float2CoordUnits(dotRatio) / 2.0, filled));

        switch (symbol) {
        case CalChart::SYMBOL_SL:
        case CalChart::SYMBOL_X: {
            drawCmds.push_back(CalChart::DrawCommands::Line(pos.x - plineoff, pos.y + plineoff, pos.x + plineoff, pos.y - plineoff));
        } break;
        case CalChart::SYMBOL_SOLSL:
        case CalChart::SYMBOL_SOLX:
            drawCmds.push_back(CalChart::DrawCommands::Line(pos.x - slineoff, pos.y + slineoff, pos.x + slineoff, pos.y - slineoff));
            break;
        default:
            break;
        }
        switch (symbol) {
        case CalChart::SYMBOL_BKSL:
        case CalChart::SYMBOL_X:
            drawCmds.push_back(CalChart::DrawCommands::Line(pos.x - plineoff, pos.y - plineoff, pos.x + plineoff, pos.y + plineoff));
            break;
        case CalChart::SYMBOL_SOLBKSL:
        case CalChart::SYMBOL_SOLX:
            drawCmds.push_back(CalChart::DrawCommands::Line(pos.x - slineoff, pos.y - slineoff, pos.x + slineoff, pos.y + slineoff));
            break;
        default:
            break;
        }
        if (point.LabelIsVisible()) {
            auto anchor = toUType(CalChart::DrawCommands::Text::TextAnchor::Bottom);
            anchor |= point.GetFlip() ? toUType(CalChart::DrawCommands::Text::TextAnchor::Left) : toUType(CalChart::DrawCommands::Text::TextAnchor::Right);
            drawCmds.push_back(CalChart::DrawCommands::Text(pos - CalChart::Coord(0, circ_r), label, anchor));
        }
        return drawCmds;
    }
}
namespace Field {
    // Construct commands for the field outline
    std::vector<CalChart::DrawCommand> CreateOutline(Coord const& fieldsize, Coord const& border1)
    {
        return {
            CalChart::DrawCommands::Line{ CalChart::Coord(0, 0) + border1, CalChart::Coord(fieldsize.x, 0) + border1 },
            CalChart::DrawCommands::Line{ CalChart::Coord(fieldsize.x, 0) + border1, CalChart::Coord(fieldsize.x, fieldsize.y) + border1 },
            CalChart::DrawCommands::Line{ CalChart::Coord(fieldsize.x, fieldsize.y) + border1, CalChart::Coord(0, fieldsize.y) + border1 },
            CalChart::DrawCommands::Line{ CalChart::Coord(0, fieldsize.y) + border1, CalChart::Coord(0, 0) + border1 },
        };
    }

    // Construct commands for the vertical solid line down from the top
    std::vector<CalChart::DrawCommand> CreateVerticalSolidLine(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int step1)
    {
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        for (auto j = 0; j <= fieldsize.x; j += step8) {
            result.emplace_back(CalChart::DrawCommands::Line{ CalChart::Coord(j, 0) + border1, CalChart::Coord(j, fieldsize.y) + border1 });
        }
        return result;
    }

    // Construct commands for the vertical solid line down from the top
    std::vector<CalChart::DrawCommand> CreateVerticalDottedLine(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int step1)
    {
        auto step2 = 2 * step1;
        auto step4 = 4 * step1;
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        for (auto j = step4; j < fieldsize.x; j += step8) {
            // draw mid-dotted lines
            for (auto k = 0; k < fieldsize.y; k += step2) {
                result.emplace_back(CalChart::DrawCommands::Line{ CalChart::Coord(j, k) + border1, CalChart::Coord(j, k + step1) + border1 });
            }
        }
        return result;
    }

    std::vector<CalChart::DrawCommand> CreateHorizontalDottedLine(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int mode_HashW, int mode_HashE, int step1)
    {
        auto step2 = 2 * step1;
        auto step4 = 4 * step1;
        std::vector<CalChart::DrawCommand> result;
        for (auto j = step4; j < fieldsize.y; j += step4) {
            if ((j == CalChart::Int2CoordUnits(mode_HashW)) || j == CalChart::Int2CoordUnits(mode_HashE))
                continue;
            for (auto k = 0; k < fieldsize.x; k += step2) {
                result.emplace_back(CalChart::DrawCommands::Line{ CalChart::Coord(k, j) + border1, CalChart::Coord(k + step1, j) + border1 });
            }
        }
        return result;
    }

    // Draw the hashes
    std::vector<CalChart::DrawCommand> CreateHashes(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int mode_HashW, int mode_HashE, int step1)
    {
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        auto hashW = CalChart::Int2CoordUnits(mode_HashW);
        auto hashE = CalChart::Int2CoordUnits(mode_HashE);
        for (auto j = CalChart::Int2CoordUnits(0); j < fieldsize.x; j += step8) {
            result.emplace_back(CalChart::DrawCommands::Line{
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.0 * 8), hashW) + border1,
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.1 * 8), hashW) + border1 });
            result.emplace_back(CalChart::DrawCommands::Line{
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.9 * 8), hashW) + border1,
                CalChart::Coord(j + CalChart::Float2CoordUnits(1.0 * 8), hashW) + border1 });

            result.emplace_back(CalChart::DrawCommands::Line{
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.0 * 8), hashE) + border1,
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.1 * 8), hashE) + border1 });
            result.emplace_back(CalChart::DrawCommands::Line{
                CalChart::Coord(j + CalChart::Float2CoordUnits(0.9 * 8), hashE) + border1,
                CalChart::Coord(j + CalChart::Float2CoordUnits(1.0 * 8), hashE) + border1 });
        }
        return result;
    }

    // Draw the hashes
    std::vector<CalChart::DrawCommand> CreateHashTicks(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int mode_HashW, int mode_HashE, int step1)
    {
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        auto hashW = CalChart::Int2CoordUnits(mode_HashW);
        auto hashE = CalChart::Int2CoordUnits(mode_HashE);
        for (auto j = CalChart::Int2CoordUnits(0); j < fieldsize.x; j += step8) {
            for (size_t midhash = 1; midhash < 5; ++midhash) {
                result.emplace_back(CalChart::DrawCommands::Line{
                    CalChart::Coord(j + CalChart::Float2CoordUnits(midhash / 5.0 * 8), hashW) + border1,
                    CalChart::Coord(j + CalChart::Float2CoordUnits(midhash / 5.0 * 8), CalChart::Float2CoordUnits(mode_HashW - (0.2 * 8))) + border1 });
                result.emplace_back(CalChart::DrawCommands::Line{
                    CalChart::Coord(j + CalChart::Float2CoordUnits(midhash / 5.0 * 8), hashE) + border1,
                    CalChart::Coord(j + CalChart::Float2CoordUnits(midhash / 5.0 * 8), CalChart::Float2CoordUnits(mode_HashE + (0.2 * 8))) + border1 });
            }
        }
        return result;
    }

    std::vector<CalChart::DrawCommand> CreateYardlineLabels(std::span<std::string const> yard_text, CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int offset, int step1)
    {
        auto step8 = 8 * step1;
        std::vector<CalChart::DrawCommand> result;
        for (int i = 0; i < CalChart::CoordUnits2Int(fieldsize.x) / 8 + 1; i++) {
            auto text = yard_text[i];

            using TextAnchor = CalChart::DrawCommands::Text::TextAnchor;
            result.emplace_back(CalChart::DrawCommands::Text{ CalChart::Coord(i * step8 + border1.x, border1.y + offset), text, toUType(TextAnchor::Bottom) | toUType(TextAnchor::HorizontalCenter) | toUType(TextAnchor::ScreenTop), true });
            result.emplace_back(CalChart::DrawCommands::Text{ CalChart::Coord(i * step8 + border1.x, border1.y + fieldsize.y - offset), text, toUType(TextAnchor::Top) | toUType(TextAnchor::HorizontalCenter), true });
        }
        return result;
    }

}

}
