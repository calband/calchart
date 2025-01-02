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
#include <optional>
#include <utility>
#include <wx/docview.h>

// CalChartView connects together the Frames and the Doc.

class CalChartFrame;

class CalChartView : public wxView {
public:
    CalChartView() = default;
    ~CalChartView() override = default;

    [[nodiscard]] auto OnCreate(wxDocument* doc, long flags) -> bool override;
    void OnUpdate(wxView* sender, wxObject* hint = nullptr) override;
    [[nodiscard]] auto OnClose(bool deleteWindow = true) -> bool override;

    void OnDraw(wxDC* dc) override;
    void OnDrawBackground(wxDC& dc);

    static void OnWizardSetup(CalChartDoc& show, wxWindow* parent);

    ///// Modify the show. /////
    // Results from these commands are not generally issued - think of this as a "request"
    // to modify the show with no result of success or failure returned.
    void DoRotatePointPositions(int rotateAmount);
    void DoMovePoints(std::map<int, CalChart::Coord> const& transmat);
    void DoDeletePoints();
    void DoResetReferencePoint();
    void DoSetPointsSymbol(CalChart::SYMBOL_TYPE sym);
    void DoSetMode(CalChart::ShowMode const& mode);
    void DoSetupMarchers(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int numColumns);
    void DoSetInstruments(std::map<int, std::string> const& dotToInstrument);
    void DoSetSheetTitle(wxString const& descr);
    void DoSetSheetBeats(int beats);
    void DoSetPointsLabel(bool right);
    void DoSetPointsLabelVisibility(bool isVisible);
    void DoSetPointsLabelFlip();
    void DoTogglePointsLabelVisibility();
    void DoInsertSheets(CalChart::Show::Sheet_container_t const& sht, int where);
    void DoDeleteSheet(int where);
    void DoImportPrintableContinuity(const wxString& file);
    void DoSetPrintContinuity(int which_sheet, const wxString& number, const wxString& cont);
    void DoSetContinuityCommand(CalChart::SYMBOL_TYPE sym, CalChart::Continuity const& new_cont);
    [[nodiscard]] auto DoRelabel() -> bool;
    [[nodiscard]] auto DoAppendShow(std::unique_ptr<CalChartDoc> other_show) -> std::pair<bool, std::string>;

    ///// query show attributes /////
    [[nodiscard]] auto FindMarcher(CalChart::Coord pos) const { return mShow->FindMarcher(pos); }
    [[nodiscard]] auto PointPosition(int which) const { return mShow->GetCurrentSheet()->GetPosition(which, mShow->GetCurrentReferencePoint()); }
    [[nodiscard]] auto GetCurrentSheetNum() const { return (mShow != nullptr) ? mShow->GetCurrentSheetNum() : 0; }
    [[nodiscard]] auto GetNumSheets() const { return (mShow != nullptr) ? mShow->GetNumSheets() : 0; }
    [[nodiscard]] auto GetNumPoints() const { return (mShow != nullptr) ? mShow->GetNumPoints() : 0; }

    [[nodiscard]] auto GetShowFieldOffset() const { return mShow->GetShowFieldOffset(); }
    [[nodiscard]] auto GetShowFullSize() const { return mShow->GetShowMode().Size(); }
    [[nodiscard]] auto GetShowFieldSize() const { return mShow->GetShowMode().FieldSize(); }

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
    // Generate Draw Commands
    [[nodiscard]] auto GeneratePhatomPointsDrawCommands(std::map<int, CalChart::Coord> const& positions) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateFieldWithMarchersDrawCommands() const { return mShow->GenerateFieldWithMarchersDrawCommands(); }

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

    CalChartView(CalChartView const&) = delete;
    CalChartView(CalChartView&&) = delete;
    CalChartView& operator=(CalChartView const&) = delete;
    CalChartView& operator=(CalChartView&&) = delete;
};
