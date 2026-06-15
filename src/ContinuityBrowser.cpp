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
#include "CalChartSplash.h"
#include "CalChartView.h"
#include "ContinuityBrowserPanel.h"
#include "basic_ui.h"

#include <wx/artprov.h>
#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wxUI/wxUI.hpp>

// a panel consists of the name, canvas
class ContinuityBrowserPerCont : public wxPanel {
    using super = wxPanel;

public:
    ContinuityBrowserPerCont(wxWindow* parent, CalChart::SYMBOL_TYPE sym, CalChart::Configuration const& config);
    ~ContinuityBrowserPerCont() override = default;

    void DoSetContinuity(CalChart::Continuity const& new_cont);

    void SetHandlers(ContinuityBrowserPanel::Handlers handlers);

private:
    wxUI::Factory<ContinuityBrowserPanel>::Proxy mCanvas{};
    CalChart::SYMBOL_TYPE mSym{};
};

ContinuityBrowserPerCont::ContinuityBrowserPerCont(wxWindow* parent, CalChart::SYMBOL_TYPE sym, CalChart::Configuration const& config)
    : super(parent)
    , mSym(sym)
{
    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 2).Proportion(0).Expand(),
        wxUI::HSizer{
            wxUI::Text{ CalChart::GetLongNameForSymbol(mSym) },
            wxUI::BitmapButton{ ScaleButtonBitmap(wxArtProvider::GetBitmap(wxART_PLUS)) }
                .bind([this]() {
                    mCanvas->AddNewEntry();
                })
                .withFlags(BasicSizerFlags()) },
        // here's a canvas
        wxUI::Factory{
            [this, &config](wxWindow* parent) {
                return new ContinuityBrowserPanel(mSym, config, parent);
            } }
            .withProxy(mCanvas),
        wxUI::Line{},
    }
        .fitTo(this);
}

void ContinuityBrowserPerCont::DoSetContinuity(CalChart::Continuity const& new_cont)
{
    mCanvas->DoSetContinuity(new_cont);
    if (auto* sizer = GetSizer()) {
        sizer->SetSizeHints(this); // Apply minimum size constraints from DoGetBestClientSize()
    }
    // Invalidate cached size so sizer recalculates from DoGetBestClientSize()
    InvalidateBestSize();
}

void ContinuityBrowserPerCont::SetHandlers(ContinuityBrowserPanel::Handlers handlers)
{
    mCanvas->SetHandlers(handlers);
}

ContinuityBrowser::ContinuityBrowser(wxWindow* parent, wxSize const& size, CalChart::Configuration const& config)
    : super(parent, wxID_ANY, wxDefaultPosition, size, wxScrolledWindowStyle)
{
    wxUI::VSizer{
        wxSizerFlags{}.Proportion(1).Expand(),
        wxUI::Factory{
            [this, &config](wxWindow* parent) {
                // create a scrollable window to contain all of the frame's content
                auto scrolledWindow = new wxScrolledWindow(parent, wxID_ANY);
                wxUI::VSizer{
                    wxSizerFlags{}.Border(wxALL, 2).Proportion(0).Expand(),
                    wxUI::Line{},
                    wxUI::VForEach(
                        CalChart::k_symbols,
                        [this, &config](auto eachcont) {
                            return wxUI::Factory{
                                [this, eachcont, &config](wxWindow* parent) {
                                    auto perCont = new ContinuityBrowserPerCont(parent, eachcont, config);
                                    perCont->Show(false);
                                    mPerCont.push_back(perCont);
                                    return perCont;
                                }
                            };
                        }),
                    wxUI::HSizer{
                        wxUI::Button{ wxID_HELP, "&Help" }
                            .bind([] {
                                CalChartSplash::Help();
                            }),
                    },
                }
                    .fitTo(scrolledWindow);

                // configure the minimum size of the window, and then add scroll bars
                scrolledWindow->SetMinSize(scrolledWindow->GetSizer()->ComputeFittingWindowSize(scrolledWindow));
                scrolledWindow->SetScrollRate(1, 1);

                return scrolledWindow;
            } }
    }
        .fitTo(this);
    // now update the current screen
    OnUpdate();
}

void ContinuityBrowser::OnUpdate()
{
    if (!mHandleGetContinuities) {
        return;
    }
    for (auto [i, optContBrowser] : CalChart::Ranges::enumerate_view(CalChart::Ranges::zip_view(mHandleGetContinuities(), mPerCont))) {
        auto&& [optCont, contBrowser] = optContBrowser;
        contBrowser->Show(optCont.has_value());
        if (optCont.has_value()) {
            contBrowser->DoSetContinuity(*optCont);
        }
    }
    // Relayout with new panel sizes, then update virtual size for scrolling
    if (auto* sizer = GetSizer()) {
        sizer->Layout(); // Use new best sizes from invalidated panels
        FitInside(); // Update virtual size - scrollbars appear if needed
    }
}

void ContinuityBrowser::SetHandlers(Handlers handlers)
{
    mHandleGetContinuities = std::get<0>(handlers);
    for (auto&& cont : mPerCont) {
        cont->SetHandlers(std::get<1>(handlers));
    }
    OnUpdate();
}
