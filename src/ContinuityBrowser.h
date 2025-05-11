#pragma once
/*
 * ContinuityBrowser
 * Header for continuity editors
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

#include <wx/wx.h>
#include <wxUI/wxUI.hpp>

class CalChartView;
class ContinuityBrowserPerCont;
namespace CalChart {
class Configuration;
}

// ContinuityBrowser
// The way you view the continuity for marchers

class ContinuityBrowser : public wxScrolledWindow {
    using super = wxScrolledWindow;

public:
    ContinuityBrowser(wxWindow* parent, wxSize const& size, CalChart::Configuration const& config);
    ~ContinuityBrowser() override;

    void OnUpdate(); // Refresh from the View
    void SetView(CalChartView* view);
    auto GetView() const { return mView; }

private:
    void Init();
    void CreateControls(CalChart::Configuration const& config);

    // Internals
    CalChartView* mView{};
    std::vector<ContinuityBrowserPerCont*> mPerCont;
};
