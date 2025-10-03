/*
 * animation_canvas.cpp
 * Animation canvas interface
 */

/*
   Copyright (C) 1995-2024  Richard Michael Powell

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
#include "AnimationPanel.h"
#include "CalChartConfiguration.h"
#include "CalChartCoord.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "basic_ui.h"
#include "ui_enums.h"
#include <wx/dcbuffer.h>
#include <wxUI/wxUI.hpp>

BEGIN_EVENT_TABLE(AnimationCanvas, AnimationCanvas::super)
EVT_CHAR(AnimationCanvas::OnChar)
EVT_LEFT_DOWN(AnimationCanvas::OnLeftDownMouseEvent)
EVT_LEFT_UP(AnimationCanvas::OnLeftUpMouseEvent)
EVT_MOTION(AnimationCanvas::OnMouseMove)
EVT_PAINT(AnimationCanvas::OnPaint)
END_EVENT_TABLE()

AnimationCanvas::AnimationCanvas(
    CalChart::Configuration const& config,
    AnimationPanel& parent,
    wxSize const& size)
    : super(&parent, wxID_ANY, wxDefaultPosition, size)
    , mPanel{ parent }
    , mConfig(config)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Init();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
}

void AnimationCanvas::Init()
{
}

void AnimationCanvas::CreateControls()
{
    wxUI::VSizer{}.fitTo(this);
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

// predeclared utility function
static auto CalcUserScaleAndOffset(std::pair<wxSize, wxPoint> const& sizeAndOffset, wxSize const& windowSize)
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
        newvalue = newX / (float)CalChart::CoordUnits2Int(x);
    } else {
        newvalue = newY / (float)CalChart::CoordUnits2Int(y);
    }
    auto userScale = newvalue * (CalChart::CoordUnits2Int(1 << 16) / 65536.0);

    auto offsetx = sizeAndOffset.second.x + sizeAndOffset.first.x / 2;
    auto offsety = sizeAndOffset.second.y + sizeAndOffset.first.y / 2;
    auto scaledx = userScale * offsetx;
    auto scaledy = userScale * offsety;
    auto plus_windowx = windowSize.x / 2 - scaledx;
    auto plus_windowy = windowSize.y / 2 - scaledy;

    return std::pair<double, std::pair<wxCoord, wxCoord>>{ userScale, { plus_windowx, plus_windowy } };
}

void AnimationCanvas::OnUpdate()
{
}

void AnimationCanvas::OnPaint(wxPaintEvent&)
{
    // update the scale and origin
    UpdateScaleAndOrigin();

    wxBufferedPaintDC dc(this);

    wxCalChart::setBackground(dc, mConfig.Get_CalChartBrushAndPen(CalChart::Colors::FIELD));
    dc.Clear();
    dc.SetUserScale(mUserScale, mUserScale);
    dc.SetDeviceOrigin(mUserOrigin.first, mUserOrigin.second);

    // draw the box
    if (mMouseDown) {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        wxCalChart::setPen(dc, mConfig.Get_CalChartBrushAndPen(CalChart::Colors::SHAPES));
        dc.DrawRectangle(mMouseStart.x, mMouseStart.y, mMouseEnd.x - mMouseStart.x, mMouseEnd.y - mMouseStart.y);
    }
    wxCalChart::Draw::DrawCommandList(dc,
        mPanel.GenerateDrawCommands());
}

void AnimationCanvas::OnLeftDownMouseEvent(wxMouseEvent& event)
{
    auto point = TranslatePosition(event.GetPosition());

    if (!event.AltDown() && !event.ShiftDown()) {
        mPanel.UnselectAll();
    }

    mMouseEnd = mMouseStart = point;
    mMouseDown = true;
}

void AnimationCanvas::OnLeftUpMouseEvent(wxMouseEvent& event)
{
    auto point = TranslatePosition(event.GetPosition());
    mMouseEnd = point;
    mMouseDown = false;

    // if mouse lifted very close to where clicked, then it is a previous beat
    // move
    if ((std::abs(mMouseEnd.x - mMouseStart.x) < CalChart::Int2CoordUnits(1) / 2) && (std::abs(mMouseEnd.y - mMouseStart.y) < CalChart::Int2CoordUnits(1) / 2)) {
        mPanel.ToggleTimer();
    } else {
        mPanel.SelectMarchersInBox(mMouseStart, mMouseEnd, event.AltDown());
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

void AnimationCanvas::OnChar(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_LEFT)
        mPanel.PrevBeat();
    else if (event.GetKeyCode() == WXK_RIGHT)
        mPanel.NextBeat();
    else if (event.GetKeyCode() == WXK_SPACE) {
        mPanel.ToggleTimer();
    } else {
        event.Skip();
    }
    Refresh();
}

auto AnimationCanvas::TranslatePosition(wxPoint const& point) -> wxPoint
{
    wxClientDC dc{ this };
    dc.SetUserScale(mUserScale, mUserScale);
    dc.SetDeviceOrigin(mUserOrigin.first, mUserOrigin.second);
    return { dc.DeviceToLogicalX(point.x), dc.DeviceToLogicalY(point.y) };
}

namespace {
auto towxSize(CalChart::Coord const& c)
{
    return fDIP(wxSize{ c.x, c.y });
}

auto towxPoint(CalChart::Coord const& c)
{
    return fDIP(wxPoint{ c.x, c.y });
}

auto towxBox(std::pair<CalChart::Coord, CalChart::Coord> input) -> std::pair<wxSize, wxPoint>
{
    return { towxSize(input.first), towxPoint(input.second) };
}
} // namespace

void AnimationCanvas::UpdateScaleAndOrigin()
{
    auto window_size = GetSize();
    auto boundingBox = towxBox(mPanel.GetAnimationBoundingBox(mZoomOnMarchers));
    if (mZoomOnMarchers) {
        auto amount = CalChart::Int2CoordUnits(mStepsOutForMarcherZoom);
        boundingBox.first += wxSize(amount, amount) * 2;
        boundingBox.second -= wxPoint(amount, amount);
    }
    auto userScaleAndOffset = CalcUserScaleAndOffset(boundingBox, window_size);
    mUserScale = userScaleAndOffset.first;
    mUserOrigin = userScaleAndOffset.second;
}
