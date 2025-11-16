#pragma once
/*
 * CalChartDoc.h
 */

/*
   Copyright (C) 1995-2025  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartMovePointsTool.h"
#include "CalChartSelectTool.h"
#include "CalChartShow.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <wx/cmdproc.h>
#include <wx/docview.h> // For basic wx defines
#include <wx/wx.h> // For basic wx defines

// CalChartDoc is the document in the CalChart Document/View model.  It represents
// the loaded show.  Commands to manipulate the documents are processed as commands
// so we may maintain an undo history.

class CalChartDoc;
namespace CalChart {
class ShowMode;
class Show;
class Sheet;
class Lasso;
class Animation;
class Configuration;
struct TransitionSolverParams;
class TransitionSolverDelegate;

}

enum class GhostSource {
    disabled,
    next,
    previous,
    specific,
};

using CC_doc_command = std::function<void(CalChartDoc&)>;
using CC_doc_command_pair = std::pair<CC_doc_command, CC_doc_command>;

// The CalChartDoc_modified class is used for indicating to views if the doc has
// been modified
// some views behave differently if the show has been modified
class CalChartDoc_modified : public wxObject {
    DECLARE_DYNAMIC_CLASS(CalChartDoc_modified)
};

// The CalChartDoc_modified class is used for indicating to views to save any
// text
class CalChartDoc_FlushAllViews : public wxObject {
    DECLARE_DYNAMIC_CLASS(CalChartDoc_FlushAllViews)
};

// The CalChartDoc_FinishedLoading class is used for indicating to views that a
// new file has been loaded
class CalChartDoc_FinishedLoading : public wxObject {
    DECLARE_DYNAMIC_CLASS(CalChartDoc_FinishedLoading)
};

// The CalChartDoc_setup class is used for indicating to views to set up a new
// show
class CalChartDoc_setup : public wxObject {
    DECLARE_DYNAMIC_CLASS(CalChartDoc_setup)
};

// CalChart Document.
// This holds the CalChart::Show, the core part of CalChart.
class CalChartDoc : public wxDocument {
    using super = wxDocument;
    DECLARE_DYNAMIC_CLASS(CalChartDoc)
public:
    CalChartDoc();
    ~CalChartDoc() override = default;

    // Override the wxDocument functions:
    // Need to override OnOpenDoc so we can report errors, handle recovery file
    bool OnOpenDocument(wxString const& filename) override;
    // Need to override OnOpenDoc so we can report errors, handle recovery file
    bool OnCloseDocument() override;
    // Need to override OnNewDoc so we can start the setup wizard
    bool OnNewDocument() override;
    // Need to override OnSaveDoc so we can handle recovery files
    bool OnSaveDocument(const wxString& filename) override;
    // Update the views that the doc been modified
    void Modify(bool b) override;

    // How we save and load a show:
    wxSTD ostream& SaveObject(wxSTD ostream& stream) override;
    wxSTD istream& LoadObject(wxSTD istream& stream) override;

    /*!
     * @brief Exports the show to a file that can be animated by
     * the CalChart Online Viewer.
     * @param filepath The filepath (relative or absolute) to which the
     * Viewer file will be saved. Note that the CalChart Online Viewer
     * expects a viewer file to end with the '.viewer' extension.
     * @return True if the file was saved successfully; false otherwise.
     */
    bool exportViewerFile(std::filesystem::path const& filepath);

private:
    template <typename T>
    T& LoadObjectGeneric(T& stream);
    template <typename T>
    T& SaveObjectGeneric(T& stream);
    template <typename T>
    T& SaveObjectInternal(T& stream);

