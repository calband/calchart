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

#include <wx/dc.h>
#include <wx/dcmemory.h>
#include "confgr.h"
#include "calchartdoc.h"
#include "cc_show.h"
#include "cc_sheet.h"
#include "cc_point.h"
#include "cc_text.h"
#include "animate.h"
#include "animatecommand.h"
#include "draw_utils.h"
#include "cc_drawcommand.h"
#include "modes.h"
#include <memory>

extern wxFont* pointLabelFont;

static void DrawPoint(wxDC& dc, const CalChartConfiguration& config,
    const CalChart::Point& point, unsigned reference,
    const CalChart::Coord& origin, const wxString& label);
static void DrawPointHelper(wxDC& dc, const CalChartConfiguration& config,
    const CalChart::Coord& pos, const CalChart::Point& point,
    const wxString& label);

// draw text centered around x (though still at y down)
void DrawCenteredText(wxDC& dc, const wxString& text, const wxPoint& pt)
{
    wxCoord w, h;
    dc.GetTextExtent(text, &w, &h);
    w = std::max(0, pt.x - w / 2);
    dc.DrawText(text, w, pt.y);
}

// draw text centered around x (though still at y down) with a line over it.
void DrawLineOverText(wxDC& dc, const wxString& text, const wxPoint& pt,
    wxCoord lineLength)
{
    DrawCenteredText(dc, text, pt + wxPoint(0, 2));
    dc.DrawLine(pt - wxPoint(lineLength / 2, 0), pt + wxPoint(lineLength / 2, 0));
}

static const size_t ArrowSize = 4;
void DrawArrow(wxDC& dc, const wxPoint& pt, wxCoord lineLength,
    bool pointRight)
{
    dc.DrawLine(pt + wxPoint(-lineLength / 2, ArrowSize),
        pt + wxPoint(lineLength / 2, ArrowSize));
    if (pointRight) {
        dc.DrawLine(pt + wxPoint(lineLength / 2 - (wxCoord)ArrowSize, 0),
            pt + wxPoint(lineLength / 2, ArrowSize));
        dc.DrawLine(pt + wxPoint(lineLength / 2 - (wxCoord)ArrowSize, ArrowSize * 2),
            pt + wxPoint(lineLength / 2, ArrowSize));
    }
    else {
        dc.DrawLine(pt + wxPoint(-(lineLength / 2 - (wxCoord)ArrowSize), 0),
            pt + wxPoint(-lineLength / 2, ArrowSize));
        dc.DrawLine(
            pt + wxPoint(-(lineLength / 2 - (wxCoord)ArrowSize), ArrowSize * 2),
            pt + wxPoint(-lineLength / 2, ArrowSize));
    }
}

// calculate the distance for tab stops
auto TabStops(size_t which, bool landscape)
{
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
static const double kBitmapScale = 2.0; // the factor to scale the bitmap
static const double kFieldTop = 0.14;
static const double kFieldBorderOffset = 0.06;
static const double kSizeX = 576, kSizeY = 734;
static const double kSizeXLandscape = 917, kSizeYLandscape = 720;
static const double kHeaderLocation[2][2] = { { 0.5, 18 / kSizeY },
    { 0.5, 22 / kSizeYLandscape } };
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
    { 1.0 - 124 / kSizeXLandscape, 674 / kSizeYLandscape, 56 / kSizeXLandscape,
        28 / kSizeYLandscape }
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
    { 1.0 - 76 / kSizeXLandscape, (536) / kSizeYLandscape,
        40 / kSizeXLandscape }
};

static const double kContinuityStart[2] = { 606 / kSizeY, 556 / kSizeYLandscape };

