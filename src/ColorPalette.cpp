/*
 * ColorPalette.cpp
 * For selecting color palette
 */

/*
   Copyright (C) 1995-2012  Richard Michael Powell

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

#include "ColorPalette.h"
#include "CalChartConfiguration.h"
#include "CalChartSizes.h"
#include "CalChartView.h"
#include "ColorSetupDialog.h"
#include "ui_enums.h"

#include <wx/dcbuffer.h>

static const auto kBoxSize = GetColorPaletteBoxSize();
static const auto kBoxBorder = GetColorPaletteBoxBorderSize();
static const auto kBoxRadius = GetColorPaletteBoxRadiusSize();

BEGIN_EVENT_TABLE(ColorPalettePanel, ColorPalettePanel::super)
EVT_LEFT_UP(ColorPalettePanel::OnLeftClick)
EVT_LEFT_DCLICK(ColorPalettePanel::OnLeftDoubleClick)
EVT_PAINT(ColorPalettePanel::OnPaint)
END_EVENT_TABLE()

ColorPalettePanel::ColorPalettePanel(wxWindow* parent, wxWindowID winid)
    : super(parent, winid)
{
    Init();
}

void ColorPalettePanel::Init()
{
    SetMinClientSize(wxSize(kBoxBorder + kNumberPalettes * (kBoxSize + kBoxBorder), 2 * kBoxBorder + kBoxSize));
}

void ColorPalettePanel::OnPaint(wxPaintEvent& event)
{
    auto dc = wxBufferedPaintDC{ this };
    auto& config = CalChartConfiguration::GetGlobalConfig();

    dc.Clear();

    // draw the rounded rectangles
    auto point = wxPoint{ kBoxBorder, kBoxBorder };
    auto size = wxSize{ kBoxSize, kBoxSize };
    for (auto i = 0; i < kNumberPalettes; ++i) {
        if (config.GetActiveColorPalette() == i) {
            dc.SetPen(*wxYELLOW_PEN);
        } else {
            dc.SetPen(*wxBLACK_PEN);
        }
        dc.SetBrush(config.GetColorPaletteColor(i));
        dc.DrawRoundedRectangle(point, size, kBoxRadius);
        point.x += kBoxBorder + kBoxSize;
    }
}

void ColorPalettePanel::OnLeftClick(wxMouseEvent& event)
{
    // change the color.
    auto box = WhichBox(event.GetPosition());
    if (box < 0 || box >= kNumberPalettes) {
        return;
    }
    // now we switch to that color
    auto& config = CalChartConfiguration::GetGlobalConfig();
    config.SetActiveColorPalette(box);
    // sort of sneaky, we act like a button has been pressed
    QueueEvent(new wxCommandEvent{ wxEVT_BUTTON, CALCHART__ChangedColorPalette });
}

int ColorPalettePanel::WhichBox(wxPoint const& where)
{
    if (where.y < kBoxBorder || where.y > (kBoxBorder + kBoxSize)) {
        return -1;
    }
    if (where.x < kBoxBorder / 2) {
        return -1;
    }
    auto x = where.x - kBoxBorder / 2;
    auto mod_x = x % (kBoxBorder + kBoxSize);
    if (mod_x < kBoxBorder / 2 || mod_x > (kBoxBorder / 2 + kBoxSize)) {
        return -1;
    }
    return x / (kBoxBorder + kBoxSize);
}

void ColorPalettePanel::OnLeftDoubleClick(wxMouseEvent& event)
{
    // change the color.
    auto box = WhichBox(event.GetPosition());
    if (box < 0 || box >= kNumberPalettes) {
        return;
    }
    // this opens the dialog that
    ColorSetupDialog dialog(this, box);
    if (dialog.ShowModal() == wxID_OK) {
        QueueEvent(new wxCommandEvent{ wxEVT_BUTTON, CALCHART__ChangedColorPalette });
    }
}
