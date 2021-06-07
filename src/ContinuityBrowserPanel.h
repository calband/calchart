#pragma once
/*
 * ContinuityBrowserPanel
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

#include "CalChartContinuity.h"
#include "CalChartTypes.h"
#include "CustomListViewPanel.h"

#include <wx/wx.h>

class CalChartView;
class CalChartConfiguration;

class ContinuityBrowserPanel : public CustomListViewPanel {
    using super = CustomListViewPanel;
    DECLARE_EVENT_TABLE()

public:
    // Basic functions
    ContinuityBrowserPanel(SYMBOL_TYPE sym, CalChartConfiguration& config, wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxScrolledWindowStyle,
        const wxString& name = wxPanelNameStr);
    virtual ~ContinuityBrowserPanel() = default;

    void SetView(CalChartView* view) { mView = view; }
    auto GetView() const { return mView; }

    void DoSetContinuity(CalChart::Continuity const& new_cont);
    void AddNewEntry();

private:
    void Init();

    // Event Handlers
    void OnNewEntry(int cell) override;
    void OnEditEntry(int cell) override;
    void OnDeleteEntry(int cell) override;
    void OnMoveEntry(int start_cell, int end_cell) override;

    // Internals
    void DoSetFocus(wxFocusEvent& event);
    void DoKillFocus(wxFocusEvent& event);

    void UpdateCont(CalChart::Continuity const& new_cont);

    CalChartView* mView{};
    CalChart::Continuity mCont{};
    SYMBOL_TYPE mSym{};
    CalChartConfiguration& mConfig;
};
