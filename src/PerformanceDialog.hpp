#pragma once
/*
 * PerformanceDialog.hpp
 * Dialog for displaying performance metrics
 */

/*
   Copyright (C) 2024  Richard Michael Powell

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

namespace CalChart {
class PerformanceRegistry;
}

class wxListCtrl;

// Modeless dialog for displaying paint performance metrics
class PerformanceDialog : public wxDialog {
    using super = wxDialog;

public:
    PerformanceDialog(wxWindow* parent, CalChart::PerformanceRegistry& registry);
    ~PerformanceDialog() override = default;

private:
    void CreateControls();
    void RefreshData();
    void ResetStats();
    void OnClose(wxCloseEvent& event);

    CalChart::PerformanceRegistry& mRegistry;
#if 1
    wxUI::Factory<wxListCtrl>::Proxy mListCtrl;
    wxUI::Text::Proxy mSummaryText;
#else
    wxListCtrl* mListCtrl{};
    wxStaticText* mSummaryText{};
#endif

    DECLARE_EVENT_TABLE()
};
