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

#include "CalChartAngles.h"
#include "DCSaveRestore.h"
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
    struct ViewPoint {
        float x, y, z;
    };

    CCOmniviewCanvas(wxWindow* parent, CalChartConfiguration& config, wxSize const& size = wxDefaultSize);
    ~CCOmniviewCanvas() override = default;

    void OnUpdate(); // Refresh from the View
    void SetView(AnimationView* view);
    auto GetView() const { return mView; }

    void OnCmd_ShowKeyboardControls();

private:
    void Init();
    void CreateControls();

    // Event Handlers
    void OnPaint(wxPaintEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnCmd_FollowMarcher(int which);
    void OnCmd_SaveCameraAngle(size_t which);
    void OnCmd_GoToCameraAngle(size_t which);
    void OnCmd_ToggleCrowd();
    void OnCmd_ToggleMarching();
    void OnCmd_ToggleShowOnlySelected();

    std::shared_ptr<CCOmniView_GLContext> m_glContext;
    AnimationView* mView{};
    CalChartConfiguration& config;

    ViewPoint mViewPoint{};

    // a -1 means not following any marcher
    int mFollowMarcher = -1;
    bool mCrowdOn = false;
    bool mShowMarching = true;
    CalChart::Radian mViewAngle{};
    CalChart::Radian mViewAngleZ{};
    float mFOV = 60;

    // for mouse camera move:
    bool mShiftMoving = false;
    wxPoint mStartShiftMoveMousePosition{};
    CalChart::Radian mStartShiftMoveViewAngle{};
    CalChart::Radian mStartShiftMoveViewAngleZ{};
};
