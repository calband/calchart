/*
 * PrintContinuityEditor.cpp
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

#include "PrintContinuityEditor.h"
#include "CalChartApp.h"
#include "CalChartSheet.h"
#include "CalChartText.h"
#include "CalChartView.h"
#include "basic_ui.h"
#include "draw.h"
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

class PrintContinuityPreview : public wxScrolled<wxWindow> {
    using super = wxScrolled<wxWindow>;
    DECLARE_EVENT_TABLE()

public:
    PrintContinuityPreview(wxWindow* parent);

    void OnPaint(wxPaintEvent& event);
    void SetPrintContinuity(const CalChart::Textline_list& printContinuity)
    {
        mPrintContinuity = printContinuity;
    }
    void SetOrientation(bool landscape) { m_landscape = landscape; }
    void OnSizeEvent(wxSizeEvent& event);

private:
    CalChart::Textline_list mPrintContinuity;
    bool m_landscape;
};

BEGIN_EVENT_TABLE(PrintContinuityPreview, PrintContinuityPreview::super)
EVT_SIZE(PrintContinuityPreview::OnSizeEvent)
END_EVENT_TABLE()

static constexpr auto kPrintContinuityPreviewMinX = 256;
static constexpr auto kPrintContinuityPreviewMinY = 734 - 606;
PrintContinuityPreview::PrintContinuityPreview(wxWindow* parent)
    : wxScrolled<wxWindow>(parent, wxID_ANY)
    , m_landscape(false)
{
    static const double kSizeX = 576, kSizeY = 734 - 606;
    SetVirtualSize(wxSize(kSizeX, kSizeY));
    SetScrollRate(10, 10);
    SetBackgroundColour(*wxWHITE);
    Connect(wxEVT_PAINT, wxPaintEventHandler(PrintContinuityPreview::OnPaint));
}

void PrintContinuityPreview::OnPaint(wxPaintEvent&)
{
    wxPaintDC dc(this);
    PrepareDC(dc);
    wxSize virtSize = GetVirtualSize();

    dc.Clear();
    dc.DrawRectangle(wxRect(wxPoint(0, 0), virtSize));
    DrawContForPreview(dc, mPrintContinuity, wxRect(wxPoint(0, 0), virtSize));
}

void PrintContinuityPreview::OnSizeEvent(wxSizeEvent& event)
{
    auto x = std::max(event.m_size.x, kPrintContinuityPreviewMinX);
    auto y = std::max(event.m_size.y, kPrintContinuityPreviewMinY);
    SetVirtualSize(wxSize(x, y));
    SetScrollRate(10, 10);
}

enum {
    CALCHART__CONT_NEW = 100,
    PrintContinuityEditor_KeyPress,
    PrintContinuityEditor_PrintNumber,
    PrintContinuityEditor_TimerExpiration,
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
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : super(parent, id, pos, size, style, name)
    , mTimer(new wxTimer(this, PrintContinuityEditor_TimerExpiration))
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
    // Add the field canvas here so that it gets the focus when we switch to
    // frame.
    mSplitter = new wxSplitterWindow(this, wxID_ANY);
    mSplitter->SetSize(GetClientSize());
    mSplitter->SetSashGravity(0.0);
    mSplitter->SetMinimumPaneSize(20);
    mSplitter->SetWindowStyleFlag(mSplitter->GetWindowStyleFlag() | wxSP_LIVE_UPDATE);

    // create a sizer for laying things out top down:
    SetSizer(VStack([this](auto sizer) {
        HStack(sizer, [this](auto sizer) {
            mItemsToHide.push_back(CreateBitmapButton(this, sizer, BasicSizerFlags(), CALCHART__prev_ss, wxArtProvider::GetBitmap(wxART_GO_BACK)));
            mItemsToHide.push_back(CreateBitmapButton(this, sizer, BasicSizerFlags(), CALCHART__next_ss, wxArtProvider::GetBitmap(wxART_GO_FORWARD)));
            mItemsToHide.push_back(CreateText(this, sizer, BasicSizerFlags(), "Name:"));
            mItemsToHide.push_back(CreateTextCtrl(this, sizer, BasicSizerFlags(), PrintContinuityEditor_PrintNumber, wxTE_PROCESS_ENTER));
            mItemsToHide.push_back(CreateButton(this, sizer, BasicSizerFlags(), wxID_HELP, wxT("&Help")));
        });
        sizer->Add(mSplitter, wxSizerFlags(1).Expand());
    }));

    mUserInput = new FancyTextWin(mSplitter, PrintContinuityEditor_KeyPress);

    mPrintContDisplay = new PrintContinuityPreview(mSplitter);
    mSplitter->Initialize(mPrintContDisplay);
    mSplitter->SplitHorizontally(mPrintContDisplay, mUserInput);
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
    auto text = static_cast<wxTextCtrl*>(FindWindow(PrintContinuityEditor_PrintNumber));
    text->SetValue(mView->GetCurrentSheet()->GetNumber());

    if (mView->GetCurrentSheet()->GetRawPrintContinuity() != mUserInput->GetValue()) {
        mUserInput->Clear();
        mUserInput->DiscardEdits();
        mUserInput->WriteText(mView->GetCurrentSheet()->GetRawPrintContinuity());
        mUserInput->SetInsertionPoint(0);
    }
    mPrintContDisplay->SetPrintContinuity(mView->GetCurrentSheet()->GetPrintableContinuity());
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
    auto current_sheet = mView->GetCurrentSheet();
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(PrintContinuityEditor_PrintNumber);
    try {
        if ((mUserInput->GetValue() != current_sheet->GetRawPrintContinuity()) || (text->GetValue() != current_sheet->GetNumber())) {
            mView->DoSetPrintContinuity(
                static_cast<int>(std::distance(mView->GetSheetBegin(), current_sheet)), text->GetValue(),
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
    if (mView->GetCurrentSheet()->GetRawPrintContinuity() == mUserInput->GetValue()) {
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
    if (mView->GetCurrentSheet()->GetRawPrintContinuity() == mUserInput->GetValue()) {
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
    for (auto&& i : mItemsToHide) {
        i->Show(!mInMiniMode);
    }
    Layout();
}

void PrintContinuityEditor::OnSizeEvent(wxSizeEvent& event)
{
    SetInMiniMode(event.m_size.y < 200);
    event.Skip();
}
