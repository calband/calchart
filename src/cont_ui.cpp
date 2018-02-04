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

#include "cont_ui.h"
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

enum {
    CALCHART__CONT_CLOSE = 100,
    ContinuityEditor_ContEditSelect,
    ContinuityEditor_Save,
    ContinuityEditor_Discard,
    ContinuityEditor_ContEditCurrent,
    ContinuityEditor_KeyPress,
};

BEGIN_EVENT_TABLE(ContinuityEditor, wxFrame)
EVT_MENU(wxID_CLOSE, ContinuityEditor::OnCloseWindow)
EVT_MENU(wxID_HELP, ContinuityEditor::OnCmdHelp)
EVT_MENU(ContinuityEditor_ContEditSelect, ContinuityEditor::ContEditSelect)
EVT_MENU(ContinuityEditor_Save, ContinuityEditor::OnSave)
EVT_MENU(ContinuityEditor_Discard, ContinuityEditor::OnDiscard)
EVT_BUTTON(wxID_CLOSE, ContinuityEditor::OnCloseWindow)
EVT_BUTTON(wxID_HELP, ContinuityEditor::OnCmdHelp)
EVT_BUTTON(ContinuityEditor_ContEditSelect, ContinuityEditor::ContEditSelect)
EVT_BUTTON(ContinuityEditor_Save, ContinuityEditor::OnSave)
EVT_BUTTON(ContinuityEditor_Discard, ContinuityEditor::OnDiscard)
EVT_CHOICE(ContinuityEditor_ContEditCurrent, ContinuityEditor::ContEditCurrent)
EVT_TEXT(ContinuityEditor_KeyPress, ContinuityEditor::OnKeyPress)
END_EVENT_TABLE()

ContinuityEditorView::ContinuityEditorView() {}
ContinuityEditorView::~ContinuityEditorView() {}

void ContinuityEditorView::OnDraw(wxDC* dc) {}
void ContinuityEditorView::OnUpdate(wxView* sender, wxObject* hint)
{
    ContinuityEditor* editor = static_cast<ContinuityEditor*>(GetFrame());
    if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_FlushAllViews))) {
        editor->FlushText();
    } else {
        editor->Update();
    }
}

