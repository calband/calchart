/*
 * cont_ui.cpp
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

#include "basic_ui.h"
#include "print_cont_ui.h"
#include "cc_sheet.h"
#include "cc_command.h"
#include "calchartapp.h"
#include "cc_text.h"
#include "draw.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/statline.h>
#include <wx/msgdlg.h>
#include <wx/dcbuffer.h>
#include <wx/splitter.h>

#include <string>
#include <vector>
#include <sstream>

enum {
    CALCHART__CONT_NEW = 100,
    PrintContinuityEditor_Save,
    PrintContinuityEditor_Discard,
    PrintContinuityEditor_KeyPress,
    PrintContinuityEditor_PrintNumber,
    PrintContinuityEditor_PrintOrientation,
};

BEGIN_EVENT_TABLE(PrintContinuityEditor, wxFrame)
EVT_MENU(wxID_CLOSE, PrintContinuityEditor::OnCloseWindow)
EVT_MENU(wxID_HELP, PrintContinuityEditor::OnCmdHelp)
EVT_MENU(PrintContinuityEditor_Save, PrintContinuityEditor::OnSave)
EVT_MENU(PrintContinuityEditor_Discard, PrintContinuityEditor::OnDiscard)
EVT_BUTTON(wxID_CLOSE, PrintContinuityEditor::OnCloseWindow)
EVT_BUTTON(wxID_HELP, PrintContinuityEditor::OnCmdHelp)
EVT_BUTTON(PrintContinuityEditor_Save, PrintContinuityEditor::OnSave)
EVT_BUTTON(PrintContinuityEditor_Discard, PrintContinuityEditor::OnDiscard)
EVT_TEXT(PrintContinuityEditor_KeyPress, PrintContinuityEditor::OnKeyPress)
EVT_CHOICE(PrintContinuityEditor_PrintOrientation,
    PrintContinuityEditor::UpdateOrientation)
END_EVENT_TABLE()

class FancyTextPanel : public wxScrolled<wxWindow> {
public:
    FancyTextPanel(wxWindow* parent);

    void OnPaint(wxPaintEvent& event);
    void SetPrintContinuity(const CC_textline_list& print_continuity)
    {
        m_print_continuity = print_continuity;
    }
    void SetOrientation(bool landscape) { m_landscape = landscape; }

private:
    CC_textline_list m_print_continuity;
    bool m_landscape;
};

PrintContinuityEditorView::PrintContinuityEditorView() {}
PrintContinuityEditorView::~PrintContinuityEditorView() {}

void PrintContinuityEditorView::OnDraw(wxDC* dc) {}
void PrintContinuityEditorView::OnUpdate(wxView* sender, wxObject* hint)
{
    PrintContinuityEditor* editor = static_cast<PrintContinuityEditor*>(GetFrame());
    if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_FlushAllViews))) {
        editor->FlushText();
    }
    else {
        editor->Update();
    }
}

void PrintContinuityEditorView::DoSetPrintContinuity(int which_sheet,
    const wxString& number,
    const wxString& cont)
{
    std::map<int, std::pair<std::string, std::string> > data{ { which_sheet, { number.ToStdString(), cont.ToStdString() } } };
    auto cmd = static_cast<CalChartDoc*>(GetDocument())->Create_SetPrintableContinuity(data);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

PrintContinuityEditor::PrintContinuityEditor() { Init(); }

PrintContinuityEditor::PrintContinuityEditor(CalChartDoc* show,
    wxWindow* parent, wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size, long style)
{
    Init();

    Create(show, parent, id, caption, pos, size, style);
}

void PrintContinuityEditor::Init() {}

bool PrintContinuityEditor::Create(CalChartDoc* show, wxWindow* parent,
    wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size,
    long style)
{
    if (!wxFrame::Create(parent, id, caption, pos, size, style))
        return false;

    mDoc = show;
    mView = new PrintContinuityEditorView;
    mView->SetDocument(show);
    mView->SetFrame(this);

    CreateControls();

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);

    Center();

    // now update the current screen
    Update();

    return true;
}

void PrintContinuityEditor::CreateControls()
{
    // menu bar
    wxMenu* cont_menu = new wxMenu;
    cont_menu->Append(PrintContinuityEditor_Save, wxT("&Save Continuity\tCTRL-S"),
        wxT("Save continuity"));
    cont_menu->Append(wxID_CLOSE, wxT("Close Window\tCTRL-W"),
        wxT("Close this window"));

    wxMenu* help_menu = new wxMenu;
    help_menu->Append(wxID_HELP, wxT("&Help on Continuity...\tCTRL-H"),
        wxT("Help on Continuity"));

    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(cont_menu, wxT("&File"));
    menu_bar->Append(help_menu, wxT("&Help"));
    SetMenuBar(menu_bar);

    // Add the field canvas here so that it gets the focus when we switch to
    // frame.
    mSplitter = new wxSplitterWindow(this, wxID_ANY);
    mSplitter->SetSize(GetClientSize());
    mSplitter->SetSashGravity(0.5);
    mSplitter->SetMinimumPaneSize(20);
    mSplitter->SetWindowStyleFlag(mSplitter->GetWindowStyleFlag() | wxSP_LIVE_UPDATE);
    mSplitter->SetMinSize(wxSize(300, 400));

    // create a sizer for laying things out top down:
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    // add buttons to the top row
    // New, delete, choices
    wxBoxSizer* top_button_sizer = new wxBoxSizer(wxHORIZONTAL);

    top_button_sizer->Add(new wxStaticText(this, wxID_STATIC, "Print Number:",
                              wxDefaultPosition, wxDefaultSize, 0),
        wxSizerFlags(0).Border(wxALL, 2).Left());
    top_button_sizer->Add(new wxTextCtrl(this, PrintContinuityEditor_PrintNumber),
        wxSizerFlags(0).Border(wxALL, 2).Center());
    top_button_sizer->Add(new wxStaticText(this, wxID_STATIC,
                              "Print Orientation:",
                              wxDefaultPosition, wxDefaultSize, 0),
        wxSizerFlags(0).Border(wxALL, 2).Left());

    wxString choices[] = { "Portrait", "Landscape" };
    top_button_sizer->Add(new wxChoice(
        this, PrintContinuityEditor_PrintOrientation, wxDefaultPosition,
        wxDefaultSize, sizeof(choices) / sizeof(*choices), choices));

    // Set, select
    topsizer->Add(top_button_sizer);

    mUserInput = new FancyTextWin(mSplitter, PrintContinuityEditor_KeyPress, wxEmptyString,
        wxDefaultPosition, wxSize(50, 100));

    mPrintContDisplay = new FancyTextPanel(mSplitter);
    mSplitter->Initialize(mUserInput);
    mSplitter->SplitHorizontally(mUserInput, mPrintContDisplay);

    topsizer->Add(mSplitter, wxSizerFlags(1).Expand());

    // add a horizontal bar to make things clear:
    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition,
        wxDefaultSize, wxLI_HORIZONTAL);
    topsizer->Add(line, 0, wxGROW | wxALL, 5);

    // add a save, discard, close, and help
    top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* button = new wxButton(this, PrintContinuityEditor_Save, wxT("&Save"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    button = new wxButton(this, PrintContinuityEditor_Discard, wxT("&Discard"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    button = new wxButton(this, wxID_CLOSE, wxT("Close"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    button = new wxButton(this, wxID_HELP, wxT("&Help"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    topsizer->Add(top_button_sizer);
}

PrintContinuityEditor::~PrintContinuityEditor()
{
    if (mView)
        delete mView;
}

void PrintContinuityEditor::OnCloseWindow(wxCommandEvent& event)
{
    // if the current field is modified, then do something
    if (mUserInput->IsModified()) {
        // give the user a chance to save, discard, or cancle the action
        int userchoice = wxMessageBox(wxT("Continuity modified.  Save changes or cancel?"),
            wxT("Save changes?"), wxYES_NO | wxCANCEL);
        if (userchoice == wxYES) {
            Save();
        }
        if (userchoice == wxNO) {
            Discard();
        }
        if (userchoice == wxCANCEL) {
            wxString message = wxT("Close cancelled.");
            wxMessageBox(message, message);
            return;
        }
    }

    FlushText();
    Close();
}

void PrintContinuityEditor::OnCmdHelp(wxCommandEvent& event)
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
    mUserInput->Clear();
    mUserInput->DiscardEdits();
    CalChart::show::const_CC_sheet_iterator_t current_sheet = mDoc->GetCurrentSheet();

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(PrintContinuityEditor_PrintNumber);
    text->SetValue(current_sheet->GetNumber());

    mUserInput->WriteText(current_sheet->GetRawPrintContinuity());
    mUserInput->SetInsertionPoint(0);
    mPrintContDisplay->SetPrintContinuity(
        current_sheet->GetPrintableContinuity());
    // disable the save and discard buttons as they are not active.
    wxButton* button = (wxButton*)FindWindow(PrintContinuityEditor_Save);
    button->Disable();
    button = (wxButton*)FindWindow(PrintContinuityEditor_Discard);
    button->Disable();
}

// text = (wxTextCtrl*) FindWindow(HEADERSIZE);
// text->GetValue().ToDouble(&mPrintValues[0]);

// flush out the text to the show.  This will treat the text box as unedited
// it is assumed that the user has already been notified that this will modify
// the show
void PrintContinuityEditor::FlushText()
{
    auto current_sheet = mDoc->GetCurrentSheet();
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(PrintContinuityEditor_PrintNumber);
    try {
        if ((mUserInput->GetValue() != current_sheet->GetRawPrintContinuity()) || (text->GetValue() != current_sheet->GetNumber())) {
            mView->DoSetPrintContinuity(
                static_cast<int>(std::distance(mDoc->GetSheetBegin(), current_sheet)), text->GetValue(),
                mUserInput->GetValue());
        }
    }
    catch (const std::runtime_error& e) {
        wxString message = wxT("Error encountered:\n");
        message += e.what();
        wxMessageBox(message, wxT("Print continuity cannot be changed."));
    }
    mUserInput->DiscardEdits();
}

void PrintContinuityEditor::OnSave(wxCommandEvent&) { Save(); }

void PrintContinuityEditor::Save() { FlushText(); }

void PrintContinuityEditor::OnDiscard(wxCommandEvent&) { Discard(); }

void PrintContinuityEditor::Discard() { UpdateText(); }

void PrintContinuityEditor::OnKeyPress(wxCommandEvent&)
{
    wxButton* button = (wxButton*)FindWindow(PrintContinuityEditor_Save);
    button->Enable();
    button = (wxButton*)FindWindow(PrintContinuityEditor_Discard);
    button->Enable();
}

void PrintContinuityEditor::UpdateOrientation(wxCommandEvent&)
{
    mPrintContDisplay->SetOrientation(
        static_cast<wxChoice*>(
            FindWindow(PrintContinuityEditor_PrintOrientation))
            ->GetSelection()
        == 1);
    Update();
}

FancyTextPanel::FancyTextPanel(wxWindow* parent)
    : wxScrolled<wxWindow>(parent, wxID_ANY)
    , m_landscape(false)
{
    static const double kSizeX = 576, kSizeY = 734 - 606;
    SetVirtualSize(wxSize(kSizeX, kSizeY));
    SetScrollRate(10, 10);
    SetBackgroundColour(*wxWHITE);
    Connect(wxEVT_PAINT, wxPaintEventHandler(FancyTextPanel::OnPaint));
}

void FancyTextPanel::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    PrepareDC(dc);
    wxSize virtSize = GetVirtualSize();

    dc.Clear();
    dc.DrawRectangle(wxRect(wxPoint(0, 0), virtSize));
    DrawCont(dc, m_print_continuity, wxRect(wxPoint(0, 0), virtSize),
        m_landscape);
}
