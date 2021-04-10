/*
 * show_ui.h
 * Classes for interacting with shows
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

#pragma once

#include "CalChartDoc.h"
#include <wx/docview.h>
#include <wx/wizard.h>

#include <vector>

class PointPickerView;

class PointPicker : public wxDialog {
    using super = wxDialog;

public:
    PointPicker(CalChartDoc& shw, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Select Points"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    ~PointPicker() = default;

    void Update();

private:
    CalChartDoc& mShow;
    PointPickerView* mView;
    wxListBox* mList;
    std::vector<wxString> mCachedLabels;
    SelectionList mCachedSelection;

    void CreateControls();

    void PointPickerAll(wxCommandEvent&);
    void PointPickerSelect(wxCommandEvent&);

    DECLARE_EVENT_TABLE()
};

