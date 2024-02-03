/*
 * ContinuityBrowser
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
#include <wxUI/wxUI.h>

// a panel consists of the name, canvas
class ContinuityBrowserPerCont : public wxPanel {
    using super = wxPanel;

public:
    ContinuityBrowserPerCont(wxWindow* parent, CalChart::SYMBOL_TYPE sym, CalChartConfiguration const& config);
    ~ContinuityBrowserPerCont() override = default;

    void DoSetContinuity(CalChart::Continuity const& new_cont);

    void OnUpdate();
    void SetView(CalChartView* view);
    auto GetView() const { return mView; }

private:
    void CreateControls(CalChartConfiguration const& config);

    // Internals
    CalChartView* mView{};
    wxUI::Generic<ContinuityBrowserPanel>::Proxy mCanvas{};
    CalChart::SYMBOL_TYPE mSym{};
};

ContinuityBrowserPerCont::ContinuityBrowserPerCont(wxWindow* parent, CalChart::SYMBOL_TYPE sym, CalChartConfiguration const& config)
    : super(parent)
    , mSym(sym)
{
    CreateControls(config);
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    OnUpdate();
}

void ContinuityBrowserPerCont::CreateControls(CalChartConfiguration const& config)
{
    wxUI::VSizer{
        BasicSizerFlags(),
        wxUI::HSizer{
            wxUI::Text{ ExpandSizerFlags(), CalChart::GetLongNameForSymbol(mSym) },
            wxUI::BitmapButton{ BasicSizerFlags(), ScaleButtonBitmap(wxArtProvider::GetBitmap(wxART_PLUS)) }
                .bind([this]() {
                    mCanvas->AddNewEntry();
                }) },
        // here's a canvas
        mCanvas = wxUI::Generic<ContinuityBrowserPanel>{
            ExpandSizerFlags(),
            [this, &config](wxWindow* parent) {
                return new ContinuityBrowserPanel(mSym, config, parent);
            } },
    }
        .attachTo(this);
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

ContinuityBrowser::ContinuityBrowser(wxWindow* parent, wxSize const& size, CalChartConfiguration const& config)
    : super(parent, wxID_ANY, wxDefaultPosition, size, wxScrolledWindowStyle)
{
    CreateControls(config);
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    OnUpdate();
}

ContinuityBrowser::~ContinuityBrowser() = default;

void ContinuityBrowser::Init()
{
}

void ContinuityBrowser::CreateControls(CalChartConfiguration const& config)
{
    wxUI::VSizer{
        wxUI::Line{}.withStyle(wxLI_HORIZONTAL),
        wxUI::ForEach{
            wxSizerFlags{ 1 }.Border(wxALL, 2).Expand(),
            CalChart::k_symbols,
            [this, &config](auto eachcont) { return wxUI::Generic{
                                                 [this, eachcont, &config](wxWindow* parent) {
                                                     auto perCont = new ContinuityBrowserPerCont(parent, eachcont, config);
                                                     perCont->Show(false);
                                                     mPerCont.push_back(perCont);
                                                     return perCont;
                                                 }
                                             }; } },

        wxUI::HSizer{
            wxUI::Button{ wxID_HELP, "&Help" }
                .bind([] {
                    wxGetApp().GetGlobalHelpController().LoadFile();
                    wxGetApp().GetGlobalHelpController().KeywordSearch(wxT("Animation Commands"));
                }),
        },
    }
        .attachTo(this);

    SetScrollRate(1, 1);
    // now update the current screen
}

void ContinuityBrowser::OnUpdate()
{
    if (!mView) {
        return;
    }
    auto sht = mView->GetCurrentSheet();

    for (auto i = 0ul; i < sizeof(CalChart::k_symbols) / sizeof(CalChart::k_symbols[0]); ++i) {
        mPerCont.at(i)->Show(sht->ContinuityInUse(CalChart::k_symbols[i]));
        mPerCont.at(i)->DoSetContinuity(sht->GetContinuityBySymbol(CalChart::k_symbols[i]));
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
