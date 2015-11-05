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

AnimationCanvas::AnimationCanvas(AnimationView* view, wxWindow* parent,
    const wxSize& size)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, size)
    , mAnimationView(view)
    , mMouseDown(false)
{
}

AnimationCanvas::~AnimationCanvas() {}

void AnimationCanvas::SetView(AnimationView* view) { mAnimationView = view; }

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
    if (mMouseDown) {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_SHAPES).second);
        dc.DrawRectangle(mMouseXStart, mMouseYStart, mMouseXEnd - mMouseXStart,
            mMouseYEnd - mMouseYStart);
    }
    if (mAnimationView) {
        mAnimationView->OnDraw(&dc, config);
    }
}

// utilities
static std::pair<float, std::pair<wxCoord, wxCoord> >
CalcUserScaleAndOffset(const std::pair<CC_coord, CC_coord>& sizeAndOffset,
    const wxSize& windowSize)
{
    float newX = windowSize.x;
    float newY = windowSize.y;
    float showSizeX = sizeAndOffset.first.x;
    float showSizeY = sizeAndOffset.first.y;
    float x = (showSizeX) ? showSizeX : newX;
    float y = (showSizeY) ? showSizeY : newY;

    float showAspectRatio = x / y;
    float newSizeRatio = newX / newY;
    float newvalue = 1.0;
    // always choose x when the new aspect ratio is smaller than the show.
    // This will keep the whole field on in the canvas
    if (newSizeRatio < showAspectRatio) {
        newvalue = newX / (float)Coord2Int(x);
    }
    else {
        newvalue = newY / (float)Coord2Int(y);
    }
    float userScale = newvalue * (Coord2Int(1 << 16) / 65536.0);

    auto offsetx = sizeAndOffset.second.x + sizeAndOffset.first.x / 2;
    auto offsety = sizeAndOffset.second.y + sizeAndOffset.first.y / 2;
    auto scaledx = userScale * offsetx;
    auto scaledy = userScale * offsety;
    auto plus_windowx = windowSize.x / 2 - scaledx;
    auto plus_windowy = windowSize.y / 2 - scaledy;

    return { userScale, { plus_windowx, plus_windowy } };
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
    long x, y;
    event.GetPosition(&x, &y);
    x = dc.DeviceToLogicalX(x);
    y = dc.DeviceToLogicalY(y);

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
    long x, y;
    event.GetPosition(&x, &y);
    x = dc.DeviceToLogicalX(x);
    y = dc.DeviceToLogicalY(y);
    mMouseXEnd = x;
    mMouseYEnd = y;
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
    long x, y;
    event.GetPosition(&x, &y);
    x = dc.DeviceToLogicalX(x);
    y = dc.DeviceToLogicalY(y);
    mMouseXEnd = x;
    mMouseYEnd = y;
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

bool AnimationCanvas::GetZoomOnMarchers() const { return mZoomOnMarchers; }

void AnimationCanvas::SetZoomOnMarchers(bool zoomOnMarchers)
{
    mZoomOnMarchers = zoomOnMarchers;
    Refresh();
}

size_t AnimationCanvas::GetStepsOutForMarchersZoom() const
{
    return mStepsOutForMarcherZoom;
}

void AnimationCanvas::SetStepsOutForMarchersZoom(size_t steps)
{
    mStepsOutForMarcherZoom = steps;
    Refresh();
}
