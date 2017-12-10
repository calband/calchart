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

#include "animate_types.h"
#include "animate.h"

#include <wx/docview.h>

#include <memory>
#include <map>

class AnimationFrame;
class FieldView;
class CalChartConfiguration;
namespace CalChart {
class Continuity;
class Animation;
}
class CalChartDoc;

class AnimationView : public wxView {
public:
    AnimationView();
    ~AnimationView() = default;

    virtual void OnDraw(wxDC* dc) override;
    void OnDraw(wxDC& dc, const CalChartConfiguration& config);
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL) override;

    void RefreshFrame();

    void SetCollisionType(CalChart::CollisionWarning col);
    auto GetCollisionType() const { return mCollisionWarningType; }
    void SelectCollisions();

    void Generate();

    // true if changes made
    bool PrevBeat();
    bool NextBeat();
    void GotoBeat(unsigned i);
    bool PrevSheet();
    bool NextSheet();
    void GotoSheet(unsigned i);
    void GotoAnimationSheet(unsigned i);
    void SetSelection(const SelectionList& sl);

    // info
    auto GetNumberSheets() const { return (mAnimation) ? mAnimation->GetNumberSheets() : 0; }
    auto GetCurrentSheet() const { return (mAnimation) ? mAnimation->GetCurrentSheet() : 0; }
    int GetNumberBeats() const { return (mAnimation) ? mAnimation->GetNumberBeats() : 0; }
    int GetCurrentBeat() const { return (mAnimation) ? mAnimation->GetCurrentBeat() : 0; }

    wxString GetStatusText() const;

    std::pair<CalChart::Coord, CalChart::Coord> GetShowSizeAndOffset() const;
    std::pair<CalChart::Coord, CalChart::Coord> GetMarcherSizeAndOffset() const;

    void UnselectMarchers();
    void SelectMarchersInBox(long mouseXStart, long mouseYStart, long mouseXEnd,
        long mouseYEnd, bool altDown);

    void ToggleTimer();
    bool OnBeat() const;

    CalChart::Continuity GetContinuityOnSheet(unsigned whichSheet,
        SYMBOL_TYPE whichSymbol) const;

    const CalChartDoc* GetShow() const;
    CalChartDoc* GetShow();

    const CalChart::Animation* GetAnimation() const;

private:
    void OnNotifyStatus(const wxString& status);
    bool OnNotifyErrorList(const std::map<CalChart::AnimateError, CalChart::ErrorMarker>& error_markers,
        unsigned sheetnum, const wxString& message);

    bool mErrorOccurred;

    const AnimationFrame* GetAnimationFrame() const;
    AnimationFrame* GetAnimationFrame();

    std::unique_ptr<CalChart::Animation> mAnimation;
    CalChart::CollisionWarning mCollisionWarningType;
};
