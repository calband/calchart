#pragma once
/*
 * AnimationView.h
 * Header for animation user interface
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

#include "draw_utils.h"
#include <map>
#include <array>
#include <wx/docview.h>

class AnimationPanel;
class CalChartView;
class CalChartConfiguration;
namespace CalChart {
class Continuity;
class Animation;
class ShowMode;
}

// AnimationView is sort of a "proxy" view.  It allows a separate view for Animation into the show.
// So it sort-of wraps CalChartView.
class AnimationView : public wxView {
    using super = wxView;

public:
    AnimationView(CalChartView* view, wxWindow* frame);
    ~AnimationView() override;

    void OnDraw(wxDC* dc) override;
    void OnDraw(wxDC& dc, CalChartConfiguration const& config);
    void OnDrawDots(wxDC& dc, CalChartConfiguration const& config);
    void OnDrawSprites(wxDC& dc, CalChartConfiguration const& config);
    void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)nullptr) override;

    void PrevBeat();
    void NextBeat();
    void GotoTotalBeat(unsigned i);
    bool AtEndOfShow() const;

    void RefreshAnimationSheet();

    // info
    int GetTotalNumberBeats() const;
    int GetTotalCurrentBeat() const;

    std::pair<wxPoint, wxPoint> GetShowSizeAndOffset() const;
    std::pair<wxPoint, wxPoint> GetMarcherSizeAndOffset() const;

    CalChart::ShowMode const& GetShowMode() const;
    MarcherInfo GetMarcherInfo(int which) const;
    std::multimap<double, MarcherInfo> GetMarchersByDistance(ViewPoint const& from) const;

    void UnselectAll();
    void SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown);

    void ToggleTimer();
    bool OnBeat() const;

    void SetDrawCollisionWarning(bool b) { mDrawCollisionWarning = b; }
    auto GetDrawCollisionWarning() const { return mDrawCollisionWarning; }

    void SetPlayCollisionWarning(bool b) { mPlayCollisionWarning = b; }
    auto GetPlayCollisionWarning() const { return mPlayCollisionWarning; }

private:
    void Generate();
    void RefreshFrame();

    AnimationPanel const* GetAnimationFrame() const;
    AnimationPanel* GetAnimationFrame();

    // Yes, this view has a view...
    CalChartView* mView{};

    std::unique_ptr<CalChart::Animation> mAnimation;
    bool mDrawCollisionWarning = true;
    bool mPlayCollisionWarning = false;

    enum ImageBeat {
        Standing,
        Right,
        Left,
        Size
    };

    std::array<wxImage, 8 * ImageBeat::Size> mSpriteImages;
};
