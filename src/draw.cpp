/*
 * draw.cpp
 * Member functions for drawing stuntsheets
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

#include "draw.h"

#include "CalChartDoc.h"
#include "CalChartAnimation.h"
#include "CalChartAnimationCommand.h"
#include "basic_ui.h"
#include "cc_drawcommand.h"
#include "cc_point.h"
#include "cc_sheet.h"
#include "cc_show.h"
#include "cc_text.h"
#include "confgr.h"
#include "draw_utils.h"
#include "modes.h"
#include <memory>
#include <wx/dc.h>
#include <wx/dcmemory.h>

// Conventions:  When drawing with wxPoints/Size, assume that is already in the correct DIP.
// Drawing constants

static const auto ArrowSize = fDIP(4);
static const auto kStep8 = fDIP(Int2CoordUnits(8));
static const auto kStep4 = fDIP(Int2CoordUnits(4));
static const auto kStep2 = fDIP(Int2CoordUnits(2));
static const auto kStep1 = fDIP(Int2CoordUnits(1));

// draw text centered around x (though still at y down)
void DrawCenteredText(wxDC& dc, const wxString& text, wxPoint pt)
{
    auto size = dc.GetTextExtent(text);
    pt.x -= size.x / 2;
    dc.DrawText(text, pt);
}

// draw text centered around x (though still at y down) with a line over it.
void DrawLineOverText(wxDC& dc, wxString const& text, wxPoint const& pt, wxCoord lineLength)
{
    DrawCenteredText(dc, text, pt + wxPoint(0, 2));
    dc.DrawLine(pt - wxPoint(lineLength / 2, 0), pt + wxPoint(lineLength / 2, 0));
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
auto TabStops(int which, bool landscape, int useConstantTabs)
{
    if (useConstantTabs != 0) {
        return which * useConstantTabs;
    }
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
static const double kBitmapScale = 4.0; // the factor to scale the bitmap
static const double kFieldTop = 0.14;
static const double kFieldBorderOffset = 0.06;
static const double kSizeX = 576, kSizeY = 734;
static const double kSizeXLandscape = 917, kSizeYLandscape = 720;
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
static const double kMusicLabelPosition[2][3] = {
    { 0.5, 60 / kSizeY, 240 / kSizeX },
    { 0.5, 60 / kSizeYLandscape, 400 / kSizeXLandscape }
};
static const wxString kMusicLabel = wxT("Music");
static const double kFormationLabelPosition[2][3] = {
    { 0.5, 82 / kSizeY, 240 / kSizeX },
    { 0.5, 82 / kSizeYLandscape, 400 / kSizeXLandscape }
};
static const wxString kFormationLabel = wxT("Formation");

static const double kGameLabelPosition[2][3] = {
    { 62 / kSizeX, 50 / kSizeY, 64 / kSizeX },
    { 96 / kSizeXLandscape, 54 / kSizeYLandscape, 78 / kSizeXLandscape }
};
static const wxString kGameLabel = wxT("game");
static const double kPageLabelPosition[2][3] = {
    { 1.0 - 62 / kSizeX, 50 / kSizeY, 64 / kSizeX },
    { 1.0 - 96 / kSizeXLandscape, 54 / kSizeYLandscape, 78 / kSizeXLandscape }
};
static const wxString kPageLabel = wxT("page");
static const double kSideLabelPosition[2][2] = { { 0.5, 580 / kSizeY },
    { 0.5, 544 / kSizeYLandscape } };
static const wxString kSideLabel = wxT("CAL SIDE");

static const double kUpperSouthPosition[2][2] = {
    { 52 / kSizeX, (76 - 8) / kSizeY },
    { 76 / kSizeXLandscape, (80 - 8) / kSizeYLandscape }
};
static const wxString kUpperSouthLabel = wxT("south");
static const double kUpperSouthArrow[2][3] = {
    { 52 / kSizeX, (76) / kSizeY, 40 / kSizeX },
    { 76 / kSizeXLandscape, (80) / kSizeYLandscape, 40 / kSizeXLandscape }
};
static const double kUpperNorthPosition[2][2] = {
    { 1.0 - 52 / kSizeX, (76 - 8) / kSizeY },
    { 1.0 - 76 / kSizeXLandscape, (80 - 8) / kSizeYLandscape }
};
static const wxString kUpperNorthLabel = wxT("north");
static const double kUpperNorthArrow[2][3] = {
    { 1.0 - 52 / kSizeX, (76) / kSizeY, 40 / kSizeX },
    { 1.0 - 76 / kSizeXLandscape, (80) / kSizeYLandscape, 40 / kSizeXLandscape }
};
static const double kLowerSouthPosition[2][2] = {
    { 52 / kSizeX, (570 + 8) / kSizeY },
    { 76 / kSizeXLandscape, (536 + 8) / kSizeYLandscape }
};
static const wxString kLowerSouthLabel = wxT("south");
static const double kLowerSouthArrow[2][3] = {
    { 52 / kSizeX, (570) / kSizeY, 40 / kSizeX },
    { 76 / kSizeXLandscape, (536) / kSizeYLandscape, 40 / kSizeXLandscape }
};
static const double kLowerNorthPosition[2][2] = {
    { 1.0 - 52 / kSizeX, (570 + 8) / kSizeY },
    { 1.0 - 76 / kSizeXLandscape, (536 + 8) / kSizeYLandscape }
};
static const wxString kLowerNorthLabel = wxT("north");
static const double kLowerNorthArrow[2][3] = {
    { 1.0 - 52 / kSizeX, (570) / kSizeY, 40 / kSizeX },
    { 1.0 - 76 / kSizeXLandscape, (536) / kSizeYLandscape, 40 / kSizeXLandscape }
};

static const double kContinuityStart[2] = { 606 / kSizeY, 556 / kSizeYLandscape };

static void DrawPoint(wxDC& dc, CalChartConfiguration const& config, CalChart::Point const& point, SYMBOL_TYPE symbol, int reference, CalChart::Coord const& origin, wxString const& label);
static void DrawPointHelper(wxDC& dc, CalChartConfiguration const& config, CalChart::Coord const& pos, CalChart::Point const& point, SYMBOL_TYPE symbol, wxString const& label);

void DrawSheetPoints(wxDC& dc, CalChartConfiguration const& config, CalChart::Coord origin, SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref, CalChartColors unselectedColor, CalChartColors selectedColor, CalChartColors unselectedTextColor, CalChartColors selectedTextColor)
{
    SaveAndRestore_Font orig_font(dc);
    dc.SetFont(CreateFont(Float2CoordUnits(config.Get_DotRatio() * config.Get_NumRatio())));
    dc.SetTextForeground(config.Get_CalChartBrushAndPen(COLOR_POINT_TEXT).first.GetColour());
    for (auto i = 0; i < numberPoints; i++) {
        if (selection_list.count(i)) {
            auto brushAndPen = config.Get_CalChartBrushAndPen(selectedColor);
            dc.SetBrush(brushAndPen.first);
            dc.SetPen(brushAndPen.second);
            dc.SetTextForeground(config.Get_CalChartBrushAndPen(selectedTextColor).second.GetColour());
        } else {
            auto brushAndPen = config.Get_CalChartBrushAndPen(unselectedColor);
            dc.SetBrush(brushAndPen.first);
            dc.SetPen(brushAndPen.second);
            dc.SetTextForeground(config.Get_CalChartBrushAndPen(unselectedTextColor).second.GetColour());
        }
        DrawPoint(dc, config, sheet.GetPoint(i), sheet.GetSymbol(i), ref, origin, labels.at(i));
    }
}

void DrawGhostSheet(wxDC& dc, CalChartConfiguration const& config, CalChart::Coord origin, SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref)
{
    DrawSheetPoints(dc, config, origin, selection_list, numberPoints, labels, sheet, ref, COLOR_GHOST_POINT, COLOR_GHOST_POINT_HLIT, COLOR_GHOST_POINT_TEXT, COLOR_GHOST_POINT_HLIT_TEXT);
}

void DrawPoints(wxDC& dc, CalChartConfiguration const& config, CalChart::Coord origin, SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref, bool primary)
{
    auto unselectedColor = primary ? COLOR_POINT : COLOR_REF_POINT;
    auto selectedColor = primary ? COLOR_POINT_HILIT : COLOR_REF_POINT_HILIT;
    auto unselectedTextColor = primary ? COLOR_POINT_TEXT : COLOR_REF_POINT_TEXT;
    auto selectedTextColor = primary ? COLOR_POINT_HILIT_TEXT : COLOR_REF_POINT_HILIT_TEXT;
    DrawSheetPoints(dc, config, origin, selection_list, numberPoints, labels, sheet, ref, unselectedColor, selectedColor, unselectedTextColor, selectedTextColor);
}

// draw the continuity starting at a specific offset
void DrawContForPreview(wxDC& dc, CalChart::Textline_list const& print_continuity, wxRect const& bounding)
{
    return DrawCont(dc, print_continuity, bounding, false, 6);
}

void DrawCont(wxDC& dc, CalChart::Textline_list const& print_continuity, wxRect const& bounding, bool landscape, int useConstantTabs)
{
    SaveAndRestore_DeviceOrigin orig_dev(dc);
    SaveAndRestore_UserScale orig_scale(dc);
    SaveAndRestore_TextForeground orig_text(dc);
    SaveAndRestore_Font orig_font(dc);
#if DEBUG
    dc.DrawRectangle(bounding);
#endif

    dc.SetTextForeground(*wxBLACK);

    auto pageMiddle = (bounding.GetWidth() / 2);

    auto numLines = std::count_if(print_continuity.begin(), print_continuity.end(), [](auto&& i) { return i.GetOnSheet(); });

    auto font_size = ((bounding.GetBottom() - bounding.GetTop()) - (numLines - 1) * 2) / (numLines ? numLines : 1);
    // font size, we scale to be no more than 256 pixels.
    font_size = std::min<int>(font_size, 10);

    auto contPlainFont = CreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    auto contBoldFont = CreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    auto contItalFont = CreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);
    auto contBoldItalFont = CreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD);

    dc.SetFont(contPlainFont);
    const wxCoord maxtexth = font_size + 2;

    float y = bounding.GetTop();
    const wxCoord charWidth = dc.GetCharWidth();
    for (auto& cont : print_continuity) {
        float x = bounding.GetLeft();
        // Determine how to center the text
        if (cont.GetCenter()) {
            x += pageMiddle;
            auto chunks = cont.GetChunks();
            for (auto& c : chunks) {
                bool do_tab = false;
                switch (c.font) {
                case PSFONT::SYMBOL: {
                    wxCoord textw, texth;
                    dc.GetTextExtent(wxT("O"), &textw, &texth);
                    x += textw * c.text.length();
                } break;
                case PSFONT::NORM:
                    dc.SetFont(contPlainFont);
                    break;
                case PSFONT::BOLD:
                    dc.SetFont(contBoldFont);
                    break;
                case PSFONT::ITAL:
                    dc.SetFont(contItalFont);
                    break;
                case PSFONT::BOLDITAL:
                    dc.SetFont(contBoldItalFont);
                    break;
                case PSFONT::TAB:
                    do_tab = true;
                    break;
                }
                if (!do_tab && (c.font != PSFONT::SYMBOL)) {
                    wxCoord textw, texth;
                    dc.GetTextExtent(c.text, &textw, &texth);
                    x -= textw / 2;
                }
            }
        }
        // now draw the text
        unsigned tabnum = 0;
        auto chunks = cont.GetChunks();
        for (auto& c : chunks) {
            bool do_tab = false;
            switch (c.font) {
            case PSFONT::NORM:
            case PSFONT::SYMBOL:
                dc.SetFont(contPlainFont);
                break;
            case PSFONT::BOLD:
                dc.SetFont(contBoldFont);
                break;
            case PSFONT::ITAL:
                dc.SetFont(contItalFont);
                break;
            case PSFONT::BOLDITAL:
                dc.SetFont(contBoldItalFont);
                break;
            case PSFONT::TAB: {
                tabnum++;
                wxCoord textw = bounding.GetLeft() + charWidth * TabStops(tabnum, landscape, useConstantTabs);
                if (textw >= x)
                    x = textw;
                else
                    x += charWidth;
                do_tab = true;
            } break;
            default:
                break;
            }
            if (c.font == PSFONT::SYMBOL) {
                wxCoord textw, texth, textd;
                dc.GetTextExtent(wxT("O"), &textw, &texth, &textd);
                const float d = textw;
                const float top_y = y + texth - textd - textw;

                for (std::string::const_iterator s = c.text.begin(); s != c.text.end();
                     s++) {
                    {
                        dc.SetPen(*wxBLACK_PEN);
                        SYMBOL_TYPE sym = (SYMBOL_TYPE)(*s - 'A');
                        switch (sym) {
                        case SYMBOL_SOL:
                        case SYMBOL_SOLBKSL:
                        case SYMBOL_SOLSL:
                        case SYMBOL_SOLX:
                            dc.SetBrush(*wxBLACK_BRUSH);
                            break;
                        default:
                            dc.SetBrush(*wxTRANSPARENT_BRUSH);
                        }
                        dc.DrawEllipse(x, top_y, d, d);
                        switch (sym) {
                        case SYMBOL_SL:
                        case SYMBOL_X:
                        case SYMBOL_SOLSL:
                        case SYMBOL_SOLX:
                            dc.DrawLine(x - 1, top_y + d + 1, x + d + 1, top_y - 1);
                            break;
                        default:
                            break;
                        }
                        switch (sym) {
                        case SYMBOL_BKSL:
                        case SYMBOL_X:
                        case SYMBOL_SOLBKSL:
                        case SYMBOL_SOLX:
                            dc.DrawLine(x - 1, top_y - 1, x + d + 1, top_y + d + 1);
                            break;
                        default:
                            break;
                        }
                    }
                    x += d;
                }
            } else {
                if (!do_tab) {
                    wxCoord textw, texth;
                    dc.GetTextExtent(c.text, &textw, &texth);
                    dc.DrawText(c.text, x, y);
                    x += textw;
                }
            }
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

CalChart::ShowMode
CreateFieldForPrinting(int left_limit, int right_limit, bool landscape, CalChart::ShowMode::YardLinesInfo_t const& yardlines)
{
    auto size = CalChart::Coord{ Int2CoordUnits(CalChart::kFieldStepSizeNorthSouth[landscape]), Int2CoordUnits(CalChart::kFieldStepSizeEastWest) };

    // extend the limit to the next largest 5 yard line
    left_limit = (left_limit / 8) * 8 + (left_limit % 8 ? (left_limit < 0 ? -8 : 8) : 0);
    right_limit = (right_limit / 8) * 8 + (right_limit % 8 ? (right_limit < 0 ? -8 : 8) : 0);
    auto left_edge = -CalChart::kFieldStepSizeSouthEdgeFromCenter[landscape];
    if (left_limit < left_edge) {
        left_edge = left_limit;
    } else if ((left_edge + size.x) < right_limit) {
        left_edge = right_limit - size.x;
    }
    CalChart::Coord off = { Int2CoordUnits(-left_edge), Int2CoordUnits(CalChart::kFieldStepSizeWestEdgeFromCenter) };

    return CalChart::ShowMode::CreateShowMode(size, off, { 0, 0 }, { 0, 0 }, CalChart::kFieldStepWestHashFromWestSideline, CalChart::kFieldStepEastHashFromWestSideline, yardlines);
}

// Return a bounding box of the show of where the marchers are.  If they are
// outside the show, we don't see them.
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

void DrawForPrintingHelper(wxDC& dc, CalChartConfiguration const& config, CalChartDoc const& show, CalChart::Sheet const& sheet, int ref, bool landscape)
{
    // set up everything to be restored after we print
    SaveAndRestore_DeviceOrigin orig_dev(dc);
    SaveAndRestore_UserScale orig_scale(dc);
    SaveAndRestore_Font orig_font(dc);

    // get the page dimensions
    auto page = dc.GetSize();

    dc.Clear();
    dc.SetLogicalFunction(wxCOPY);

    // Print the field:
    // create a field for drawing:
    const auto pts = sheet.GetPoints();
    auto boundingBox = GetMarcherBoundingBox(pts);
    auto mode = CreateFieldForPrinting(CoordUnits2Int(boundingBox.first.x), CoordUnits2Int(boundingBox.second.x), landscape, show.GetShowMode().Get_yard_text());

    // set the origin and scaling for drawing the field
    dc.SetDeviceOrigin(kFieldBorderOffset * page.x, kFieldTop * page.y);
    // Because we are drawing to a bitmap, DIP isn't happening.  So we compensate by changing the scaling.
    auto scale = tDIP(page.x - 2 * kFieldBorderOffset * page.x) / (double)mode.Size().x;
    dc.SetUserScale(scale, scale);

    // draw the field.
    DrawMode(dc, config, mode, ShowMode_kPrinting);
    dc.SetFont(CreateFont(Float2CoordUnits(config.Get_DotRatio() * config.Get_NumRatio())));
    for (auto i = 0u; i < pts.size(); i++) {
        const auto point = pts.at(i);
        const auto pos = point.GetPos(ref) + mode.Offset();
        dc.SetBrush(*wxBLACK_BRUSH);
        DrawPointHelper(dc, config, pos, point, sheet.GetSymbol(i), show.GetPointLabel(i));
    }

    // now reset everything to draw the rest of the text
    dc.SetDeviceOrigin(Int2CoordUnits(0), Int2CoordUnits(0));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    // set the page for drawing:
    page = dc.GetSize();
    auto sizeX = (landscape) ? kSizeXLandscape : kSizeX;
    auto sizeY = (landscape) ? kSizeYLandscape : kSizeY;
    // because we are drawing to a bitmap, we manipulate the scaling factor so it just draws as normal
    dc.SetUserScale(tDIP(page.x) / sizeX, tDIP(page.y) / sizeY);
    page.x = fDIP(sizeX);
    page.y = fDIP(sizeY);

    // draw the header
    dc.SetFont(CreateFont(16, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 1, wxPENSTYLE_SOLID));

    DrawCenteredText(dc, kHeader, wxPoint(page.x * kHeaderLocation[landscape][0], page.y * kHeaderLocation[landscape][1]));

    DrawCenteredText(dc, sheet.GetNumber(), wxPoint(page.x * kUpperNumberPosition[landscape][0], page.y * kUpperNumberPosition[landscape][1]));
    DrawCenteredText(dc, sheet.GetNumber(), wxPoint(page.x * kLowerNumberPosition[landscape][0], page.y * kLowerNumberPosition[landscape][1]));
    dc.DrawRectangle(page.x * kLowerNumberBox[landscape][0], page.y * kLowerNumberBox[landscape][1], page.x * kLowerNumberBox[landscape][2], page.y * kLowerNumberBox[landscape][3]);

    dc.SetFont(CreateFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    DrawLineOverText(dc, kMusicLabel, wxPoint(page.x * kMusicLabelPosition[landscape][0], page.y * kMusicLabelPosition[landscape][1]), page.x * kMusicLabelPosition[landscape][2]);
    DrawLineOverText(dc, kFormationLabel, wxPoint(page.x * kFormationLabelPosition[landscape][0], page.y * kFormationLabelPosition[landscape][1]), page.x * kFormationLabelPosition[landscape][2]);
    DrawLineOverText(dc, kGameLabel, wxPoint(page.x * kGameLabelPosition[landscape][0], page.y * kGameLabelPosition[landscape][1]), page.x * kGameLabelPosition[landscape][2]);
    DrawLineOverText(dc, kPageLabel, wxPoint(page.x * kPageLabelPosition[landscape][0], page.y * kPageLabelPosition[landscape][1]), page.x * kPageLabelPosition[landscape][2]);
    DrawCenteredText(dc, kSideLabel, wxPoint(page.x * kSideLabelPosition[landscape][0], page.y * kSideLabelPosition[landscape][1]));

    // draw arrows
    DrawCenteredText(dc, kUpperSouthLabel, wxPoint(page.x * kUpperSouthPosition[landscape][0], page.y * kUpperSouthPosition[landscape][1]));
    DrawArrow(dc, wxPoint(page.x * kUpperSouthArrow[landscape][0], page.y * kUpperSouthArrow[landscape][1]), page.x * kUpperSouthArrow[landscape][2], false);
    DrawCenteredText(dc, kUpperNorthLabel, wxPoint(page.x * kUpperNorthPosition[landscape][0], page.y * kUpperNorthPosition[landscape][1]));
    DrawArrow(dc, wxPoint(page.x * kUpperNorthArrow[landscape][0], page.y * kUpperNorthArrow[landscape][1]), page.x * kUpperNorthArrow[landscape][2], true);
    DrawCenteredText(dc, kLowerSouthLabel, wxPoint(page.x * kLowerSouthPosition[landscape][0], page.y * kLowerSouthPosition[landscape][1]));
    DrawArrow(dc, wxPoint(page.x * kLowerSouthArrow[landscape][0], page.y * kLowerSouthArrow[landscape][1]), page.x * kLowerSouthArrow[landscape][2], false);
    DrawCenteredText(dc, kLowerNorthLabel, wxPoint(page.x * kLowerNorthPosition[landscape][0], page.y * kLowerNorthPosition[landscape][1]));
    DrawArrow(dc, wxPoint(page.x * kLowerNorthArrow[landscape][0], page.y * kLowerNorthArrow[landscape][1]), page.x * kLowerNorthArrow[landscape][2], true);

    DrawCont(dc, sheet.GetPrintableContinuity(), wxRect(wxPoint(10, page.y * kContinuityStart[landscape]), wxSize(page.x - 20, page.y - page.y * kContinuityStart[landscape])), landscape);
}

void DrawForPrinting(wxDC* printerdc, CalChartConfiguration const& config, CalChartDoc const& show, CalChart::Sheet const& sheet, int ref, bool landscape)
{
    auto boundingBox = GetMarcherBoundingBox(sheet.GetPoints());
    auto forced_landscape = !landscape && (boundingBox.second.x - boundingBox.first.x) > Int2CoordUnits(CalChart::kFieldStepSizeNorthSouth[0]);

    auto bitmapWidth = (landscape || forced_landscape ? kSizeXLandscape : kSizeX) * kBitmapScale;
    auto bitmapHeight = (landscape || forced_landscape ? kSizeYLandscape : kSizeY) * kBitmapScale;
    // construct a bitmap for drawing on.
    wxBitmap membm(bitmapWidth, bitmapHeight);
    // first convert to image
    wxMemoryDC memdc(membm);
    DrawForPrintingHelper(memdc, config, show, sheet, ref, landscape || forced_landscape);

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

void DrawPointHelper(wxDC& dc, CalChartConfiguration const& config, CalChart::Coord const& pos, CalChart::Point const& point, SYMBOL_TYPE symbol, wxString const& label)
{
    SaveAndRestore_Brush restore(dc);
    switch (symbol) {
    case SYMBOL_SOL:
    case SYMBOL_SOLBKSL:
    case SYMBOL_SOLSL:
    case SYMBOL_SOLX:
        break;
    default:
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
    }

    auto const circ_r = fDIP(Float2CoordUnits(config.Get_DotRatio()) / 2.0);
    auto const plineoff = circ_r * config.Get_PLineRatio();
    auto const slineoff = circ_r * config.Get_SLineRatio();
    auto const textoff = circ_r * 1.25;

    auto where = fDIP(wxPoint{ pos.x, pos.y });
    dc.DrawCircle(where, circ_r);
    switch (symbol) {
    case SYMBOL_SL:
    case SYMBOL_X:
        dc.DrawLine(where.x - plineoff, where.y + plineoff, where.x + plineoff, where.y - plineoff);
        break;
    case SYMBOL_SOLSL:
    case SYMBOL_SOLX:
        dc.DrawLine(where.x - slineoff, where.y + slineoff, where.x + slineoff, where.y - slineoff);
        break;
    default:
        break;
    }
    switch (symbol) {
    case SYMBOL_BKSL:
    case SYMBOL_X:
        dc.DrawLine(where.x - plineoff, where.y - plineoff, where.x + plineoff, where.y + plineoff);
        break;
    case SYMBOL_SOLBKSL:
    case SYMBOL_SOLX:
        dc.DrawLine(where.x - slineoff, where.y - slineoff, where.x + slineoff, where.y + slineoff);
        break;
    default:
        break;
    }
    if (point.LabelIsVisible()) {
        wxCoord textw, texth, textd;
        dc.GetTextExtent(label, &textw, &texth, &textd);
        dc.DrawText(label, point.GetFlip() ? where.x : (where.x - textw), where.y - textoff - texth + textd);
    }
}

static void DrawPoint(wxDC& dc, CalChartConfiguration const& config, CalChart::Point const& point, SYMBOL_TYPE symbol, int reference, CalChart::Coord const& origin, wxString const& label)
{
    DrawPointHelper(dc, config, point.GetPos(reference) + origin, point, symbol, label);
}

void DrawPhatomPoints(wxDC& dc, const CalChartConfiguration& config,
    const CalChartDoc& show, const CalChart::Sheet& sheet,
    const std::map<int, CalChart::Coord>& positions)
{
    SaveAndRestore_Font orig_font(dc);
    auto pointLabelFont = CreateFont(Float2CoordUnits(config.Get_DotRatio() * config.Get_NumRatio()));
    dc.SetFont(pointLabelFont);
    auto origin = show.GetShowMode().Offset();
    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_GHOST_POINT);
    dc.SetPen(brushAndPen.second);
    dc.SetBrush(brushAndPen.first);
    dc.SetTextForeground(config.Get_CalChartBrushAndPen(COLOR_GHOST_POINT_TEXT).first.GetColour());

    for (auto& i : positions) {
        DrawPointHelper(dc, config, i.second + origin, sheet.GetPoint(i.first), sheet.GetSymbol(i.first), show.GetPointLabel(i.first));
    }
}

void DrawCC_DrawCommandList(wxDC& dc,
    const std::vector<CalChart::DrawCommand>& draw_commands)
{
    for (auto iter = draw_commands.begin(); iter != draw_commands.end(); ++iter) {
        switch (iter->mType) {
        case CalChart::DrawCommand::Line: {
            auto start = fDIP(wxPoint{ iter->x1, iter->y1 });
            auto end = fDIP(wxPoint{ iter->x2, iter->y2 });
            dc.DrawLine(start, end);
        } break;
        case CalChart::DrawCommand::Arc: {
            auto start = fDIP(wxPoint{ iter->x1, iter->y1 });
            auto end = fDIP(wxPoint{ iter->x2, iter->y2 });
            auto center = fDIP(wxPoint{ iter->xc, iter->yc });
            dc.DrawArc(start, end, center);
        } break;
        case CalChart::DrawCommand::Ellipse: {
            auto start = fDIP(wxPoint{ iter->x1, iter->y1 });
            auto end = fDIP(wxSize{ iter->x2 - iter->x1, iter->y2 - iter->y1 });
            dc.DrawEllipse(start, end);
        } break;
        case CalChart::DrawCommand::Ignore:
            break;
        }
    }
}

void DrawPath(wxDC& dc, const CalChartConfiguration& config,
    const std::vector<CalChart::DrawCommand>& draw_commands,
    const CalChart::Coord& end)
{
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_PATHS);
    dc.SetPen(brushAndPen.second);
    DrawCC_DrawCommandList(dc, draw_commands);
    dc.SetBrush(brushAndPen.first);
    auto where = fDIP(wxPoint{ end.x, end.y });
    float circ_r = fDIP(Float2CoordUnits(config.Get_DotRatio()) / 2);
    dc.DrawCircle(where, circ_r);
}

static void ShowModeStandard_DrawHelper_Labels(wxDC& dc, CalChartConfiguration const& config, std::string const* yard_text, wxSize const& fieldsize, wxPoint const& border1, HowToDraw howToDraw)
{
    // Draw labels
    dc.SetFont(CreateFont(Float2CoordUnits(config.Get_YardsSize()), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    for (int i = 0; i < CoordUnits2Int(tDIP(fieldsize.x)) / 8 + 1; i++) {
        auto text = yard_text[i];
        auto textSize = dc.GetTextExtent(text);
        const auto top_row_x = i * kStep8 - textSize.x / 2 + border1.x;
        const auto top_row_y = std::max(border1.y - textSize.y + ((howToDraw == ShowMode_kOmniView) ? kStep8 : 0), dc.DeviceToLogicalY(0));
        const auto bottom_row_x = top_row_x;
        const auto bottom_row_y = border1.y + fieldsize.y - ((howToDraw == ShowMode_kOmniView) ? kStep8 : 0);
        auto top_row = wxPoint{ top_row_x, top_row_y };
        auto bottom_row = wxPoint{ bottom_row_x, bottom_row_y };
        if (howToDraw != ShowMode_kPrinting) {
            // set color so when we make background box it blends
            dc.SetBrush(config.Get_CalChartBrushAndPen(COLOR_FIELD).first);
            dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_FIELD).second);
            dc.DrawRectangle(top_row, textSize);
        }
        dc.DrawText(text, top_row);
        dc.DrawText(text, bottom_row);
    }
}

static void ShowMode_DrawHelper_Outline(wxDC& dc, wxSize const& fieldsize, wxPoint const& border1)
{
    wxPoint points[5] = {
        { 0, 0 },
        { fieldsize.x, 0 },
        { fieldsize.x, fieldsize.y },
        { 0, fieldsize.y },
        { 0, 0 },
    };
    dc.DrawLines(5, points, border1.x, border1.y);
}

static void ShowMode_DrawHelper_VerticalSolid(wxDC& dc, wxSize const& fieldsize, wxPoint const& border1)
{
    for (auto j = 0; j <= fieldsize.x; j += kStep8) {
        // draw solid yardlines
        wxPoint points[2] = {
            { j, 0 },
            { j, fieldsize.y },
        };
        dc.DrawLines(2, points, border1.x, border1.y);
    }
}

static void ShowMode_DrawHelper_VerticalMidDotted(wxDC& dc, wxSize const& fieldsize, wxPoint const& border1)
{
    for (auto j = kStep4; j < fieldsize.x; j += kStep8) {
        // draw mid-dotted lines
        for (auto k = 0; k < fieldsize.y; k += kStep2) {
            wxPoint points[2] = {
                { j, k },
                { j, k + kStep1 },
            };
            dc.DrawLines(2, points, border1.x, border1.y);
        }
    }
}

static void ShowMode_DrawHelper_HorizontalMidDotted(wxDC& dc, wxSize const& fieldsize, wxPoint const& border1, int mode_HashW, int mode_HashE)
{
    for (auto j = kStep4; j < fieldsize.y; j += kStep4) {
        if ((j == Int2CoordUnits(mode_HashW)) || j == Int2CoordUnits(mode_HashE))
            continue;
        for (auto k = 0; k < fieldsize.x; k += kStep2) {
            wxPoint points[2] = {
                { k, j },
                { k + kStep1, j },
            };
            dc.DrawLines(2, points, border1.x, border1.y);
        }
    }
}

static void ShowMode_DrawHelper_Hashes(wxDC& dc, wxSize const& fieldsize, wxPoint const& border1, int mode_HashW, int mode_HashE)
{
    for (auto j = tDIP(Int2CoordUnits(0)); j < tDIP(fieldsize.x); j += tDIP(kStep8)) {
        wxPoint points[2] = {
            fDIP(wxPoint{ j + Float2CoordUnits(0.0 * 8), Int2CoordUnits(mode_HashW) }),
            fDIP(wxPoint{ j + Float2CoordUnits(0.1 * 8), Int2CoordUnits(mode_HashW) }),
        };
        dc.DrawLines(2, points, border1.x, border1.y);
        points[0] = fDIP(wxPoint(j + Float2CoordUnits(0.9 * 8), Int2CoordUnits(mode_HashW)));
        points[1] = fDIP(wxPoint(j + Float2CoordUnits(1.0 * 8), Int2CoordUnits(mode_HashW)));
        dc.DrawLines(2, points, border1.x, border1.y);

        points[0] = fDIP(wxPoint(j + Float2CoordUnits(0.0 * 8), Int2CoordUnits(mode_HashE)));
        points[1] = fDIP(wxPoint(j + Float2CoordUnits(0.1 * 8), Int2CoordUnits(mode_HashE)));
        dc.DrawLines(2, points, border1.x, border1.y);
        points[0] = fDIP(wxPoint(j + Float2CoordUnits(0.9 * 8), Int2CoordUnits(mode_HashE)));
        points[1] = fDIP(wxPoint(j + Float2CoordUnits(1.0 * 8), Int2CoordUnits(mode_HashE)));
        dc.DrawLines(2, points, border1.x, border1.y);
    }
}

static void ShowMode_DrawHelper_HashTicks(wxDC& dc, wxSize const& fieldsize, wxPoint const& border1, int mode_HashW, int mode_HashE)
{
    for (auto j = tDIP(Int2CoordUnits(0)); j < tDIP(fieldsize.x); j += tDIP(kStep8)) {
        for (size_t midhash = 1; midhash < 5; ++midhash) {
            wxPoint points[2] = {
                fDIP(wxPoint{ j + Float2CoordUnits(midhash / 5.0 * 8), Int2CoordUnits(mode_HashW) }),
                fDIP(wxPoint{ j + Float2CoordUnits(midhash / 5.0 * 8), Float2CoordUnits(mode_HashW - (0.2 * 8)) }),
            };
            dc.DrawLines(2, points, border1.x, border1.y);

            points[0] = fDIP(wxPoint(j + Float2CoordUnits(midhash / 5.0 * 8), Int2CoordUnits(mode_HashE)));
            points[1] = fDIP(wxPoint(j + Float2CoordUnits(midhash / 5.0 * 8), Float2CoordUnits(mode_HashE + (0.2 * 8))));
            dc.DrawLines(2, points, border1.x, border1.y);
        }
    }
}

static void ShowModeStandard_DrawHelper(wxDC& dc, CalChartConfiguration const& config, CalChart::ShowMode const& mode, HowToDraw howToDraw)
{
    auto tfieldsize = mode.FieldSize();
    auto tborder1 = (howToDraw == ShowMode_kOmniView) ? CalChart::Coord(0, 0) : mode.Border1();
    auto fieldsize = fDIP(wxSize{ tfieldsize.x, tfieldsize.y });
    auto border1 = fDIP(wxPoint{ tborder1.x, tborder1.y });

    ShowMode_DrawHelper_Outline(dc, fieldsize, border1);

    ShowMode_DrawHelper_VerticalSolid(dc, fieldsize, border1);

    if (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) {
        ShowMode_DrawHelper_VerticalMidDotted(dc, fieldsize, border1);
    }

    if (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) {
        ShowMode_DrawHelper_HorizontalMidDotted(dc, fieldsize, border1, mode.HashW(), mode.HashE());
    }

    if (mode.HashW() != static_cast<unsigned short>(-1)) {
        ShowMode_DrawHelper_Hashes(dc, fieldsize, border1, mode.HashW(), mode.HashE());
        if (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) {
            ShowMode_DrawHelper_HashTicks(dc, fieldsize, border1, mode.HashW(), mode.HashE());
        }
    }

    // Draw labels
    if (howToDraw == ShowMode_kAnimation) {
        return;
    }
    auto yard_text = mode.Get_yard_text().data();
    yard_text += (-CoordUnits2Int((mode.Offset() - mode.Border1()).x) + (CalChart::kYardTextValues - 1) * 4) / 8;

    ShowModeStandard_DrawHelper_Labels(dc, config, yard_text, fieldsize, border1, howToDraw);
}

void DrawMode(wxDC& dc, CalChartConfiguration const& config, CalChart::ShowMode const& mode, HowToDraw howToDraw)
{
    switch (howToDraw) {
    case ShowMode_kFieldView:
    case ShowMode_kAnimation:
    case ShowMode_kOmniView:
        dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_FIELD_DETAIL).second);
        dc.SetTextForeground(config.Get_CalChartBrushAndPen(COLOR_FIELD_TEXT).second.GetColour());
        break;
    case ShowMode_kPrinting:
        dc.SetPen(*wxBLACK_PEN);
        dc.SetTextForeground(*wxBLACK);
        break;
    }
    ShowModeStandard_DrawHelper(dc, config, mode, howToDraw);
}

wxImage GetOmniLinesImage(const CalChartConfiguration& config, const CalChart::ShowMode& mode)
{
    auto fieldsize = mode.FieldSize();
    wxBitmap bmp(fieldsize.x, fieldsize.y, 32);
    wxMemoryDC dc;
    dc.SelectObject(bmp);
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    DrawMode(dc, config, mode, ShowMode_kOmniView);
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
