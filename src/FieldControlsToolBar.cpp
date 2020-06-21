/*
 * Field frame controls
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "FieldControlsToolBar.h"
#include "CalChartSizes.h"
#include "ColorPalette.h"
#include "basic_ui.h"
#include "cc_point.h"
#include "platconf.h"
#include "ui_enums.h"

#include <wx/artprov.h>
#include <wx/aui/auibar.h>
#include <wx/toolbar.h>
#include <wx/wx.h>

#include "tb_paths.xbm"

static const wxString gridtext[] = {
    wxT("None"),
    wxT("1"),
    wxT("2"),
    wxT("4"),
    wxT("Mil"),
    wxT("2-Mil"),
};

static const std::string ghostChoice[] = {
    "Off",
    "Next",
    "Previous",
    "Sheet...",
};

// zero is special and means fit
static constexpr auto zoom_min = 1.0;
static constexpr auto zoom_max = 5.0;
static constexpr auto zoom_steps = 100;

static constexpr std::pair<CalChart::Coord::units, CalChart::Coord::units> gridvalue[] = {
    { 1, 0 },
    { Int2CoordUnits(1), 0 },
    { Int2CoordUnits(2), 0 },
    { Int2CoordUnits(4), 0 },
    { Int2CoordUnits(4), static_cast<CalChart::Coord::units>(Int2CoordUnits(4) / 3) },
    { Int2CoordUnits(8), static_cast<CalChart::Coord::units>(Int2CoordUnits(8) / 3) },
};

namespace FieldControls {

wxAuiToolBar* CreateToolBar(wxWindow* parent, wxWindowID id, long style)
{
    auto tb = new wxAuiToolBar(parent, id, wxDefaultPosition, wxDefaultSize, style);

    // colors
    tb->AddControl(new ColorPalettePanel(tb), "Colors");

    tb->AddSeparator();

    // zoom
    tb->AddTool(CALCHART__ViewZoomOut, "", ScaleButtonBitmap(wxArtProvider::GetBitmap(wxART_MINUS)), "Zoom out", wxITEM_NORMAL);
    tb->AddControl(new wxSlider(tb, CALCHART__slider_zoom, 0, 0, zoom_steps, wxDefaultPosition, wxSize(GetToolBarControlsZoomSize(), -1)), "Zoom");
    tb->AddTool(CALCHART__ViewZoomIn, "", ScaleButtonBitmap(wxArtProvider::GetBitmap(wxART_PLUS)), "Zoom out", wxITEM_NORMAL);

    tb->AddSeparator();

    // Grid choice
    auto gridChoice = new wxChoice(tb, CALCHART__GridSize);
    for (auto&& i : gridtext) {
        gridChoice->Append(i);
    }
    gridChoice->SetMaxSize(wxSize{ BestSizeX(gridChoice, gridtext) + GetToolBarControlsPadding(), -1 });
    gridChoice->SetSelection(2);
    tb->AddControl(gridChoice, "Grid");

    auto toolGridChoice = new wxChoice(tb, CALCHART__GridToolSize);
    for (auto&& i : gridtext) {
        toolGridChoice->Append(i);
    }
    toolGridChoice->SetMaxSize(wxSize{ BestSizeX(toolGridChoice, gridtext) + GetToolBarControlsPadding(), -1 });
    toolGridChoice->SetSelection(2);
    tb->AddControl(toolGridChoice, "Tool");

    // Reference choice
    auto refChoice = new wxChoice(tb, CALCHART__refnum_callback);
    refChoice->Append(wxT("Off"));
    for (auto i = 1; i <= CalChart::Point::kNumRefPoints; i++) {
        wxString buf;
        buf.sprintf(wxT("%u"), i);
        refChoice->Append(buf);
    }
    refChoice->SetMaxSize(wxSize{ BestSizeX(refChoice, std::vector<std::string>{ "Off" }) + GetToolBarControlsPadding(), -1 });
    refChoice->SetSelection(0);
    tb->AddControl(refChoice, "Ref Group");

    // paths
    tb->AddTool(CALCHART__draw_paths, "Path", ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_paths))), "Show path to next", wxITEM_CHECK);

    // Ghost choice
    auto ghostChioce = new wxChoice(tb, CALCHART__GhostControls);
    for (auto&& i : ghostChoice) {
        ghostChioce->Append(i);
    }
    ghostChioce->SetMaxSize(wxSize{ BestSizeX(ghostChioce, gridtext) + GetToolBarControlsPadding(), -1 });
    ghostChioce->SetSelection(0);
    tb->AddControl(ghostChioce, "Ghost");

    tb->SetFont(ResizeFont(tb->GetFont(), GetToolBarFontSize()));

    tb->Realize();

    return tb;
}

std::pair<CalChart::Coord::units, CalChart::Coord::units> GridChoice(wxWindow* target)
{
    return gridvalue[static_cast<wxChoice*>(target->FindWindow(CALCHART__GridSize))->GetSelection()];
}

std::pair<CalChart::Coord::units, CalChart::Coord::units> ToolGridChoice(wxWindow* target)
{
    return gridvalue[static_cast<wxChoice*>(target->FindWindow(CALCHART__GridToolSize))->GetSelection()];
}

double GetZoomAmount(wxWindow* target)
{
    auto zoom = static_cast<wxSlider*>(target->FindWindow(CALCHART__slider_zoom))->GetValue();
    return zoom_min + (zoom_max - zoom_min) * (zoom / static_cast<double>(zoom_steps));
}

void SetZoomAmount(wxWindow* target, double zoom)
{
    auto newSliderValue = (zoom <= zoom_min) ? 0 : (zoom >= zoom_max) ? zoom_steps : (zoom - zoom_min) / (zoom_max - zoom_min) * zoom_steps;
    auto slider = static_cast<wxSlider*>(target->FindWindow(CALCHART__slider_zoom));
    if (newSliderValue == slider->GetValue()) {
        return;
    }
    slider->SetValue(newSliderValue);
}

int GetRefChoice(wxWindow* target)
{
    return static_cast<wxChoice*>(target->FindWindow(CALCHART__refnum_callback))->GetSelection();
}

int GetGhostChoice(wxWindow* target)
{
    return static_cast<wxChoice*>(target->FindWindow(CALCHART__GhostControls))->GetSelection();
}

void SetGhostChoice(wxWindow* target, int which)
{
    static_cast<wxChoice*>(target->FindWindow(CALCHART__GhostControls))->SetSelection(which);
}

}
