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

#pragma once

#include <wx/wx.h>

class AnimationView;
class CC_coord;

class AnimationCanvas : public wxPanel {
public:
    AnimationCanvas(AnimationView* view, wxWindow* parent,
        const wxSize& size = wxDefaultSize);
    ~AnimationCanvas();

    void SetView(AnimationView* view);

    void OnPaint(wxPaintEvent& event);
    void OnLeftDownMouseEvent(wxMouseEvent& event);
    void OnLeftUpMouseEvent(wxMouseEvent& event);
    void OnRightUpMouseEvent(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnChar(wxKeyEvent& event);
    void SetZoomOnMarchers(bool zoomOnMarchers);
    bool GetZoomOnMarchers() const;
    void SetStepsOutForMarchersZoom(int steps);
    int GetStepsOutForMarchersZoom() const;

private:
    AnimationView* mAnimationView;

    float mUserScale = 1;
    std::pair<wxCoord, wxCoord> mUserOrigin = { 0, 0 };
    bool mZoomOnMarchers = false;
    int mStepsOutForMarcherZoom = 4;

    void UpdateScaleAndOrigin();

    // for mouse and drawing
    bool mMouseDown{};
    wxCoord mMouseXStart{};
    wxCoord mMouseYStart{};
    wxCoord mMouseXEnd{};
    wxCoord mMouseYEnd{};

    wxDECLARE_EVENT_TABLE();
};
