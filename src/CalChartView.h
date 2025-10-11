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

    ///// Modify the show. /////
    // Issue the change to the docs command processor, it allows for the ability to undo the change.
    void DoRotatePointPositions(int rotateAmount);
    void DoMovePoints(CalChart::MarcherToPosition const& transmat);
    void DoAssignPointsToCurve(size_t whichCurve, std::vector<CalChart::MarcherIndex> whichMarchers);
    void DoDeletePoints();
    void DoResetReferencePoint();
    void DoSetPointsSymbol(CalChart::SYMBOL_TYPE sym);
    void DoSetMode(CalChart::ShowMode const& mode);
    void DoSetupMarchers(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int numColumns);
    void DoSetInstruments(std::map<CalChart::MarcherIndex, std::string> const& dotToInstrument);
    void DoSetSheetTitle(wxString const& descr);
    void DoSetSheetBeats(CalChart::Beats beats);
    void DoSetPointsLabel(bool right);
    void DoSetPointsLabelVisibility(bool isVisible);
    void DoSetPointsLabelFlip();
    void DoTogglePointsLabelVisibility();
    void DoInsertSheets(CalChart::Show::Sheet_container_t const& sht, int where);
    void DoDeleteSheet(int where);
    void DoImportPrintableContinuity(std::string const& file);
    void DoSetPrintContinuity(int which_sheet, const wxString& number, const wxString& cont);
    void DoSetContinuityCommand(CalChart::SYMBOL_TYPE sym, CalChart::Continuity const& new_cont);
    void DoAddSheetCurveCommand(CalChart::Curve const& curve);
    void DoReplaceSheetCurveCommand(CalChart::Curve const& curve, int whichCurve);
    void DoRemoveSheetCurveCommand(int whichCurve);
    // These commands may fail, and return a string with user facing information
    [[nodiscard]] auto DoRelabel() -> std::optional<std::string>;
    [[nodiscard]] auto DoAppendShow(std::unique_ptr<CalChartDoc> other_show) -> std::optional<std::string>;

    ///// query show attributes /////
    [[nodiscard]] auto FindMarcher(CalChart::Coord pos) const { return mShow->FindMarcher(pos); }
    [[nodiscard]] auto FindCurveControlPoint(CalChart::Coord pos) const { return mShow->FindCurveControlPoint(pos); }
    [[nodiscard]] auto FindCurve(CalChart::Coord pos) const { return mShow->FindCurve(pos); }
    [[nodiscard]] auto PointPosition(CalChart::MarcherIndex which) const { return mShow->GetMarcherPositionOnCurrentSheet(which, mShow->GetCurrentReferencePoint()); }
    [[nodiscard]] auto GetCurrentSheetNum() const { return (mShow != nullptr) ? mShow->GetCurrentSheetNum() : 0; }
    [[nodiscard]] auto GetNumSheets() const { return (mShow != nullptr) ? mShow->GetNumSheets() : 0; }
    [[nodiscard]] auto GetNumPoints() const { return (mShow != nullptr) ? mShow->GetNumPoints() : 0; }
    [[nodiscard]] auto GetShowFieldOffset() const { return mShow->GetShowFieldOffset(); }
    [[nodiscard]] auto GetShowFullSize() const { return mShow->GetShowMode().Size(); }
    [[nodiscard]] auto GetShowFieldSize() const { return mShow->GetShowMode().FieldSize(); }
    [[nodiscard]] auto GetSheetsName() const { return mShow->GetSheetsName(); }
    [[nodiscard]] auto GetPrintNumber() const { return mShow->GetPrintNumber(); }
    [[nodiscard]] auto GetRawPrintContinuity() const { return mShow->GetRawPrintContinuity(); }
    [[nodiscard]] auto GetPrintContinuity() const { return mShow->GetPrintContinuity(); }
    [[nodiscard]] auto GetAnimationInfo(CalChart::Beats whichBeat) const -> std::vector<CalChart::Animate::Info>;
    [[nodiscard]] auto GetAnimationInfo(CalChart::Beats whichBeat, CalChart::MarcherIndex which) const -> std::optional<CalChart::Animate::Info>;
    [[nodiscard]] auto GetAnimationErrors() const -> std::vector<CalChart::Animate::Errors>;
    [[nodiscard]] auto GetAnimationCollisions() const -> std::map<int, CalChart::SelectionList>;
    [[nodiscard]] auto GetTotalNumberAnimationBeats() const -> std::optional<CalChart::Beats>;
    [[nodiscard]] auto GetAnimationBoundingBox(bool zoomInOnMarchers, CalChart::Beats whichBeat) const -> std::pair<CalChart::Coord, CalChart::Coord>;
    [[nodiscard]] auto BeatToSheetOffsetAndBeat(CalChart::Beats beat) const -> std::optional<std::tuple<size_t, CalChart::Beats>>;
    [[nodiscard]] auto BeatForSheet(int sheet) const -> CalChart::Beats;
    [[nodiscard]] auto GetTotalNumberBeatsUpTo(int whichSheet) const -> CalChart::Beats;

    [[nodiscard]] auto GetContinuities() const { return mShow->GetContinuities(); }
    [[nodiscard]] auto ContinuitiesInUse() const { return mShow->ContinuitiesInUse(); }
    [[nodiscard]] auto BeatHasCollision(CalChart::Beats whichBeat) const -> bool;
    [[nodiscard]] auto GetAnimationBeatForCurrentSheet() const -> CalChart::Beats;
    [[nodiscard]] auto ClipPositionToShowMode(CalChart::Coord const& pos) const { return mShow->GetShowMode().ClipPosition(pos); }

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
    void SetSelectionList(const CalChart::SelectionList& sl);
    [[nodiscard]] auto GetSelectionList() const { return mShow->GetSelectionList(); }
    [[nodiscard]] auto GetSelectedPoints() const { return mShow->GetSelectedPoints(); }
    [[nodiscard]] auto GetSelect() const { return mShow->GetSelect(); }
    void SetSelect(CalChart::Select select);
    [[nodiscard]] auto MakeSelectBySymbol(CalChart::SYMBOL_TYPE symbol) const { return mShow->MakeSelectBySymbol(symbol); }
    [[nodiscard]] auto GetCurrentMove() const { return mShow->GetCurrentMove(); }
    void SetCurrentMove(CalChart::MoveMode move) { mShow->SetCurrentMove(move); }
    [[nodiscard]] auto IsDrawingCurve() const { return mShow->IsDrawingCurve(); }
    void SetDrawingCurve(bool drawingCurve) const { mShow->SetDrawingCurve(drawingCurve); }

    [[nodiscard]] auto GetCurrentCurve(size_t index) const { return mShow->GetCurrentCurve(index); }
    [[nodiscard]] auto GetCurrentNumberCurves() const { return mShow->GetCurrentNumberCurves(); }
    [[nodiscard]] auto GetMarchersAssignedToCurve(size_t whichCurve) const { return mShow->GetMarchersAssignedToCurve(whichCurve); }

    [[nodiscard]] auto GetGhostModuleIsActive() const { return mShow->GetGhostModuleIsActive(); }
    [[nodiscard]] auto GetGhostSource() const { return mShow->GetGhostSource(); };
    void SetGhostSource(GhostSource source, int which = 0) { mShow->SetGhostSource(source, which); }

    void GoToSheetAndSetSelectionList(int which, const CalChart::SelectionList& sl);
    [[nodiscard]] auto IsSelected(CalChart::MarcherIndex i) const { return mShow->IsSelected(i); }

    ///// Drawing marcher's paths /////
    // Generate Draw Commands
    [[nodiscard]] auto GeneratePhatomPointsDrawCommands(CalChart::MarcherToPosition const& positions) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateFieldWithMarchersDrawCommands() const { return mShow->GenerateFieldWithMarchersDrawCommands(); }
    [[nodiscard]] auto GenerateAnimationDrawCommands(
        CalChart::Beats whichBeat,
        bool drawCollisionWarning,
        std::optional<bool> onBeat,
        CalChart::Animation::AngleStepToImageFunction imageFunction) const -> std::vector<CalChart::Draw::DrawCommand>;

    // call this when we need to generate the marcher's paths.
    void OnEnableDrawPaths(bool enable);

    void DoDrawBackground(bool enable);
    [[nodiscard]] auto DoingDrawBackground() const -> bool;
    void DoPictureAdjustment(bool enable);
    [[nodiscard]] auto DoingPictureAdjustment() const -> bool;
    [[nodiscard]] auto AddBackgroundImage(const wxImage& image) -> bool;
    void OnBackgroundMouseLeftDown(wxMouseEvent& event, wxDC& dc);
    void OnBackgroundMouseLeftUp(wxMouseEvent& event, wxDC& dc);
    void OnBackgroundMouseMove(wxMouseEvent& event, wxDC& dc);
    void OnBackgroundImageDelete();

private:
    void UpdateBackgroundImages();

    CalChartFrame* mFrame{};
    CalChartDoc* mShow{};
    // a cached version of the sheet background images for convenience.
    BackgroundImages mBackgroundImages;

    DECLARE_DYNAMIC_CLASS(CalChartView)

    CalChartView(CalChartView const&) = delete;
    CalChartView(CalChartView&&) = delete;
    auto operator=(CalChartView const&) -> CalChartView& = delete;
    auto operator=(CalChartView&&) -> CalChartView& = delete;
};
