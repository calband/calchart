#pragma once
/*
 * CalChartShow.h
 * Definitions for the calchart show classes
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

/**
 * CalChart Show
 *
 *  The CalChart show object is what most parts of the system interact with.  It is essentially a collection of
 *  CalChartSheets that you iterate through to either look at, or change.
 *
 * Show Modification
 *  There are a large number of functions to read the current values from the show.  However, in order to
 *  "modify" the show, you do so by creating a Show_command_pair, which is effectively a "Do/Undo" pair of
 *  functions that execute the desired modification.  This convention is done so that it is straightfoward to
 *  use these in a command "stack" where in order to do or undo an action you simply call the functions.
 *
 */

#include "CalChartAnimation.h"
#include "CalChartConstants.h"
#include "CalChartCoord.h"
#include "CalChartFileFormat.h"
#include "CalChartImage.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "CalChartShowMode.h"
#include "CalChartTypes.h"

#include <cstddef>
#include <functional>
#include <istream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace CalChart {
class Show;
class Sheet;
class Reader;
struct ParseErrorHandlers;

using Show_command = std::function<void(Show&)>;
using Show_command_pair = std::pair<Show_command, Show_command>;

class Show {
public:
    using Sheet_container_t = std::vector<Sheet>;
    using Sheet_iterator_t = Sheet_container_t::iterator;
    using const_Sheet_iterator_t = Sheet_container_t::const_iterator;

    // you can create a show in two ways, from nothing, or from an input stream
    static auto Create(ShowMode const& mode) -> std::unique_ptr<Show>;
    static auto Create(ShowMode const& mode, std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, unsigned columns) -> std::unique_ptr<Show>;
    static auto Create(ShowMode const& mode, std::istream& stream, ParseErrorHandlers const* correction = nullptr) -> std::unique_ptr<Show>;

    // These constructors are exposed for testing purposes, and generally should not be used
    explicit Show(ShowMode const& mode);
    Show(Version_3_3_and_earlier, ShowMode const& mode, Reader reader, ParseErrorHandlers const* correction = nullptr);
    Show(ShowMode const& mode, Reader reader, ParseErrorHandlers const* correction = nullptr);