void ContinuityEditorView::DoSetContinuityText(SYMBOL_TYPE which,
    const wxString& text)
{
    auto cmd = static_cast<CalChartDoc*>(GetDocument())->Create_SetContinuityTextCommand(which, text);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

ContinuityEditor::ContinuityEditor() { Init(); }

ContinuityEditor::ContinuityEditor(CalChartDoc* show, wxWindow* parent,
    wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size,
    long style)
{
    Init();

    Create(show, parent, id, caption, pos, size, style);
}

void ContinuityEditor::Init() {}

bool ContinuityEditor::Create(CalChartDoc* show, wxWindow* parent,
    wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size,
    long style)
{
    if (!wxFrame::Create(parent, id, caption, pos, size, style))
        return false;

    mDoc = show;
    mCurrentContinuityChoice = 0;
    mView = std::make_unique<ContinuityEditorView>();
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

void ContinuityEditor::CreateControls()
{
    // menu bar
    wxMenu* cont_menu = new wxMenu;
    cont_menu->Append(ContinuityEditor_Save, wxT("&Save Continuity\tCTRL-S"),
        wxT("Save continuity"));
    cont_menu->Append(ContinuityEditor_ContEditSelect,
        wxT("Select &Points\tCTRL-P"), wxT("Select Points"));
    cont_menu->Append(wxID_CLOSE, wxT("Close Window\tCTRL-W"),
        wxT("Close this window"));

    wxMenu* help_menu = new wxMenu;
    help_menu->Append(wxID_HELP, wxT("&Help on Continuity...\tCTRL-H"),
        wxT("Help on Continuity"));

    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(cont_menu, wxT("&File"));
    menu_bar->Append(help_menu, wxT("&Help"));
    SetMenuBar(menu_bar);

    // create a sizer for laying things out top down:
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    // add buttons to the top row
    // New, delete, choices
    wxBoxSizer* top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    mContinuityChoices = new wxChoice(this, ContinuityEditor_ContEditCurrent);
    top_button_sizer->Add(mContinuityChoices, 0, wxALIGN_CENTER_VERTICAL | wxALL,
        5);

    // select
    wxButton* button = new wxButton(this, ContinuityEditor_ContEditSelect,
        wxT("Select &Points"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    topsizer->Add(top_button_sizer);

    mUserInput = new FancyTextWin(this, ContinuityEditor_KeyPress, wxEmptyString,
        wxDefaultPosition, wxSize(50, 300));

    topsizer->Add(mUserInput, 0, wxGROW | wxALL, 5);

    // add a horizontal bar to make things clear:
    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition,
        wxDefaultSize, wxLI_HORIZONTAL);
    topsizer->Add(line, 0, wxGROW | wxALL, 5);

    // add a save, discard, close, and help
    top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    button = new wxButton(this, ContinuityEditor_Save, wxT("&Save"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    button = new wxButton(this, ContinuityEditor_Discard, wxT("&Discard"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    button = new wxButton(this, wxID_CLOSE, wxT("Close"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    button = new wxButton(this, wxID_HELP, wxT("&Help"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    topsizer->Add(top_button_sizer);
}

void ContinuityEditor::OnCloseWindow(wxCommandEvent& event)
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

void ContinuityEditor::OnCmdHelp(wxCommandEvent& event)
{
    wxGetApp().GetGlobalHelpController().LoadFile();
    wxGetApp().GetGlobalHelpController().KeywordSearch(wxT("Animation Commands"));
}

void ContinuityEditor::Update()
{
    auto sht = mDoc->GetCurrentSheet();

    mContinuityChoices->Clear();
    for (auto& curranimcont : k_symbols) {
        if (sht->ContinuityInUse(curranimcont)) {
            mContinuityChoices->Append(CalChart::GetNameForSymbol(curranimcont));
        }
    }
    if (mCurrentContinuityChoice >= mContinuityChoices->GetCount() && mContinuityChoices->GetCount() > 0)
        mCurrentContinuityChoice = mContinuityChoices->GetCount() - 1;
    mContinuityChoices->SetSelection(mCurrentContinuityChoice);
    UpdateText();
}

void ContinuityEditor::SetInsertionPoint(int x, int y)
{
    mUserInput->SetInsertionPoint(
        mUserInput->XYToPosition((long)x - 1, (long)y - 1));
    mUserInput->SetFocus();
}

SYMBOL_TYPE ContinuityEditor::CurrentSymbolChoice() const
{
    auto name = mContinuityChoices->GetString(mCurrentContinuityChoice);
    return CalChart::GetSymbolForName(name.ToStdString());
}

void ContinuityEditor::UpdateText()
{
    mUserInput->Clear();
    auto current_sheet = mDoc->GetCurrentSheet();
    auto& c = current_sheet->GetContinuityBySymbol(CurrentSymbolChoice());
    if (!c.GetText().empty()) {
        mUserInput->WriteText(c.GetText());
        mUserInput->SetInsertionPoint(0);
    }
    mUserInput->DiscardEdits();
    // disable the save and discard buttons as they are not active.
    wxButton* button = (wxButton*)FindWindow(ContinuityEditor_Save);
    button->Disable();
    button = (wxButton*)FindWindow(ContinuityEditor_Discard);
    button->Disable();
}

// flush out the text to the show.  This will treat the text box as unedited
// it is assumed that the user has already been notified that this will modify
// the show
void ContinuityEditor::FlushText()
{
    wxString conttext;

    conttext = mUserInput->GetValue();
    auto current_sheet = mDoc->GetCurrentSheet();
    auto& cont = current_sheet->GetContinuityBySymbol(CurrentSymbolChoice());
    if (conttext != cont.GetText()) {
        mView->DoSetContinuityText(CurrentSymbolChoice(), conttext);
    }
    mUserInput->DiscardEdits();
}

void ContinuityEditor::SetCurrent(unsigned i)
{
    mCurrentContinuityChoice = i;
    if (mCurrentContinuityChoice >= mContinuityChoices->GetCount()) {
        mCurrentContinuityChoice = mContinuityChoices->GetCount() - 1;
    }
    mContinuityChoices->SetSelection(mCurrentContinuityChoice);
    UpdateText();
}

void ContinuityEditor::ContEditSelect(wxCommandEvent&)
{
    auto sht = mDoc->GetCurrentSheet();
    mDoc->SetSelection(sht->MakeSelectPointsBySymbol(CurrentSymbolChoice()));
}

void ContinuityEditor::OnSave(wxCommandEvent&) { Save(); }

void ContinuityEditor::Save() { FlushText(); }

void ContinuityEditor::OnDiscard(wxCommandEvent&) { Discard(); }

void ContinuityEditor::Discard() { UpdateText(); }

void ContinuityEditor::ContEditCurrent(wxCommandEvent&)
{
    // which value did we choose
    int newSelection = mContinuityChoices->GetSelection();
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
            mContinuityChoices->SetSelection(mCurrentContinuityChoice);
            return;
        }
    }
    SetCurrent(newSelection);
}

void ContinuityEditor::OnKeyPress(wxCommandEvent&)
{
    wxButton* button = (wxButton*)FindWindow(ContinuityEditor_Save);
    button->Enable();
    button = (wxButton*)FindWindow(ContinuityEditor_Discard);
    button->Enable();
}

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

    mUserInput = new FancyTextWin(this, ContinuityEditor_KeyPress, wxEmptyString, wxDefaultPosition, wxSize(60, 300));
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

class ContinuityBrowserCanvas : public wxScrolledWindow {
    using super = wxScrolledWindow;

public:
    ContinuityBrowserCanvas(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~ContinuityBrowserCanvas() override = default;
    void DoSetContinuity(CalChart::Continuity const& text);

    void OnPaint(wxPaintEvent& event);
    void HandleKey(wxKeyEvent& event);
    void HandleDoubleClick(wxMouseEvent& event);

private:
    CalChartDoc* mDoc;
    SYMBOL_TYPE mSym;
    std::string m_current_text;
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ContinuityBrowserCanvas, wxScrolledWindow)
EVT_PAINT(ContinuityBrowserCanvas::OnPaint)
EVT_CHAR(ContinuityBrowserCanvas::HandleKey)
EVT_LEFT_DCLICK(ContinuityBrowserCanvas::HandleDoubleClick)
END_EVENT_TABLE()

ContinuityBrowserCanvas::ContinuityBrowserCanvas(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent, wxWindowID id,
    const wxPoint& pos,
    const wxSize& size, long style)
    : super(parent, id, pos, size, style)
    , mDoc(doc)
    , mSym(sym)
{
    SetScrollRate(1, 1);
}

void ContinuityBrowserCanvas::HandleKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN) {
        ContinuityEditorPopup::ProcessEditContinuity(mDoc, mSym, this);
    }
}

void ContinuityBrowserCanvas::HandleDoubleClick(wxMouseEvent& event)
{
    ContinuityEditorPopup::ProcessEditContinuity(mDoc, mSym, this);
}

// Define the repainting behaviour
void ContinuityBrowserCanvas::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.SetPen(*wxBLACK_PEN);
    dc.Clear();
    dc.DrawText(m_current_text, 4, 0);
}

void ContinuityBrowserCanvas::DoSetContinuity(CalChart::Continuity const& text)
{
    m_current_text = text.GetText();
    auto num_lines = std::count(m_current_text.begin(), m_current_text.end(), '\n') + 1;
    SetVirtualSize(wxSize{ 30, int(num_lines) * 16 + 4 });
    SetScrollRate(1, 1);
    Refresh();
}

// a panel consists of the name, canvas

class ContinuityBrowserPerCont : public wxPanel {
    using super = wxPanel;

public:
    ContinuityBrowserPerCont(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent);
    virtual ~ContinuityBrowserPerCont() = default;

    void DoSetContinuity(CalChart::Continuity const& new_cont);

private:
    void DoSetFocus(wxFocusEvent& event);
    void DoChildFocus(wxChildFocusEvent& event);
    void DoKillFocus(wxFocusEvent& event);
    void HandleEnter(wxCommandEvent& event);
    void HandleKey(wxKeyEvent& event);
    void HandleDoubleClick(wxMouseEvent& event);
    CalChartDoc* mDoc;
    SYMBOL_TYPE mSym;
    ContinuityBrowserCanvas* mCanvas;
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ContinuityBrowserPerCont, wxPanel)
EVT_SET_FOCUS(ContinuityBrowserPerCont::DoSetFocus)
EVT_CHILD_FOCUS(ContinuityBrowserPerCont::DoChildFocus)
EVT_KILL_FOCUS(ContinuityBrowserPerCont::DoKillFocus)
EVT_CHAR(ContinuityBrowserPerCont::HandleKey)
EVT_LEFT_DCLICK(ContinuityBrowserPerCont::HandleDoubleClick)
END_EVENT_TABLE()

ContinuityBrowserPerCont::ContinuityBrowserPerCont(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent)
    : wxPanel(parent)
    , mDoc(doc)
    , mSym(sym)
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    auto staticText = new wxStaticText(this, wxID_STATIC, CalChart::GetLongNameForSymbol(sym), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    topsizer->Add(staticText, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    mCanvas = new ContinuityBrowserCanvas(mDoc, mSym, this, wxID_ANY, wxDefaultPosition, wxSize(50, 3.5 * 16));
    topsizer->Add(mCanvas, 1, wxEXPAND);
    topsizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);
}

void ContinuityBrowserPerCont::DoSetContinuity(CalChart::Continuity const& text)
{
    mCanvas->DoSetContinuity(text);
}

void ContinuityBrowserPerCont::DoSetFocus(wxFocusEvent& event)
{
    mDoc->SetSelection(mDoc->GetCurrentSheet()->MakeSelectPointsBySymbol(mSym));
}

void ContinuityBrowserPerCont::DoChildFocus(wxChildFocusEvent& event)
{
    mDoc->SetSelection(mDoc->GetCurrentSheet()->MakeSelectPointsBySymbol(mSym));
}

void ContinuityBrowserPerCont::DoKillFocus(wxFocusEvent& event)
{
    mDoc->SetSelection({});
}

void ContinuityBrowserPerCont::HandleKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN) {
        ContinuityEditorPopup::ProcessEditContinuity(mDoc, mSym, this);
    }
}

void ContinuityBrowserPerCont::HandleDoubleClick(wxMouseEvent& event)
{
    ContinuityEditorPopup::ProcessEditContinuity(mDoc, mSym, this);
}

// View for linking CalChartDoc with ContinuityBrowser
class ContinuityBrowserView : public wxView {
public:
    ContinuityBrowserView() = default;
    virtual ~ContinuityBrowserView() = default;
    virtual void OnDraw(wxDC* dc) {}
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL);
};

void ContinuityBrowserView::OnUpdate(wxView* sender, wxObject* hint)
{
    dynamic_cast<ContinuityBrowser*>(GetFrame())->Update();
}

BEGIN_EVENT_TABLE(ContinuityBrowser, wxScrolledWindow)
EVT_BUTTON(wxID_HELP, ContinuityBrowser::OnCmdHelp)
END_EVENT_TABLE()

ContinuityBrowser::ContinuityBrowser(CalChartDoc* doc, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxScrolledWindow(parent, id, pos, size, style, name)
    , mDoc(doc)
{
    mView = std::make_unique<ContinuityBrowserView>();
    mView->SetDocument(doc);
    mView->SetFrame(this);

    CreateControls();

    SetScrollRate(1, 1);
    // now update the current screen
    Update();
}

void ContinuityBrowser::CreateControls()
{
    // create a sizer for laying things out top down:
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    // add a horizontal bar to make things clear:
    topsizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);

    // we lay things out from top to bottom, saying what point we're dealing with, then the continuity
    for (auto& eachcont : k_symbols) {
        mPerCont.push_back(new ContinuityBrowserPerCont(mDoc, eachcont, this));
        topsizer->Add(mPerCont.back(), 0, wxGROW | wxALL, 5);
        mPerCont.back()->Show(false);
    }

    // add a save, discard, close, and help
    auto top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    top_button_sizer->Add(new wxButton(this, wxID_HELP, wxT("&Help")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    topsizer->Add(top_button_sizer);
}

void ContinuityBrowser::OnCmdHelp(wxCommandEvent& event)
{
    wxGetApp().GetGlobalHelpController().LoadFile();
    wxGetApp().GetGlobalHelpController().KeywordSearch(wxT("Animation Commands"));
}

void ContinuityBrowser::Update()
{
    auto sht = mDoc->GetCurrentSheet();

    for (auto i = 0ul; i < sizeof(k_symbols) / sizeof(k_symbols[0]); ++i) {
        mPerCont.at(i)->Show(sht->ContinuityInUse(k_symbols[i]));
        mPerCont.at(i)->DoSetContinuity(sht->GetContinuityBySymbol(k_symbols[i]));
    }
    GetSizer()->FitInside(this);
}
