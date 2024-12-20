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
#include <algorithm>
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
    RegenerateImages();
    auto onBeat = std::optional<bool>{ std::nullopt };
    if (GetAnimationFrame()->TimerOn()) {
        onBeat = OnBeat();
    }
    wxCalChart::Draw::DrawCommandList(*dc,
        mView->GenerateAnimationDrawCommands(
            mCurrentBeat,
            mDrawCollisionWarning,
            onBeat,
            [this](CalChart::Radian angle, CalChart::Animation::ImageBeat imageBeat) {
                auto imageIndex = CalChart::AngleToQuadrant(angle) + CalChart::toUType(imageBeat) * 8;
                return mSpriteCalChartImages[imageIndex];
            }));
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
    if (mPlayCollisionWarning && mView->BeatHasCollision(mCurrentBeat) && mConfig.Get_BeepOnCollisions()) {
        wxBell();
    }
    GetAnimationFrame()->UpdatePanel();
    GetAnimationFrame()->Refresh();
}

void AnimationView::RefreshAnimationSheet()
{
    mCurrentBeat = mView->GetAnimationBeatForCurrentSheet();
    RefreshFrame();
}

int AnimationView::GetTotalNumberBeats() const
{
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    return totalBeats.value_or(0);
}

int AnimationView::GetTotalCurrentBeat() const
{
    return mCurrentBeat;
}

// #define GENERATE_SHOW_DUMP 1

void AnimationView::Generate()
{
    mCurrentBeat = 0;
}

void AnimationView::PrevBeat()
{
    if (mCurrentBeat == 0) {
        return;
    }
    mCurrentBeat -= 1;
    RefreshFrame();
}

void AnimationView::NextBeat()
{
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    if (totalBeats) {
        if (mCurrentBeat >= (*totalBeats - 1)) {
            return;
        }
        mCurrentBeat += 1;
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
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    if (totalBeats) {
        return (mCurrentBeat == *totalBeats);
    }
    return false;
}

namespace {
static auto towxSize(CalChart::Coord const& c)
{
    return fDIP(wxSize{ c.x, c.y });
}

static auto towxPoint(CalChart::Coord const& c)
{
    return fDIP(wxPoint{ c.x, c.y });
}
} // namespace

// Return a bounding box of the show
auto AnimationView::GetShowSizeAndOffset() const -> std::pair<wxSize, wxPoint>
{
    auto size = mView->GetShowFieldSize();
    return { towxSize(size), towxPoint({ 0, 0 }) };
}

// Return a bounding box of the show of where the marchers are.  If they are
// outside the show, we don't see them.
auto AnimationView::GetMarcherSizeAndOffset() const -> std::pair<wxSize, wxPoint>
{
    auto mode_size = mView->GetShowFieldSize();
    auto [bounding_box_upper_left, bounding_box_low_right] = mView->GetAnimationBoundingBox(mCurrentBeat);
    return { towxSize(bounding_box_low_right - bounding_box_upper_left), towxPoint(mode_size / 2 + bounding_box_upper_left) };
}

void AnimationView::UnselectAll()
{
    mView->UnselectAll();
}

void AnimationView::SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown)
{
    auto mouseStartTranslated = tDIP(mouseStart);
    auto mouseEndTranslated = tDIP(mouseEnd);

    auto [x_off, y_off] = mView->GetShowFieldOffset();
    auto polygon = CalChart::RawPolygon_t{
        CalChart::Coord(mouseStartTranslated.x - x_off, mouseStartTranslated.y - y_off),
        CalChart::Coord(mouseStartTranslated.x - x_off, mouseEndTranslated.y - y_off),
        CalChart::Coord(mouseEndTranslated.x - x_off, mouseEndTranslated.y - y_off),
        CalChart::Coord(mouseEndTranslated.x - x_off, mouseStartTranslated.y - y_off),
    };
    mView->SelectWithinPolygon(polygon, altDown);
}

// Keystroke command toggles the timer, but the timer
// lives in the Frame.  So we have this weird path...
void AnimationView::ToggleTimer()
{
    GetAnimationFrame()->ToggleTimer();
}

auto AnimationView::OnBeat() const -> bool
{
    return GetAnimationFrame()->OnBeat();
}

auto AnimationView::GetAnimationFrame() const -> AnimationPanel const*
{
    return static_cast<AnimationPanel const*>(GetFrame());
}

auto AnimationView::GetAnimationFrame() -> AnimationPanel*
{
    return static_cast<AnimationPanel*>(GetFrame());
}

auto AnimationView::GetShowFieldSize() const -> CalChart::Coord
{
    return mView->GetShowMode().FieldSize();
}

auto AnimationView::GetMarcherInfo(int which) const -> std::optional<CalChart::Animate::Info>
{
    return mView->GetAnimationInfo(mCurrentBeat, which);
}

auto AnimationView::GetMarchersByDistance(float fromX, float fromY) const -> std::multimap<double, CalChart::Animate::Info>
{
    return mView->GetSelectedAnimationInfoWithDistanceFromPoint(mCurrentBeat, CalChart::Coord(fromX, fromY));
}