    // Create command, consists of an action and undo action
    [[nodiscard]] auto Create_SetCurrentSheetCommand(int n) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetSelectionListCommand(SelectionList const& sl) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetCurrentSheetAndSelectionCommand(int n, SelectionList const& sl) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetShowModeCommand(CalChart::ShowMode const& newmode) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetupMarchersCommand(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int numColumns, Coord const& new_march_position) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetInstrumentsCommand(std::map<MarcherIndex, std::string> const& dotToInstrument) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetSheetTitleCommand(std::string const& newname) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetSheetBeatsCommand(Beats beats) const -> Show_command_pair;
    [[nodiscard]] auto Create_AddSheetsCommand(Show::Sheet_container_t const& sheets, int where) const -> Show_command_pair;
    [[nodiscard]] auto Create_RemoveSheetCommand(int where) const -> Show_command_pair;
    [[nodiscard]] auto Create_ApplyRelabelMapping(int sheet_num_first, std::vector<MarcherIndex> const& mapping) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string>> const& data) const -> Show_command_pair;
    [[nodiscard]] auto Create_MovePointsCommand(MarcherToPosition const& new_positions, int ref) const -> Show_command_pair;
    [[nodiscard]] auto Create_MovePointsCommand(int whichSheet, MarcherToPosition const& new_positions, int ref) const -> Show_command_pair;
    [[nodiscard]] auto Create_AssignPointsToCurve(size_t whichCurve, std::vector<MarcherIndex> whichMarchers) -> Show_command_pair;
    [[nodiscard]] auto Create_DeletePointsCommand() const -> Show_command_pair;
    [[nodiscard]] auto Create_RotatePointPositionsCommand(int rotateAmount, int ref) const -> Show_command_pair;
    [[nodiscard]] auto Create_ResetReferencePointToRef0(int ref) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetSymbolCommand(SYMBOL_TYPE sym) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetSymbolCommand(SelectionList const& whichDots, SYMBOL_TYPE sym) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetContinuityCommand(SYMBOL_TYPE which_sym, Continuity const& cont) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetLabelFlipCommand(std::map<MarcherIndex, bool> const& new_flip) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetLabelRightCommand(bool right) const -> Show_command_pair;
    [[nodiscard]] auto Create_ToggleLabelFlipCommand() const -> Show_command_pair;
    [[nodiscard]] auto Create_SetLabelVisiblityCommand(std::map<MarcherIndex, bool> const& new_visibility) const -> Show_command_pair;
    [[nodiscard]] auto Create_SetLabelVisibleCommand(bool isVisible) const -> Show_command_pair;
    [[nodiscard]] auto Create_ToggleLabelVisibilityCommand() const -> Show_command_pair;
    [[nodiscard]] auto Create_AddNewBackgroundImageCommand(ImageInfo const& image) const -> Show_command_pair;
    [[nodiscard]] auto Create_RemoveBackgroundImageCommand(int which) const -> Show_command_pair;
    [[nodiscard]] auto Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height) const -> Show_command_pair;
    [[nodiscard]] auto Create_AddSheetCurveCommand(CalChart::Curve const& curve) const -> Show_command_pair;
    [[nodiscard]] auto Create_ReplaceSheetCurveCommand(CalChart::Curve const& curve, int whichCurve) const -> Show_command_pair;
    [[nodiscard]] auto Create_RemoveSheetCurveCommand(int whichCurve) const -> Show_command_pair;

    // Accessors
    [[nodiscard]] auto GetSheetBegin() const { return mSheets.begin(); }
    [[nodiscard]] auto GetSheetEnd() const { return mSheets.end(); }
    [[nodiscard]] auto GetNthSheet(unsigned n) const { return GetSheetBegin() + n; }
    [[nodiscard]] auto GetCurrentSheet() const { return GetNthSheet(mSheetNum); }
    [[nodiscard]] auto GetNumSheets() const -> int;
    [[nodiscard]] auto GetCurrentSheetNum() const { return mSheetNum; }
    [[nodiscard]] auto GetNumPoints() const { return static_cast<int>(mDotLabelAndInstrument.size()); }
    [[nodiscard]] auto GetPointLabel(MarcherIndex i) const -> std::string;
    [[nodiscard]] auto GetPointsLabel() const -> std::vector<std::string>;
    [[nodiscard]] auto GetPointsLabel(CalChart::SelectionList const& sl) const -> std::vector<std::string>;
    [[nodiscard]] auto GetPointInstrument(MarcherIndex i) const -> std::string;
    [[nodiscard]] auto GetPointInstrument(std::string const& label) const -> std::optional<std::string>;
    [[nodiscard]] auto GetPointsInstrument() const -> std::vector<std::string>;
    [[nodiscard]] auto GetPointsInstrument(CalChart::SelectionList const& sl) const -> std::vector<std::string>;
    [[nodiscard]] auto GetPointSymbol(MarcherIndex i) const -> SYMBOL_TYPE;
    [[nodiscard]] auto GetPointSymbol(std::string const& label) const -> std::optional<SYMBOL_TYPE>;
    [[nodiscard]] auto GetPointsSymbol() const -> std::vector<SYMBOL_TYPE>;
    [[nodiscard]] auto GetPointsSymbol(CalChart::SelectionList const& sl) const -> std::vector<SYMBOL_TYPE>;
    [[nodiscard]] auto GetPointFromLabel(std::string const& label) const -> std::optional<CalChart::MarcherIndex>;
    [[nodiscard]] auto GetPointsFromLabels(std::vector<std::string> const& labels) const -> std::vector<CalChart::MarcherIndex>;
    [[nodiscard]] auto GetAllMarcherPositions(int sheet, unsigned ref = 0) const -> std::vector<Coord>;
    [[nodiscard]] auto GetCurrentSheetAllMarcherPositions(unsigned ref = 0) const -> std::vector<Coord>;
    [[nodiscard]] auto AlreadyHasPrintContinuity() const -> bool;
    [[nodiscard]] auto const& GetShowMode() const { return mMode; }

    [[nodiscard]] auto GetSheetsName() const -> std::vector<std::string>;

    // utility
    [[nodiscard]] static auto GetRelabelMapping(std::vector<Coord> const& source_marchers, std::vector<Coord> const& target_marchers, CalChart::Coord::units tolerance) -> std::optional<std::vector<MarcherIndex>>;
    [[nodiscard]] auto MakeSelectAll() const -> SelectionList;
    [[nodiscard]] auto MakeUnselectAll() const -> SelectionList;
    [[nodiscard]] auto MakeAddToSelection(SelectionList const& sl) const -> SelectionList;
    [[nodiscard]] auto MakeRemoveFromSelection(SelectionList const& sl) const -> SelectionList;
    [[nodiscard]] auto MakeToggleSelection(SelectionList const& sl) const -> SelectionList;
    [[nodiscard]] auto MakeSelectWithinPolygon(CalChart::RawPolygon_t const& polygon, int ref) const -> SelectionList;
    [[nodiscard]] auto MakeSelectBySymbol(SYMBOL_TYPE i) const -> SelectionList;
    [[nodiscard]] auto MakeSelectByInstrument(std::string const& instrumentName) const -> SelectionList;
    [[nodiscard]] auto MakeSelectByLabel(std::string const& labelName) const -> SelectionList;
    [[nodiscard]] auto MakeSelectByLabels(std::vector<std::string> const& labelName) const -> SelectionList;

    // Point selection
    [[nodiscard]] auto IsSelected(MarcherIndex i) const { return mSelectionList.contains(i); }
    [[nodiscard]] auto GetSelectionList() const { return mSelectionList; }
    [[nodiscard]] auto WillMovePoints(MarcherToPosition const& new_positions, int ref) const -> bool;

    /*!
     * @brief Generates a JSON that could represent this
     * show in an Online Viewer '.viewer' file.
     * @param compiledShow An up-to-date Animation of the show.
     * @return A JSON which could represent this show in
     * a '.viewer' file.
     */
    [[nodiscard]] auto toOnlineViewerJSON(Animation const& compiledShow) const -> nlohmann::json;

    // Saving the show.
    [[nodiscard]] auto SerializeShow() const -> std::vector<std::byte>;

    [[nodiscard]] auto AreSheetsInAnimation() const
    {
        return mSheets | std::views::transform([](auto&& sheet) { return sheet.IsInAnimation(); });
    }

    [[nodiscard]] auto SheetsInAnimation() const
    {
        return mSheets | std::views::filter([](auto&& sheet) { return sheet.IsInAnimation(); });
    }

    [[nodiscard]] auto GenerateSheetElements(
        CalChart::Configuration const& config,
        int ref) const -> std::vector<CalChart::Draw::DrawCommand>;

    [[nodiscard]] auto GenerateGhostPointsDrawCommands(
        CalChart::Configuration const& config,
        CalChart::SelectionList const& selection_list,
        CalChart::Sheet const& sheet) const -> std::vector<CalChart::Draw::DrawCommand>;

    [[nodiscard]] auto GenerateFieldWithMarchersDrawCommands(CalChart::Configuration const& config) const -> std::vector<std::vector<CalChart::Draw::DrawCommand>>;

