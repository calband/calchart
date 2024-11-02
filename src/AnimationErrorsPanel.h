#pragma once
/*
 * AnimationErrorsPanel
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

#include "CalChartAnimationErrors.h"

#include <map>
#include <vector>
#include <wx/treelist.h>
#include <wx/wx.h>
#include <wxUI/wxUI.h>

class CalChartView;

class AnimationErrorsPanel : public wxPanel {
    using super = wxPanel;
    DECLARE_EVENT_TABLE()

public:
    AnimationErrorsPanel(wxWindow* parent, wxWindowID winid = wxID_ANY, wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER, wxString const& name = wxPanelNameStr);
    ~AnimationErrorsPanel() override = default;

    void OnUpdate(); // Refresh from the View
    void SetView(CalChartView* view) { mView = view; }
    auto GetView() const { return mView; }

private:
    void Init();
    void CreateControls();

    // Event Handlers
    void OnSelectionChanged(wxTreeListEvent& event);
    void OnItemActivated(wxTreeListEvent& event);

    // Internals
    void UpdateErrors(std::vector<CalChart::Animate::Errors> const& errors, std::map<int, CalChart::SelectionList> const& collisions);

    CalChartView* mView{};
    wxUI::Generic<wxTreeListCtrl>::Proxy mTreeCtrl{};
    std::vector<CalChart::Animate::Errors> mCurrentErrors{};
    std::map<wxTreeListItem, std::tuple<int, CalChart::SelectionList>> mErrorLookup{};
};
