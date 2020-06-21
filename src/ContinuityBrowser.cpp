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
#include "CalChartApp.h"
#include "CalChartView.h"
#include "ContinuityBrowserPanel.h"
#include "basic_ui.h"
#include "cc_sheet.h"
#include "confgr.h"

#include <wx/artprov.h>
#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>

// a panel consists of the name, canvas
class ContinuityBrowserPerCont : public wxPanel {
    using super = wxPanel;

public:
    ContinuityBrowserPerCont(SYMBOL_TYPE sym, wxWindow* parent);
    ~ContinuityBrowserPerCont() override = default;

    void DoSetContinuity(CalChart::Continuity const& new_cont);

    void OnUpdate();
    void SetView(CalChartView* view);
    auto GetView() const { return mView; }

private:
    void Init();
    void CreateControls();

    // Internals
    CalChartView* mView{};
    ContinuityBrowserPanel* mCanvas{};
    SYMBOL_TYPE mSym{};
};

ContinuityBrowserPerCont::ContinuityBrowserPerCont(SYMBOL_TYPE sym, wxWindow* parent)
    : super(parent)
    , mSym(sym)
{
    Init();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    OnUpdate();
}

void ContinuityBrowserPerCont::Init()
{
}

void ContinuityBrowserPerCont::CreateControls()
{
    auto topSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topSizer);

    auto lineSizer = new wxBoxSizer(wxHORIZONTAL);
    AddToSizerBasic(topSizer, lineSizer);

    auto staticText = new wxStaticText(this, wxID_STATIC, CalChart::GetLongNameForSymbol(mSym), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    AddToSizerExpand(lineSizer, staticText);

    auto button = new wxBitmapButton(this, wxID_ANY, ScaleButtonBitmap(wxArtProvider::GetBitmap(wxART_PLUS)));
    AddToSizerBasic(lineSizer, button);
    button->Bind(wxEVT_BUTTON, [this](auto const&) {
        mCanvas->AddNewEntry();
    });

    // here's a canvas
    mCanvas = new ContinuityBrowserPanel(mSym, CalChartConfiguration::GetGlobalConfig(), this);
    topSizer->Add(mCanvas, 1, wxEXPAND);
    topSizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);
}

void ContinuityBrowserPerCont::OnUpdate()
{
}

void ContinuityBrowserPerCont::DoSetContinuity(CalChart::Continuity const& new_cont)
{
    mCanvas->DoSetContinuity(new_cont);
}

void ContinuityBrowserPerCont::SetView(CalChartView* view)
{
    mView = view;
    mCanvas->SetView(view);
}

ContinuityBrowser::ContinuityBrowser(wxWindow* parent, wxWindowID id, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : super(parent, id, pos, size, style, name)
{
    Init();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    OnUpdate();
}

ContinuityBrowser::~ContinuityBrowser() = default;

void ContinuityBrowser::Init()
{
}

void ContinuityBrowser::CreateControls()
{
    // create a sizer for laying things out top down:
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    // add a horizontal bar to make things clear:
    topsizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);

    // we lay things out from top to bottom, saying what point we're dealing with, then the continuity
    for (auto&& eachcont : k_symbols) {
        mPerCont.push_back(new ContinuityBrowserPerCont(eachcont, this));
        topsizer->Add(mPerCont.back(), 0, wxGROW | wxALL, 5);
        mPerCont.back()->Show(false);
    }

    // add help
    auto top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(top_button_sizer);
    auto button = new wxButton(this, wxID_HELP, wxT("&Help"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    button->Bind(wxEVT_BUTTON, [](auto const&) {
        wxGetApp().GetGlobalHelpController().LoadFile();
        wxGetApp().GetGlobalHelpController().KeywordSearch(wxT("Animation Commands"));
    });

    SetScrollRate(1, 1);
    // now update the current screen
}

void ContinuityBrowser::OnUpdate()
{
    if (!mView) {
        return;
    }
    auto sht = mView->GetCurrentSheet();

    for (auto i = 0ul; i < sizeof(k_symbols) / sizeof(k_symbols[0]); ++i) {
        mPerCont.at(i)->Show(sht->ContinuityInUse(k_symbols[i]));
        mPerCont.at(i)->DoSetContinuity(sht->GetContinuityBySymbol(k_symbols[i]));
    }
    GetSizer()->FitInside(this);
}

void ContinuityBrowser::SetView(CalChartView* view)
{
    mView = view;
    for (auto&& cont : mPerCont) {
        cont->SetView(view);
    }
}
