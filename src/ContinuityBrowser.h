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

#include "ContinuityBrowserPanel.h"
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
    ~ContinuityBrowser() override = default;

    void OnUpdate(); // Refresh from the View

    using HandleGetContinuities = std::function<std::vector<std::optional<CalChart::Continuity>>()>;
    using Handlers = std::tuple<HandleGetContinuities, ContinuityBrowserPanel::Handlers>;

    void SetHandlers(Handlers handlers);

private:
    HandleGetContinuities mHandleGetContinuities{};
    std::vector<ContinuityBrowserPerCont*> mPerCont;
};
