#pragma once
/*
 * cc_onmiview_canvas.h
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

#include "draw_utils.h"
#include <map>
#include <memory>
#include <wx/glcanvas.h>

class AnimationView;
class CCOmniView_GLContext;
class CalChartConfiguration;

class CCOmniviewCanvas : public wxGLCanvas {
    using super = wxGLCanvas;
    wxDECLARE_EVENT_TABLE();

public:
    CCOmniviewCanvas(wxWindow* parent, CalChartConfiguration& config, const wxSize& size = wxDefaultSize);
    ~CCOmniviewCanvas();

    void SetView(AnimationView* view);
    auto GetView() const { return mView; }

    void OnPaint(wxPaintEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMouseMove(wxMouseEvent& event);

    // negative -1 is to stop following
    void OnCmd_FollowMarcher(int which);
    void OnCmd_SaveCameraAngle(size_t which);
    void OnCmd_GoToCameraAngle(size_t which);
    void OnCmd_ToggleCrowd();
    void OnCmd_ToggleMarching();
    void OnCmd_ToggleShowOnlySelected();
    void OnCmd_ShowKeyboardControls();

private:
    std::shared_ptr<CCOmniView_GLContext> m_glContext;
    AnimationView* mView{};
    CalChartConfiguration& config;
    ViewPoint mViewPoint;

    // a -1 means not following any marcher
    int mFollowMarcher;
    bool mCrowdOn;
    bool mShowMarching;
    float mViewAngle, mViewAngleZ;
    float mFOV;

    // for mouse camera move:
    bool mShiftMoving;
    wxPoint mStartShiftMoveMousePosition;
    float mStartShiftMoveViewAngle, mStartShiftMoveViewAngleZ;
};