private:
    // modification of show is private, and externally done through create and exeucte commands
    auto RemoveNthSheet(int sheetidx) -> Sheet_container_t;
    void InsertSheet(Sheet const& nsheet, int sheetidx);
    void InsertSheet(Sheet_container_t const& nsheet, int sheetidx);
    void SetCurrentSheet(int n);
    void SetSelectionList(SelectionList const& sl);

    void SetNumPoints(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int columns, Coord const& new_march_position);
    void DeletePoints(SelectionList const& sl);
    void SetPointLabelAndInstrument(std::vector<std::pair<std::string, std::string>> const& labels);

    // Descriptions aren't used, but keeping this alive.  See issue #203
    [[nodiscard]] auto GetDescr() const
    {
        return mDescr;
    }
    void SetDescr(std::string const& newdescr)
    {
        mDescr = newdescr;
    }

    auto GetSheetBegin()
    {
        return mSheets.begin();
    }
    auto GetSheetEnd()
    {
        return mSheets.end();
    }
    auto GetNthSheet(unsigned n)
    {
        return GetSheetBegin() + n;
    }
    auto GetCurrentSheet()
    {
        return GetNthSheet(mSheetNum);
    }

    void SetShowMode(ShowMode const&);

    // implementation and helper functions
    [[nodiscard]] auto SerializeShowData() const -> std::vector<std::byte>;

    // members
    std::string mDescr;
    // labels and instruments go hand in hand, put them together to make sure we don't have extras or missing
    std::vector<std::pair<std::string, std::string>> mDotLabelAndInstrument;
    Sheet_container_t mSheets;
    ShowMode mMode;

    // the more "transient" settings, representing a current set of manipulations by the user
    SelectionList mSelectionList; // order of selections
    int mSheetNum{};
};

}
