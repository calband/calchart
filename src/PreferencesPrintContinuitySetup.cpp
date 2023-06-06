/*
 * CalChartPrintContinuitySetup.cpp
 * Dialox box for PrintContinuity Setup part of preferences
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

#include "PreferencesPrintContinuitySetup.h"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "PrintContinuityPreview.h"
#include "basic_ui.h"

#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/stattext.h>

IMPLEMENT_CLASS(PrintContinuitySetup, PreferencePage)

enum {
    CALCHART__CONT_NEW = 100,
    PrintContinuitySetup_KeyPress,
    PrintContinuitySetup_LandScapeChanged,
};

BEGIN_EVENT_TABLE(PrintContinuitySetup, PrintContinuitySetup::super)
EVT_TEXT(PrintContinuitySetup_KeyPress, PrintContinuitySetup::OnKeyPress)
EVT_CHECKBOX(PrintContinuitySetup_LandScapeChanged, PrintContinuitySetup::OnKeyPress)
END_EVENT_TABLE()

static constexpr auto DefaultText
    = "~This is a centered line of text\n"
      "Normal \\bsBold \\isBold+Italics \\beItalics \\ieNormal\n"
      "Next line is all tabs with numbers\n"
      "1\t2\t3\t4\t5\t6\t7\n"
      "All the symbols with two tabs\n"
      "\t\t\\po:\tplainman\n"
      "\t\t\\pb:\tbackslashman\n"
      "\t\t\\ps:\tslashman\n"
      "\t\t\\px:\txman\n"
      "\t\t\\so:\tsolidman\n"
      "\t\t\\sb:\tsolidbackslashman\n"
      "\t\t\\ss:\tsolidslashman\n"
      "\t\t\\sx:\tsolidxman\n"
      "";

void PrintContinuitySetup::CreateControls()
{
    mSplitter = new wxSplitterWindow(this, wxID_ANY);
    mSplitter->SetSize(GetClientSize());
    mSplitter->SetSashGravity(0.0);
    mSplitter->SetMinimumPaneSize(300);
    mSplitter->SetWindowStyleFlag(mSplitter->GetWindowStyleFlag() | wxSP_LIVE_UPDATE);

    // create a sizer for laying things out top down:
    SetSizer(VStack([this](auto sizer) {
        mLandscape = CreateCheckboxWithCaption(this, sizer, PrintContinuitySetup_LandScapeChanged, "Landscape");
        sizer->Add(mSplitter, wxSizerFlags(1).Expand());
    }));

    mUserInput = new FancyTextWin(mSplitter, PrintContinuitySetup_KeyPress);

    mPrintContDisplay = new PrintContinuityPreview(mSplitter, mConfig);
    mSplitter->Initialize(mPrintContDisplay);
    mSplitter->SplitHorizontally(mPrintContDisplay, mUserInput);

    mUserInput->SetValue(DefaultText);

    mPrintContDisplay->SetPrintContinuity(CalChart::PrintContinuity("", mUserInput->GetValue()).GetChunks());
    TransferDataToWindow();
}

void PrintContinuitySetup::InitFromConfig()
{
}

bool PrintContinuitySetup::TransferDataToWindow()
{
    return true;
}

bool PrintContinuitySetup::TransferDataFromWindow()
{
    return true;
}

bool PrintContinuitySetup::ClearValuesToDefault()
{
    return TransferDataToWindow();
}
void PrintContinuitySetup::OnKeyPress(wxCommandEvent&)
{
    mPrintContDisplay->SetOrientation(mLandscape->GetValue());
    mPrintContDisplay->SetPrintContinuity(CalChart::PrintContinuity("", mUserInput->GetValue()).GetChunks());
    Refresh();
}