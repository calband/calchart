#pragma once
/*
 * PrintContinuityEditor.h
 * Header for continuity editors
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

#include <wx/dialog.h>
#include <wx/docview.h>
#include <wx/wx.h>

class CalChartView;
class PrintContinuityEditor;
class FancyTextWin;
class PrintContinuityPreview;
class wxSplitterWindow;

// PrintContinuityEditor
// The way you edit the continuity for individual marchers
// This dialog should notify the user to save if there are any outstanding
// edits.
class PrintContinuityEditor : public wxPanel {
    using super = wxPanel;
    DECLARE_EVENT_TABLE()

public:
    PrintContinuityEditor(wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxPanelNameStr);
    ~PrintContinuityEditor() = default;

    void OnCmdHelp(wxCommandEvent& event);

    void SetView(CalChartView* view) { mView = view; }
    void OnUpdate() { Update(); }

    void Update(); // Refresh all window controls
    // Update text window to current continuity
    // quick doesn't flush other windows
    void UpdateText();

    void FlushText(); // Flush changes in text window

    void SetInsertionPoint(int x, int y);

    auto GetInMiniMode() const { return mInMiniMode; }
    void SetInMiniMode(bool);

private:
    void Init();

    bool Create(wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxPanelNameStr);

    void CreateControls();

    void ContEditCurrent(wxCommandEvent&);
    void OnKeyPress(wxCommandEvent&);
    void OnNameEnter(wxCommandEvent&);
    void OnPrevious(wxCommandEvent&);
    void OnNext(wxCommandEvent&);
    void OnSaveTimerExpired(wxTimerEvent& event);
    void OnSizeEvent(wxSizeEvent& event);

    CalChartView* mView{};
    FancyTextWin* mUserInput{};
    PrintContinuityPreview* mPrintContDisplay{};
    wxSplitterWindow* mSplitter{};
    wxTimer* mTimer{};
    std::vector<wxWindow*> mItemsToHide;

    wxString mPreviousText{};
    bool mInMiniMode{};
};
