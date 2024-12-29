#pragma once
/*
 * CalChartView.h
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

#include "BackgroundImages.h"
#include "CalChartCoord.h"
#include "CalChartDoc.h"
#include "CalChartShowMode.h"
#include "CalChartTypes.h"

#include <map>
#include <memory>
#include <wx/docview.h>

// CalChartView connects together the Frames and the Doc.

class BackgroundImages;
class CalChartFrame;
class AnimationFrame;
class Animation;
class Matrix;
class CalChartConfiguration;

class CalChartView : public wxView {
public:
    CalChartView() = default;
    ~CalChartView() = default;

    bool OnCreate(wxDocument* doc, long flags) override;
    void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL) override;
    bool OnClose(bool deleteWindow = true) override;

    void OnDraw(wxDC* dc) override;
    [[nodiscard]] auto GeneratePhatomPointsDrawCommands(std::map<int, CalChart::Coord> const& positions) const -> std::vector<CalChart::Draw::DrawCommand>;

    [[nodiscard]] auto GenerateFieldWithMarchersDrawCommands() const { return mShow->GenerateFieldWithMarchersDrawCommands(); }

    void OnDrawBackground(wxDC& dc);

    static void OnWizardSetup(CalChartDoc& show, wxWindow* parent);

    ///// Modify the show /////
    bool DoRotatePointPositions(int rotateAmount);
    bool DoMovePoints(std::map<int, CalChart::Coord> const& transmat);
    bool DoDeletePoints();
    bool DoResetReferencePoint();
    bool DoSetPointsSymbol(CalChart::SYMBOL_TYPE sym);
    void DoSetMode(CalChart::ShowMode const& mode);
    void DoSetupMarchers(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int numColumns);
    void DoSetInstruments(std::map<int, std::string> const& dotToInstrument);
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
    bool DoSetContinuityCommand(CalChart::SYMBOL_TYPE sym, CalChart::Continuity const& new_cont);

    ///// query show attributes /////
    int FindPoint(CalChart::Coord pos) const;
    CalChart::Coord PointPosition(int which) const;
    auto GetCurrentSheetNum() const { return mShow ? mShow->GetCurrentSheetNum() : 0; }
    auto GetNumSheets() const { return mShow ? mShow->GetNumSheets() : 0; }
    auto GetNumPoints() const { return mShow ? mShow->GetNumPoints() : 0; }

    CalChart::ShowMode const& GetShowMode() const { return mShow->GetShowMode(); }
    [[nodiscard]] auto GetShowFieldOffset() const { return mShow->GetShowMode().Offset(); }
    auto GetShowFullSize() const { return mShow->GetShowMode().Size(); }

    auto GetSheets() const { return mShow->GetSheets(); }
    auto GetCurrentSheet() const { return mShow->GetCurrentSheet(); }

    [[nodiscard]] auto GetSheetsName() const { return mShow->GetSheetsName(); }

    [[nodiscard]] auto GetAnimationInfo(CalChart::beats_t whichBeat, int which) const -> std::optional<CalChart::Animate::Info>;
    [[nodiscard]] auto GetSelectedAnimationInfoWithDistanceFromPoint(CalChart::beats_t whichBeat, CalChart::Coord origin) const -> std::multimap<double, CalChart::Animate::Info>;
    std::vector<CalChart::Animate::Errors> GetAnimationErrors() const;
    // Sheet -> all collisions
    std::map<int, CalChart::SelectionList> GetAnimationCollisions() const;
    [[nodiscard]] auto GenerateAnimationDrawCommands(
        CalChart::beats_t whichBeat,
        bool drawCollisionWarning,
        std::optional<bool> onBeat,
        CalChart::Animation::AngleStepToImageFunction imageFunction) const -> std::vector<CalChart::Draw::DrawCommand>;

    [[nodiscard]] auto GetTotalNumberAnimationBeats() const -> std::optional<CalChart::beats_t>;
    [[nodiscard]] auto GetAnimationBoundingBox(bool zoomInOnMarchers, CalChart::beats_t whichBeat) const -> std::pair<CalChart::Coord, CalChart::Coord>;

    [[nodiscard]] auto BeatHasCollision(CalChart::beats_t whichBeat) const -> bool;
    [[nodiscard]] auto GetAnimationBeatForCurrentSheet() const -> CalChart::beats_t;

    auto ClipPositionToShowMode(CalChart::Coord const& pos) const { return mShow->GetShowMode().ClipPosition(pos); }

    ///// Change show attributes /////
    void GoToSheet(int which);
    void GoToNextSheet() { GoToSheet(mShow->GetCurrentSheetNum() + 1); }
    void GoToPrevSheet() { GoToSheet(mShow->GetCurrentSheetNum() - 1); }

    void SetActiveReferencePoint(int which);

    ///// Select /////
    void UnselectAll() { SetSelectionList(mShow->MakeUnselectAll()); }
    void AddToSelection(const CalChart::SelectionList& sl) { SetSelectionList(mShow->MakeAddToSelection(sl)); }
    void ToggleSelection(const CalChart::SelectionList& sl) { SetSelectionList(mShow->MakeToggleSelection(sl)); }
    void SelectWithinPolygon(CalChart::RawPolygon_t const& polygon, bool toggleSelected);
    void SelectPointsInRect(const CalChart::Coord& c1, const CalChart::Coord& c2, bool toggleSelected);
    auto GetSelectionList() const { return mShow->GetSelectionList(); }
    void SetSelectionList(const CalChart::SelectionList& sl);
    auto GetSelect() const { return mShow->GetSelect(); }
    void SetSelect(CalChart::Select select);
    auto GetCurrentMove() const { return mShow->GetCurrentMove(); }
    void SetCurrentMove(CalChart::MoveMode move) { mShow->SetCurrentMove(move); }
    auto GetGhostModuleIsActive() const { return mShow->GetGhostModuleIsActive(); }
    auto GetGhostSource() const { return mShow->GetGhostSource(); };
    void SetGhostSource(GhostSource source, int which = 0) { mShow->SetGhostSource(source, which); }

    void GoToSheetAndSetSelectionList(int which, const CalChart::SelectionList& sl);
    auto IsSelected(int i) const { return mShow->IsSelected(i); }

    ///// Drawing marcher's paths /////
    // call this when we need to generate the marcher's paths.
    void OnEnableDrawPaths(bool enable);

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
    void GeneratePaths();
    void UpdateBackgroundImages();

    CalChartFrame* mFrame{};
    CalChartDoc* mShow{};
    // a cached version of the sheet background images for convenience.
    BackgroundImages mBackgroundImages;

    DECLARE_DYNAMIC_CLASS(CalChartView)
};
