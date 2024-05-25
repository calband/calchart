/*
 * AnimationView.cpp
 * Animation user interface
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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
#include "CalChartAngles.h"
#include "CalChartAnimation.h"
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "CalChartRanges.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "CalChartShowMode.h"
#include "CalChartSizes.h"
#include "CalChartUtils.h"
#include "CalChartView.h"
#include "platconf.h"
#include <ranges>
#include <wx/dcbuffer.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

namespace {

// split the source image into a number of horizontal images
auto GenerateSpriteImages(wxImage const& image, int numberImages, int imageX, int imageY, double scale)
{
    return std::views::iota(0, numberImages) | std::views::transform([&image, imageX, imageY, scale](auto i) {
        auto newImage = image.GetSubImage({ i * imageX + 0, 0, imageX, imageY });
        return newImage.Scale(newImage.GetWidth() * scale, newImage.GetHeight() * scale);
    });
}

template <std::ranges::input_range Range>
    requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Info>)
auto GeneratePointDrawCommand(Range&& infos) -> std::vector<CalChart::Draw::DrawCommand>
{
    return CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(infos | std::views::transform([](auto&& info) {
        auto size = CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(1) };
        return CalChart::Draw::Rectangle{
            info.mMarcherInfo.mPosition - size / 2, { CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(1) }
        };
    }));
}

template <std::ranges::input_range Range, typename Function>
    requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Info>)
auto GeneratePointDrawCommand(Range&& range, Function predicate, CalChart::BrushAndPen brushAndPen) -> CalChart::Draw::DrawCommand
{
    auto filteredRange = range | std::views::filter(predicate);
    if (!filteredRange.empty()) {
        return CalChart::Draw::withBrushAndPen(brushAndPen, GeneratePointDrawCommand(filteredRange));
    }
    return CalChart::Draw::Ignore{};
}

}

AnimationView::AnimationView(CalChartView* view, CalChart::Configuration const& config, wxWindow* frame)
    : mView(view)
    , mConfig(config)
    , mMeasure{ "AnimationViewDraw" }
{
    SetFrame(frame);

    RegenerateImages();
}

AnimationView::~AnimationView()
{
}

void AnimationView::RegenerateImages() const
{
    auto spriteScale = mConfig.Get_SpriteBitmapScale();
    if (spriteScale == mScaleSize) {
        return;
    }
    mScaleSize = spriteScale;
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
    constexpr auto image_X = 64;
    constexpr auto image_Y = 128;
    auto images = GenerateSpriteImages(image, mSpriteCalChartImages.size(), image_X, image_Y, mScaleSize);
    std::transform(images.begin(), images.end(), mSpriteCalChartImages.begin(), [](auto&& image) {
        return std::make_shared<CalChart::ImageData>(wxCalChart::ConvertToImageData(image));
    });
}

void AnimationView::OnDraw(wxDC* dc)
{
    // std::cout << mMeasure << "\n";
    // auto snapshot = mMeasure.doMeasurement();
    wxCalChart::Draw::DrawCommandList(*dc, GenerateDraw(mConfig));
}

auto AnimationView::GenerateDraw(CalChart::Configuration const& config) -> std::vector<CalChart::Draw::DrawCommand>
{
    if (!mAnimation) {
        // no animation, our job is done.
        return {};
    }

    auto tborder1 = mView->GetShowMode().Border1();
    auto drawCmds = CalChartDraw::GenerateModeDrawCommands(config, mView->GetShowMode(), ShowMode_kAnimation) + tborder1;
    auto useSprites = config.Get_UseSprites();
    if (useSprites) {
        CalChart::append(drawCmds, GenerateDrawSprites(config));
    } else {
        CalChart::append(drawCmds, GenerateDrawDots(config));
    }
    return drawCmds;
}

auto AnimationView::GenerateDrawDots(CalChart::Configuration const& config) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto allEnumeratedInfo = CalChart::Ranges::enumerate_view(mAnimation->GetAllAnimateInfo(mCurrentBeat));
    auto allInfo = allEnumeratedInfo
        | std::views::transform([](auto&& info) { return std::get<1>(info); });
    auto allSelected = allEnumeratedInfo
        | std::views::filter([this](auto&& info) { return mView->IsSelected(std::get<0>(info)); })
        | std::views::transform([](auto&& info) { return std::get<1>(info); });
    auto allNotSelected = allEnumeratedInfo
        | std::views::filter([this](auto&& info) { return !mView->IsSelected(std::get<0>(info)); })
        | std::views::transform([](auto&& info) { return std::get<1>(info); });

    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allNotSelected, [](auto&& info) { return FacingBack(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_BACK)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allNotSelected, [](auto&& info) { return FacingFront(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_FRONT)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allNotSelected, [](auto&& info) { return FacingSide(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_SIDE)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allSelected, [](auto&& info) { return FacingBack(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_HILIT_BACK)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allSelected, [](auto&& info) { return FacingFront(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_HILIT_FRONT)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allSelected, [](auto&& info) { return FacingSide(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_HILIT_SIDE)));

    if (mDrawCollisionWarning) {
        CalChart::append(drawCmds,
            GeneratePointDrawCommand(
                allInfo, [](auto&& info) { return CollisionWarning(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_COLLISION_WARNING)));
        CalChart::append(drawCmds,
            GeneratePointDrawCommand(
                allInfo, [](auto&& info) { return CollisionIntersect(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_COLLISION)));
    }
    return drawCmds + mView->GetShowMode().Offset();
}

auto AnimationView::GenerateDrawSprites(CalChart::Configuration const& config) -> std::vector<CalChart::Draw::DrawCommand>
{
    RegenerateImages();
    constexpr auto comp_X = 0.5;
    auto comp_Y = config.Get_SpriteBitmapOffsetY();

    auto drawCmds = CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(
        CalChart::Ranges::enumerate_view(mAnimation->GetAllAnimateInfo(mCurrentBeat)) | std::views::transform([this, comp_Y, timerOn = GetAnimationFrame()->TimerOn()](auto&& enum_info) {
            auto&& [index, info] = enum_info;
            auto image_offset = [&]() {
                if (info.mMarcherInfo.mStepStyle == CalChart::MarchingStyle::Close) {
                    return 0;
                }
                return !timerOn ? 0 : OnBeat() ? 1
                                               : 2;
            }();
            auto image_index = CalChart::AngleToQuadrant(info.mMarcherInfo.mFacingDirection) + image_offset * 8;
            auto image = mSpriteCalChartImages[image_index];
            auto position = info.mMarcherInfo.mPosition;
            auto offset = CalChart::Coord(image->image_width * comp_X, image->image_height * comp_Y);

            return CalChart::Draw::Image{ position, image, mView->IsSelected(index) } - offset;
        }));
    return drawCmds + mView->GetShowMode().Offset();
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
    if (mPlayCollisionWarning && mAnimation && mAnimation->BeatHasCollision(mCurrentBeat) && mConfig.Get_BeepOnCollisions()) {
        wxBell();
    }
    GetAnimationFrame()->UpdatePanel();
    GetAnimationFrame()->Refresh();
}

void AnimationView::RefreshAnimationSheet()
{
    if (mAnimation) {
        // todo, update this
        mCurrentBeat = mAnimation->GetTotalNumberBeatsUpTo(mView->GetCurrentSheetNum());
        RefreshFrame();
    }
}

int AnimationView::GetTotalNumberBeats() const { return (mAnimation) ? mAnimation->GetTotalNumberBeats() : 0; }
int AnimationView::GetTotalCurrentBeat() const { return mCurrentBeat; }

// #define GENERATE_SHOW_DUMP 1

void AnimationView::Generate()
{
    mAnimation = mView->GenerateAnimation();
    mCurrentBeat = 0;
}

void AnimationView::PrevBeat()
{
    if (mAnimation) {
        auto currentBeat = mAnimation->GetTotalCurrentBeat();
        if (currentBeat == 0) {
            return;
        }
        GotoTotalBeat(currentBeat - 1);
        RefreshFrame();
    }
}

void AnimationView::NextBeat()
{
    if (mAnimation) {
        auto totalBeats = mAnimation->GetTotalNumberBeats();
        auto currentBeat = mAnimation->GetTotalCurrentBeat();
        if (currentBeat == (totalBeats - 1)) {
            return;
        }
        GotoTotalBeat(currentBeat + 1);
        RefreshFrame();
    }
}

void AnimationView::GotoTotalBeat(CalChart::beats_t whichBeat)
{
    mCurrentBeat = whichBeat;
    RefreshFrame();
}

auto AnimationView::AtEndOfShow() const -> bool
{
    if (mAnimation) {
        return (mCurrentBeat == mAnimation->GetTotalNumberBeats());
    }
    return false;
}

static auto towxPoint(CalChart::Coord const& c)
{
    return fDIP(wxPoint{ c.x, c.y });
}

// Return a bounding box of the show
auto AnimationView::GetShowSizeAndOffset() const -> std::pair<wxPoint, wxPoint>
{
    auto size = mView->GetShowFieldSize();
    return { towxPoint(size), towxPoint({ 0, 0 }) };
}

// Return a bounding box of the show of where the marchers are.  If they are
// outside the show, we don't see them.
auto AnimationView::GetMarcherSizeAndOffset() -> std::pair<wxPoint, wxPoint>
{
    auto mode_size = towxPoint(mView->GetShowFieldSize());
    auto bounding_box_upper_left = mode_size;
    auto bounding_box_low_right = towxPoint({ 0, 0 });

    for (auto i = 0; mAnimation && i < mView->GetNumPoints(); ++i) {
        auto position = towxPoint(mAnimation->GetAnimateInfo(mCurrentBeat, i).mMarcherInfo.mPosition);
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

auto AnimationView::OnBeat() const -> bool { return GetAnimationFrame()->OnBeat(); }

auto AnimationView::GetAnimationFrame() const -> AnimationPanel const*
{
    return static_cast<AnimationPanel const*>(GetFrame());
}

auto AnimationView::GetAnimationFrame() -> AnimationPanel*
{
    return static_cast<AnimationPanel*>(GetFrame());
}

auto AnimationView::GetShowMode() const -> CalChart::ShowMode const&
{
    return mView->GetShowMode();
}

auto AnimationView::GetMarcherInfo(int which) -> AnimationView::MarcherInfo
{
    MarcherInfo info{};
    if (mAnimation) {
        auto anim_info = mAnimation->GetAnimateInfo(mCurrentBeat, which);
        info.direction = CalChart::Radian{ CalChart::NormalizeAngle(anim_info.mMarcherInfo.mFacingDirection) };

        auto position = anim_info.mMarcherInfo.mPosition;
        info.x = CalChart::CoordUnits2Float(position.x);
        // because the coordinate system for continuity and OpenGL are different,
        // correct here.
        info.y = -1.0 * CalChart::CoordUnits2Float(position.y);
    }
    return info;
}

auto AnimationView::GetMarchersByDistance(float fromX, float fromY) -> std::multimap<double, AnimationView::MarcherInfo>
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