public:
    // translates input into a mapping of which sheet to number,lines pair.
    [[nodiscard]] auto ImportPrintableContinuity(std::vector<std::string> const& lines) const -> std::optional<std::map<int, std::pair<std::string, std::string>>>;

    void FlushAllTextWindows();

    void WizardSetupNewShow(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int columns, CalChart::ShowMode const& newmode);

    [[nodiscard]] auto GetNumSheets() const { return mShow ? mShow->GetNumSheets() : 0; }

    [[nodiscard]] auto GetSheetsName() const { return mShow->GetSheetsName(); }

    [[nodiscard]] auto GetNthSheet(int n) const { return static_cast<CalChart::Show const&>(*mShow).GetNthSheet(n); }
    [[nodiscard]] auto CopyAllSheets() const { return mShow->CopyAllSheets(); }
    [[nodiscard]] auto CopyCurrentSheet() const { return mShow->CopyCurrentSheet(); }

    [[nodiscard]] auto GetCurrentSheetNum() const { return mShow->GetCurrentSheetNum(); }
    [[nodiscard]] auto GetCurrentSheetName() const { return mShow->GetCurrentSheetName(); }
    [[nodiscard]] auto GetCurrentSheetBeats() const { return mShow->GetCurrentSheetBeats(); }
    [[nodiscard]] auto GetCurrentSheetSymbols() const { return mShow->GetCurrentSheetSymbols(); }
    [[nodiscard]] auto GetCurrentSheetBackgroundImages() const { return mShow->GetCurrentSheetBackgroundImages(); }
    void SetCurrentSheet(int n);

    [[nodiscard]] auto GetNumPoints() const { return mShow->GetNumPoints(); }
    [[nodiscard]] auto GetRelabelMapping(std::vector<CalChart::Coord> const& source_marchers, std::vector<CalChart::Coord> const& target_marchers) const -> std::optional<std::vector<CalChart::MarcherIndex>>;

    [[nodiscard]] auto GetPointLabel(CalChart::MarcherIndex i) const { return mShow->GetPointLabel(i); }
    [[nodiscard]] auto GetPointsLabel() const { return mShow->GetPointsLabel(); }
    [[nodiscard]] auto GetPointsLabel(CalChart::SelectionList const& sl) const { return mShow->GetPointsLabel(sl); }
    [[nodiscard]] auto GetPointInstrument(CalChart::MarcherIndex i) const { return mShow->GetPointInstrument(i); }
    [[nodiscard]] auto GetPointInstrument(std::string const& label) const { return mShow->GetPointInstrument(label); }
    [[nodiscard]] auto GetPointsInstrument() const { return mShow->GetPointsInstrument(); }
    [[nodiscard]] auto GetPointsInstrument(CalChart::SelectionList const& sl) const { return mShow->GetPointsInstrument(sl); }
    [[nodiscard]] auto GetPointSymbol(CalChart::MarcherIndex i) const { return mShow->GetPointSymbol(i); }
    [[nodiscard]] auto GetPointSymbol(std::string const& label) const { return mShow->GetPointSymbol(label); }
    [[nodiscard]] auto GetPointsSymbol() const { return mShow->GetPointsSymbol(); }
    [[nodiscard]] auto GetPointsSymbol(CalChart::SelectionList const& sl) const { return mShow->GetPointsSymbol(sl); }
    [[nodiscard]] auto GetPointFromLabel(std::string const& label) const { return mShow->GetPointFromLabel(label); }
    [[nodiscard]] auto GetPointsFromLabels(std::vector<std::string> const& labels) const { return mShow->GetPointsFromLabels(labels); }
    [[nodiscard]] auto GetMarcherPositionOnCurrentSheet(CalChart::MarcherIndex i, unsigned ref = 0) const { return mShow->GetMarcherPositionOnCurrentSheet(i, ref); }
    [[nodiscard]] auto GetAllMarcherPositions(int sheet, unsigned ref = 0) const { return mShow->GetAllMarcherPositions(sheet, ref); }
    [[nodiscard]] auto GetAllMarcherPositionsOnCurrentSheet(unsigned ref = 0) const { return mShow->GetAllMarcherPositionsOnCurrentSheet(ref); }

    [[nodiscard]] auto GetContinuities() const { return mShow->GetContinuitiesOnCurrentSheet(); }
    [[nodiscard]] auto ContinuitiesInUse() const { return mShow->GetContinuitiesInUseOnCurrentSheet(); }

    [[nodiscard]] auto GetCurrentCurve(size_t index) const { return mShow->GetCurveOnCurrentSheet(index); }
    [[nodiscard]] auto GetCurrentNumberCurves() const { return mShow->GetNumberCurvesOnCurrentSheet(); }
    [[nodiscard]] auto GetMarchersAssignedToCurve(size_t whichCurve) const { return mShow->GetCurveAssignmentsOnCurrentSheet().at(whichCurve); }
    [[nodiscard]] auto GetCurrentSheetSerialized() const { return mShow->GetCurrentSheetSerialized(); }

    // Transition Solver
    [[nodiscard]] auto validateCurrentSheetForTransitionSolver() const { return mShow->validateCurrentSheetForTransitionSolver(); }
    [[nodiscard]] auto validateNextSheetForTransitionSolver() const { return mShow->validateNextSheetForTransitionSolver(); }
    void runTransitionSolver(CalChart::TransitionSolverParams const& params, CalChart::TransitionSolverDelegate* delegate) const
    {
        return mShow->runTransitionSolver(params, delegate);
    }

    // how to select points
    // Utility functions for constructing new selection lists
    // Then you push the selection list with the Create_SetSelectionListCommand
    [[nodiscard]] auto MakeSelectAll() const { return mShow->MakeSelectAll(); }
    [[nodiscard]] auto MakeUnselectAll() const { return mShow->MakeUnselectAll(); }
    [[nodiscard]] auto MakeAddToSelection(CalChart::SelectionList const& sl) const { return mShow->MakeAddToSelection(sl); }
    [[nodiscard]] auto MakeRemoveFromSelection(CalChart::SelectionList const& sl) const { return mShow->MakeRemoveFromSelection(sl); }
    [[nodiscard]] auto MakeToggleSelection(CalChart::SelectionList const& sl) const { return mShow->MakeToggleSelection(sl); }
    [[nodiscard]] auto MakeSelectWithinPolygon(CalChart::RawPolygon_t const& polygon) const { return mShow->MakeSelectWithinPolygon(polygon, mShow->GetCurrentReferencePoint()); }
    [[nodiscard]] auto MakeSelectBySymbol(CalChart::SYMBOL_TYPE symbol) const { return mShow->MakeSelectBySymbol(symbol); }
    [[nodiscard]] auto MakeSelectByInstrument(std::string const& instrument) const { return mShow->MakeSelectByInstrument(instrument); }
    [[nodiscard]] auto MakeSelectByLabel(std::string const& label) const { return mShow->MakeSelectByLabel(label); }
    [[nodiscard]] auto MakeSelectByLabels(std::vector<std::string> const& label) const { return mShow->MakeSelectByLabels(label); }

    void SetSelectionList(CalChart::SelectionList const& sl);

    [[nodiscard]] auto IsSelected(CalChart::MarcherIndex i) const { return mShow->IsSelected(i); }
    [[nodiscard]] auto GetSelectionList() const { return mShow->GetSelectionList(); }
    // get the selection list and where the points are.
    [[nodiscard]] auto GetSelectedPoints() const -> CalChart::MarcherToPosition;

    [[nodiscard]] auto GetSelect() const { return mSelect; }
    void SetSelect(CalChart::Select select);
    [[nodiscard]] auto GetCurrentReferencePoint() const { return mShow->GetCurrentReferencePoint(); }
    void SetCurrentReferencePoint(int currentReferencePoint);
    [[nodiscard]] auto FindMarcher(CalChart::Coord pos) const -> std::optional<CalChart::MarcherIndex>;
    // Determine if the position is on a curve control point, if so, return which curve and which control point.
    [[nodiscard]] auto FindCurveControlPoint(CalChart::Coord pos) const -> std::optional<std::tuple<size_t, size_t>>;
    // if found curve, return which curve, the control point lower to where we clicked, the distance [0.0, 1.0] of the pos
    [[nodiscard]] auto FindCurve(CalChart::Coord pos) const -> std::optional<std::tuple<size_t, size_t, double>>;
    [[nodiscard]] auto GetDrawPaths() const { return mDrawPaths; }
    void SetDrawPaths(bool drawPaths);
    [[nodiscard]] auto GetDrawBackground() const { return mDrawBackground; }
    void SetDrawBackground(bool drawBackground);
    [[nodiscard]] auto GetCurrentMove() const { return mCurrentMove; }
    void SetCurrentMove(CalChart::MoveMode move);
    [[nodiscard]] auto IsDrawingCurve() const { return mDrawingCurve; }
    void SetDrawingCurve(bool drawingCurve) { mDrawingCurve = drawingCurve; }

    [[nodiscard]] auto GetGhostModuleIsActive() const { return mGhostSource != GhostSource::disabled; }
    [[nodiscard]] auto GetGhostSource() const { return mGhostSource; };
    void SetGhostSource(GhostSource source, int which = 0);

    [[nodiscard]] CalChart::ShowMode const& GetShowMode() const;
    [[nodiscard]] auto GetShowFieldOffset() const { return GetShowMode().Offset(); }
    [[nodiscard]] CalChart::Configuration& GetConfiguration() const { return mConfig; }

    [[nodiscard]] auto GetAnimationInfo(CalChart::Beats whichBeat) const -> std::vector<CalChart::Animate::Info>;
    [[nodiscard]] auto GetAnimationInfo(CalChart::MarcherIndex whichMarcher, CalChart::Beats whichBeat) const -> std::optional<CalChart::Animate::Info>;
    [[nodiscard]] auto GetAnimationErrors() const -> std::vector<CalChart::Animate::Errors>;
    [[nodiscard]] auto GetAnimationCollisions() const -> std::map<int, CalChart::SelectionList>;
    [[nodiscard]] auto GenerateAnimationDrawCommands(
        CalChart::Beats whichBeat,
        bool drawCollisionWarning,
        std::optional<bool> onBeat,
        CalChart::Animation::AngleStepToImageFunction imageFunction) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GetTotalNumberAnimationBeats() const -> std::optional<CalChart::Beats>;
    [[nodiscard]] auto BeatToSheetOffsetAndBeat(CalChart::Beats whichBeat) const -> std::optional<std::tuple<size_t, CalChart::Beats>>;
    [[nodiscard]] auto BeatForSheet(int whichSheet) const -> CalChart::Beats;
    [[nodiscard]] auto GetTotalNumberBeatsUpTo(int whichSheet) const -> CalChart::Beats;
    [[nodiscard]] auto GetAnimationBoundingBox(bool zoomInOnMarchers, CalChart::Beats whichBeat) const -> std::pair<CalChart::Coord, CalChart::Coord>;
    [[nodiscard]] auto BeatHasCollision(CalChart::Beats whichBeat) const -> bool;
    [[nodiscard]] auto GetAnimationBeatForCurrentSheet() const -> CalChart::Beats;

    [[nodiscard]] auto GenerateGhostPointsDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateCurrentSheetPointsDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>;

    [[nodiscard]] auto GeneratePhatomPointsDrawCommands(CalChart::MarcherToPosition const& positions) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateFieldWithMarchersDrawCommands() const { return mShow->GenerateFieldWithMarchersDrawCommands(mConfig); }

    [[nodiscard]] auto AlreadyHasPrintContinuity() const { return mShow->AlreadyHasPrintContinuity(); }
    [[nodiscard]] auto GetPrintNumber() const { return mShow->GetCurrentSheetPrintNumber(); }
    [[nodiscard]] auto GetRawPrintContinuity() const { return mShow->GetCurrentSheetRawPrintContinuity(); }
    [[nodiscard]] auto GetPrintContinuity() const { return mShow->GetCurrentSheetPrintContinuity(); }

    [[nodiscard]] auto WillMovePoints(CalChart::MarcherToPosition const& new_positions) const { return mShow->WillMovePoints(new_positions, mShow->GetCurrentReferencePoint()); }
    [[nodiscard]] auto PrintToPS(bool overview, int min_yards, std::set<size_t> const& isPicked, CalChart::Configuration const& config_) const -> std::tuple<std::string, int>;

    // create a set of commands to apply to the document.  This is the best way to interact with the doc.
    [[nodiscard]] auto Create_SetCurrentSheetCommand(int n) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetSelectionListCommand(const CalChart::SelectionList& sl) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetCurrentSheetAndSelectionCommand(int n, const CalChart::SelectionList& sl) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetShowModeCommand(CalChart::ShowMode const& newmode) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetupMarchersCommand(std::vector<std::pair<std::string, std::string>> const& labels, int numColumns) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetInstrumentsCommand(std::map<CalChart::MarcherIndex, std::string> const& dotToInstrument) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetSheetTitleCommand(wxString const& newname) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetSheetBeatsCommand(CalChart::Beats beats) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_AddSheetsCommand(CalChart::Show::Sheet_container_t const& sheets, int where) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_RemoveSheetCommand(int where) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_ApplyRelabelMapping(int sheet, std::vector<CalChart::MarcherIndex> const& mapping) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_AppendShow(std::unique_ptr<CalChartDoc> sheets) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string>> const& data) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_MovePointsCommand(CalChart::MarcherToPosition const& new_positions) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_MovePointsCommand(unsigned whichSheet, CalChart::MarcherToPosition const& new_positions) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_AssignPointsToCurve(size_t whichCurve, std::vector<CalChart::MarcherIndex> whichMarchers) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_DeletePointsCommand() -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_RotatePointPositionsCommand(int rotateAmount) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_ResetReferencePointToRef0() -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetSymbolCommand(CalChart::SYMBOL_TYPE sym) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetContinuityCommand(CalChart::SYMBOL_TYPE i, CalChart::Continuity const& new_cont) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetLabelRightCommand(bool right) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_ToggleLabelFlipCommand() -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetLabelVisibleCommand(bool isVisible) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_ToggleLabelVisibilityCommand() -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_AddNewBackgroundImageCommand(CalChart::ImageInfo const& image) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_RemoveBackgroundImageCommand(int which) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_SetTransitionCommand(std::vector<CalChart::Coord> const& finalPositions, const std::map<CalChart::SYMBOL_TYPE, std::string>& continuities, const std::vector<CalChart::SYMBOL_TYPE>& marcherDotTypes) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_AddSheetCurveCommand(CalChart::Curve const& curve) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_ReplaceSheetCurveCommand(CalChart::Curve const& curve, int whichCurve) -> std::unique_ptr<wxCommand>;
    [[nodiscard]] auto Create_RemoveSheetCurveCommand(int whichCurve) -> std::unique_ptr<wxCommand>;