void DrawSheetPoints(
    wxDC& dc, const CalChartConfiguration& config, CalChart::Coord origin,
    const SelectionList& selection_list, unsigned short numberPoints,
    const std::vector<std::string>& labels, const CalChart::Sheet& sheet, unsigned ref,
    CalChartColors unselectedColor, CalChartColors selectedColor,
    CalChartColors unselectedTextColor, CalChartColors selectedTextColor)
{
    SaveAndRestore_Font orig_font(dc);
    wxFont* pointLabelFont = wxTheFontList->FindOrCreateFont((int)Float2CoordUnits(config.Get_DotRatio() * config.Get_NumRatio()), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(*pointLabelFont);
    dc.SetTextForeground(
        config.Get_CalChartBrushAndPen(COLOR_POINT_TEXT).first.GetColour());
    for (unsigned i = 0; i < numberPoints; i++) {
        wxBrush fillBrush;
        if (selection_list.count(i)) {
            auto brushAndPen = config.Get_CalChartBrushAndPen(selectedColor);
            fillBrush = brushAndPen.first;
            dc.SetBrush(brushAndPen.first);
            dc.SetPen(brushAndPen.second);
            dc.SetTextForeground(
                config.Get_CalChartBrushAndPen(selectedTextColor).second.GetColour());
        }
        else {
            auto brushAndPen = config.Get_CalChartBrushAndPen(unselectedColor);
            fillBrush = brushAndPen.first;
            dc.SetBrush(brushAndPen.first);
            dc.SetPen(brushAndPen.second);
            dc.SetTextForeground(config.Get_CalChartBrushAndPen(unselectedTextColor)
                                     .second.GetColour());
        }
        DrawPoint(dc, config, sheet.GetPoint(i), ref, origin, labels.at(i));
    }
}

void DrawGhostSheet(wxDC& dc, const CalChartConfiguration& config,
    CalChart::Coord origin, const SelectionList& selection_list,
    unsigned short numberPoints,
    const std::vector<std::string>& labels,
    const CalChart::Sheet& sheet, unsigned ref)
{
    DrawSheetPoints(dc, config, origin, selection_list, numberPoints, labels,
        sheet, ref, COLOR_GHOST_POINT, COLOR_GHOST_POINT_HLIT,
        COLOR_GHOST_POINT_TEXT, COLOR_GHOST_POINT_HLIT_TEXT);
}

void DrawPoints(wxDC& dc, const CalChartConfiguration& config, CalChart::Coord origin,
    const SelectionList& selection_list,
    unsigned short numberPoints,
    const std::vector<std::string>& labels, const CalChart::Sheet& sheet,
    unsigned ref, bool primary)
{
    CalChartColors unselectedColor, selectedColor, unselectedTextColor,
        selectedTextColor;
    if (primary) {
        unselectedColor = COLOR_POINT;
        selectedColor = COLOR_POINT_HILIT;
        unselectedTextColor = COLOR_POINT_TEXT;
        selectedTextColor = COLOR_POINT_HILIT_TEXT;
    }
    else {
        unselectedColor = COLOR_REF_POINT;
        selectedColor = COLOR_REF_POINT_HILIT;
        unselectedTextColor = COLOR_REF_POINT_TEXT;
        selectedTextColor = COLOR_REF_POINT_HILIT_TEXT;
    }
    DrawSheetPoints(dc, config, origin, selection_list, numberPoints, labels,
        sheet, ref, unselectedColor, selectedColor,
        unselectedTextColor, selectedTextColor);
}

// draw the continuity starting at a specific offset
void DrawCont(wxDC& dc, const CalChart::Textline_list& print_continuity,
    const wxRect& bounding, bool landscape)
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

    auto numLines = 0;
    for (auto text = print_continuity.begin(); text != print_continuity.end();
         ++text) {
        if (text->GetOnSheet()) {
            ++numLines;
        }
    }

    auto font_size = ((bounding.GetBottom() - bounding.GetTop()) - (numLines - 1) * 2) / (numLines ? numLines : 1);
    // font size, we scale to be no more than 256 pixels.
    font_size = std::min(font_size, 10);

    wxFont* contPlainFont = wxTheFontList->FindOrCreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxFont* contBoldFont = wxTheFontList->FindOrCreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    wxFont* contItalFont = wxTheFontList->FindOrCreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);
    wxFont* contBoldItalFont = wxTheFontList->FindOrCreateFont(font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD);

    dc.SetFont(*contPlainFont);
    const wxCoord maxtexth = contPlainFont->GetPointSize() + 2;

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
                case PSFONT_SYMBOL: {
                    wxCoord textw, texth;
                    dc.GetTextExtent(wxT("O"), &textw, &texth);
                    x += textw * c.text.length();
                } break;
                case PSFONT_NORM:
                    dc.SetFont(*contPlainFont);
                    break;
                case PSFONT_BOLD:
                    dc.SetFont(*contBoldFont);
                    break;
                case PSFONT_ITAL:
                    dc.SetFont(*contItalFont);
                    break;
                case PSFONT_BOLDITAL:
                    dc.SetFont(*contBoldItalFont);
                    break;
                case PSFONT_TAB:
                    do_tab = true;
                    break;
                }
                if (!do_tab && (c.font != PSFONT_SYMBOL)) {
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
            case PSFONT_NORM:
            case PSFONT_SYMBOL:
                dc.SetFont(*contPlainFont);
                break;
            case PSFONT_BOLD:
                dc.SetFont(*contBoldFont);
                break;
            case PSFONT_ITAL:
                dc.SetFont(*contItalFont);
                break;
            case PSFONT_BOLDITAL:
                dc.SetFont(*contBoldItalFont);
                break;
            case PSFONT_TAB: {
                tabnum++;
                wxCoord textw = bounding.GetLeft() + charWidth * TabStops(tabnum, landscape);
                if (textw >= x)
                    x = textw;
                else
                    x += charWidth;
                do_tab = true;
            } break;
            default:
                break;
            }
            if (c.font == PSFONT_SYMBOL) {
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
            }
            else {
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
    snprintf(buffer, sizeof(buffer), "TopLeft %d, %d", bounding.GetTopLeft().x,
        bounding.GetTopLeft().y);
    dc.DrawText(buffer, bounding.GetTopLeft());
    snprintf(buffer, sizeof(buffer), "BottomRight %d, %d",
        bounding.GetBottomRight().x, bounding.GetBottomRight().y);
    dc.DrawText(buffer, bounding.GetBottomRight());
#endif
}

