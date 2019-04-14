/*
 * ContinuityEditorPopup
 * Header for continuity editors
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

#include "calchartdoc.h"
#include "basic_ui.h"

class ContinuityBrowserPerCont;

// ContinuityEditorPopup, for browser style adjustments
class ContinuityEditorPopup : public wxDialog {
    using super = wxDialog;

public:
    // all in one function for editing
    static void ProcessEditContinuity(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent);

    ContinuityEditorPopup(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Edit Continuity"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    virtual ~ContinuityEditorPopup() override = default;

    virtual void Update() override;
    auto GetValue() const { return mUserInput->GetValue(); }

private:
    void CreateControls();

    CalChartDoc* mDoc;
    SYMBOL_TYPE mSym;
    FancyTextWin* mUserInput;
};