private:
    [[nodiscard]] static auto Inject_CalChartDocArg(CalChart::Show_command_pair) -> CC_doc_command_pair;
    [[nodiscard]] auto Create_SetSheetPair() const -> std::vector<CC_doc_command_pair>;
    [[nodiscard]] auto Create_SetSheetAndSelectionPair() const -> std::vector<CC_doc_command_pair>;

    [[nodiscard]] auto GetGhostSheet() const -> CalChart::Sheet const*;
    [[nodiscard]] auto GeneratePathsDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>;

    // Autosaving:
    // goal is to allow the user to have a recoverable file.
    //
    // When the timer goes off, and if the show is modified,
    // we will write the file to a version of the file that the same
    // but with the extension .shw~, to indicate that there is a recovery
    // file at that location.
    // When a file is opened, we first check to see if there is a temporary
    // file, and if there is, prompt the user to see if they would like use
    // that file instead.
    // When we save a file, the recovery file should be removed to prevent
    // a false detection that the file writing failed.
    static auto TranslateNameToAutosaveName(const wxString& name) -> wxString;
    void Autosave();

    class AutoSaveTimer : public wxTimer {
    public:
        AutoSaveTimer(CalChartDoc& show)
            : mShow(show)
        {
        }
        ~AutoSaveTimer() { Stop(); }
        void Notify();

    private:
        CalChartDoc& mShow;
    };

    // CalChart doc contains the state of the show and all ancillary data objects for displaying/manipulating the show
    // This include temporary non-saved aspects like what configuration tools are in (select mode), or what reference
    // points are currently being moved.
    CalChart::Configuration& mConfig;
    std::unique_ptr<CalChart::Show> mShow;
    std::optional<CalChart::Animation> mAnimation;
    CalChart::Select mSelect = CalChart::Select::Box;
    CalChart::MoveMode mCurrentMove = CalChart::MoveMode::Normal;
    bool mDrawPaths{};
    bool mDrawBackground{};
    GhostSource mGhostSource = GhostSource::disabled;
    int mGhostSheet = 0;
    AutoSaveTimer mTimer;
    bool mDrawingCurve = false;
};
