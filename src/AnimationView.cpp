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

void AnimationView::RegenerateImages()
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
        return BitmapSize_t{ std::make_shared<wxCalChart::BitmapHolder>(image), wxCalChart::toCoord(image.GetSize()) };
    });
    std::transform(images.begin(), images.end(), mSelectedSpriteCalChartImages.begin(), [](auto&& image) {
        return BitmapSize_t{ std::make_shared<wxCalChart::BitmapHolder>(image.ConvertToGreyscale()), wxCalChart::toCoord(image.GetSize()) };
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
            [this](CalChart::Radian angle, CalChart::Animation::ImageBeat imageBeat, bool selected) {
                auto imageIndex = CalChart::AngleToQuadrant(angle) + CalChart::toUType(imageBeat) * 8;
                return selected ? mSelectedSpriteCalChartImages[imageIndex] : mSpriteCalChartImages[imageIndex];
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

void AnimationView::GotoTotalBeat(CalChart::Beats whichBeat)
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

auto AnimationView::GetAnimationBoundingBox(bool zoomInOnMarchers) const -> std::pair<CalChart::Coord, CalChart::Coord>
{
    return mView->GetAnimationBoundingBox(zoomInOnMarchers, mCurrentBeat);
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
    return mView->GetShowFieldSize();
}

auto AnimationView::GetMarcherInfo(int which) const -> std::optional<CalChart::Animate::Info>
{
    return mView->GetAnimationInfo(mCurrentBeat, which);
}

auto AnimationView::GetMarchersByDistance(float fromX, float fromY) const -> std::multimap<double, CalChart::Animate::Info>
{
    return mView->GetSelectedAnimationInfoWithDistanceFromPoint(mCurrentBeat, CalChart::Coord(fromX, fromY));
}
