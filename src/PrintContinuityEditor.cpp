/*
 * PrintContinuityEditor.cpp
 * Continuity editors
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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

#include "PrintContinuityEditor.h"
#include "CalChartApp.h"
#include "CalChartDrawing.h"
#include "CalChartSheet.h"
#include "CalChartText.h"
#include "CalChartView.h"
#include "PrintContinuityPreview.h"
#include "basic_ui.h"
#include "ui_enums.h"

#include <wx/artprov.h>
#include <wx/dcbuffer.h>
#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/splitter.h>
#include <wx/statline.h>

#include <sstream>
#include <string>
#include <vector>

enum {
    CALCHART__CONT_NEW = 100,
    PrintContinuityEditor_KeyPress,
    PrintContinuityEditor_PrintName,
    PrintContinuityEditor_PrintNumber,
    PrintContinuityEditor_TimerExpiration,
};

static constexpr int itemsToHide[] = {
    CALCHART__prev_ss,
    CALCHART__next_ss,
    PrintContinuityEditor_PrintName,
    PrintContinuityEditor_PrintNumber,
    wxID_HELP,
};

BEGIN_EVENT_TABLE(PrintContinuityEditor, PrintContinuityEditor::super)
EVT_BUTTON(wxID_HELP, PrintContinuityEditor::OnCmdHelp)
EVT_BUTTON(CALCHART__prev_ss, PrintContinuityEditor::OnPrevious)
EVT_BUTTON(CALCHART__next_ss, PrintContinuityEditor::OnNext)
EVT_TEXT(PrintContinuityEditor_KeyPress, PrintContinuityEditor::OnKeyPress)
EVT_TEXT_ENTER(PrintContinuityEditor_PrintNumber, PrintContinuityEditor::OnNameEnter)
EVT_TIMER(PrintContinuityEditor_TimerExpiration, PrintContinuityEditor::OnSaveTimerExpired)
EVT_SIZE(PrintContinuityEditor::OnSizeEvent)
END_EVENT_TABLE()

PrintContinuityEditor::PrintContinuityEditor(wxWindow* parent,
    CalChart::Configuration const& config,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : super(parent, id, pos, size, style, name)
    , mTimer(new wxTimer(this, PrintContinuityEditor_TimerExpiration))
    , mConfig(config)
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

PrintContinuityEditor::~PrintContinuityEditor()
{
    mTimer->Stop();
}

void PrintContinuityEditor::CreateControls()
{
    wxUI::VSizer{
        BasicSizerFlags(),
        wxUI::HSizer{
            wxUI::BitmapButton{ CALCHART__prev_ss, wxArtProvider::GetBitmap(wxART_GO_BACK) },
            wxUI::BitmapButton{ CALCHART__next_ss, wxArtProvider::GetBitmap(wxART_GO_FORWARD) },
            wxUI::Text{ PrintContinuityEditor_PrintName, "Name:" },
            wxUI::TextCtrl{ PrintContinuityEditor_PrintNumber }.withStyle(wxTE_PROCESS_ENTER),
            wxUI::Button{ wxID_HELP, "&Help" },
        },
        wxUI::HSplitter{
            wxSizerFlags{ 1 }.Expand(),
            mPrintContDisplay = [this](wxWindow* parent) { return new PrintContinuityPreview(parent, mConfig); },
            mUserInput = [](wxWindow* parent) { return new FancyTextWin(parent, PrintContinuityEditor_KeyPress); } }
            .withStashGravity(0.5),
    }
        .attachTo(this);
}

void PrintContinuityEditor::OnCmdHelp(wxCommandEvent&)
{
    wxGetApp().GetGlobalHelpController().LoadFile();
    wxGetApp().GetGlobalHelpController().KeywordSearch(wxT("Animation Commands"));
}

void PrintContinuityEditor::Update()
{
    UpdateText();
    mPrintContDisplay->Refresh();
}

void PrintContinuityEditor::SetInsertionPoint(int x, int y)
{
    mUserInput->SetInsertionPoint(
        mUserInput->XYToPosition((long)x - 1, (long)y - 1));
    mUserInput->SetFocus();
}

void PrintContinuityEditor::UpdateText()
{
    if (!mView) {
        return;
    }

    // if the user input has changed, then refresh it
    auto printContinuity = mView->GetPrintContinuity();

    auto text = static_cast<wxTextCtrl*>(FindWindow(PrintContinuityEditor_PrintNumber));
    text->SetValue(printContinuity.GetPrintNumber());

    if (mView->GetRawPrintContinuity() != mUserInput->GetValue()) {
        mUserInput->Clear();
        mUserInput->DiscardEdits();
        mUserInput->WriteText(printContinuity.GetOriginalLine());
        mUserInput->SetInsertionPoint(0);
    }
    mPrintContDisplay->SetPrintContinuity(printContinuity);
    Refresh();
}

// flush out the text to the show.  This will treat the text box as unedited
// it is assumed that the user has already been notified that this will modify
// the show
void PrintContinuityEditor::FlushText()
{
    if (!mView) {
        return;
    }
    auto current_sheet_num = mView->GetCurrentSheetNum();
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(PrintContinuityEditor_PrintNumber);
    try {
        if ((mUserInput->GetValue() != mView->GetRawPrintContinuity()) || (text->GetValue() != mView->GetPrintNumber())) {
            mView->DoSetPrintContinuity(
                current_sheet_num, text->GetValue(),
                mUserInput->GetValue());
        }
    } catch (const std::runtime_error& e) {
        wxString message = wxT("Error encountered:\n");
        message += e.what();
        wxMessageBox(message, wxT("Print continuity cannot be changed."));
    }
    mUserInput->DiscardEdits();
}

void PrintContinuityEditor::OnKeyPress(wxCommandEvent&)
{
    if (!mView) {
        return;
    }
    if (mView->GetRawPrintContinuity() == mUserInput->GetValue()) {
        return;
    }
    // cache out the current text, and only after it's stopped changing do we flush it out.
    mPreviousText = mUserInput->GetValue();
    if (mTimer->IsRunning()) {
        return;
    }
    mTimer->StartOnce(1000);
}

void PrintContinuityEditor::OnSaveTimerExpired(wxTimerEvent&)
{
    if (!mView) {
        return;
    }
    if (mPreviousText != mUserInput->GetValue()) {
        mPreviousText = mUserInput->GetValue();
        mTimer->StartOnce(1000);
        return;
    }
    // one last check here, don't write anything out if nothing changed
    if (mView->GetRawPrintContinuity() == mUserInput->GetValue()) {
        return;
    }

    FlushText();
}

void PrintContinuityEditor::OnNameEnter(wxCommandEvent&)
{
    if (!mView) {
        return;
    }
    FlushText();
}

void PrintContinuityEditor::OnPrevious(wxCommandEvent&)
{
    if (!mView) {
        return;
    }
    mView->GoToPrevSheet();
}

void PrintContinuityEditor::OnNext(wxCommandEvent&)
{
    if (!mView) {
        return;
    }
    mView->GoToNextSheet();
}

void PrintContinuityEditor::SetInMiniMode(bool miniMode)
{
    mInMiniMode = miniMode;
    for (auto&& i : itemsToHide) {
        FindWindow(i)->Show(!mInMiniMode);
    }
    Layout();
}

void PrintContinuityEditor::OnSizeEvent(wxSizeEvent& event)
{
    SetInMiniMode(event.m_size.y < 200);
    event.Skip();
}
