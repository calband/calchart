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
#include "CalChartAnimation.h"
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartDrawing.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "CalChartShowMode.h"
#include "CalChartSizes.h"
#include "CalChartUtils.h"
#include "CalChartView.h"
#include "platconf.h"

#include <wx/dcbuffer.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

AnimationView::AnimationView(CalChartView* view, wxWindow* frame)
    : mView(view)
{
    SetFrame(frame);

#if defined(__APPLE__) && (__APPLE__)
    const static wxString kImageDir = "CalChart.app/Contents/Resources/default_sprite_strip.png";
#else
    const static wxString kImageDir = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath().Append(PATH_SEPARATOR "resources" PATH_SEPARATOR "default_sprite_strip.png");
#endif
    auto image = []() {
        wxImage image;
        if (!image.LoadFile(kImageDir)) {
            wxLogError("Couldn't load image from " + kImageDir + ".");
            return image;
        }
        return image;
    }();
    // now slice up all the images
    auto points = std::vector<int>(mSpriteImages.size());
    std::iota(points.begin(), points.end(), 0);
    std::transform(points.begin(), points.end(), mSpriteImages.begin(), [&image](auto i) {
        constexpr auto image_X = 64;
        constexpr auto image_Y = 128;
        return image.GetSubImage({ i * image_X + 0, 0, image_X, image_Y });
    });
}

AnimationView::~AnimationView()
{
}

void AnimationView::OnDraw(wxDC* dc)
{
    auto& config = CalChartConfiguration::GetGlobalConfig();
    OnDraw(*dc, config);
}

void AnimationView::OnDraw(wxDC& dc, CalChartConfiguration const& config)
{
    if (!mAnimation) {
        // no animation, our job is done.
        return;
    }
    dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_FIELD_DETAIL).second);
    CalChartDraw::DrawMode(dc, config, mView->GetShowMode(), ShowMode_kAnimation);
    auto useSprites = config.Get_UseSprites();
    if (useSprites) {
        return OnDrawSprites(dc, config);
    }
    return OnDrawDots(dc, config);
}

