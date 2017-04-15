/*
 * animation_canvas.cpp
 * Animation canvas interface
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

#include "animation_canvas.h"
#include "animation_view.h"
#include "confgr.h"

#include <wx/dcbuffer.h>

BEGIN_EVENT_TABLE(AnimationCanvas, wxPanel)
EVT_CHAR(AnimationCanvas::OnChar)
EVT_LEFT_DOWN(AnimationCanvas::OnLeftDownMouseEvent)
EVT_LEFT_UP(AnimationCanvas::OnLeftUpMouseEvent)
EVT_RIGHT_UP(AnimationCanvas::OnRightUpMouseEvent)
EVT_MOTION(AnimationCanvas::OnMouseMove)
EVT_PAINT(AnimationCanvas::OnPaint)
END_EVENT_TABLE()

AnimationCanvas::AnimationCanvas(AnimationView* view, wxWindow* parent)
    : wxPanel(parent)
    , mAnimationView(view)
{
}

void AnimationCanvas::OnPaint(wxPaintEvent& event)
{
    // update the scale and origin
    UpdateScaleAndOrigin();

    wxBufferedPaintDC dc(this);
    auto& config = CalChartConfiguration::GetGlobalConfig();

    dc.SetBackground(config.Get_CalChartBrushAndPen(COLOR_FIELD).first);
    dc.Clear();
    dc.SetUserScale(mUserScale, mUserScale);
    dc.SetDeviceOrigin(mUserOrigin.first, mUserOrigin.second);

    // draw the box
    if (mMouseDown) {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_SHAPES).second);
        dc.DrawRectangle(mMouseXStart, mMouseYStart, mMouseXEnd - mMouseXStart,
            mMouseYEnd - mMouseYStart);
    }
    // draw the view
    if (mAnimationView) {
        mAnimationView->OnDraw(dc, config);
    }
}

// utilities
static auto CalcUserScaleAndOffset(std::pair<CC_coord, CC_coord> const& sizeAndOffset, wxSize const& windowSize)
{
    auto newX = static_cast<float>(windowSize.x);
    auto newY = static_cast<float>(windowSize.y);
    auto showSizeX = static_cast<float>(sizeAndOffset.first.x);
    auto showSizeY = static_cast<float>(sizeAndOffset.first.y);
    auto x = (showSizeX) ? showSizeX : newX;
    auto y = (showSizeY) ? showSizeY : newY;

    auto showAspectRatio = x / y;
    auto newSizeRatio = newX / newY;
    auto newvalue = 1.0;
    // always choose x when the new aspect ratio is smaller than the show.
    // This will keep the whole field on in the canvas
    if (newSizeRatio < showAspectRatio) {
        newvalue = newX / (float)Coord2Int(x);
    }
    else {
        newvalue = newY / (float)Coord2Int(y);
    }
    auto userScale = newvalue * (Coord2Int(1 << 16) / 65536.0);

    auto offsetx = sizeAndOffset.second.x + sizeAndOffset.first.x / 2;
    auto offsety = sizeAndOffset.second.y + sizeAndOffset.first.y / 2;
    auto scaledx = userScale * offsetx;
    auto scaledy = userScale * offsety;
    auto plus_windowx = windowSize.x / 2 - scaledx;
    auto plus_windowy = windowSize.y / 2 - scaledy;

    return std::pair<float, std::pair<wxCoord, wxCoord> >{ userScale, { plus_windowx, plus_windowy } };
}

void AnimationCanvas::UpdateScaleAndOrigin()
{
    if (!mAnimationView)
        return;
    auto window_size = GetSize();
    auto boundingBox = mZoomOnMarchers ? mAnimationView->GetMarcherSizeAndOffset()
                                       : mAnimationView->GetShowSizeAndOffset();
    if (mZoomOnMarchers) {
        auto amount = Int2Coord(mStepsOutForMarcherZoom);
        boundingBox.first += CC_coord(amount, amount) * 2;
        boundingBox.second -= CC_coord(amount, amount);
    }
    auto userScaleAndOffset = CalcUserScaleAndOffset(boundingBox, window_size);
    mUserScale = userScaleAndOffset.first;
    mUserOrigin = userScaleAndOffset.second;
}

void AnimationCanvas::OnLeftDownMouseEvent(wxMouseEvent& event)
{
    wxClientDC dc(this);
    dc.SetUserScale(mUserScale, mUserScale);
    dc.SetDeviceOrigin(mUserOrigin.first, mUserOrigin.second);
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    if (!event.AltDown() && !event.ShiftDown()) {
        mAnimationView->UnselectMarchers();
    }

    mMouseXEnd = mMouseXStart = x;
    mMouseYEnd = mMouseYStart = y;
    mMouseDown = true;
}

void AnimationCanvas::OnLeftUpMouseEvent(wxMouseEvent& event)
{
    wxClientDC dc(this);
    dc.SetUserScale(mUserScale, mUserScale);
    dc.SetDeviceOrigin(mUserOrigin.first, mUserOrigin.second);
    auto point = event.GetPosition();
    mMouseXEnd = dc.DeviceToLogicalX(point.x);
    mMouseYEnd = dc.DeviceToLogicalY(point.y);
    mMouseDown = false;

    // if mouse lifted very close to where clicked, then it is a previous beat
    // move
    if ((std::abs(mMouseXEnd - mMouseXStart) < Int2Coord(1) / 2) && (std::abs(mMouseYEnd - mMouseYStart) < Int2Coord(1) / 2)) {
        if (mAnimationView) {
            mAnimationView->PrevBeat();
        }
    }
    else {
        if (mAnimationView) {
            mAnimationView->SelectMarchersInBox(
                mMouseXStart, mMouseYStart, mMouseXEnd, mMouseYEnd, event.AltDown());
        }
    }
    Refresh();
}

void AnimationCanvas::OnRightUpMouseEvent(wxMouseEvent& event)
{
    if (mAnimationView) {
        mAnimationView->NextBeat();
    }
}

void AnimationCanvas::OnMouseMove(wxMouseEvent& event)
{
    wxClientDC dc(this);
    dc.SetUserScale(mUserScale, mUserScale);
    dc.SetDeviceOrigin(mUserOrigin.first, mUserOrigin.second);
    auto point = event.GetPosition();
    mMouseXEnd = dc.DeviceToLogicalX(point.x);
    mMouseYEnd = dc.DeviceToLogicalY(point.y);
    if (event.Dragging()) {
        Refresh();
    }
}

void AnimationCanvas::OnChar(wxKeyEvent& event)
{
    if (mAnimationView) {
        if (event.GetKeyCode() == WXK_LEFT)
            mAnimationView->PrevBeat();
        else if (event.GetKeyCode() == WXK_RIGHT)
            mAnimationView->NextBeat();
        else if (event.GetKeyCode() == WXK_SPACE) {
            mAnimationView->ToggleTimer();
        }
        else {
            event.Skip();
        }
    }
    else {
        event.Skip();
    }
}

void AnimationCanvas::SetZoomOnMarchers(bool zoomOnMarchers)
{
    mZoomOnMarchers = zoomOnMarchers;
    Refresh();
}

void AnimationCanvas::SetStepsOutForMarchersZoom(int steps)
{
    mStepsOutForMarcherZoom = steps;
    Refresh();
}
