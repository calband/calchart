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

#include "animatecompile.h"

#include <wx/treelist.h>
#include <wx/wx.h>

class CalChartView;

class AnimationErrorsPanel : public wxPanel {
    using super = wxPanel;
    DECLARE_EVENT_TABLE()

public:
    AnimationErrorsPanel(wxWindow* parent);
    virtual ~AnimationErrorsPanel() override = default;

    virtual void OnUpdate(); // Refresh all window controls

    void SetView(CalChartView* view) { mView = view; }
    auto GetView() const { return mView; }
    void OnSelectionChanged(wxTreeListEvent& event);
    void OnItemActivated(wxTreeListEvent& event);

private:
    void UpdateErrors(std::vector<CalChart::AnimationErrors> const& errors);

    CalChartView* mView;
    wxTreeListCtrl* mTreeCtrl;
    std::vector<CalChart::AnimationErrors> mCurrentErrors;
    std::map<wxTreeListItem, std::tuple<int, CalChart::ErrorMarker>> mErrorLookup;
};