void AnimationView::OnDrawDots(wxDC& dc, CalChartConfiguration const& config)
{
    auto checkForCollision = mDrawCollisionWarning;
    for (auto info : mAnimation->GetAllAnimateInfo()) {
        if (checkForCollision && (info.mCollision != CalChart::Coord::CollisionType::none)) {
            if (info.mCollision == CalChart::Coord::CollisionType::warning) {
                auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_COLLISION_WARNING);
                dc.SetBrush(brushAndPen.first);
                dc.SetPen(brushAndPen.second);
            } else if (info.mCollision == CalChart::Coord::CollisionType::intersect) {
                auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_COLLISION);
                dc.SetBrush(brushAndPen.first);
                dc.SetPen(brushAndPen.second);
            }
        } else if (mView->IsSelected(info.index)) {
            switch (AngleToDirection(info.mFacingDirection)) {
            case CalChart::Direction::SouthWest:
            case CalChart::Direction::West:
            case CalChart::Direction::NorthWest: {
                auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_HILIT_BACK);
                dc.SetBrush(brushAndPen.first);
                dc.SetPen(brushAndPen.second);
            } break;
            case CalChart::Direction::SouthEast:
            case CalChart::Direction::East:
            case CalChart::Direction::NorthEast: {
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
            switch (AngleToDirection(info.mFacingDirection)) {
            case CalChart::Direction::SouthWest:
            case CalChart::Direction::West:
            case CalChart::Direction::NorthWest: {
                auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_BACK);
                dc.SetBrush(brushAndPen.first);
                dc.SetPen(brushAndPen.second);
            } break;
            case CalChart::Direction::SouthEast:
            case CalChart::Direction::East:
            case CalChart::Direction::NorthEast: {
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
        auto position = info.mPosition;
        auto x = position.x + mView->GetShowMode().Offset().x;
        auto y = position.y + mView->GetShowMode().Offset().y;
        auto drawPosition = fDIP(wxPoint(x, y));
        auto rectangleSize = fDIP(wxSize(CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(1)));

        dc.DrawRectangle(drawPosition - rectangleSize / 2, rectangleSize);
    }
}

void AnimationView::OnDrawSprites(wxDC& dc, CalChartConfiguration const& config)
{
    auto scale = config.Get_SpriteBitmapScale();
    constexpr auto comp_X = 0.5;
    auto comp_Y = config.Get_SpriteBitmapOffsetY();

    for (auto info : mAnimation->GetAllAnimateInfo()) {
        auto image_offset = !GetAnimationFrame()->TimerOn() ? 0 : OnBeat() ? 1
                                                                           : 2;
        auto image_index = AngleToQuadrant(info.mFacingDirection) + image_offset * 8;
        auto image = mSpriteImages[image_index];
        image = image.Scale(image.GetWidth() * scale, image.GetHeight() * scale);
        if (mView->IsSelected(info.index)) {
            image = image.ConvertToGreyscale();
        }

        auto position = info.mPosition;
        auto x = position.x + mView->GetShowMode().Offset().x;
        auto y = position.y + mView->GetShowMode().Offset().y;
        auto drawPosition = fDIP(wxPoint(x, y));
        auto rectangleSize = fDIP(wxSize(image.GetWidth() * comp_X, image.GetHeight() * comp_Y));

        dc.DrawBitmap(image, drawPosition - rectangleSize);
    }
}

void AnimationView::OnUpdate(wxView* sender, wxObject* hint)
{
    super::OnUpdate(sender, hint);
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

void AnimationView::RefreshAnimationSheet()
{
    if (mAnimation) {
        mAnimation->GotoSheet(mView->GetCurrentSheetNum());
        RefreshFrame();
    }
}

int AnimationView::GetTotalNumberBeats() const { return (mAnimation) ? mAnimation->GetTotalNumberBeats() : 0; }
int AnimationView::GetTotalCurrentBeat() const { return (mAnimation) ? mAnimation->GetTotalCurrentBeat() : 0; }

//#define GENERATE_SHOW_DUMP 1

void AnimationView::Generate()
{
    mAnimation = mView->GetAnimationInstance();
}

void AnimationView::PrevBeat()
{
    if (mAnimation && mAnimation->PrevBeat()) {
        RefreshFrame();
    }
}

void AnimationView::NextBeat()
{
    if (mAnimation && mAnimation->NextBeat()) {
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

bool AnimationView::AtEndOfShow() const
{
    if (mAnimation) {
        return (mAnimation->GetCurrentBeat() == mAnimation->GetTotalNumberBeats());
    }
    return false;
}

static auto towxPoint(CalChart::Coord const& c)
{
    return fDIP(wxPoint{ c.x, c.y });
}

// Return a bounding box of the show
std::pair<wxPoint, wxPoint> AnimationView::GetShowSizeAndOffset() const
{
    auto size = mView->GetShowFieldSize();
    return { towxPoint(size), towxPoint({ 0, 0 }) };
}

// Return a bounding box of the show of where the marchers are.  If they are
// outside the show, we don't see them.
std::pair<wxPoint, wxPoint> AnimationView::GetMarcherSizeAndOffset() const
{
    auto mode_size = towxPoint(mView->GetShowFieldSize());
    auto bounding_box_upper_left = mode_size;
    auto bounding_box_low_right = towxPoint({ 0, 0 });

    for (auto i = 0; mAnimation && i < mView->GetNumPoints(); ++i) {
        auto position = towxPoint(mAnimation->GetAnimateInfo(i).mPosition);
        bounding_box_upper_left = wxPoint(std::min(bounding_box_upper_left.x, position.x), std::min(bounding_box_upper_left.y, position.y));
        bounding_box_low_right = wxPoint(std::max(bounding_box_low_right.x, position.x), std::max(bounding_box_low_right.y, position.y));
    }

    return { bounding_box_low_right - bounding_box_upper_left, mode_size / 2 + bounding_box_upper_left };
}

void AnimationView::UnselectAll()
{
    mView->UnselectAll();
}

void AnimationView::SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown)
{
    using namespace CalChart;
    auto mouseStartTranslated = tDIP(mouseStart);
    auto mouseEndTranslated = tDIP(mouseEnd);

    auto x_off = mView->GetShowMode().Offset().x;
    auto y_off = mView->GetShowMode().Offset().y;
    auto polygon = CalChart::RawPolygon_t{
        Coord(mouseStartTranslated.x - x_off, mouseStartTranslated.y - y_off),
        Coord(mouseStartTranslated.x - x_off, mouseEndTranslated.y - y_off),
        Coord(mouseEndTranslated.x - x_off, mouseEndTranslated.y - y_off),
        Coord(mouseEndTranslated.x - x_off, mouseStartTranslated.y - y_off),
    };
    mView->SelectWithinPolygon(polygon, altDown);
}

// Keystroke command toggles the timer, but the timer
// lives in the Frame.  So we have this weird path...
void AnimationView::ToggleTimer() { GetAnimationFrame()->ToggleTimer(); }

bool AnimationView::OnBeat() const { return GetAnimationFrame()->OnBeat(); }

AnimationPanel const* AnimationView::GetAnimationFrame() const
{
    return static_cast<AnimationPanel const*>(GetFrame());
}

AnimationPanel* AnimationView::GetAnimationFrame()
{
    return static_cast<AnimationPanel*>(GetFrame());
}

CalChart::ShowMode const& AnimationView::GetShowMode() const
{
    return mView->GetShowMode();
}

AnimationView::MarcherInfo AnimationView::GetMarcherInfo(int which) const
{
    MarcherInfo info{};
    if (mAnimation) {
        auto anim_info = mAnimation->GetAnimateInfo(which);
        info.direction = NormalizeAngleRad((anim_info.mFacingDirection * M_PI / 180.0));

        auto position = anim_info.mPosition;
        info.x = CalChart::CoordUnits2Float(position.x);
        // because the coordinate system for continuity and OpenGL are different,
        // correct here.
        info.y = -1.0 * CalChart::CoordUnits2Float(position.y);
    }
    return info;
}

std::multimap<double, AnimationView::MarcherInfo> AnimationView::GetMarchersByDistance(float fromX, float fromY) const
{
    auto anySelected = !mView->GetSelectionList().empty();
    std::multimap<double, MarcherInfo> result;
    for (auto i = 0; (i < mView->GetNumPoints()); ++i) {
        if (anySelected && !mView->IsSelected(i)) {
            continue;
        }
        auto info = GetMarcherInfo(i);
        auto distance = sqrt(pow(fromX - info.x, 2) + pow(fromY - info.y, 2));
        result.insert(std::pair<double, MarcherInfo>(distance, info));
    }
    return result;
}
