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
#include "cc_point.h"
#include "ui_enums.h"

static const wxString gridtext[] = {
    wxT("None"),
    wxT("1"),
    wxT("2"),
    wxT("4"),
    wxT("Mil"),
    wxT("2-Mil"),
};

// zero is special and means fit
static constexpr double zoom_amounts[] = {
    5, 2, 1.5, 1, 0.75, 0.5, 0.25, 0.1, 0
};

static constexpr std::pair<CalChart::Coord::units, CalChart::Coord::units> gridvalue[] = { { 1, 0 },
    { Int2CoordUnits(1), 0 },
    { Int2CoordUnits(2), 0 },
    { Int2CoordUnits(4), 0 },
    { Int2CoordUnits(4), static_cast<CalChart::Coord::units>(Int2CoordUnits(4) / 3) },
    { Int2CoordUnits(8), static_cast<CalChart::Coord::units>(Int2CoordUnits(8) / 3) } };

FieldFrameControls::FieldFrameControls(double zoom, wxWindow* parent, wxWindowID id,
const wxPoint& pos,
const wxSize& size,
long style,
const wxString& name)
    : wxPanel(parent, id, pos, size, style, name)
{
    auto topRowSizerFlags = wxSizerFlags(1).Expand().Border(0, 5);
    auto centerWidget = wxSizerFlags(0).Expand().Border(wxALL, 5);

    auto fullsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(fullsizer);

    auto toprow = new wxBoxSizer(wxHORIZONTAL);
    fullsizer->Add(toprow, wxSizerFlags(0).Border(0, 5));

    // Grid choice
    auto staticSize = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Grid Spacing")), wxVERTICAL);
    toprow->Add(staticSize, topRowSizerFlags);
    mGridChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sizeof(gridtext) / sizeof(gridtext[0]), gridtext);
    staticSize->Add(mGridChoice, centerWidget);
    mGridChoice->SetSelection(2);

    staticSize = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Tool Grid Spacing")), wxVERTICAL);
    toprow->Add(staticSize, topRowSizerFlags);
    mToolGridChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sizeof(gridtext) / sizeof(gridtext[0]), gridtext);
    staticSize->Add(mToolGridChoice, centerWidget);
    mToolGridChoice->SetSelection(2);

    // Zoom slider
    staticSize = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Zoom")), wxVERTICAL);
    toprow->Add(staticSize, topRowSizerFlags);
    wxArrayString zoomtext;
    for (auto&& zoom_amount : zoom_amounts) {
        if (zoom_amount == 0)
            continue;
        wxString buf;
        buf.sprintf(wxT("%d%%"), int(zoom_amount * 100.0));
        zoomtext.Add(buf);
    }
    zoomtext.Add(wxT("Fit"));
    mZoomBox = new wxComboBox(this, CALCHART__slider_zoom, wxEmptyString, wxDefaultPosition, wxDefaultSize, zoomtext, wxTE_PROCESS_ENTER);
    staticSize->Add(mZoomBox, centerWidget);

    // set the text to the default zoom level
    wxString zoomtxt;
    zoomtxt.sprintf("%d%%", (int)(zoom * 100));
    mZoomBox->SetValue(zoomtxt);

    // Reference choice
    staticSize = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Reference Group")), wxVERTICAL);
    toprow->Add(staticSize, topRowSizerFlags);
    mRefChoice = new wxChoice(this, CALCHART__refnum_callback);
    staticSize->Add(mRefChoice, centerWidget);
    mRefChoice->Append(wxT("Off"));
    for (auto i = 1; i <= CalChart::Point::kNumRefPoints; i++) {
        wxString buf;
        buf.sprintf(wxT("%u"), i);
        mRefChoice->Append(buf);
    }

    staticSize = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Draw Paths")), wxVERTICAL);
    toprow->Add(staticSize, topRowSizerFlags);
    mDrawPath = new wxCheckBox(this, CALCHART__draw_paths, wxT("Draw Paths"));
    mDrawPath->SetValue(false);
    staticSize->Add(mDrawPath, centerWidget);

    // Reference choice
    staticSize = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Ghost Sheet")), wxVERTICAL);
    toprow->Add(staticSize, topRowSizerFlags);
    mGhostChoice = new wxChoice(this, CALCHART__GhostControls);
    staticSize->Add(mGhostChoice, centerWidget);
    mGhostChoice->Append(wxT("Off"));
    mGhostChoice->Append(wxT("Next"));
    mGhostChoice->Append(wxT("Previous"));
    mGhostChoice->Append(wxT("Sheet..."));
}

std::pair<CalChart::Coord::units, CalChart::Coord::units> FieldFrameControls::GridChoice() const
{
    return gridvalue[mGridChoice->GetSelection()];
}

std::pair<CalChart::Coord::units, CalChart::Coord::units> FieldFrameControls::ToolGridChoice() const
{
    return gridvalue[mToolGridChoice->GetSelection()];
}

double FieldFrameControls::GetZoomAmount() const
{
    wxString zoomtxt = mZoomBox->GetValue();
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

void FieldFrameControls::SetZoomAmount(double zoom)
{
    wxString zoomtxt;
    zoomtxt.sprintf(wxT("%d%%"), int(zoom * 100.0));
    mZoomBox->SetValue(zoomtxt);
}

int FieldFrameControls::GetRefChoice() const
{
    return mRefChoice->GetSelection();
}

int FieldFrameControls::GetGhostChoice() const
{
    return mGhostChoice->GetSelection();
}

void FieldFrameControls::SetGhostChoice(int which)
{
    mGhostChoice->SetSelection(which);
}

void FieldFrameControls::SetDrawPath(bool enable)
{
    mDrawPath->SetValue(enable);
}
