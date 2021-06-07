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
#include "CalChartConfiguration.h"
#include "CalChartSheet.h"
#include "CalChartView.h"
#include "ContinuityBrowserPanel.h"
#include "basic_ui.h"

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
    SetSizer(VStack([this](auto sizer) {
        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            CreateText(this, sizer, ExpandSizerFlags(), CalChart::GetLongNameForSymbol(mSym));
            CreateBitmapButtonWithHandler(this, sizer, BasicSizerFlags(), ScaleButtonBitmap(wxArtProvider::GetBitmap(wxART_PLUS)), [this]() {
                mCanvas->AddNewEntry();
            });
        });

        // here's a canvas
        mCanvas = new ContinuityBrowserPanel(mSym, CalChartConfiguration::GetGlobalConfig(), this);
        sizer->Add(mCanvas, 1, wxEXPAND);
        CreateHLine(this, sizer);
    }));
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
    SetSizer(VStack([this](auto sizer) {
        // add a horizontal bar to make things clear:
        CreateHLine(this, sizer);

        // we lay things out from top to bottom, saying what point we're dealing with, then the continuity
        for (auto&& eachcont : k_symbols) {
            mPerCont.push_back(new ContinuityBrowserPerCont(eachcont, this));
            sizer->Add(mPerCont.back(), 0, wxGROW | wxALL, 5);
            mPerCont.back()->Show(false);
        }

        // add help
        HStack(sizer, [this](auto sizer) {
            CreateButtonWithHandler(this, sizer, wxID_HELP, "&Help", [this]() {
                wxGetApp().GetGlobalHelpController().LoadFile();
                wxGetApp().GetGlobalHelpController().KeywordSearch(wxT("Animation Commands"));
            });
        });
    }));

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
