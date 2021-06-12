/*
 * ContinuityEditorPopup
 * Continuity editors
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

#include "ContinuityEditorPopup.h"
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartContinuity.h"
#include "CalChartDoc.h"
#include "CalChartDocCommand.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "ContinuityBrowser.h"
#include "basic_ui.h"

#include <wx/dcbuffer.h>
#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>

ContinuityEditorPopup::ContinuityEditorPopup(wxString const& whatError, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : super(parent, id, caption, pos, size, style, caption)
    , mWhatError(whatError)
{
    CreateControls();
    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);
    Center();
    // now update the current screen
    Update();
}

void ContinuityEditorPopup::CreateControls()
{
    SetSizer(VStack([this](auto sizer) {
        CreateText(this, sizer, mWhatError);

        mUserInput = new FancyTextWin(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, 100));
        sizer->Add(mUserInput, 0, wxGROW | wxALL, 5);

        // add a horizontal bar to make things clear:
        CreateHLine(this, sizer);

        // add a discard, done
        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            CreateButton(this, sizer, wxID_CANCEL, "&Cancel");
            CreateButton(this, sizer, wxID_OK, "Done");
        });
    }));
}

void ContinuityEditorPopup::SetValue(wxString const& value, int line, int column)
{
    mUserInput->Clear();
    mUserInput->WriteText(value);
    mUserInput->SetInsertionPoint(mUserInput->XYToPosition(column - 1, line - 1));
}

wxString ContinuityEditorPopup::ProcessEditContinuity(wxWindow* parent, wxString const& whatError, wxString const& input, int line, int column)
{
    ContinuityEditorPopup dialog(whatError, parent);
    dialog.SetValue(input, line, column);
    if (dialog.ShowModal() == wxID_OK) {
        // set the continuity back
        return dialog.GetValue();
    }
    throw std::runtime_error("Did not parse file correctly");
}
