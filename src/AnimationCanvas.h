#pragma once
/*
 * animation_canvas.h
 * Header for animation canvas interface
 */

/*
   Copyright (C) 1995-2012  Richard Michael Powell

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

class AnimationView;

// holds an instance of animation for the reference to draw.
class AnimationCanvas : public wxPanel {
    using super = wxPanel;
    wxDECLARE_EVENT_TABLE();

public:
    AnimationCanvas(wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxPanelNameStr);
    ~AnimationCanvas() = default;

    void SetView(AnimationView* view) { mView = view; }
    auto GetView() const { return mView; }

    void SetZoomOnMarchers(bool zoomOnMarchers);
    auto GetZoomOnMarchers() const { return mZoomOnMarchers; }
    void SetStepsOutForMarchersZoom(int steps);
    auto GetStepsOutForMarchersZoom() const { return mStepsOutForMarcherZoom; }

private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDownMouseEvent(wxMouseEvent& event);
    void OnLeftUpMouseEvent(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnChar(wxKeyEvent& event);

    wxPoint TranslatePosition(wxPoint const& point);
    void UpdateScaleAndOrigin();

    AnimationView* mView{};

    float mUserScale = 1.0f;
    std::pair<wxCoord, wxCoord> mUserOrigin = { 0, 0 };
    bool mZoomOnMarchers = true;
    int mStepsOutForMarcherZoom = 8;

    // for mouse and drawing
    bool mMouseDown{};
    wxPoint mMouseStart{};
    wxPoint mMouseEnd{};
};
