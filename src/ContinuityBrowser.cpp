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

#include "ContinuityBrowser.h"
#include "ContinuityEditorPopup.h"
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
