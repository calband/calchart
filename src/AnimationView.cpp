/*
 * AnimationView.cpp
 * Animation user interface
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

#include "AnimationView.h"
#include "AnimationPanel.h"
#include "CalChartApp.h"
#include "CalChartView.h"
#include "animate.h"
#include "animate_types.h"
#include "cc_shapes.h"
#include "cc_sheet.h"
#include "confgr.h"
#include "draw.h"
#include "modes.h"

#include <wx/dcbuffer.h>

#include <fstream>

using namespace CalChart;
// IMPLEMENT_DYNAMIC_CLASS(AnimationView, wxView)

template <typename Float>
static auto NormalizeAngle(Float angle)
{
    while (angle > 2 * M_PI) {
        angle -= (Float)(2 * M_PI);
    }
    while (angle < 0.0) {
        angle += (Float)(2 * M_PI);
    }
    return angle;
}

AnimationView::AnimationView(CalChartView* view, wxWindow* frame)
    : mView(view)
{
    SetFrame(frame);
}

void AnimationView::OnDraw(wxDC* dc)
{
    auto& config = CalChartConfiguration::GetGlobalConfig();
    OnDraw(*dc, config);
}

void AnimationView::OnDraw(wxDC& dc, const CalChartConfiguration& config)
{
    dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_FIELD_DETAIL).second);
    DrawMode(dc, config, mView->GetShowMode(), ShowMode_kAnimation);
    const bool checkForCollision = mDrawCollisionWarning;
    if (mAnimation) {
        for (auto i = 0; i < mView->GetNumPoints(); ++i) {
            auto info = mAnimation->GetAnimateInfo(i);

            if (checkForCollision && info.mCollision) {
                if (info.mCollision == Coord::COLLISION_WARNING) {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(
                        COLOR_POINT_ANIM_COLLISION_WARNING);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } else if (info.mCollision == Coord::COLLISION_INTERSECT) {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_COLLISION);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                }
            } else if (mView->IsSelected(i)) {
                switch (info.mDirection) {
                case ANIMDIR_SW:
                case ANIMDIR_W:
                case ANIMDIR_NW: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_HILIT_BACK);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } break;
                case ANIMDIR_SE:
                case ANIMDIR_E:
                case ANIMDIR_NE: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_HILIT_FRONT);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } break;
                default: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_HILIT_SIDE);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                }
                }
            } else {
                switch (info.mDirection) {
                case ANIMDIR_SW:
                case ANIMDIR_W:
                case ANIMDIR_NW: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_BACK);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } break;
                case ANIMDIR_SE:
                case ANIMDIR_E:
                case ANIMDIR_NE: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_FRONT);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } break;
                default: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_SIDE);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                }
                }
            }
            Coord position = info.mPosition;
            auto x = position.x + mView->GetShowMode().Offset().x;
            auto y = position.y + mView->GetShowMode().Offset().y;
            dc.DrawRectangle(x - Int2CoordUnits(1) / 2, y - Int2CoordUnits(1) / 2,
                Int2CoordUnits(1), Int2CoordUnits(1));
        }
    }
}

void AnimationView::OnUpdate(wxView* sender, wxObject* hint)
{
    wxView::OnUpdate(sender, hint);
    if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_modified))) {
        Generate();
    }
    if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_FinishedLoading))) {
        Generate();
    }
}

void AnimationView::RefreshFrame()
{
    if (mPlayCollisionWarning && mAnimation && mAnimation->CurrentBeatHasCollision() && CalChartConfiguration::GetGlobalConfig().Get_BeepOnCollisions()) {
        wxBell();
    }
    GetAnimationFrame()->UpdatePanel();
    GetAnimationFrame()->Refresh();
}

void AnimationView::RefreshSheet()
{
    GotoSheet(mView->GetCurrentSheetNum());
}

int AnimationView::GetNumberSheets() const { return (mAnimation) ? mAnimation->GetNumberSheets() : 0; }
int AnimationView::GetCurrentSheet() const { return (mAnimation) ? mAnimation->GetCurrentSheet() : 0; }
int AnimationView::GetNumberBeats() const { return (mAnimation) ? mAnimation->GetNumberBeats() : 0; }
int AnimationView::GetCurrentBeat() const { return (mAnimation) ? mAnimation->GetCurrentBeat() : 0; }
int AnimationView::GetTotalNumberBeats() const { return (mAnimation) ? mAnimation->GetTotalNumberBeats() : 0; }
int AnimationView::GetTotalCurrentBeat() const { return (mAnimation) ? mAnimation->GetTotalCurrentBeat() : 0; }

//#define GENERATE_SHOW_DUMP 1

void AnimationView::Generate()
{
    mAnimation = mView->GetAnimationInstance();
}

// true if changes made
bool AnimationView::PrevBeat()
{
    if (mAnimation && mAnimation->PrevBeat()) {
        RefreshFrame();
        return true;
    }
    return false;
}

bool AnimationView::NextBeat()
{
    if (mAnimation && mAnimation->NextBeat()) {
        RefreshFrame();
        return true;
    }
    return false;
}

void AnimationView::GotoBeat(unsigned i)
{
    if (mAnimation) {
        mAnimation->GotoBeat(i);
        RefreshFrame();
    }
}

void AnimationView::GotoTotalBeat(unsigned i)
{
    if (mAnimation) {
        mAnimation->GotoTotalBeat(i);
        RefreshFrame();
    }
}

bool AnimationView::PrevSheet()
{
    if (mAnimation && mAnimation->PrevSheet()) {
        RefreshFrame();
        return true;
    }
    return false;
}

bool AnimationView::NextSheet()
{
    if (mAnimation && mAnimation->NextSheet()) {
        RefreshFrame();
        return true;
    }
    return false;
}

void AnimationView::GotoSheet(unsigned i)
{
    if (mAnimation) {
        mAnimation->GotoSheet(i);
        RefreshFrame();
    }
}

void AnimationView::GotoAnimationSheet(unsigned i)
{
    if (mAnimation) {
        mAnimation->GotoAnimationSheet(i);
        RefreshFrame();
    }
}

static wxPoint towxPoint(CalChart::Coord const& c)
{
    return { c.x, c.y };
}

// Return a bounding box of the show
std::pair<wxPoint, wxPoint> AnimationView::GetShowSizeAndOffset() const
{
    auto size = mView->GetShowMode().Size();
    return { towxPoint(size), wxPoint(0, 0) };
}

// Return a bounding box of the show of where the marchers are.  If they are
// outside the show, we don't see them.
std::pair<wxPoint, wxPoint> AnimationView::GetMarcherSizeAndOffset() const
{
    auto mode_size = towxPoint(mView->GetShowMode().Size());
    auto bounding_box_upper_left = mode_size;
    auto bounding_box_low_right = wxPoint(0, 0);

    for (auto i = 0; mAnimation && i < mView->GetNumPoints(); ++i) {
        auto position = towxPoint(mAnimation->GetAnimateInfo(i).mPosition);
        bounding_box_upper_left = wxPoint(std::min(bounding_box_upper_left.x, position.x),
            std::min(bounding_box_upper_left.y, position.y));
        bounding_box_low_right = wxPoint(std::max(bounding_box_low_right.x, position.x),
            std::max(bounding_box_low_right.y, position.y));
    }

    return { bounding_box_low_right - bounding_box_upper_left,
        mode_size / 2 + bounding_box_upper_left };
}

void AnimationView::UnselectAll()
{
    mView->UnselectAll();
}

void AnimationView::SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown)
{
    using namespace CalChart;
    // otherwise, Select points within rectangle
    auto x_off = mView->GetShowMode().Offset().x;
    auto y_off = mView->GetShowMode().Offset().y;
    Lasso lasso(Coord(mouseStart.x - x_off, mouseStart.y - y_off));
    lasso.Append(Coord(mouseStart.x - x_off, mouseEnd.y - y_off));
    lasso.Append(Coord(mouseEnd.x - x_off, mouseEnd.y - y_off));
    lasso.Append(Coord(mouseEnd.x - x_off, mouseStart.y - y_off));
    lasso.End();
    mView->SelectWithLasso(&lasso, altDown);
}

// Keystroke command toggles the timer, but the timer
// lives in the Frame.  So we have this weird path...
void AnimationView::ToggleTimer() { GetAnimationFrame()->ToggleTimer(); }

bool AnimationView::OnBeat() const { return GetAnimationFrame()->OnBeat(); }

AnimationPanel* AnimationView::GetAnimationFrame()
{
    return static_cast<AnimationPanel*>(GetFrame());
}

const AnimationPanel* AnimationView::GetAnimationFrame() const
{
    return static_cast<const AnimationPanel*>(GetFrame());
}

CalChart::ShowMode const& AnimationView::GetShowMode() const
{
    return mView->GetShowMode();
}

MarcherInfo AnimationView::GetMarcherInfo(int which) const
{
    MarcherInfo info{};
    if (mAnimation) {
        auto anim_info = mAnimation->GetAnimateInfo(which);
        info.direction = NormalizeAngle((anim_info.mRealDirection * M_PI / 180.0));

        auto position = anim_info.mPosition;
        info.x = CoordUnits2Float(position.x);
        // because the coordinate system for continuity and OpenGL are different,
        // correct here.
        info.y = -1.0 * CoordUnits2Float(position.y);
    }
    return info;
}

std::multimap<double, MarcherInfo> AnimationView::GetMarchersByDistance(ViewPoint const& from) const
{
    auto anySelected = !mView->GetSelectionList().empty();
    std::multimap<double, MarcherInfo> result;
    for (auto i = 0; (i < mView->GetNumPoints()); ++i) {
        if (anySelected && !mView->IsSelected(i)) {
            continue;
        }
        auto info = GetMarcherInfo(i);
        auto distance = sqrt(pow(from.x - info.x, 2) + pow(from.y - info.y, 2));
        result.insert(std::pair<double, MarcherInfo>(distance, info));
    }
    return result;
}

int AnimationView::GetNumPoints() const
{
    return mView->GetNumPoints();
}
