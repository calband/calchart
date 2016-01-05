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

#pragma once

#include "animate.h"
#include "calchartdoc.h"
#include "cc_continuity.h"

#include <memory>

class AnimationFrame;
class FieldView;
class CalChartConfiguration;

class AnimationView : public wxView {
public:
    AnimationView();
    ~AnimationView();

    //	virtual bool OnCreate(wxDocument *doc, long flags);
    //	virtual bool OnClose(bool deleteWindow = true);
    virtual void OnDraw(wxDC* dc);
    void OnDraw(wxDC* dc, const CalChartConfiguration& config);
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL);

    void RefreshFrame();

    void SetCollisionType(CollisionWarning col);
    CollisionWarning GetCollisionType() const { return mCollisionWarningType; }
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
    int GetNumberSheets() const;
    int GetCurrentSheet() const;
    int GetNumberBeats() const;
    int GetCurrentBeat() const;

    wxString GetStatusText() const;

    std::pair<CC_coord, CC_coord> GetShowSizeAndOffset() const;
    std::pair<CC_coord, CC_coord> GetMarcherSizeAndOffset() const;

    void UnselectMarchers();
    void SelectMarchersInBox(long mouseXStart, long mouseYStart, long mouseXEnd,
        long mouseYEnd, bool altDown);

    void ToggleTimer();
    bool OnBeat() const;

    CC_continuity GetContinuityOnSheet(unsigned whichSheet,
        SYMBOL_TYPE whichSymbol) const;

    const CalChartDoc* GetShow() const;
    CalChartDoc* GetShow();

    const Animation* GetAnimation() const;

private:
    void OnNotifyStatus(const wxString& status);
    bool
    OnNotifyErrorList(const std::map<AnimateError, ErrorMarker>& error_markers,
        unsigned sheetnum, const wxString& message);

    bool mErrorOccurred;

    const AnimationFrame* GetAnimationFrame() const;
    AnimationFrame* GetAnimationFrame();

    std::unique_ptr<Animation> mAnimation;
    CollisionWarning mCollisionWarningType;
};