#pragma once
/*
 * FieldThumbnailBrowser
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

#include <wx/docview.h>

class CalChartView;
namespace CalChart {
class Configuration;
}

class FieldThumbnailBrowser : public wxScrolledWindow {
    using super = wxScrolledWindow;
    DECLARE_EVENT_TABLE()

public:
    FieldThumbnailBrowser(
        CalChart::Configuration const& config,
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxScrolledWindowStyle,
        const wxString& name = wxPanelNameStr);
    virtual ~FieldThumbnailBrowser() override = default;

    void OnUpdate(); // Refresh from the View
    void SetView(CalChartView* view) { mView = view; }
    auto GetView() const { return mView; }

private:
    void OnPaint(wxPaintEvent& event);
    void HandleKey(wxKeyEvent& event);
    void HandleMouseDown(wxMouseEvent& event);
    void HandleSizeEvent(wxSizeEvent& event);

    wxSize SizeOfOneCell() const;
    wxSize SizeOfOneCell(bool horizontal) const;
    int WhichCell(wxPoint const& p) const;

    CalChartView* mView{};

    // const int mXLeftPadding{ 4 };
    // const int mXRightPadding{ 4 };
    const int mXScrollPadding{};
    // const int mYUpperPadding{ 4 };
    const int mYNameSize{ 16 };
    // const int mYNamePadding{ 4 };
    // const int mYBottomPadding{ 4 };
    const int mYScrollPadding{};

    bool mLayoutHorizontal{ false };
    CalChart::Configuration const& mConfig;
};
