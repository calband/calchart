#pragma once
/*
 * field_frame_controls
 * Pane for the field frame controls window
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

#include "cc_coord.h"
#include <wx/wx.h>

// Frame for holding on to the controls
class FieldFrameControls : public wxPanel {
    typedef wxPanel super;

public:
    // FieldFrame will own the show that is passed in
    FieldFrameControls(wxWindow* parent, double zoom);
    virtual ~FieldFrameControls() = default;

    std::pair<CalChart::Coord::units, CalChart::Coord::units> GridChoice() const;
    std::pair<CalChart::Coord::units, CalChart::Coord::units> ToolGridChoice() const;
    double GetZoomAmount() const;
    void SetZoomAmount(double zoom);
    int GetRefChoice() const;
    int GetGhostChoice() const;
    void SetGhostChoice(int which);
    void SetDrawPath(bool enable);

private:
    wxChoice* mGridChoice;
    wxChoice* mToolGridChoice;
    wxComboBox* mZoomBox;
    wxChoice* mRefChoice;
    wxChoice* mGhostChoice;
    wxCheckBox* mDrawPath;
};
