#pragma once
/*
 * AnimationView.h
 * Header for animation user interface
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

#include "CalChartAnimation.h"
#include "CalChartDrawCommand.h"
#include "CalChartMeasure.h"
#include <array>
#include <map>
#include <memory>
#include <wx/docview.h>

class AnimationPanel;
class CalChartView;
namespace wxCalChart {
struct BitmapHolder;
}
namespace CalChart {
class Configuration;
class Continuity;
class ShowMode;
}

// AnimationView is sort of a "proxy" view.  It allows a separate view for Animation into the show.
// So it sort-of wraps CalChartView.
class AnimationView : public wxView {
    using super = wxView;

public:
    AnimationView(CalChartView* view, CalChart::Configuration const& config, wxWindow* frame);
    ~AnimationView() override;
    AnimationView(AnimationView const&) = delete;
    auto operator=(AnimationView const&) = delete;
    AnimationView(AnimationView&&) = delete;
    auto operator=(AnimationView&&) = delete;

    void OnDraw(wxDC* dc) override;
    void OnUpdate(wxView* sender, wxObject* hint = nullptr) override;

    void PrevBeat();
    void NextBeat();
    void GotoTotalBeat(CalChart::Beats whichBeat);
    [[nodiscard]] auto AtEndOfShow() const -> bool;

    void RefreshAnimationSheet();

    // info
    [[nodiscard]] auto GetTotalNumberBeats() const -> int;
    [[nodiscard]] auto GetTotalCurrentBeat() const -> int;

    [[nodiscard]] auto GetAnimationBoundingBox(bool zoomInOnMarchers) const -> std::pair<CalChart::Coord, CalChart::Coord>;

    [[nodiscard]] auto GetShowFieldSize() const -> CalChart::Coord;

    [[nodiscard]] auto GetMarcherInfo(int which) const -> std::optional<CalChart::Animate::Info>;

    [[nodiscard]] auto GetMarchersByDistance(float fromX, float fromY) const -> std::multimap<double, CalChart::Animate::Info>;

    void UnselectAll();
    void SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown);

    void ToggleTimer();
    [[nodiscard]] auto OnBeat() const -> bool;

    void SetDrawCollisionWarning(bool b) { mDrawCollisionWarning = b; }
    [[nodiscard]] auto GetDrawCollisionWarning() const { return mDrawCollisionWarning; }

    void SetPlayCollisionWarning(bool b) { mPlayCollisionWarning = b; }
    [[nodiscard]] auto GetPlayCollisionWarning() const { return mPlayCollisionWarning; }

private:
    void Generate();
    void RefreshFrame();

    void RegenerateImages();
    [[nodiscard]] auto GenerateDraw(CalChart::Configuration const& config) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateDrawSprites(CalChart::Configuration const& config, CalChart::Animation::AngleStepToImageFunction imageFunction) const -> std::vector<CalChart::Draw::DrawCommand>;

    [[nodiscard]] auto GetAnimationFrame() const -> AnimationPanel const*;
    [[nodiscard]] auto GetAnimationFrame() -> AnimationPanel*;

    // Yes, this view has a view...
    CalChartView* mView{};
    CalChart::Configuration const& mConfig;

    CalChart::Beats mCurrentBeat;
    bool mDrawCollisionWarning = true;
    bool mPlayCollisionWarning = false;

    static constexpr auto kAngles = 8;
    double mScaleSize = 0;
    using BitmapSize_t = std::tuple<std::shared_ptr<wxCalChart::BitmapHolder>, CalChart::Coord>;
    std::array<BitmapSize_t, kAngles * CalChart::toUType(CalChart::Animation::ImageBeat::Size)> mSpriteCalChartImages;
    std::array<BitmapSize_t, kAngles * CalChart::toUType(CalChart::Animation::ImageBeat::Size)> mSelectedSpriteCalChartImages;

    CalChart::MeasureDuration mMeasure;
};