static std::unique_ptr<CalChart::ShowMode>
CreateFieldForPrinting(int left_limit, int right_limit, bool landscape)
{
    CalChart::Coord siz = { Int2CoordUnits(CalChart::kFieldStepSizeNorthSouth[landscape]),
        Int2CoordUnits(CalChart::kFieldStepSizeEastWest) };

    // extend the limit to the next largest 5 yard line
    left_limit = (left_limit / 8) * 8 + (left_limit % 8 ? (left_limit < 0 ? -8 : 8) : 0);
    right_limit = (right_limit / 8) * 8 + (right_limit % 8 ? (right_limit < 0 ? -8 : 8) : 0);
    auto left_edge = -CalChart::kFieldStepSizeSouthEdgeFromCenter[landscape];
    if (left_limit < left_edge) {
        left_edge = left_limit;
    }
    else if ((left_edge + siz.x) < right_limit) {
        left_edge = right_limit - siz.x;
    }
    CalChart::Coord off = { Int2CoordUnits(-left_edge),
        Int2CoordUnits(CalChart::kFieldStepSizeWestEdgeFromCenter) };

    return CalChart::ShowModeStandard::CreateShowMode("Standard", siz, off, { 0, 0 }, { 0, 0 },
        CalChart::kFieldStepWestHashFromWestSideline,
        CalChart::kFieldStepEastHashFromWestSideline);
}

// Return a bounding box of the show of where the marchers are.  If they are
// outside the show, we don't see them.
std::pair<CalChart::Coord, CalChart::Coord>
GetMarcherBoundingBox(const std::vector<CalChart::Point>& pts)
{
    CalChart::Coord bounding_box_upper_left{ 10000, 10000 };
    CalChart::Coord bounding_box_low_right{ -10000, -10000 };

    for (auto& i : pts) {
        auto position = i.GetPos();
        bounding_box_upper_left = CalChart::Coord(std::min(bounding_box_upper_left.x, position.x),
            std::min(bounding_box_upper_left.y, position.y));
        bounding_box_low_right = CalChart::Coord(std::max(bounding_box_low_right.x, position.x),
            std::max(bounding_box_low_right.y, position.y));
    }

    return { bounding_box_upper_left, bounding_box_low_right };
}

