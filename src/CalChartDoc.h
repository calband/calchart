#pragma once
/*
 * CalChartDoc.h
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
    bool exportViewerFile(wxString const& filepath);

private:
    template <typename T>
    T& LoadObjectGeneric(T& stream);
    template <typename T>
    T& SaveObjectGeneric(T& stream);
    template <typename T>
    T& SaveObjectInternal(T& stream);

public:
    // translates input into a mapping of which sheet to number,lines pair.
    std::pair<bool, std::map<int, std::pair<std::string, std::string>>> ImportPrintableContinuity(std::vector<std::string> const& lines) const;

    void FlushAllTextWindows();

    void WizardSetupNewShow(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int columns, CalChart::ShowMode const& newmode);

    auto GetNumSheets() const { return mShow ? mShow->GetNumSheets() : 0; }

    struct CalChartDocSheetRange {
        CalChart::Show const& mShow;
        auto begin() { return mShow.GetSheetBegin(); }
        auto end() { return mShow.GetSheetEnd(); }
    };
    CalChartDocSheetRange GetSheets() const { return CalChartDocSheetRange{ *mShow }; }

    [[nodiscard]] auto GetSheetsName() const { return static_cast<CalChart::Show const&>(*mShow).GetSheetsName(); }

    auto GetSheetBegin() const { return static_cast<CalChart::Show const&>(*mShow).GetSheetBegin(); }
    auto GetSheetEnd() const { return static_cast<CalChart::Show const&>(*mShow).GetSheetEnd(); }
    auto GetNthSheet(int n) const { return static_cast<CalChart::Show const&>(*mShow).GetNthSheet(n); }
    auto GetCurrentSheet() const { return static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet(); }

    auto GetCurrentSheetNum() const { return static_cast<CalChart::Show const&>(*mShow).GetCurrentSheetNum(); }
    void SetCurrentSheet(int n);

    auto GetNumPoints() const { return mShow->GetNumPoints(); }
    std::pair<bool, std::vector<size_t>> GetRelabelMapping(CalChart::Show::const_Sheet_iterator_t source_sheet, CalChart::Show::const_Sheet_iterator_t target_sheets, CalChart::Coord::units tolerance) const;

    auto GetPointLabel(int i) const { return mShow->GetPointLabel(i); }
    auto GetPointsLabel() const { return mShow->GetPointsLabel(); }
    auto GetPointInstrument(int i) const { return mShow->GetPointInstrument(i); }
    auto GetPointsInstrument() const { return mShow->GetPointsInstrument(); }
    auto GetPointSymbol(int i) const { return mShow->GetPointSymbol(i); }
    auto GetPointsSymbol() const { return mShow->GetPointsSymbol(); }

    // how to select points
    // Utility functions for constructing new selection lists
    // Then you push the selection list with the Create_SetSelectionListCommand
    auto MakeSelectAll() const { return mShow->MakeSelectAll(); }
    auto MakeUnselectAll() const { return mShow->MakeUnselectAll(); }
    auto MakeAddToSelection(const CalChart::SelectionList& sl) const { return mShow->MakeAddToSelection(sl); }
    auto MakeRemoveFromSelection(const CalChart::SelectionList& sl) const { return mShow->MakeRemoveFromSelection(sl); }
    auto MakeToggleSelection(const CalChart::SelectionList& sl) const { return mShow->MakeToggleSelection(sl); }
    auto MakeSelectWithinPolygon(CalChart::RawPolygon_t const& polygon) const { return mShow->MakeSelectWithinPolygon(polygon, mCurrentReferencePoint); }
    auto MakeSelectBySymbol(CalChart::SYMBOL_TYPE symbol) const { return mShow->MakeSelectBySymbol(symbol); }
    auto MakeSelectByInstrument(std::string const& instrument) const { return mShow->MakeSelectByInstrument(instrument); }
    auto MakeSelectByLabel(std::string const& label) const { return mShow->MakeSelectByLabel(label); }

    void SetSelectionList(CalChart::SelectionList const& sl);

    auto IsSelected(int i) const { return mShow->IsSelected(i); }
    auto GetSelectionList() const { return mShow->GetSelectionList(); }

    auto GetSelect() const { return mSelect; }
    void SetSelect(CalChart::Select select);
    auto GetCurrentReferencePoint() const { return mCurrentReferencePoint; }
    void SetCurrentReferencePoint(int currentReferencePoint);
    auto GetDrawPaths() const { return mDrawPaths; }
    void SetDrawPaths(bool drawPaths);
    auto GetDrawBackground() const { return mDrawBackground; }
    void SetDrawBackground(bool drawBackground);
    auto GetCurrentMove() const { return mCurrentMove; }
    void SetCurrentMove(CalChart::MoveMode move);

    auto GetGhostModuleIsActive() const { return mGhostSource != GhostSource::disabled; }
    auto GetGhostSource() const { return mGhostSource; };
    void SetGhostSource(GhostSource source, int which = 0);

    CalChart::ShowMode const& GetShowMode() const;
    [[nodiscard]] auto GetShowFieldOffset() const { return GetShowMode().Offset(); }
    CalChart::Configuration& GetConfiguration() const { return mConfig; }

    [[nodiscard]] auto GetAnimationInfo(CalChart::beats_t whichBeat, int which) const -> std::optional<CalChart::Animate::Info>;
    [[nodiscard]] auto GetSelectedAnimationInfoWithDistanceFromPoint(CalChart::beats_t whichBeat, CalChart::Coord origin) const -> std::multimap<double, CalChart::Animate::Info>;
    [[nodiscard]] auto GetAnimationErrors() const -> std::vector<CalChart::Animate::Errors>;
    [[nodiscard]] auto GetAnimationCollisions() const -> std::map<int, CalChart::SelectionList>;
    [[nodiscard]] auto GenerateAnimationDrawCommands(
        CalChart::beats_t whichBeat,
        bool drawCollisionWarning,
        std::optional<bool> onBeat,
        CalChart::Animation::AngleStepToImageFunction imageFunction) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GetTotalNumberAnimationBeats() const -> std::optional<CalChart::beats_t>;
    [[nodiscard]] auto GetAnimationBoundingBox(bool zoomInOnMarchers, CalChart::beats_t whichBeat) const -> std::pair<CalChart::Coord, CalChart::Coord>;
    [[nodiscard]] auto BeatHasCollision(CalChart::beats_t whichBeat) const -> bool;
    [[nodiscard]] auto GetAnimationBeatForCurrentSheet() const -> CalChart::beats_t;

    [[nodiscard]] auto GenerateGhostPointsDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateCurrentSheetPointsDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>;

    [[nodiscard]] auto GeneratePhatomPointsDrawCommands(
        const std::map<int, CalChart::Coord>& positions) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateFieldWithMarchersDrawCommands() const { return mShow->GenerateFieldWithMarchersDrawCommands(mConfig); }

    auto AlreadyHasPrintContinuity() const { return mShow->AlreadyHasPrintContinuity(); }
    auto WillMovePoints(std::map<int, CalChart::Coord> const& new_positions) const { return mShow->WillMovePoints(new_positions, mCurrentReferencePoint); }
    auto PrintToPS(bool overview, int min_yards, std::set<size_t> const& isPicked, CalChart::Configuration const& config_) const -> std::tuple<std::string, int>;

    // create a set of commands to apply to the document.  This is the best way to interact with the doc.
    std::unique_ptr<wxCommand> Create_SetCurrentSheetCommand(int n);
    std::unique_ptr<wxCommand> Create_SetSelectionListCommand(const CalChart::SelectionList& sl);
    std::unique_ptr<wxCommand> Create_SetCurrentSheetAndSelectionCommand(int n, const CalChart::SelectionList& sl);
    std::unique_ptr<wxCommand> Create_SetShowModeCommand(CalChart::ShowMode const& newmode);
    std::unique_ptr<wxCommand> Create_SetupMarchersCommand(std::vector<std::pair<std::string, std::string>> const& labels, int numColumns);
    std::unique_ptr<wxCommand> Create_SetInstrumentsCommand(std::map<int, std::string> const& dotToInstrument);
    std::unique_ptr<wxCommand> Create_SetSheetTitleCommand(const wxString& newname);
    std::unique_ptr<wxCommand> Create_SetSheetBeatsCommand(int beats);
    std::unique_ptr<wxCommand> Create_AddSheetsCommand(const CalChart::Show::Sheet_container_t& sheets, int where);
    std::unique_ptr<wxCommand> Create_RemoveSheetCommand(int where);
    std::unique_ptr<wxCommand> Create_ApplyRelabelMapping(int sheet, std::vector<size_t> const& mapping);
    std::unique_ptr<wxCommand> Create_AppendShow(std::unique_ptr<CalChartDoc> sheets, CalChart::Coord::units tolerance);
    std::unique_ptr<wxCommand> Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string>> const& data);
    std::unique_ptr<wxCommand> Create_MovePointsCommand(std::map<int, CalChart::Coord> const& new_positions);
    std::unique_ptr<wxCommand> Create_MovePointsCommand(unsigned whichSheet, std::map<int, CalChart::Coord> const& new_positions);
    std::unique_ptr<wxCommand> Create_DeletePointsCommand();
    std::unique_ptr<wxCommand> Create_RotatePointPositionsCommand(int rotateAmount);
    std::unique_ptr<wxCommand> Create_ResetReferencePointToRef0();
    std::unique_ptr<wxCommand> Create_SetSymbolCommand(CalChart::SYMBOL_TYPE sym);
    std::unique_ptr<wxCommand> Create_SetContinuityCommand(CalChart::SYMBOL_TYPE i, CalChart::Continuity const& new_cont);
    std::unique_ptr<wxCommand> Create_SetLabelRightCommand(bool right);
    std::unique_ptr<wxCommand> Create_ToggleLabelFlipCommand();
    std::unique_ptr<wxCommand> Create_SetLabelVisibleCommand(bool isVisible);
    std::unique_ptr<wxCommand> Create_ToggleLabelVisibilityCommand();
    std::unique_ptr<wxCommand> Create_AddNewBackgroundImageCommand(CalChart::ImageInfo const& image);
    std::unique_ptr<wxCommand> Create_RemoveBackgroundImageCommand(int which);
    std::unique_ptr<wxCommand> Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height);
    std::unique_ptr<wxCommand> Create_SetTransitionCommand(const std::vector<CalChart::Coord>& finalPositions, const std::map<CalChart::SYMBOL_TYPE, std::string>& continuities, const std::vector<CalChart::SYMBOL_TYPE>& marcherDotTypes);

private:
    static CC_doc_command_pair Inject_CalChartDocArg(CalChart::Show_command_pair);
    std::vector<CC_doc_command_pair> Create_SetSheetPair() const;
    std::vector<CC_doc_command_pair> Create_SetSheetAndSelectionPair() const;

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
    static wxString TranslateNameToAutosaveName(const wxString& name);
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
    int mCurrentReferencePoint{};
    bool mDrawPaths{};
    bool mDrawBackground{};
    GhostSource mGhostSource = GhostSource::disabled;
    int mGhostSheet = 0;
    AutoSaveTimer mTimer;
};
