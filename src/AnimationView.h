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
#include <wx/docview.h>

class AnimationPanel;
class CalChartView;
class CalChartConfiguration;
namespace CalChart {
class Continuity;
class Animation;
class ShowMode;
}

class AnimationView : public wxView {
public:
    AnimationView(CalChartView* view, wxWindow* frame);
    ~AnimationView() = default;

    virtual void OnDraw(wxDC* dc) override;
    void OnDraw(wxDC& dc, const CalChartConfiguration& config);
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL) override;

    void SetDrawCollisionWarning(bool b) { mDrawCollisionWarning = b; }
    auto GetDrawCollisionWarning() const { return mDrawCollisionWarning; }

    // true if changes made
    bool PrevBeat();
    bool NextBeat();
    void GotoBeat(unsigned i);
    void GotoTotalBeat(unsigned i);
    bool PrevSheet();
    bool NextSheet();
    void GotoSheet(unsigned i);
    void GotoAnimationSheet(unsigned i);
    void RefreshSheet();

    // info
    int GetNumberSheets() const;
    int GetCurrentSheet() const;
    int GetNumberBeats() const;
    int GetCurrentBeat() const;
    int GetTotalNumberBeats() const;
    int GetTotalCurrentBeat() const;

    std::pair<wxPoint, wxPoint> GetShowSizeAndOffset() const;
    std::pair<wxPoint, wxPoint> GetMarcherSizeAndOffset() const;

    void UnselectAll();
    void SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown);

    void ToggleTimer();
    bool OnBeat() const;

    void SetPlayCollisionWarning(bool b) { mPlayCollisionWarning = b; }
    auto GetPlayCollisionWarning() const { return mPlayCollisionWarning; }

    CalChart::ShowMode const& GetShowMode() const;
    MarcherInfo GetMarcherInfo(int which) const;
    std::multimap<double, MarcherInfo> GetMarchersByDistance(ViewPoint const& from) const;
    int GetNumPoints() const;

private:
    void Generate();
    void RefreshFrame();

    const AnimationPanel* GetAnimationFrame() const;
    AnimationPanel* GetAnimationFrame();

    // Yes, this view has a view.
    CalChartView* mView{};

    std::unique_ptr<CalChart::Animation> mAnimation;
    bool mDrawCollisionWarning = true;
    bool mPlayCollisionWarning = false;
};
