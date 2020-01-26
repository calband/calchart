/*
 * ContinuityBrowser
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
#include "ContinuityBrowser.h"
#include "basic_ui.h"
#include "calchartapp.h"
#include "calchartdoc.h"
#include "cc_command.h"
#include "cc_continuity.h"
#include "cc_sheet.h"
#include "cc_show.h"
#include "confgr.h"

#include <wx/dcbuffer.h>
#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>

ContinuityEditorPopup::ContinuityEditorPopup(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, caption, pos, size, style, caption)
    , mDoc(doc)
    , mSym(sym)
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
    // create a sizer for laying things out top down:
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    auto top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(top_button_sizer);
    auto staticText = new wxStaticText(this, wxID_STATIC, CalChart::GetLongNameForSymbol(mSym), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    top_button_sizer->Add(staticText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    mUserInput = new FancyTextWin(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, 300));
    topsizer->Add(mUserInput, 0, wxGROW | wxALL, 5);

    // add a horizontal bar to make things clear:
    topsizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);

    // add a discard, done
    top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(top_button_sizer);
    auto button = new wxButton(this, wxID_CANCEL, wxT("&Cancel"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    button = new wxButton(this, wxID_OK, wxT("Done"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
}

void ContinuityEditorPopup::Update()
{
    mUserInput->Clear();
    auto current_sheet = mDoc->GetCurrentSheet();
    auto& c = current_sheet->GetContinuityBySymbol(mSym);
    if (!c.GetText().empty()) {
        mUserInput->WriteText(c.GetText());
        mUserInput->SetInsertionPoint(0);
    }
}

void ContinuityEditorPopup::ProcessEditContinuity(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent)
{
    ContinuityEditorPopup dialog(doc, sym, parent);
    if (dialog.ShowModal() == wxID_OK) {
        // set the continuity back
        auto conttext = dialog.GetValue();
        auto current_sheet = doc->GetCurrentSheet();
        auto& cont = current_sheet->GetContinuityBySymbol(sym);
        if (conttext != cont.GetText()) {
            auto cmd = doc->Create_SetContinuityTextCommand(sym, conttext);
            doc->GetCommandProcessor()->Submit(cmd.release());
        }
    }
}

