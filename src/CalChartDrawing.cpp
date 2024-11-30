/*
 * draw.cpp
 * Member functions for drawing stuntsheets
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

#define _LIBCPP_ENABLE_EXPERIMENTAL 1
#include "CalChartDrawing.h"

#include "CalChartAnimation.h"
#include "CalChartAnimationCommand.h"
#include "CalChartConfiguration.h"
#include "CalChartCoordHelper.h"
#include "CalChartDoc.h"
#include "CalChartDrawCommand.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawingGetMinSize.h"
#include "CalChartDrawingLayout.h"
#include "CalChartPoint.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "CalChartShowMode.h"
#include "CalChartText.h"
#include "CalChartUtils.h"
#include "DCSaveRestore.h"
#include "basic_ui.h"
#include <memory>
#include <ranges>
#include <span>
#include <wx/dc.h>
#include <wx/dcmemory.h>

// Conventions:  When drawing with wxPoints/Size, assume that is already in the correct DIP.
// Drawing constants

namespace CalChartDraw {

static const auto ArrowSize = fDIP(4);
static const auto kStep8 = fDIP(CalChart::Int2CoordUnits(8));
static const auto kStep4 = fDIP(CalChart::Int2CoordUnits(4));
static const auto kStep2 = fDIP(CalChart::Int2CoordUnits(2));
static const auto kStep1 = fDIP(CalChart::Int2CoordUnits(1));

// draw text centered around x (though still at y down)
auto GenerateDrawCenteredText(std::string const& text) -> std::vector<CalChart::Draw::DrawCommand>
{
    using TextAnchor = CalChart::Draw::Text::TextAnchor;
    return std::vector<CalChart::Draw::DrawCommand>{
        CalChart::Draw::Text{ {}, text, TextAnchor::HorizontalCenter | TextAnchor::Top }
    };
}
// draw text centered around x (though still at y down)
void DrawCenteredText(wxDC& dc, const wxString& text, wxPoint pt)
{
    auto size = dc.GetTextExtent(text);
    pt.x -= size.x / 2;
    dc.DrawText(text, pt);
}

// draw text centered around x (though still at y down)
auto GenerateDrawLineOverText(const wxString& text, int lineLength) -> std::vector<CalChart::Draw::DrawCommand>
{
    using TextAnchor = CalChart::Draw::Text::TextAnchor;
    return std::vector<CalChart::Draw::DrawCommand>{
        CalChart::Draw::Text{ {}, text, TextAnchor::HorizontalCenter | TextAnchor::Top } + CalChart::Coord{ 0, 2 },
        CalChart::Draw::Line{ { -lineLength / 2, 0 }, { lineLength / 2, 0 } }
    };
}
// draw text centered around x (though still at y down) with a line over it.
void DrawLineOverText(wxDC& dc, wxString const& text, wxPoint const& pt, wxCoord lineLength)
{
    DrawCenteredText(dc, text, pt + wxPoint(0, 2));
    dc.DrawLine(pt - wxPoint(lineLength / 2, 0), pt + wxPoint(lineLength / 2, 0));
}

auto GenerateDrawArrow(CalChart::Coord::units lineLength, bool pointRight) -> std::vector<CalChart::Draw::DrawCommand>
{
    static constexpr auto ArrowSize = 4;
    if (pointRight) {
        return std::vector<CalChart::Draw::DrawCommand>{
            CalChart::Draw::Line{ CalChart::Coord{ -lineLength / 2, ArrowSize }, CalChart::Coord{ lineLength / 2, ArrowSize } },
            CalChart::Draw::Line{ CalChart::Coord{ lineLength / 2 - ArrowSize, 0 }, CalChart::Coord{ lineLength / 2, ArrowSize } },
            CalChart::Draw::Line{ CalChart::Coord{ lineLength / 2 - ArrowSize, 2 * ArrowSize }, CalChart::Coord{ lineLength / 2, ArrowSize } },
        };
    }
    return std::vector<CalChart::Draw::DrawCommand>{
        CalChart::Draw::Line{ CalChart::Coord{ -lineLength / 2, ArrowSize }, CalChart::Coord{ lineLength / 2, ArrowSize } },
        CalChart::Draw::Line{ CalChart::Coord{ -(lineLength / 2 - ArrowSize), 0 }, CalChart::Coord{ -lineLength / 2, ArrowSize } },
        CalChart::Draw::Line{ CalChart::Coord{ -(lineLength / 2 - ArrowSize), 2 * ArrowSize }, CalChart::Coord{ -lineLength / 2, ArrowSize } },
    };
}

void DrawArrow(wxDC& dc, wxPoint const& pt, wxCoord lineLength, bool pointRight)
{
    dc.DrawLine(pt + wxPoint(-lineLength / 2, ArrowSize), pt + wxPoint(lineLength / 2, ArrowSize));
    if (pointRight) {
        dc.DrawLine(pt + wxPoint(lineLength / 2 - (wxCoord)ArrowSize, 0), pt + wxPoint(lineLength / 2, ArrowSize));
        dc.DrawLine(pt + wxPoint(lineLength / 2 - (wxCoord)ArrowSize, ArrowSize * 2), pt + wxPoint(lineLength / 2, ArrowSize));
    } else {
        dc.DrawLine(pt + wxPoint(-(lineLength / 2 - (wxCoord)ArrowSize), 0), pt + wxPoint(-lineLength / 2, ArrowSize));
        dc.DrawLine(pt + wxPoint(-(lineLength / 2 - (wxCoord)ArrowSize), ArrowSize * 2), pt + wxPoint(-lineLength / 2, ArrowSize));
    }
}

// calculate the distance for tab stops
auto TabStops(int which, bool landscape)
{
    // all this can be config
    auto tab = 0;
    while (which > 4) {
        which--;
        tab += (landscape) ? 8 : 6;
    }
    switch (which) {
    case 3:
        tab += 8;
    case 2:
        tab += (landscape) ? 18 : 14;
    case 1:
        tab += (landscape) ? 10 : 6;
    }
    return tab;
}

// these are the sizes that the page is set up to do.
static constexpr double kBitmapScale = 4.0; // the factor to scale the bitmap
static constexpr double kFieldTop = 0.14;
static constexpr double kFieldBorderOffset = 0.06;
static constexpr double kSizeX = 576, kSizeY = 734;
static constexpr double kSizeXLandscape = 917, kSizeYLandscape = 720;
static const double kHeaderLocation[2][2] = {
    { 0.5, 18 / kSizeY },
    { 0.5, 22 / kSizeYLandscape }
};
static const wxString kHeader = wxT("UNIVERSITY OF CALIFORNIA MARCHING BAND");
static const double kUpperNumberPosition[2][2] = {
    { 1.0 - 62 / kSizeX, 36 / kSizeY },
    { 1.0 - 96 / kSizeXLandscape, 36 / kSizeYLandscape }
};
static const double kLowerNumberPosition[2][2] = {
    { 1.0 - 62 / kSizeX, 714 / kSizeY },
    { 1.0 - 96 / kSizeXLandscape, 680 / kSizeYLandscape }
};
static const double kLowerNumberBox[2][4] = {
    { 1.0 - 90 / kSizeX, 708 / kSizeY, 56 / kSizeX, 22 / kSizeY },
    { 1.0 - 124 / kSizeXLandscape, 674 / kSizeYLandscape, 56 / kSizeXLandscape, 28 / kSizeYLandscape }
};

static const auto kTextLabels = std::array{
    std::array{
        std::tuple{ "Music", 0.5, 60 / kSizeY, 240 / kSizeX },
        std::tuple{ "Formation", 0.5, 82 / kSizeY, 240 / kSizeX },
        std::tuple{ "game", 62 / kSizeX, 50 / kSizeY, 64 / kSizeX },
        std::tuple{ "page", 1.0 - 62 / kSizeX, 50 / kSizeY, 64 / kSizeX },
    },
    std::array{
        std::tuple{ "Music", 0.5, 60 / kSizeYLandscape, 400 / kSizeXLandscape },
        std::tuple{ "Formation", 0.5, 82 / kSizeYLandscape, 400 / kSizeXLandscape },
        std::tuple{ "game", 96 / kSizeXLandscape, 54 / kSizeYLandscape, 78 / kSizeXLandscape },
        std::tuple{ "page", 1.0 - 96 / kSizeXLandscape, 54 / kSizeYLandscape, 78 / kSizeXLandscape },
    },
};

// convention is upper south, upper north, lower south, lower north
static const auto kLabels = std::array{
    std::array{
        std::tuple{ "CAL SIDE", 0.5, 580 / kSizeY },
        std::tuple{ "south", 52 / kSizeX, (76 - 8) / kSizeY },
        std::tuple{ "north", 1.0 - 52 / kSizeX, (76 - 8) / kSizeY },
        std::tuple{ "south", 52 / kSizeX, (570 + 8) / kSizeY },
        std::tuple{ "north", 1.0 - 52 / kSizeX, (570 + 8) / kSizeY },
    },
    std::array{
        std::tuple{ "CAL SIDE", 0.5, 544 / kSizeYLandscape },
        std::tuple{ "south", 76 / kSizeXLandscape, (80 - 8) / kSizeYLandscape },
        std::tuple{ "north", 1.0 - 76 / kSizeXLandscape, (80 - 8) / kSizeYLandscape },
        std::tuple{ "south", 76 / kSizeXLandscape, (536 + 8) / kSizeYLandscape },
        std::tuple{ "north", 1.0 - 76 / kSizeXLandscape, (536 + 8) / kSizeYLandscape },
    },
};

static constexpr auto kArrows = std::array{
    std::array{
        std::tuple{ 52 / kSizeX, (76) / kSizeY, 40 / kSizeX, false },
        std::tuple{ 1.0 - 52 / kSizeX, (76) / kSizeY, 40 / kSizeX, true },
        std::tuple{ 52 / kSizeX, (570) / kSizeY, 40 / kSizeX, false },
        std::tuple{ 1.0 - 52 / kSizeX, (570) / kSizeY, 40 / kSizeX, true },
    },
    std::array{
        std::tuple{ 76 / kSizeXLandscape, (80) / kSizeYLandscape, 40 / kSizeXLandscape, false },
        std::tuple{ 1.0 - 76 / kSizeXLandscape, (80) / kSizeYLandscape, 40 / kSizeXLandscape, true },
        std::tuple{ 76 / kSizeXLandscape, (536) / kSizeYLandscape, 40 / kSizeXLandscape, false },
        std::tuple{ 1.0 - 76 / kSizeXLandscape, (536) / kSizeYLandscape, 40 / kSizeXLandscape, true },
    },
};

static const double kContinuityStart[2] = { 606 / kSizeY, 556 / kSizeYLandscape };

auto toCoord(wxPoint point)
{
    return CalChart::Coord(point.x, point.y);
}

namespace CalChartDraw::Point {

    auto DrawPoint(CalChart::Configuration const& config, CalChart::Point const& point, int reference, CalChart::Coord const& origin, wxString const& label)
    {
        return point.GetDrawCommands(
                   reference,
                   label,
                   config.Get_DotRatio(),
                   config.Get_PLineRatio(),
                   config.Get_SLineRatio())
            + origin;
    }

    // Returns a view adaptor that will transform a range of point indices to Draw point commands.
    auto TransformIndexToDrawCommands(CalChart::Sheet const& sheet, std::vector<std::string> const& labels, int ref, double dotRatio, double pLineRatio, double sLineRatio)
    {
        return std::views::transform([&sheet, ref, labels, dotRatio, pLineRatio, sLineRatio](int i) {
            return sheet.GetPoint(i).GetDrawCommands(
                ref,
                labels.at(i),
                dotRatio,
                pLineRatio,
                sLineRatio);
        })
            | std::ranges::views::join;
    }

    // Given a set and a size, return a range that has the numbers not in the set
    auto NegativeIntersection(CalChart::SelectionList const& set, int count)
    {
        return std::views::iota(0, count)
            | std::views::filter([set](int i) {
                  return !set.contains(i);
              });
    }

    auto GenerateSheetPointsDrawCommands(
        CalChart::Configuration const& config,
        CalChart::SelectionList const& selection_list,
        int numberPoints,
        std::vector<std::string> const& labels,
        CalChart::Sheet const& sheet,
        int ref,
        CalChart::Colors unselectedColor,
        CalChart::Colors selectedColor,
        CalChart::Colors unselectedTextColor,
        CalChart::Colors selectedTextColor) -> std::vector<CalChart::Draw::DrawCommand>
    {
        auto dotRatio = config.Get_DotRatio();
        auto pLineRatio = config.Get_PLineRatio();
        auto sLineRatio = config.Get_SLineRatio();

        return {
            CalChart::Draw::withBrushAndPen(
                config.Get_CalChartBrushAndPen(unselectedColor),
                CalChart::Draw::withTextForeground(
                    config.Get_CalChartBrushAndPen(unselectedTextColor),
                    NegativeIntersection(selection_list, numberPoints)
                        | TransformIndexToDrawCommands(sheet, labels, ref, dotRatio, pLineRatio, sLineRatio))),
            CalChart::Draw::withBrushAndPen(
                config.Get_CalChartBrushAndPen(selectedColor),
                CalChart::Draw::withTextForeground(
                    config.Get_CalChartBrushAndPen(selectedTextColor),
                    selection_list
                        | TransformIndexToDrawCommands(sheet, labels, ref, dotRatio, pLineRatio, sLineRatio))),
        };
    }
}

auto GenerateGhostPointsDrawCommands(CalChart::Configuration const& config, CalChart::SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto unselectedColor = CalChart::Colors::GHOST_POINT;
    auto selectedColor = CalChart::Colors::GHOST_POINT_HLIT;
    auto unselectedTextColor = CalChart::Colors::GHOST_POINT_TEXT;
    auto selectedTextColor = CalChart::Colors::GHOST_POINT_HLIT_TEXT;
    return CalChartDraw::Point::GenerateSheetPointsDrawCommands(config, selection_list, numberPoints, labels, sheet, ref, unselectedColor, selectedColor, unselectedTextColor, selectedTextColor);
}

auto GeneratePointsDrawCommands(CalChart::Configuration const& config, CalChart::SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref, bool primary) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto unselectedColor = primary ? CalChart::Colors::POINT : CalChart::Colors::REF_POINT;
    auto selectedColor = primary ? CalChart::Colors::POINT_HILIT : CalChart::Colors::REF_POINT_HILIT;
    auto unselectedTextColor = primary ? CalChart::Colors::POINT_TEXT : CalChart::Colors::REF_POINT_TEXT;
    auto selectedTextColor = primary ? CalChart::Colors::POINT_HILIT_TEXT : CalChart::Colors::REF_POINT_HILIT_TEXT;
    return CalChartDraw::Point::GenerateSheetPointsDrawCommands(config, selection_list, numberPoints, labels, sheet, ref, unselectedColor, selectedColor, unselectedTextColor, selectedTextColor);
}

struct DrawContFonts {
    DrawContFonts(int fontSize = 12)
        : plain(fontSize, CalChart::Font::Family::Modern)
        , bold(fontSize, CalChart::Font::Family::Modern, CalChart::Font::Style::Normal, CalChart::Font::Weight::Bold)
        , italics(fontSize, CalChart::Font::Family::Modern, CalChart::Font::Style::Italic)
        , bolditalics(fontSize, CalChart::Font::Family::Modern, CalChart::Font::Style::Italic, CalChart::Font::Weight::Bold)
    {
    }
    CalChart::Font const plain{};
    CalChart::Font const bold{};
    CalChart::Font const italics{};
    CalChart::Font const bolditalics{};
};

// draw the continuity starting at a specific offset
void DrawCont(wxDC& dc, [[maybe_unused]] CalChart::Configuration const& config, CalChart::Textline_list const& print_continuity, wxRect const& bounding, bool landscape)
{
    SaveAndRestore::DeviceOrigin orig_dev(dc);
    SaveAndRestore::UserScale orig_scale(dc);
    SaveAndRestore::TextForeground orig_text(dc);
    SaveAndRestore::Font orig_font(dc);
#if DEBUG
    dc.DrawRectangle(bounding);
#endif

    dc.SetTextForeground(*wxBLACK);

    auto pageMiddle = (bounding.GetWidth() / 2);

    auto numLines = std::count_if(print_continuity.begin(), print_continuity.end(), [](auto&& i) { return i.on_sheet; });

    auto font_size = ((bounding.GetBottom() - bounding.GetTop()) - (numLines - 1) * 2) / (numLines ? numLines : 1);
    font_size = std::min<int>(font_size, config.Get_PrintContMaxFontSize());

    auto contPlainFont = CreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    auto contBoldFont = CreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    auto contItalFont = CreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);
    auto contBoldItalFont = CreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD);

    dc.SetFont(contPlainFont);
    // 2 can be config
    const wxCoord maxtexth = font_size + 2;

    float y = bounding.GetTop();
    const wxCoord charWidth = dc.GetCharWidth();
    for (auto& cont : print_continuity) {
        float x = bounding.GetLeft();
        // Determine how to center the text
        if (cont.center) {
            x += pageMiddle;
            auto chunks = cont.chunks;
            for (auto& chunk : chunks) {
                std::visit(
                    CalChart::overloaded{
                        [&dc, &x, contPlainFont, contBoldFont, contItalFont, contBoldItalFont]([[maybe_unused]] CalChart::TextChunk const& c) {
                            switch (c.font) {
                            case CalChart::PSFONT::SYMBOL: {
                                wxCoord textw, texth;
                                dc.GetTextExtent(wxT("O"), &textw, &texth);
                                x += textw * c.text.length();
                            } break;
                            case CalChart::PSFONT::NORM:
                                dc.SetFont(contPlainFont);
                                break;
                            case CalChart::PSFONT::BOLD:
                                dc.SetFont(contBoldFont);
                                break;
                            case CalChart::PSFONT::ITAL:
                                dc.SetFont(contItalFont);
                                break;
                            case CalChart::PSFONT::BOLDITAL:
                                dc.SetFont(contBoldItalFont);
                                break;
                            }
                            if (c.font != CalChart::PSFONT::SYMBOL) {
                                wxCoord textw, texth;
                                dc.GetTextExtent(c.text, &textw, &texth);
                                x -= textw / 2;
                            }
                        },
                        []([[maybe_unused]] auto const& arg) {},
                    },
                    chunk);
            }
        }
        // now draw the text
        unsigned tabnum = 0;
        auto chunks = cont.chunks;
        for (auto& chunk : chunks) {
            std::visit(
                CalChart::overloaded{
                    [&dc, &x, y, contPlainFont, contBoldFont, contItalFont, contBoldItalFont](CalChart::TextChunk const& c) {
                        switch (c.font) {
                        case CalChart::PSFONT::NORM:
                        case CalChart::PSFONT::SYMBOL:
                            dc.SetFont(contPlainFont);
                            break;
                        case CalChart::PSFONT::BOLD:
                            dc.SetFont(contBoldFont);
                            break;
                        case CalChart::PSFONT::ITAL:
                            dc.SetFont(contItalFont);
                            break;
                        case CalChart::PSFONT::BOLDITAL:
                            dc.SetFont(contBoldItalFont);
                            break;
                        default:
                            break;
                        }
                        if (c.font == CalChart::PSFONT::SYMBOL) {
                            wxCoord textw, texth, textd;
                            dc.GetTextExtent(wxT("O"), &textw, &texth, &textd);
                            const float d = textw;
                            const float top_y = y + texth - textd - textw;

                            for (std::string::const_iterator s = c.text.begin(); s != c.text.end();
                                 s++) {
                                {
                                    dc.SetPen(*wxBLACK_PEN);
                                    CalChart::SYMBOL_TYPE sym = (CalChart::SYMBOL_TYPE)(*s - 'A');
                                    switch (sym) {
                                    case CalChart::SYMBOL_SOL:
                                    case CalChart::SYMBOL_SOLBKSL:
                                    case CalChart::SYMBOL_SOLSL:
                                    case CalChart::SYMBOL_SOLX:
                                        dc.SetBrush(*wxBLACK_BRUSH);
                                        break;
                                    default:
                                        dc.SetBrush(*wxTRANSPARENT_BRUSH);
                                    }
                                    dc.DrawEllipse(x, top_y, d, d);
                                    switch (sym) {
                                    case CalChart::SYMBOL_SL:
                                    case CalChart::SYMBOL_X:
                                    case CalChart::SYMBOL_SOLSL:
                                    case CalChart::SYMBOL_SOLX:
                                        // 1 can be config
                                        dc.DrawLine(x - 1, top_y + d + 1, x + d + 1, top_y - 1);
                                        break;
                                    default:
                                        break;
                                    }
                                    switch (sym) {
                                    case CalChart::SYMBOL_BKSL:
                                    case CalChart::SYMBOL_X:
                                    case CalChart::SYMBOL_SOLBKSL:
                                    case CalChart::SYMBOL_SOLX:
                                        // 1 can be config
                                        dc.DrawLine(x - 1, top_y - 1, x + d + 1, top_y + d + 1);
                                        break;
                                    default:
                                        break;
                                    }
                                }
                                x += d;
                            }
                        } else {
                            wxCoord textw, texth;
                            dc.GetTextExtent(c.text, &textw, &texth);
                            dc.DrawText(c.text, x, y);
                            x += textw;
                        }
                    },
                    [&tabnum, &x, bounding, charWidth, landscape]([[maybe_unused]] CalChart::TabChunk const& arg) {
                        tabnum++;
                        wxCoord textw = bounding.GetLeft() + charWidth * TabStops(tabnum, landscape);
                        if (textw >= x) {
                            x = textw;
                        } else {
                            x += charWidth;
                        }
                    },
                    []([[maybe_unused]] auto const& arg) {},
                },
                chunk);
        }
        y += maxtexth;
    }
#if DEBUG
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "TopLeft %d, %d", bounding.GetTopLeft().x, bounding.GetTopLeft().y);
    dc.DrawText(buffer, bounding.GetTopLeft());
    snprintf(buffer, sizeof(buffer), "BottomRight %d, %d", bounding.GetBottomRight().x, bounding.GetBottomRight().y);
    dc.DrawText(buffer, bounding.GetBottomRight());
#endif
}

// Return a bounding box of the show of where the marchers are.  If they are
// outside the show, we don't see them.
// can be done better with algorithms
static std::pair<CalChart::Coord, CalChart::Coord>
GetMarcherBoundingBox(std::vector<CalChart::Point> const& pts)
{
    CalChart::Coord bounding_box_upper_left{ 10000, 10000 };
    CalChart::Coord bounding_box_low_right{ -10000, -10000 };

    for (auto& i : pts) {
        auto position = i.GetPos();
        bounding_box_upper_left = CalChart::Coord(std::min(bounding_box_upper_left.x, position.x), std::min(bounding_box_upper_left.y, position.y));
        bounding_box_low_right = CalChart::Coord(std::max(bounding_box_low_right.x, position.x), std::max(bounding_box_low_right.y, position.y));
    }

    return { bounding_box_upper_left, bounding_box_low_right };
}

// Divide the draw space into lines with a padding in between each.
auto CalculatePointsPerLine(
    CalChart::PrintContinuityLayout::VStack const& print_continuity,
    wxRect const& bounding,
    int linePad)
{
    auto numLines = std::count_if(print_continuity.lines.begin(), print_continuity.lines.end(), [](auto&& i) { return i.on_sheet; });
    return ((bounding.GetBottom() - bounding.GetTop()) - (numLines - 1) * linePad) / (numLines ? numLines : 1);
}

namespace {
    auto GeneratePrintField(
        CalChart::Configuration const& config,
        CalChartDoc const& show,
        CalChart::Sheet const& sheet,
        int ref,
        bool landscape) -> std::tuple<double, std::vector<CalChart::Draw::DrawCommand>>
    {
        // Print the field:
        // create a field for drawing:
        const auto pts = sheet.GetPoints();
        auto boundingBox = GetMarcherBoundingBox(pts);
        auto mode = show.GetShowMode().CreateFieldForPrinting(CalChart::CoordUnits2Int(boundingBox.first.x), CalChart::CoordUnits2Int(boundingBox.second.x), landscape);
        auto drawCmds = CalChart::CreateModeDrawCommandsWithBorder(config, mode, CalChart::HowToDraw::Printing);
        CalChart::append(drawCmds,
            CalChart::Draw::toDrawCommands(std::views::iota(0u, pts.size())
                | std::views::transform([&](auto i) {
                      return CalChartDraw::Point::DrawPoint(
                          config,
                          pts.at(i),
                          ref,
                          mode.Offset(),
                          show.GetPointLabel(i));
                  })
                | std::views::join));

        return { static_cast<double>(mode.Size().x), drawCmds };
    }

    auto GenerateLargePrintElements(bool landscape, wxSize page, std::string sheetName)
    {
        auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};
        CalChart::append(drawCmds,
            GenerateDrawCenteredText(kHeader) + CalChart::Coord(page.x * kHeaderLocation[landscape][0], page.y * kHeaderLocation[landscape][1]));
        CalChart::append(drawCmds,
            GenerateDrawCenteredText(sheetName) + CalChart::Coord(page.x * kUpperNumberPosition[landscape][0], page.y * kUpperNumberPosition[landscape][1]));
        CalChart::append(drawCmds,
            GenerateDrawCenteredText(sheetName) + CalChart::Coord(page.x * kLowerNumberPosition[landscape][0], page.y * kLowerNumberPosition[landscape][1]));
        return drawCmds;
    }

    auto GeneratePrintElements(bool landscape, wxSize page, CalChart::Draw::DrawCommand const& cont) -> std::vector<CalChart::Draw::DrawCommand>
    {
        auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};
        CalChart::append(drawCmds,
            std::vector<CalChart::Draw::DrawCommand>{ CalChart::Draw::Rectangle(
                CalChart::Coord(page.x * kLowerNumberBox[landscape][0], page.y * kLowerNumberBox[landscape][1]),
                CalChart::Coord(page.x * kLowerNumberBox[landscape][2], page.y * kLowerNumberBox[landscape][3])) });
        CalChart::append(drawCmds,
            CalChart::Draw::toDrawCommands(
                kTextLabels[landscape] | std::views::transform([page](auto&& labelData) {
                    auto [label, x, y, lineLength] = labelData;
                    return GenerateDrawLineOverText(label, page.x * lineLength) + CalChart::Coord(page.x * x, page.y * y);
                })
                | std::views::join));

        CalChart::append(drawCmds,
            CalChart::Draw::toDrawCommands(
                kLabels[landscape] | std::views::transform([page](auto&& labelData) {
                    auto [label, x, y] = labelData;
                    return GenerateDrawCenteredText(label) + CalChart::Coord(page.x * x, page.y * y);
                })
                | std::views::join));

        CalChart::append(drawCmds,
            CalChart::Draw::toDrawCommands(
                kArrows[landscape] | std::views::transform([page](auto&& arrow) {
                    auto [x, y, length, right] = arrow;
                    return GenerateDrawArrow(page.x * length, right) + CalChart::Coord(page.x * x, page.y * y);
                })
                | std::views::join));

        // a possible future optimization would be to generate this ahead of time.
        CalChart::append(drawCmds, cont);
        return drawCmds;
    }
}

void DrawForPrintingHelper(
    wxDC& dc,
    CalChart::Configuration const& config,
    CalChart::Sheet const& sheet,
    bool landscape,
    double scale_x,
    std::vector<CalChart::Draw::DrawCommand> fieldDrawCommand)
{
    // set up everything to be restored after we print
    SaveAndRestore::DeviceOrigin orig_dev(dc);
    SaveAndRestore::UserScale orig_scale(dc);
    SaveAndRestore::Font orig_font(dc);

    // get the page dimensions
    auto page = dc.GetSize();

    // a possible future optimization would be to generate this ahead of time.
    auto printLayout = CalChart::PrintContinuityLayout::Parse(sheet.GetRawPrintContinuity());
    auto printDrawCommands = GenerateDrawCommands(dc, config, printLayout, wxRect(wxPoint(10, page.y * kContinuityStart[landscape]), wxSize(page.x - 20, page.y - page.y * kContinuityStart[landscape])), landscape);

    dc.Clear();
    dc.SetLogicalFunction(wxCOPY);

    // Print the field:

    // set the origin and scaling for drawing the field
    dc.SetDeviceOrigin(kFieldBorderOffset * page.x, kFieldTop * page.y);

    // create a field for drawing:
    auto scale = tDIP(page.x - 2 * kFieldBorderOffset * page.x) / scale_x;
    // Because we are drawing to a bitmap, DIP isn't happening.  So we compensate by changing the scaling.
    dc.SetUserScale(scale, scale);

    // draw the field.
    wxCalChart::Draw::DrawCommandList(dc, fieldDrawCommand);

    // now reset everything to draw the rest of the text
    dc.SetDeviceOrigin(CalChart::Int2CoordUnits(0), CalChart::Int2CoordUnits(0));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    // set the page for drawing:
    page = dc.GetSize();
    auto sizeX = (landscape) ? kSizeXLandscape : kSizeX;
    auto sizeY = (landscape) ? kSizeYLandscape : kSizeY;
    // because we are drawing to a bitmap, we manipulate the scaling factor so it just draws as normal
    dc.SetUserScale(tDIP(page.x) / sizeX, tDIP(page.y) / sizeY);
    page.x = fDIP(sizeX);
    page.y = fDIP(sizeY);

    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};

    CalChart::append(drawCmds,
        CalChart::Draw::withFont(
            CalChart::Font{ 16, CalChart::Font::Family::Roman, CalChart::Font::Style::Normal, CalChart::Font::Weight::Bold },
            GenerateLargePrintElements(landscape, page, sheet.GetNumber())));

    CalChart::append(drawCmds,
        CalChart::Draw::withFont(
            CalChart::Font{ 8 },
            GeneratePrintElements(
                landscape,
                page,
                printDrawCommands + CalChart::Coord(10, page.y * kContinuityStart[landscape]))));
    wxCalChart::Draw::DrawCommandList(dc, drawCmds);
}

auto GenerateDrawCommands(wxDC& dc,
    CalChart::Configuration const& config,
    CalChart::PrintContinuityLayout::VStack const& printLayout,
    wxRect const& bounding,
    bool landscape)
    -> CalChart::Draw::DrawCommand
{
    auto linePad = static_cast<int>(config.Get_PrintContLinePad());
    auto factor = config.Get_PrintContDotRatio();
    auto fontSize = std::min<int>(CalculatePointsPerLine(printLayout, bounding, linePad), config.Get_PrintContMaxFontSize());
    auto [symbolSize, symbolMiddle] = [](auto& dc, int fontSize, double factor) -> std::pair<int, CalChart::Coord> {
        auto restore = SaveAndRestore::Font{ dc };
        wxCalChart::setFont(dc, { fontSize, CalChart::Font::Family::Modern });
        wxCoord textw{};
        wxCoord texth{};
        wxCoord textd{};
        dc.GetTextExtent("O", &textw, &texth, &textd);
        auto middle = texth - textw - textd;
        return { textw * factor, CalChart::Coord(middle, middle) };
    }(dc, fontSize, factor);

    auto context = CalChart::PrintContinuityLayout::Context{
        fontSize,
        landscape,
        linePad,
        symbolSize,
        symbolMiddle,
        config.Get_PrintContPLineRatio(),
        config.Get_PrintContSLineRatio(),
    };

    try {
        return CalChart::Draw::withBrushAndPen(
            wxCalChart::toBrushAndPen(*wxBLACK_PEN),
            CalChart::Draw::withFont(
                context.plain,
                CalChart::PrintContinuityLayout::ToDrawCommand(printLayout, context)));
    } catch (std::exception const& e) {
        std::cout << "hit the excpetion " << e.what() << "\n";
        return CalChart::Draw::Ignore{};
    }
}

void DrawForPrinting(wxDC* printerdc, CalChart::Configuration const& config, CalChartDoc const& show, CalChart::Sheet const& sheet, int ref, bool landscape)
{
    auto boundingBox = GetMarcherBoundingBox(sheet.GetPoints());
    auto forced_landscape = !landscape && (boundingBox.second.x - boundingBox.first.x) > CalChart::Int2CoordUnits(CalChart::kFieldStepSizeNorthSouth[0]);
    auto should_landscape = landscape || forced_landscape;

    auto bitmapWidth = (should_landscape ? kSizeXLandscape : kSizeX) * kBitmapScale;
    auto bitmapHeight = (should_landscape ? kSizeYLandscape : kSizeY) * kBitmapScale;

    // construct a bitmap for drawing on.
    wxBitmap membm(bitmapWidth, bitmapHeight);
    // first convert to image
    wxMemoryDC memdc(membm);
    auto [scale_x, drawCmds1] = GeneratePrintField(config, show, sheet, ref, should_landscape);
    DrawForPrintingHelper(memdc, config, sheet, should_landscape, scale_x, drawCmds1);

    auto image = membm.ConvertToImage();
    if (forced_landscape) {
        image = image.Rotate90(false);
    }
    wxBitmap rotate_membm(image);
    wxMemoryDC tmemdc(rotate_membm);

    auto printer = printerdc->GetSize();
    auto scaleX = printer.x / float(rotate_membm.GetWidth());
    auto scaleY = printer.y / float(rotate_membm.GetHeight());
    printerdc->SetUserScale(scaleX, scaleY);
    printerdc->Blit(0, 0, rotate_membm.GetWidth(), rotate_membm.GetHeight(), &tmemdc, 0, 0);
}

auto GeneratePhatomPointsDrawCommands(
    const CalChart::Configuration& config,
    const CalChartDoc& show,
    const CalChart::Sheet& sheet,
    const std::map<int, CalChart::Coord>& positions)
    -> std::vector<CalChart::Draw::DrawCommand>
{
    auto pointLabelFont = CalChart::Font{ CalChart::Float2CoordUnits(config.Get_DotRatio() * config.Get_NumRatio()) };
    auto origin = show.GetShowFieldOffset();

    auto dotRatio = config.Get_DotRatio();
    auto pLineRatio = config.Get_PLineRatio();
    auto sLineRatio = config.Get_SLineRatio();

    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};
    for (auto&& i : positions) {
        CalChart::append(
            drawCmds,
            // because points draw their position, we remove it then add the new position.
            sheet.GetPoint(i.first).GetDrawCommands(
                show.GetPointLabel(i.first),
                dotRatio,
                pLineRatio,
                sLineRatio)
                + i.second
                - sheet.GetPoint(i.first).GetPos());
    }
    return std::vector<CalChart::Draw::DrawCommand>{
        CalChart::Draw::withFont(
            pointLabelFont,
            CalChart::Draw::withBrushAndPen(
                config.Get_CalChartBrushAndPen(CalChart::Colors::GHOST_POINT),
                CalChart::Draw::withTextForeground(
                    config.Get_CalChartBrushAndPen(CalChart::Colors::GHOST_POINT_TEXT),
                    drawCmds + origin)))
    };
}
}

namespace wxCalChart::Draw {

// this draws the yardline labels
static void DrawText(wxDC& dc, std::string text, wxPoint where, CalChart::Draw::Text::TextAnchor anchor, bool withBox)
{
    auto textSize = dc.GetTextExtent(text);
    using TextAnchor = CalChart::Draw::Text::TextAnchor;
    if ((anchor & TextAnchor::VerticalCenter) == TextAnchor::VerticalCenter) {
        where.y -= textSize.y / 2;
    }
    if ((anchor & TextAnchor::Bottom) == TextAnchor::Bottom) {
        where.y -= textSize.y;
    }
    if ((anchor & TextAnchor::HorizontalCenter) == TextAnchor::HorizontalCenter) {
        where.x -= textSize.x / 2;
    }
    if ((anchor & TextAnchor::Right) == TextAnchor::Right) {
        where.x -= textSize.x;
    }
    if ((anchor & TextAnchor::ScreenTop) == TextAnchor::ScreenTop) {
        where.y = std::max(where.y, dc.DeviceToLogicalY(0));
    }
    if ((anchor & TextAnchor::ScreenBottom) == TextAnchor::ScreenBottom) {
        auto edge = dc.DeviceToLogicalY(dc.GetSize().y);
        where.y = std::min(where.y, edge - textSize.y);
    }
    if ((anchor & TextAnchor::ScreenLeft) == TextAnchor::ScreenLeft) {
        where.x = std::max(where.x, dc.DeviceToLogicalX(0));
    }
    if ((anchor & TextAnchor::ScreenRight) == TextAnchor::ScreenRight) {
        auto edge = dc.DeviceToLogicalX(dc.GetSize().x);
        where.x = std::min(where.x, edge - textSize.x);
    }
    if (withBox) {
        dc.DrawRectangle(where, textSize);
    }
    dc.DrawText(text, where);
}

// The CalChart::Draw::DrawCommand is a data structure that dictates "what to draw", but not how or where specficially.
// The How is interpreted by the wxWidget's Device Context logic -- effectively we translate the `CalChart::Draw::DrawCommand`
// type to the appropriate `wxDC` API call with the appropriate offsets from the `CalChart::Draw::DrawCommand` objects.
// But Layout becomes a bit more trickly we have `CalChart::Draw::DrawStack` objects.  Stacking things stacking things
// together, especially Text, becomes complicated because the size of the Text due to the font choice can change, the
// "where" to draw becomes non-trivial.
//
// We solve this by introducing a new data structure, the `DrawingSurface` object.  We first parse out the DrawCommand
// object to determine what the surface size should be, and then pass those DrawSurface objects to the command that then
// draws them into that place.

namespace details {

    void DrawCommand(wxDC& dc, DrawSurface surface, CalChart::Draw::DrawItems const& cmd)
    {
        return std::visit(
            CalChart::overloaded{
                [&dc, layoutPoint = surface.origin](CalChart::Draw::Line const& c) {
                    auto start = wxCalChart::to_wxPoint(c.c1) + layoutPoint;
                    auto end = wxCalChart::to_wxPoint(c.c2) + layoutPoint;
                    dc.DrawLine(fDIP(start), fDIP(end));
                },
                [&dc, layoutPoint = surface.origin](CalChart::Draw::Arc const& c) {
                    // draw arc is always assumed to be radius (start to center) + center
                    auto start = wxCalChart::to_wxPoint(c.c1) + layoutPoint;
                    auto end = wxCalChart::to_wxPoint(c.c2) + layoutPoint;
                    auto center = wxCalChart::to_wxPoint(c.cc) + layoutPoint;
                    dc.DrawArc(fDIP(start), fDIP(end), fDIP(center));
                },
                [&dc, layoutPoint = surface.origin](CalChart::Draw::Ellipse const& c) {
                    SaveAndRestore::Brush restore(dc);
                    if (!c.filled) {
                        dc.SetBrush(*wxTRANSPARENT_BRUSH);
                    }
                    auto start = wxCalChart::to_wxPoint(c.c1) + layoutPoint;
                    auto end = wxCalChart::to_wxSize(c.c2 - c.c1);
                    dc.DrawEllipse(fDIP(start), fDIP(end));
                },
                [&dc, layoutPoint = surface.origin](CalChart::Draw::Circle const& c) {
                    SaveAndRestore::Brush restore(dc);
                    if (!c.filled) {
                        dc.SetBrush(*wxTRANSPARENT_BRUSH);
                    }
                    auto center = wxCalChart::to_wxPoint(c.c1) + layoutPoint;
                    dc.DrawCircle(fDIP(center), fDIP(c.radius));
                },
                [&dc, layoutPoint = surface.origin](CalChart::Draw::Rectangle const& c) {
                    SaveAndRestore::Brush restore(dc);
                    if (!c.filled) {
                        dc.SetBrush(*wxTRANSPARENT_BRUSH);
                    }
                    auto start = wxCalChart::to_wxPoint(c.start) + layoutPoint;
                    auto size = wxCalChart::to_wxSize(c.size);
                    if (c.rounding > 0) {
                        auto rounding = c.rounding;
                        dc.DrawRoundedRectangle(fDIP(start), fDIP(size), fDIP(rounding));
                    } else {
                        dc.DrawRectangle(fDIP(start), fDIP(size));
                    }
                },
                [&dc, layoutPoint = surface.origin](CalChart::Draw::Text const& c) {
                    auto where = wxCalChart::to_wxPoint(c.c1) + layoutPoint;
                    DrawText(dc, c.text, fDIP(where), c.anchor, c.withBackground);
                },
                [&dc, layoutPoint = surface.origin](CalChart::Draw::Image const& c) {
                    auto where = wxCalChart::to_wxPoint(c.mStart) + layoutPoint;
                    auto image = wxCalChart::ConvertTowxImage(*c.mImage);
                    if (c.mGreyscale) {
                        image = image.ConvertToGreyscale();
                    }
                    dc.DrawBitmap(image, fDIP(where));
                },
                []([[maybe_unused]] CalChart::Draw::Ignore const& c) {
                },
                []([[maybe_unused]] CalChart::Draw::Tab const& c) {
                },
            },
            cmd);
    }

    void DrawCommand(wxDC& dc, DrawSurface surface, CalChart::Draw::DrawManipulators const& cmd)
    {
        std::visit(
            CalChart::overloaded{
                [&dc, surface](CalChart::Draw::OverrideFont const& c) {
                    SaveAndRestore::Font restore(dc);
                    wxCalChart::setFont(dc, c.font);
                    DrawCommandList(dc, surface, c.commands);
                },
                [&dc, surface](CalChart::Draw::OverrideBrush const& c) {
                    SaveAndRestore::Brush restore(dc);
                    wxCalChart::setBrush(dc, c.brush);
                    DrawCommandList(dc, surface, c.commands);
                },
                [&dc, surface](CalChart::Draw::OverridePen const& c) {
                    SaveAndRestore::Pen restore(dc);
                    wxCalChart::setPen(dc, c.pen);
                    DrawCommandList(dc, surface, c.commands);
                },
                [&dc, surface](CalChart::Draw::OverrideBrushAndPen const& c) {
                    SaveAndRestore::BrushAndPen restore(dc);
                    wxCalChart::setBrushAndPen(dc, c.brushAndPen);
                    DrawCommandList(dc, surface, c.commands);
                },
                [&dc, surface](CalChart::Draw::OverrideTextForeground const& c) {
                    SaveAndRestore::TextForeground restore(dc);
                    wxCalChart::setTextForeground(dc, c.brushAndPen);
                    DrawCommandList(dc, surface, c.commands);
                },
            },
            cmd);
    }

    void DrawCommand(wxDC& dc, DrawSurface surface, CalChart::Draw::DrawStack const& cmd)
    {
        // first we need to get an array of the min sizes.
        // Then we determine their layout.
        // then we call draw on each.
        std::visit(
            CalChart::overloaded{
                [&dc, surface](CalChart::Draw::VStack const& c) {
                    auto layoutPoints = VLayout(GetMinSizes({ dc, surface }, c.commands), surface, c.align);
                    for (auto i = static_cast<std::size_t>(0); i < c.commands.size(); ++i) {
                        DrawCommand(dc, { layoutPoints[i], surface.size }, c.commands[i]);
                    }
                },
                [&dc, surface](CalChart::Draw::HStack const& c) {
                    auto layoutPoints = HLayout(GetMinSizes({ dc, surface }, c.commands), surface, c.align);
                    for (auto i = static_cast<std::size_t>(0); i < c.commands.size(); ++i) {
                        DrawCommand(dc, { layoutPoints[i], surface.size }, c.commands[i]);
                    }
                },
                [&dc, surface](CalChart::Draw::ZStack const& c) {
                    auto layoutPoints = ZLayout(GetMinSizes({ dc, surface }, c.commands), surface, c.align);
                    for (auto i = static_cast<std::size_t>(0); i < c.commands.size(); ++i) {
                        DrawCommand(dc, { layoutPoints[i], surface.size }, c.commands[i]);
                    }
                },
            },
            cmd);
    }

    void DrawCommand(wxDC& dc, DrawSurface surface, CalChart::Draw::DrawCommand const& cmd)
    {
        std::visit(
            [&dc, surface](auto&& c) {
                DrawCommand(dc, surface, c);
            },
            cmd);
    }
}
}

namespace CalChartDraw {
void DrawPath(wxDC& dc, CalChart::Configuration const& config, std::vector<CalChart::Draw::DrawCommand> const& draw_commands)
{
    wxCalChart::setBrushAndPen(dc, config.Get_CalChartBrushAndPen(CalChart::Colors::PATHS));
    wxCalChart::Draw::DrawCommandList(dc, draw_commands);
}

wxImage GetOmniLinesImage(const CalChart::Configuration& config, const CalChart::ShowMode& mode)
{
    auto fieldsize = mode.FieldSize();
    wxBitmap bmp(fieldsize.x, fieldsize.y, 32);
    wxMemoryDC dc;
    dc.SelectObject(bmp);
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    wxCalChart::Draw::DrawCommandList(dc, CalChart::CreateModeDrawCommands(config, mode, CalChart::HowToDraw::OmniView));
    auto image = bmp.ConvertToImage();
    image.InitAlpha();
    for (auto x = 0; x < fieldsize.x; ++x) {
        for (auto y = 0; y < fieldsize.y; ++y) {
            if (!image.GetRed(x, y) && !image.GetGreen(x, y) && !image.GetBlue(x, y)) {
                image.SetAlpha(x, y, 0);
            }
        }
    }
    return image;
}
}
