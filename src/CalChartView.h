#pragma once
/*
 * CalChartView.h
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartDoc.h"
#include "animatecompile.h"
#include "cc_coord.h"
#include "cc_types.h"
#include "ghost_module.h"
#include "modes.h"

#include <map>
#include <wx/docview.h>

// CalChartView connects together the Frames and the Doc.

class BackgroundImage;
class CalChartFrame;
class AnimationFrame;
class Animation;
class Matrix;
class CalChartConfiguration;

class CalChartView : public wxView {
public:
    CalChartView();
    ~CalChartView() = default;

    bool OnCreate(wxDocument* doc, long flags) override;
    void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL) override;
    bool OnClose(bool deleteWindow = true) override;

    void OnDraw(wxDC* dc) override;
    void DrawOtherPoints(wxDC& dc, std::map<int, CalChart::Coord> const& positions);
    void OnDrawBackground(wxDC& dc);

    void OnWizardSetup(CalChartDoc& show);

    ///// Modify the show /////
    bool DoRotatePointPositions(int rotateAmount);
    bool DoMovePoints(std::map<int, CalChart::Coord> const& transmat);
    bool DoDeletePoints();
    bool DoResetReferencePoint();
    bool DoSetPointsSymbol(SYMBOL_TYPE sym);
    void DoSetMode(CalChart::ShowMode const& mode);
    void DoSetShowInfo(std::vector<wxString> const& labels, int numColumns);
    void DoSetSheetTitle(wxString const& descr);
    bool DoSetSheetBeats(int beats);
    bool DoSetPointsLabel(bool right);
    bool DoSetPointsLabelFlip();
    bool DoSetPointsLabelVisibility(bool isVisible);
    bool DoTogglePointsLabelVisibility();
    void DoInsertSheets(CalChart::Show::Sheet_container_t const& sht, int where);
    bool DoDeleteSheet(int where);
    bool DoImportPrintableContinuity(const wxString& file);
    void DoSetPrintContinuity(int which_sheet, const wxString& number, const wxString& cont);
    bool DoRelabel();
    std::pair<bool, std::string> DoAppendShow(std::unique_ptr<CalChartDoc> other_show);
    bool DoSetContinuityCommand(SYMBOL_TYPE sym, CalChart::Continuity const& new_cont);

    ///// query show attributes /////
    int FindPoint(CalChart::Coord pos) const;
    CalChart::Coord PointPosition(int which) const;
    auto GetCurrentSheetNum() const { return mShow ? mShow->GetCurrentSheetNum() : 0; }
    auto GetNumSheets() const { return mShow ? mShow->GetNumSheets() : 0; }
    auto GetNumPoints() const { return mShow ? mShow->GetNumPoints() : 0; }

    CalChart::ShowMode const& GetShowMode() { return mShow->GetShowMode(); }
    auto GetShowFieldOffset() const { return mShow->GetShowMode().Offset(); }
    auto GetShowFieldSize() const { return mShow->GetShowMode().Size(); }

    auto GetSheetBegin() const { return mShow->GetSheetBegin(); }
    auto GetSheetEnd() const { return mShow->GetSheetEnd(); }
    auto GetCurrentSheet() const { return mShow->GetCurrentSheet(); }

    std::vector<CalChart::AnimationErrors> GetAnimationErrors() const;
    // Sheet -> all collisions
    std::map<int, SelectionList> GetAnimationCollisions() const;
    std::unique_ptr<CalChart::Animation> GetAnimationInstance() const;

    auto ClipPositionToShowMode(CalChart::Coord const& pos) const { return mShow->GetShowMode().ClipPosition(pos); }

    ///// Change show attributes /////
    void GoToSheet(int which);
    void GoToNextSheet() { GoToSheet(mShow->GetCurrentSheetNum() + 1); }
    void GoToPrevSheet() { GoToSheet(mShow->GetCurrentSheetNum() - 1); }
    auto GetNthSheet(int n) const { return mShow->GetNthSheet(n); }

    void SetActiveReferencePoint(int which);

    ///// Select /////
    void UnselectAll() { SetSelection(mShow->MakeUnselectAll()); }
    void AddToSelection(const SelectionList& sl) { SetSelection(mShow->MakeAddToSelection(sl)); }
    void ToggleSelection(const SelectionList& sl) { SetSelection(mShow->MakeToggleSelection(sl)); }
    void SelectWithLasso(const CalChart::Lasso* lasso, bool toggleSelected);
    void SelectPointsInRect(const CalChart::Coord& c1, const CalChart::Coord& c2, bool toggleSelected);
    auto GetSelectionList() const { return mShow->GetSelectionList(); }
    void SetSelection(const SelectionList& sl);
    void GoToSheetAndSetSelection(int which, const SelectionList& sl);
    auto IsSelected(int i) const { return mShow->IsSelected(i); }

    ///// Drawing marcher's paths /////
    // call this when we need to generate the marcher's paths.
    void OnEnableDrawPaths(bool enable);

    GhostModule& getGhostModule() { return mGhostModule; };

    void DoDrawBackground(bool enable);
    bool DoingDrawBackground() const;
    void DoPictureAdjustment(bool enable);
    bool DoingPictureAdjustment() const;
    bool AddBackgroundImage(const wxImage& image);
    void OnBackgroundMouseLeftDown(wxMouseEvent& event, wxDC& dc);
    void OnBackgroundMouseLeftUp(wxMouseEvent& event, wxDC& dc);
    void OnBackgroundMouseMove(wxMouseEvent& event, wxDC& dc);
    void OnBackgroundImageDelete();

private:
    void DrawPaths(wxDC& dc, const CalChart::Sheet& sheet);
    void GeneratePaths();
    void UpdateBackgroundImages();

    CalChartFrame* mFrame{};
    bool mDrawPaths{};
    GhostModule mGhostModule{};
    CalChartDoc* mShow{};
    int mCurrentReferencePoint{};
    CalChartConfiguration& mConfig;
    std::vector<BackgroundImage> mBackgroundImages;
    bool mDrawBackground{};
    bool mAdjustBackgroundMode{};
    int mWhichBackgroundIndex{};

    DECLARE_DYNAMIC_CLASS(CalChartView)
};
