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
    void DrawOtherPoints(wxDC& dc, const std::map<unsigned, CC_coord>& positions);
    void OnDrawBackground(wxDC& dc);
    void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL);
    bool OnClose(bool deleteWindow = true);

    void OnWizardSetup(CalChartDoc& show);

    ///// Modify the show /////
    bool DoRotatePointPositions(unsigned rotateAmount);
    bool DoMovePoints(const std::map<unsigned, CC_coord>& transmat);
    bool DoResetReferencePoint();
    bool DoSetPointsSymbol(SYMBOL_TYPE sym);
    bool DoSetDescription(const wxString& descr);
    void DoSetMode(const wxString& mode);
    void DoSetShowInfo(unsigned numPoints, unsigned numColumns,
        const std::vector<wxString>& labels);
    bool DoSetSheetTitle(const wxString& descr);
    bool DoSetSheetBeats(unsigned short beats);
    bool DoSetPointsLabel(bool right);
    bool DoSetPointsLabelFlip();
    bool DoSetPointsLabelVisibility(bool isVisible);
    bool DoTogglePointsLabelVisibility();
    bool DoInsertSheets(const CalChartDoc::CC_sheet_container_t& sht,
        unsigned where);
    bool DoInsertSheetsOtherShow(const CalChartDoc::CC_sheet_container_t& sht,
        unsigned where, unsigned endpoint);
    bool DoDeleteSheet(unsigned where);
    bool DoImportPrintableContinuity(const wxString& file);

    ///// query show attributes /////
    int FindPoint(CC_coord pos) const;
    CC_coord PointPosition(int which) const;
    unsigned GetCurrentSheetNum() const { return mShow->GetCurrentSheetNum(); }
    unsigned short GetNumSheets() const { return mShow->GetNumSheets(); }

    CC_coord GetShowFieldOffset() const;
    CC_coord GetShowFieldSize() const;

    ///// Change show attributes /////
    void GoToSheet(size_t which);
    void GoToNextSheet();
    void GoToPrevSheet();

    void SetReferencePoint(unsigned which);

    ///// Select /////
    void UnselectAll() { mShow->UnselectAll(); }
    void AddToSelection(const SelectionList& sl);
    void ToggleSelection(const SelectionList& sl);
    void SelectWithLasso(const CC_lasso* lasso, bool toggleSelected);
    void SelectPointsInRect(const CC_coord& c1, const CC_coord& c2,
        bool toggleSelected);
    const SelectionList& GetSelectionList();

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
    unsigned mCurrentReferencePoint;
    CalChartConfiguration& mConfig;
    std::vector<BackgroundImage> mBackgroundImages;
	bool mDrawBackground;
	bool mAdjustBackgroundMode;
	int mWhichBackgroundIndex;

    DECLARE_DYNAMIC_CLASS(FieldView)
};