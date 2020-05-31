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

#include "field_frame_controls.h"
#include "CalChartSizes.h"
#include "ColorPalette.h"
#include "basic_ui.h"
#include "cc_point.h"
#include "ui_enums.h"
#include "platconf.h"

#include <wx/toolbar.h>
#include <wx/wx.h>
#include <wx/aui/auibar.h>

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
static constexpr double zoom_amounts[] = {
    5, 2, 1.5, 1, 0.75, 0.5, 0.25, 0.1, 0
};

static constexpr std::pair<CalChart::Coord::units, CalChart::Coord::units> gridvalue[] = {
    { 1, 0 },
    { Int2CoordUnits(1), 0 },
    { Int2CoordUnits(2), 0 },
    { Int2CoordUnits(4), 0 },
    { Int2CoordUnits(4), static_cast<CalChart::Coord::units>(Int2CoordUnits(4) / 3) },
    { Int2CoordUnits(8), static_cast<CalChart::Coord::units>(Int2CoordUnits(8) / 3) },
};

namespace FieldControls {

wxAuiToolBar* CreateFieldControlsToolBar(wxWindow* parent, wxWindowID id, long style)
{
    auto tb = new wxAuiToolBar(parent, id, wxDefaultPosition, wxDefaultSize, style);

    tb->AddControl(new ColorPalettePanel(tb), "Colors");
    tb->AddSeparator();

    // Grid choice
    auto gridChoice = new wxChoice(tb, CALCHART__GridSize);
    for (auto&& i : gridtext) {
        gridChoice->Append(i);
    }
    gridChoice->SetMaxSize(wxSize{ BestSizeX(gridChoice, gridtext)+GetToolbarControlsPadding(), -1 });
    gridChoice->SetSelection(2);
    tb->AddControl(gridChoice, "Grid");

    auto toolGridChoice = new wxChoice(tb, CALCHART__GridToolSize);
    for (auto&& i : gridtext) {
        toolGridChoice->Append(i);
    }
    toolGridChoice->SetMaxSize(wxSize{ BestSizeX(toolGridChoice, gridtext)+GetToolbarControlsPadding(), -1 });
    toolGridChoice->SetSelection(2);
    tb->AddControl(toolGridChoice, "Tool");

    wxArrayString zoomtext;
    for (auto&& zoom_amount : zoom_amounts) {
        if (zoom_amount == 0)
            continue;
        wxString buf;
        buf.sprintf(wxT("%d%%"), int(zoom_amount * 100.0));
        zoomtext.Add(buf);
    }
    zoomtext.Add(wxT("Fit"));
    auto zoomBox = new wxComboBox(tb, CALCHART__slider_zoom, wxEmptyString, wxDefaultPosition, wxSize{-1, -1}, zoomtext, wxTE_PROCESS_ENTER);
    zoomBox->SetMaxSize(wxSize{ BestSizeX(zoomBox, std::vector<std::string>{ "500%" })+GetToolbarControlsPadding(), -1 });

    tb->AddControl(zoomBox, "Zoom");
    // set the text to the default zoom level
    wxString zoomtxt;
    zoomtxt.sprintf("%d%%", (int)(1 * 100));
    zoomBox->SetValue(zoomtxt);

    // Reference choice
    auto refChoice = new wxChoice(tb, CALCHART__refnum_callback);
    refChoice->Append(wxT("Off"));
    for (auto i = 1; i <= CalChart::Point::kNumRefPoints; i++) {
        wxString buf;
        buf.sprintf(wxT("%u"), i);
        refChoice->Append(buf);
    }
    refChoice->SetMaxSize(wxSize{ BestSizeX(zoomBox, std::vector<std::string>{ "Off" })+GetToolbarControlsPadding(), -1 });
    refChoice->SetSelection(0);
    tb->AddControl(refChoice, "Ref Group");

    // paths
    tb->AddTool(CALCHART__draw_paths, "Path", ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_paths))), "Show path to next", wxITEM_CHECK);

    // Ghost choice
    auto ghostChioce = new wxChoice(tb, CALCHART__GhostControls);
    for (auto&& i : ghostChoice) {
        ghostChioce->Append(i);
    }
    ghostChioce->SetMaxSize(wxSize{ BestSizeX(ghostChioce, gridtext)+GetToolbarControlsPadding(), -1 });
    ghostChioce->SetSelection(0);
    tb->AddControl(ghostChioce, "Ghost");

    tb->SetFont(ResizeFont(tb->GetFont(), GetToolbarFontSize()));

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
    auto zoomtxt = static_cast<wxComboBox*>(target->FindWindow(CALCHART__slider_zoom))->GetValue();
    // if it equals 'fit', return 0 to indicate we should fit.
    // strip the trailing '%' if it exists
    if (zoomtxt == wxT("Fit")) {
        return 0;
    }
    if (zoomtxt.Length() && (zoomtxt.Last() == wxT('%'))) {
        zoomtxt.RemoveLast();
    }
    double zoom_amount = 1.0;
    if (zoomtxt.ToDouble(&zoom_amount)) {
        return zoom_amount / 100.0;
    }
    return 1;
}

void SetZoomAmount(wxWindow* target, double zoom)
{
    wxString zoomtxt;
    zoomtxt.sprintf(wxT("%d%%"), int(zoom * 100.0));
    static_cast<wxComboBox*>(target->FindWindow(CALCHART__slider_zoom))->SetValue(zoomtxt);
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
