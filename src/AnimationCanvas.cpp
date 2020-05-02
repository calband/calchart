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

#include "AnimationCanvas.h"
#include "AnimationView.h"
#include "cc_coord.h"
#include "confgr.h"
#include "ui_enums.h"

#include <wx/dcbuffer.h>

BEGIN_EVENT_TABLE(AnimationCanvas, wxPanel)
EVT_CHAR(AnimationCanvas::OnChar)
EVT_LEFT_DOWN(AnimationCanvas::OnLeftDownMouseEvent)
EVT_LEFT_UP(AnimationCanvas::OnLeftUpMouseEvent)
EVT_MOTION(AnimationCanvas::OnMouseMove)
EVT_PAINT(AnimationCanvas::OnPaint)
END_EVENT_TABLE()

AnimationCanvas::AnimationCanvas(wxWindow* parent,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : super(parent, winid, pos, size, style, name)
{
}

void AnimationCanvas::OnPaint(wxPaintEvent& event)
{
    if (!mView) {
        return;
    }
    // update the scale and origin
    UpdateScaleAndOrigin();

    auto dc = wxBufferedPaintDC{ this };
    auto& config = CalChartConfiguration::GetGlobalConfig();

    dc.SetBackground(config.Get_CalChartBrushAndPen(COLOR_FIELD).first);
    dc.Clear();
    dc.SetUserScale(mUserScale, mUserScale);
    dc.SetDeviceOrigin(mUserOrigin.first, mUserOrigin.second);

    // draw the box
    if (mMouseDown) {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_SHAPES).second);
        dc.DrawRectangle(mMouseStart.x, mMouseStart.y, mMouseEnd.x - mMouseStart.x, mMouseEnd.y - mMouseStart.y);
    }
    // draw the view
    mView->OnDraw(&dc);
}

// utilities
static auto CalcUserScaleAndOffset(std::pair<wxPoint, wxPoint> const& sizeAndOffset, wxSize const& windowSize)
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
        newvalue = newX / (float)CoordUnits2Int(x);
    } else {
        newvalue = newY / (float)CoordUnits2Int(y);
    }
    auto userScale = newvalue * (CoordUnits2Int(1 << 16) / 65536.0);

    auto offsetx = sizeAndOffset.second.x + sizeAndOffset.first.x / 2;
    auto offsety = sizeAndOffset.second.y + sizeAndOffset.first.y / 2;
    auto scaledx = userScale * offsetx;
    auto scaledy = userScale * offsety;
    auto plus_windowx = windowSize.x / 2 - scaledx;
    auto plus_windowy = windowSize.y / 2 - scaledy;

    return std::pair<double, std::pair<wxCoord, wxCoord>>{ userScale, { plus_windowx, plus_windowy } };
}

void AnimationCanvas::UpdateScaleAndOrigin()
{
    if (!mView) {
        return;
    }
    auto window_size = GetSize();
    auto boundingBox = mZoomOnMarchers ? mView->GetMarcherSizeAndOffset() : mView->GetShowSizeAndOffset();
    if (mZoomOnMarchers) {
        auto amount = Int2CoordUnits(mStepsOutForMarcherZoom);
        boundingBox.first += wxPoint(amount, amount) * 2;
        boundingBox.second -= wxPoint(amount, amount);
    }
    auto userScaleAndOffset = CalcUserScaleAndOffset(boundingBox, window_size);
    mUserScale = userScaleAndOffset.first;
    mUserOrigin = userScaleAndOffset.second;
}

void AnimationCanvas::OnLeftDownMouseEvent(wxMouseEvent& event)
{
    if (!mView) {
        return;
    }
    auto point = TranslatePosition(event.GetPosition());

    if (!event.AltDown() && !event.ShiftDown()) {
        mView->UnselectAll();
    }

    mMouseEnd = mMouseStart = point;
    mMouseDown = true;
}

void AnimationCanvas::OnLeftUpMouseEvent(wxMouseEvent& event)
{
    if (!mView) {
        return;
    }
    auto point = TranslatePosition(event.GetPosition());
    mMouseEnd = point;
    mMouseDown = false;

    // if mouse lifted very close to where clicked, then it is a previous beat
    // move
    if ((std::abs(mMouseEnd.x - mMouseStart.x) < Int2CoordUnits(1) / 2) && (std::abs(mMouseEnd.y - mMouseStart.y) < Int2CoordUnits(1) / 2)) {
        mView->ToggleTimer();
    } else {
        mView->SelectMarchersInBox(mMouseStart, mMouseEnd, event.AltDown());
    }
    Refresh();
}

void AnimationCanvas::OnMouseMove(wxMouseEvent& event)
{
    mMouseEnd = TranslatePosition(event.GetPosition());
    if (event.Dragging()) {
        Refresh();
    }
}

wxPoint AnimationCanvas::TranslatePosition(wxPoint const& point)
{
    auto dc = wxClientDC{ this };
    dc.SetUserScale(mUserScale, mUserScale);
    dc.SetDeviceOrigin(mUserOrigin.first, mUserOrigin.second);
    return { dc.DeviceToLogicalX(point.x), dc.DeviceToLogicalY(point.y) };
}

void AnimationCanvas::OnChar(wxKeyEvent& event)
{
    if (!mView) {
        event.Skip();
        return;
    }
    if (event.GetKeyCode() == WXK_LEFT)
        mView->PrevBeat();
    else if (event.GetKeyCode() == WXK_RIGHT)
        mView->NextBeat();
    else if (event.GetKeyCode() == WXK_SPACE) {
        mView->ToggleTimer();
    } else {
        event.Skip();
    }
    Refresh();
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
