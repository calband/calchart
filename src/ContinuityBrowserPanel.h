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

#include "CustomListViewPanel.h"
#include "cc_continuity.h"
#include "cc_types.h"

#include <wx/wx.h>

class CalChartDoc;
class CalChartConfiguration;

class ContinuityBrowserPanel : public CustomListViewPanel {
    using super = CustomListViewPanel;

public:
    // Basic functions
    ContinuityBrowserPanel(CalChartDoc* doc, SYMBOL_TYPE sym, CalChartConfiguration& config, wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxScrolledWindowStyle,
        const wxString& name = wxPanelNameStr);
    virtual ~ContinuityBrowserPanel() = default;
    void DoSetContinuity(CalChart::Continuity const& new_cont);

private:
    virtual void OnNewEntry(int cell) override;
    virtual void OnEditEntry(int cell) override;
    virtual void OnDeleteEntry(int cell) override;
    virtual void OnMoveEntry(int start_cell, int end_cell) override;

    void DoSetFocus(wxFocusEvent& event);
    void DoKillFocus(wxFocusEvent& event);

    void UpdateCont(CalChart::Continuity const& new_cont);

    CalChartDoc* mDoc;
    CalChart::Continuity mCont;
    SYMBOL_TYPE mSym;
    CalChartConfiguration& mConfig;
    DECLARE_EVENT_TABLE()
};
