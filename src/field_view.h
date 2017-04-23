/*
 * field_view.h
 * Header for field view
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

#pragma once

#include "calchartdoc.h"
#include "CC_coord.h"
#include "modes.h"

#include "ghost_module.h"
#include "background_image.h"

#include <wx/docview.h>

#include <memory>

class FieldFrame;
class AnimationFrame;
class Animation;
class Matrix;
class CalChartConfiguration;

// Field:
// Field is the editable overhead view of the marchers on the field.
// This is where in the app you edit a marcher's location and continuity
class FieldView : public wxView {
public:
    FieldView();
    ~FieldView();

    bool OnCreate(wxDocument* doc, long flags);
    void OnDraw(wxDC* dc);
    void DrawOtherPoints(wxDC& dc, const std::map<int, CC_coord>& positions);
    void OnDrawBackground(wxDC& dc);
    void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL);
    bool OnClose(bool deleteWindow = true);

    void OnWizardSetup(CalChartDoc& show);

    ///// Modify the show /////
    bool DoRotatePointPositions(int rotateAmount);
    bool DoMovePoints(const std::map<int, CC_coord>& transmat);
    bool DoResetReferencePoint();
    bool DoSetPointsSymbol(SYMBOL_TYPE sym);
    void DoSetMode(const wxString& mode);
    void DoSetShowInfo(const std::vector<wxString>& labels, int numColumns);
    void DoSetSheetTitle(const wxString& descr);
    bool DoSetSheetBeats(int beats);
    bool DoSetPointsLabel(bool right);
    bool DoSetPointsLabelFlip();
    bool DoSetPointsLabelVisibility(bool isVisible);
    bool DoTogglePointsLabelVisibility();
    void DoInsertSheets(const CalChartDoc::CC_sheet_container_t& sht, int where);
    bool DoDeleteSheet(int where);
    bool DoImportPrintableContinuity(const wxString& file);
    bool DoRelabel();
    std::pair<bool, std::string> DoAppendShow(std::unique_ptr<CalChartDoc> other_show);

    ///// query show attributes /////
    int FindPoint(CC_coord pos) const;
    CC_coord PointPosition(int which) const;
    auto GetCurrentSheetNum() const { return mShow->GetCurrentSheetNum(); }
    auto GetNumSheets() const { return mShow->GetNumSheets(); }

    auto GetShowFieldOffset() const { return mShow->GetMode().Offset(); }
    auto GetShowFieldSize() const { return mShow->GetMode().Size(); }

    ///// Change show attributes /////
    void GoToSheet(int which);
    void GoToNextSheet() { GoToSheet(mShow->GetCurrentSheetNum() + 1); }
    void GoToPrevSheet() { GoToSheet(mShow->GetCurrentSheetNum() - 1); }

    void SetReferencePoint(int which);

    ///// Select /////
    void UnselectAll() { SetSelection(mShow->MakeUnselectAll()); }
    void AddToSelection(const SelectionList& sl) { SetSelection(mShow->MakeAddToSelection(sl)); }
    void ToggleSelection(const SelectionList& sl) { SetSelection(mShow->MakeToggleSelection(sl)); }
    void SelectWithLasso(const CC_lasso* lasso, bool toggleSelected);
    void SelectPointsInRect(const CC_coord& c1, const CC_coord& c2,
        bool toggleSelected);
    auto GetSelectionList() const { return mShow->GetSelectionList(); }
    void SetSelection(const SelectionList& sl);

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
#if defined(BUILD_FOR_VIEWER) && (BUILD_FOR_VIEWER != 0)
    AnimationFrame* mFrame;
#else
    FieldFrame* mFrame;
#endif

    void DrawPaths(wxDC& dc, const CC_sheet& sheet);
    void GeneratePaths();
    std::unique_ptr<Animation> mAnimation;
    bool mDrawPaths;
    void UpdateBackgroundImages();

private:
    GhostModule mGhostModule;

    CalChartDoc* mShow;
    int mCurrentReferencePoint;
    CalChartConfiguration& mConfig;
    std::vector<BackgroundImage> mBackgroundImages;
    bool mDrawBackground;
    bool mAdjustBackgroundMode;
    int mWhichBackgroundIndex;

    DECLARE_DYNAMIC_CLASS(FieldView)
};