void DrawForPrintingHelper(wxDC& dc, const CalChartConfiguration& config,
    const CalChartDoc& show, const CalChart::Sheet& sheet,
    unsigned ref, bool landscape)
{
    // set up everything to be restored after we print
    SaveAndRestore_DeviceOrigin orig_dev(dc);
    SaveAndRestore_UserScale orig_scale(dc);
    SaveAndRestore_Font orig_font(dc);

    // get the page dimensions
    int pageW, pageH;
    dc.GetSize(&pageW, &pageH);

    dc.Clear();
    dc.SetLogicalFunction(wxCOPY);

    // Print the field:
    // create a field for drawing:
    const auto pts = sheet.GetPoints();
    auto boundingBox = GetMarcherBoundingBox(pts);
    auto mode = CreateFieldForPrinting(CoordUnits2Int(boundingBox.first.x),
        CoordUnits2Int(boundingBox.second.x), landscape);

    // set the origin and scaling for drawing the field
    dc.SetDeviceOrigin(kFieldBorderOffset * pageW, kFieldTop * pageH);
    auto scale = (pageW - 2 * kFieldBorderOffset * pageW) / (double)mode->Size().x;
    dc.SetUserScale(scale, scale);

    // draw the field.
    DrawMode(dc, config, *mode, ShowMode_kPrinting);
    wxFont* pointLabelFont = wxTheFontList->FindOrCreateFont(
        (int)Float2CoordUnits(config.Get_DotRatio() * config.Get_NumRatio()), wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(*pointLabelFont);
    for (auto i = 0u; i < pts.size(); i++) {
        const auto point = pts.at(i);
        const auto pos = point.GetPos(ref) + mode->Offset();
        dc.SetBrush(*wxBLACK_BRUSH);
        DrawPointHelper(dc, config, pos, point, show.GetPointLabel(i));
    }

    // now reset everything to draw the rest of the text
    dc.SetDeviceOrigin(Int2CoordUnits(0), Int2CoordUnits(0));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    // set the page for drawing:
    dc.GetSize(&pageW, &pageH);
    if (landscape) {
        dc.SetUserScale(pageW / kSizeXLandscape, pageH / kSizeYLandscape);
        pageW = kSizeXLandscape;
        pageH = kSizeYLandscape;
    }
    else {
        dc.SetUserScale(pageW / kSizeX, pageH / kSizeY);
        pageW = kSizeX;
        pageH = kSizeY;
    }

    // draw the header
    dc.SetFont(*wxTheFontList->FindOrCreateFont(16, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 1, wxPENSTYLE_SOLID));

    DrawCenteredText(dc, kHeader, wxPoint(pageW * kHeaderLocation[landscape][0],
                                      pageH * kHeaderLocation[landscape][1]));

    DrawCenteredText(dc, sheet.GetNumber(),
        wxPoint(pageW * kUpperNumberPosition[landscape][0],
                         pageH * kUpperNumberPosition[landscape][1]));
    DrawCenteredText(dc, sheet.GetNumber(),
        wxPoint(pageW * kLowerNumberPosition[landscape][0],
                         pageH * kLowerNumberPosition[landscape][1]));
    dc.DrawRectangle(pageW * kLowerNumberBox[landscape][0],
        pageH * kLowerNumberBox[landscape][1],
        pageW * kLowerNumberBox[landscape][2],
        pageH * kLowerNumberBox[landscape][3]);

    dc.SetFont(*wxTheFontList->FindOrCreateFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    DrawLineOverText(dc, kMusicLabel,
        wxPoint(pageW * kMusicLabelPosition[landscape][0],
                         pageH * kMusicLabelPosition[landscape][1]),
        pageW * kMusicLabelPosition[landscape][2]);
    DrawLineOverText(dc, kFormationLabel,
        wxPoint(pageW * kFormationLabelPosition[landscape][0],
                         pageH * kFormationLabelPosition[landscape][1]),
        pageW * kFormationLabelPosition[landscape][2]);
    DrawLineOverText(dc, kGameLabel,
        wxPoint(pageW * kGameLabelPosition[landscape][0],
                         pageH * kGameLabelPosition[landscape][1]),
        pageW * kGameLabelPosition[landscape][2]);
    DrawLineOverText(dc, kPageLabel,
        wxPoint(pageW * kPageLabelPosition[landscape][0],
                         pageH * kPageLabelPosition[landscape][1]),
        pageW * kPageLabelPosition[landscape][2]);
    DrawCenteredText(dc, kSideLabel,
        wxPoint(pageW * kSideLabelPosition[landscape][0],
                         pageH * kSideLabelPosition[landscape][1]));

    // draw arrows
    DrawCenteredText(dc, kUpperSouthLabel,
        wxPoint(pageW * kUpperSouthPosition[landscape][0],
                         pageH * kUpperSouthPosition[landscape][1]));
    DrawArrow(dc, wxPoint(pageW * kUpperSouthArrow[landscape][0],
                      pageH * kUpperSouthArrow[landscape][1]),
        pageW * kUpperSouthArrow[landscape][2], false);
    DrawCenteredText(dc, kUpperNorthLabel,
        wxPoint(pageW * kUpperNorthPosition[landscape][0],
                         pageH * kUpperNorthPosition[landscape][1]));
    DrawArrow(dc, wxPoint(pageW * kUpperNorthArrow[landscape][0],
                      pageH * kUpperNorthArrow[landscape][1]),
        pageW * kUpperNorthArrow[landscape][2], true);
    DrawCenteredText(dc, kLowerSouthLabel,
        wxPoint(pageW * kLowerSouthPosition[landscape][0],
                         pageH * kLowerSouthPosition[landscape][1]));
    DrawArrow(dc, wxPoint(pageW * kLowerSouthArrow[landscape][0],
                      pageH * kLowerSouthArrow[landscape][1]),
        pageW * kLowerSouthArrow[landscape][2], false);
    DrawCenteredText(dc, kLowerNorthLabel,
        wxPoint(pageW * kLowerNorthPosition[landscape][0],
                         pageH * kLowerNorthPosition[landscape][1]));
    DrawArrow(dc, wxPoint(pageW * kLowerNorthArrow[landscape][0],
                      pageH * kLowerNorthArrow[landscape][1]),
        pageW * kLowerNorthArrow[landscape][2], true);

    DrawCont(
        dc, sheet.GetPrintableContinuity(),
        wxRect(wxPoint(10, pageH * kContinuityStart[landscape]),
            wxSize(pageW - 20, pageH - pageH * kContinuityStart[landscape])),
        landscape);
}

void DrawForPrinting(wxDC* printerdc, const CalChartConfiguration& config,
    const CalChartDoc& show, const CalChart::Sheet& sheet,
    unsigned ref, bool landscape)
{
    auto boundingBox = GetMarcherBoundingBox(sheet.GetPoints());
    bool forced_landscape = !landscape && (boundingBox.second.x - boundingBox.first.x) > Int2CoordUnits(CalChart::kFieldStepSizeNorthSouth[0]);

    auto bitmapWidth = (landscape || forced_landscape ? kSizeXLandscape : kSizeX) * kBitmapScale;
    auto bitmapHeight = (landscape || forced_landscape ? kSizeYLandscape : kSizeY) * kBitmapScale;
    // construct a bitmap for drawing on.  This should
    wxBitmap membm(bitmapWidth, bitmapHeight);
    // first convert to image
    wxMemoryDC memdc(membm);
    DrawForPrintingHelper(memdc, config, show, sheet, ref,
        landscape || forced_landscape);

    auto image = membm.ConvertToImage();
    if (forced_landscape) {
        image = image.Rotate90(false);
    }
    wxBitmap rotate_membm(image);
    wxMemoryDC tmemdc(rotate_membm);

    int printerW, printerH;
    printerdc->GetSize(&printerW, &printerH);
    auto scaleX = printerW / float(rotate_membm.GetWidth());
    auto scaleY = printerH / float(rotate_membm.GetHeight());
    printerdc->SetUserScale(scaleX, scaleY);
    printerdc->Blit(0, 0, rotate_membm.GetWidth(), rotate_membm.GetHeight(),
        &tmemdc, 0, 0);
}

void DrawPointHelper(wxDC& dc, const CalChartConfiguration& config,
    const CalChart::Coord& pos, const CalChart::Point& point,
    const wxString& label)
{
    SaveAndRestore_Brush restore(dc);
    const float circ_r = Float2CoordUnits(config.Get_DotRatio());
    const float offset = circ_r / 2;
    const float plineoff = offset * config.Get_PLineRatio();
    const float slineoff = offset * config.Get_SLineRatio();
    float textoff = offset * 1.25;

    auto x = pos.x;
    auto y = pos.y;
    switch (point.GetSymbol()) {
    case SYMBOL_SOL:
    case SYMBOL_SOLBKSL:
    case SYMBOL_SOLSL:
    case SYMBOL_SOLX:
        break;
    default:
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
    }
    dc.DrawEllipse(x - offset, y - offset, circ_r, circ_r);
    switch (point.GetSymbol()) {
    case SYMBOL_SL:
    case SYMBOL_X:
        dc.DrawLine(x - plineoff, y + plineoff, x + plineoff, y - plineoff);
        break;
    case SYMBOL_SOLSL:
    case SYMBOL_SOLX:
        dc.DrawLine(x - slineoff, y + slineoff, x + slineoff, y - slineoff);
        break;
    default:
        break;
    }
    switch (point.GetSymbol()) {
    case SYMBOL_BKSL:
    case SYMBOL_X:
        dc.DrawLine(x - plineoff, y - plineoff, x + plineoff, y + plineoff);
        break;
    case SYMBOL_SOLBKSL:
    case SYMBOL_SOLX:
        dc.DrawLine(x - slineoff, y - slineoff, x + slineoff, y + slineoff);
        break;
    default:
        break;
    }
    if (point.LabelIsVisible()) {
        wxCoord textw, texth, textd;
        dc.GetTextExtent(label, &textw, &texth, &textd);
        dc.DrawText(label, point.GetFlip() ? x : (x - textw),
            y - textoff - texth + textd);
    }
}

void DrawPoint(wxDC& dc, const CalChartConfiguration& config,
    const CalChart::Point& point, unsigned reference,
    const CalChart::Coord& origin, const wxString& label)
{
    DrawPointHelper(dc, config, point.GetPos(reference) + origin, point, label);
}

void DrawPhatomPoints(wxDC& dc, const CalChartConfiguration& config,
    const CalChartDoc& show, const CalChart::Sheet& sheet,
    const std::map<int, CalChart::Coord>& positions)
{
    SaveAndRestore_Font orig_font(dc);
    wxFont* pointLabelFont = wxTheFontList->FindOrCreateFont(
        (int)Float2CoordUnits(config.Get_DotRatio() * config.Get_NumRatio()), wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(*pointLabelFont);
    auto origin = show.GetMode().Offset();
    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_GHOST_POINT);
    dc.SetPen(brushAndPen.second);
    dc.SetBrush(brushAndPen.first);
    dc.SetTextForeground(
        config.Get_CalChartBrushAndPen(COLOR_GHOST_POINT_TEXT).first.GetColour());

    for (auto& i : positions) {
        DrawPointHelper(dc, config, i.second + origin, sheet.GetPoint(i.first),
            show.GetPointLabel(i.first));
    }
}

void DrawCC_DrawCommandList(wxDC& dc,
    const std::vector<CalChart::DrawCommand>& draw_commands)
{
    for (auto iter = draw_commands.begin(); iter != draw_commands.end(); ++iter) {
        switch (iter->mType) {
        case CalChart::DrawCommand::Line:
            dc.DrawLine(iter->x1, iter->y1, iter->x2, iter->y2);
            break;
        case CalChart::DrawCommand::Arc:
            dc.DrawArc(iter->x1, iter->y1, iter->x2, iter->y2, iter->xc, iter->yc);
            break;
        case CalChart::DrawCommand::Ellipse:
            dc.DrawEllipse(iter->x1, iter->y1, iter->x2 - iter->x1, iter->y2 - iter->y1);
            break;
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
    float circ_r = Float2CoordUnits(config.Get_DotRatio());
    dc.DrawEllipse(end.x - circ_r / 2, end.y - circ_r / 2, circ_r, circ_r);
}

void ShowModeStandard_DrawHelper(wxDC& dc, const CalChartConfiguration& config,
    const CalChart::ShowModeStandard& mode,
    HowToDraw howToDraw)
{
    wxPoint points[5];
    auto fieldsize = mode.FieldSize();
    auto border1 = mode.Border1();
    if (howToDraw == ShowMode_kOmniView) {
        border1 = CalChart::Coord(0, 0);
    }
    auto offsetx = 0; //-fieldsize.x/2;
    auto offsety = 0; //-fieldsize.y/2;
    auto borderoffsetx = 0; //-border1.x;
    auto borderoffsety = 0; //-border1.y;

    points[0] = wxPoint(0 + offsetx, 0 + offsety);
    points[1] = wxPoint(fieldsize.x + offsetx, 0 + offsety);
    points[2] = wxPoint(fieldsize.x + offsetx, fieldsize.y + offsety);
    points[3] = wxPoint(0 + offsetx, fieldsize.y + offsety);
    points[4] = points[0];

    // Draw outline
    dc.DrawLines(5, points, border1.x + borderoffsetx, border1.y + borderoffsety);

    // Draw vertical lines
    for (auto j = Int2CoordUnits(8) + offsetx; j < fieldsize.x + offsetx;
         j += Int2CoordUnits(8)) {
        // draw solid yardlines
        points[0] = wxPoint(j, 0 + offsety);
        points[1] = wxPoint(j, fieldsize.y + offsety);
        dc.DrawLines(2, points, border1.x + borderoffsetx,
            border1.y + borderoffsety);
    }

    for (auto j = Int2CoordUnits(4) + offsetx;
         (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) && j < fieldsize.x + offsetx;
         j += Int2CoordUnits(8)) {
        // draw mid-dotted lines
        for (auto k = 0 + offsety; k < fieldsize.y + offsety; k += Int2CoordUnits(2)) {
            points[0] = wxPoint(j, k);
            points[1] = wxPoint(j, k + Int2CoordUnits(1));
            dc.DrawLines(2, points, border1.x + borderoffsetx,
                border1.y + borderoffsety);
        }
    }

    // Draw horizontal mid-dotted lines
    for (auto j = Int2CoordUnits(4) + offsety;
         (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) && j < fieldsize.y + offsety;
         j += Int2CoordUnits(4)) {
        if ((j == Int2CoordUnits(mode.HashW())) || j == Int2CoordUnits(mode.HashE()))
            continue;
        for (auto k = 0 + offsetx; k < fieldsize.x + offsetx; k += Int2CoordUnits(2)) {
            points[0] = wxPoint(k, j);
            points[1] = wxPoint(k + Int2CoordUnits(1), j);
            dc.DrawLines(2, points, border1.x + borderoffsetx,
                border1.y + borderoffsety);
        }
    }

    // Draw hashes
    for (auto j = Int2CoordUnits(0) + offsetx; j < fieldsize.x + offsetx;
         j += Int2CoordUnits(8)) {
        points[0] = wxPoint(j + Float2CoordUnits(0.0 * 8), Int2CoordUnits(mode.HashW()));
        points[1] = wxPoint(j + Float2CoordUnits(0.1 * 8), Int2CoordUnits(mode.HashW()));
        dc.DrawLines(2, points, border1.x + borderoffsetx,
            border1.y + borderoffsety);
        points[0] = wxPoint(j + Float2CoordUnits(0.9 * 8), Int2CoordUnits(mode.HashW()));
        points[1] = wxPoint(j + Float2CoordUnits(1.0 * 8), Int2CoordUnits(mode.HashW()));
        dc.DrawLines(2, points, border1.x + borderoffsetx,
            border1.y + borderoffsety);

        points[0] = wxPoint(j + Float2CoordUnits(0.0 * 8), Int2CoordUnits(mode.HashE()));
        points[1] = wxPoint(j + Float2CoordUnits(0.1 * 8), Int2CoordUnits(mode.HashE()));
        dc.DrawLines(2, points, border1.x + borderoffsetx,
            border1.y + borderoffsety);
        points[0] = wxPoint(j + Float2CoordUnits(0.9 * 8), Int2CoordUnits(mode.HashE()));
        points[1] = wxPoint(j + Float2CoordUnits(1.0 * 8), Int2CoordUnits(mode.HashE()));
        dc.DrawLines(2, points, border1.x + borderoffsetx,
            border1.y + borderoffsety);

        for (size_t midhash = 1; (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) && midhash < 5;
             ++midhash) {
            points[0] = wxPoint(j + Float2CoordUnits(midhash / 5.0 * 8), Int2CoordUnits(mode.HashW()));
            points[1] = wxPoint(j + Float2CoordUnits(midhash / 5.0 * 8),
                Float2CoordUnits(mode.HashW() - (0.2 * 8)));
            dc.DrawLines(2, points, border1.x + borderoffsetx,
                border1.y + borderoffsety);

            points[0] = wxPoint(j + Float2CoordUnits(midhash / 5.0 * 8), Int2CoordUnits(mode.HashE()));
            points[1] = wxPoint(j + Float2CoordUnits(midhash / 5.0 * 8),
                Float2CoordUnits(mode.HashE() + (0.2 * 8)));
            dc.DrawLines(2, points, border1.x + borderoffsetx,
                border1.y + borderoffsety);
        }
    }

    // Draw labels
    wxFont* yardLabelFont = wxTheFontList->FindOrCreateFont(
        (int)Float2CoordUnits(config.Get_YardsSize()), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(*yardLabelFont);
    for (int i = 0;
         (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kOmniView || howToDraw == ShowMode_kPrinting) && i < CoordUnits2Int(fieldsize.x) / 8 + 1;
         i++) {
        auto fieldedge = mode.Offset() - mode.Border1();
        wxCoord textw, texth, textd;
        auto text = config.Get_yard_text(
            i + (-CoordUnits2Int(fieldedge.x) + (CalChart::kYardTextValues - 1) * 4) / 8);
        dc.GetTextExtent(text, &textw, &texth, &textd);
        dc.DrawText(text, offsetx + Int2CoordUnits(i * 8) - textw / 2 + border1.x + borderoffsetx,
            border1.y + borderoffsety - offsety - texth + ((howToDraw == ShowMode_kOmniView) ? Int2CoordUnits(8) : 0));
        dc.DrawText(text, offsetx + Int2CoordUnits(i * 8) - textw / 2 + border1.x + borderoffsetx,
            border1.y + borderoffsety + fieldsize.y - offsety - ((howToDraw == ShowMode_kOmniView) ? Int2CoordUnits(8) : 0));
    }
}

void ShowModeSprShow_DrawHelper(wxDC& dc, const CalChartConfiguration& config,
    const CalChart::ShowModeSprShow& mode,
    HowToDraw howToDraw)
{
    wxPoint points[2];
    auto fieldsize = mode.FieldSize();

    // Draw vertical lines
    for (auto j = 0; j <= fieldsize.x; j += Int2CoordUnits(8)) {
        // draw solid yardlines
        points[0] = wxPoint(j, 0);
        points[1] = wxPoint(j, fieldsize.y);
        dc.DrawLines(2, points, mode.Border1().x, mode.Border1().y);
    }

    for (auto j = Int2CoordUnits(4);
         (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) && j < fieldsize.x;
         j += Int2CoordUnits(8)) {
        // draw mid-dotted lines
        for (auto k = 0; k < fieldsize.y; k += Int2CoordUnits(2)) {
            points[0] = wxPoint(j, k);
            points[1] = wxPoint(j, k + Int2CoordUnits(1));
            dc.DrawLines(2, points, mode.Border1().x, mode.Border1().y);
        }
    }

    // Draw horizontal lines
    for (auto j = 0; j <= fieldsize.y; j += Int2CoordUnits(8)) {
        // draw solid yardlines
        points[0] = wxPoint(0, j);
        points[1] = wxPoint(fieldsize.x, j);
        dc.DrawLines(2, points, mode.Border1().x, mode.Border1().y);
    }

    // Draw horizontal mid-dotted lines
    for (auto j = Int2CoordUnits(4);
         (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) && j <= fieldsize.y;
         j += Int2CoordUnits(8)) {
        for (auto k = 0; k < fieldsize.x; k += Int2CoordUnits(2)) {
            points[0] = wxPoint(k, j);
            points[1] = wxPoint(k + Int2CoordUnits(1), j);
            dc.DrawLines(2, points, mode.Border1().x, mode.Border1().y);
        }
    }

    // Draw labels
    wxFont* yardLabelFont = wxTheFontList->FindOrCreateFont(
        (int)Float2CoordUnits(config.Get_YardsSize()), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(*yardLabelFont);
    for (int i = 0;
         (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) && i < CoordUnits2Int(fieldsize.x) / 8 + 1;
         i++) {
        wxCoord textw, texth, textd;
        dc.GetTextExtent(config.Get_yard_text(
                             i + (mode.StepsX() + (CalChart::kYardTextValues - 1) * 4) / 8),
            &textw, &texth, &textd);
        if (mode.WhichYards() & CalChart::SPR_YARD_ABOVE)
            dc.DrawText(config.Get_yard_text(
                            i + (mode.StepsX() + (CalChart::kYardTextValues - 1) * 4) / 8),
                Int2CoordUnits(i * 8) - textw / 2 + mode.Border1().x,
                mode.Border1().y - texth);
        if (mode.WhichYards() & CalChart::SPR_YARD_BELOW)
            dc.DrawText(config.Get_yard_text(
                            i + (mode.StepsX() + (CalChart::kYardTextValues - 1) * 4) / 8),
                Int2CoordUnits(i * 8) - textw / 2 + mode.Border1().x,
                mode.FieldSize().y + mode.Border1().y);
    }
    for (int i = 0;
         (howToDraw == ShowMode_kFieldView || howToDraw == ShowMode_kPrinting) && i <= CoordUnits2Int(fieldsize.y);
         i += 8) {
        wxCoord textw, texth, textd;
        dc.GetTextExtent(config.Get_spr_line_text(i / 8), &textw, &texth, &textd);
        if (mode.WhichYards() & CalChart::SPR_YARD_LEFT)
            dc.DrawText(config.Get_spr_line_text(i / 8), mode.Border1().x - textw,
                mode.Border1().y - texth / 2 + Int2CoordUnits(i));
        if (mode.WhichYards() & CalChart::SPR_YARD_RIGHT)
            dc.DrawText(config.Get_spr_line_text(i / 8),
                fieldsize.x + mode.Border1().x,
                mode.Border1().y - texth / 2 + Int2CoordUnits(i));
    }
}

void DrawMode(wxDC& dc, const CalChartConfiguration& config,
    const CalChart::ShowMode& mode, HowToDraw howToDraw)
{
    switch (howToDraw) {
    case ShowMode_kFieldView:
    case ShowMode_kAnimation:
    case ShowMode_kOmniView:
        dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_FIELD_DETAIL).second);
        dc.SetTextForeground(
            config.Get_CalChartBrushAndPen(COLOR_FIELD_TEXT).second.GetColour());
        break;
    case ShowMode_kPrinting:
        dc.SetPen(*wxBLACK_PEN);
        dc.SetTextForeground(*wxBLACK);
        break;
    }
    try {
        auto real_mode = dynamic_cast<const CalChart::ShowModeStandard&>(mode);
        ShowModeStandard_DrawHelper(dc, config, real_mode, howToDraw);
    }
    catch (std::bad_cast&) {
        // now try as spring show
        auto real_mode = dynamic_cast<const CalChart::ShowModeSprShow&>(mode);
        ShowModeSprShow_DrawHelper(dc, config, real_mode, howToDraw);
    }
}

wxImage GetOmniLinesImage(const CalChartConfiguration& config,
    const CalChart::ShowMode& mode)
{
    auto fieldsize = mode.FieldSize();
    wxBitmap bmp(fieldsize.x, fieldsize.y, 32);
    wxMemoryDC dc;
    dc.SelectObject(bmp);
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    DrawMode(dc, config, mode, ShowMode_kOmniView);
    return bmp.ConvertToImage();
}
