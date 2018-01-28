#pragma once
/*
 * cc_show.h
 * Definitions for the calchart show classes
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

#include "cc_fileformat.h"
#include "cc_types.h"
#include "json.h"

#include "animate.h"
#include "cc_sheet.h"
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace CalChart {
class Lasso;
class Show;
class Sheet;

using Show_command = std::function<void(Show&)>;
using Show_command_pair = std::pair<Show_command, Show_command>;

// CalChart Show
// The CalChart show object is what most parts of the system interact with
// It is essentially a collection of CC_sheets that you iterate through to
// either look at, or change.
// A show can be loaded from a input stream

class Show {
public:
    using Sheet_container_t = std::vector<Sheet>;
    using Sheet_iterator_t = Sheet_container_t::iterator;
    using const_Sheet_iterator_t = Sheet_container_t::const_iterator;

    // you can create a show in two ways, from nothing, or from an input stream
    static std::unique_ptr<Show> Create_CC_show();
    static std::unique_ptr<Show> Create_CC_show(std::vector<std::string> const& labels, unsigned columns, Coord const& new_march_position);
    static std::unique_ptr<Show> Create_CC_show(std::istream& stream);

private:
    Show();
    // using overloading with structs to determine which constructor to use
    Show(std::istream& stream, Version_3_3_and_earlier);
    Show(const uint8_t* ptr, size_t size, Current_version_and_later);

public:
    ~Show();

public:
    // How we save and load a show:
    std::vector<uint8_t> SerializeShow() const;

    // Create command, consists of an action and undo action
    Show_command_pair Create_SetCurrentSheetCommand(int n) const;
    Show_command_pair Create_SetSelectionCommand(const SelectionList& sl) const;
    Show_command_pair Create_SetShowInfoCommand(std::vector<std::string> const& labels, int numColumns, Coord const& new_march_position) const;
    Show_command_pair Create_SetSheetTitleCommand(std::string const& newname) const;
    Show_command_pair Create_SetSheetBeatsCommand(int beats) const;
    Show_command_pair Create_AddSheetsCommand(const Show::Sheet_container_t& sheets, int where) const;
    Show_command_pair Create_RemoveSheetCommand(int where) const;
    Show_command_pair Create_ApplyRelabelMapping(int sheet_num_first, std::vector<size_t> const& mapping) const;
    Show_command_pair Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string>> const& data) const;
    Show_command_pair Create_MovePointsCommand(std::map<int, Coord> const& new_positions, int ref) const;
    Show_command_pair Create_MovePointsCommand(int whichSheet, std::map<int, Coord> const& new_positions, int ref) const;
    Show_command_pair Create_DeletePointsCommand() const;
    Show_command_pair Create_RotatePointPositionsCommand(int rotateAmount, int ref) const;
    Show_command_pair Create_SetReferencePointToRef0(int ref) const;
    Show_command_pair Create_SetSymbolCommand(SYMBOL_TYPE sym) const;
    Show_command_pair Create_SetSymbolCommand(const SelectionList& whichDots, SYMBOL_TYPE sym) const;
    Show_command_pair Create_SetContinuityTextCommand(SYMBOL_TYPE which_sym, std::string const& text) const;
    Show_command_pair Create_SetLabelFlipCommand(std::map<int, bool> const& new_flip) const;
    Show_command_pair Create_SetLabelRightCommand(bool right) const;
    Show_command_pair Create_ToggleLabelFlipCommand() const;
    Show_command_pair Create_SetLabelVisiblityCommand(std::map<int, bool> const& new_visibility) const;
    Show_command_pair Create_SetLabelVisibleCommand(bool isVisible) const;
    Show_command_pair Create_ToggleLabelVisibilityCommand() const;
    Show_command_pair Create_AddNewBackgroundImageCommand(ImageData const& image) const;
    Show_command_pair Create_RemoveBackgroundImageCommand(int which) const;
    Show_command_pair Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height) const;

    // Accessors
    auto GetSheetBegin() const { return mSheets.begin(); }
    auto GetSheetEnd() const { return mSheets.end(); }
    auto GetNthSheet(unsigned n) const { return GetSheetBegin() + n; }
    auto GetCurrentSheet() const { return GetNthSheet(mSheetNum); }
    int GetNumSheets() const;
    auto GetCurrentSheetNum() const { return mSheetNum; }
    auto GetNumPoints() const { return static_cast<int>(mPtLabels.size()); }
    std::string GetPointLabel(int i) const;
    auto GetPointLabels() const { return mPtLabels; }

    bool AlreadyHasPrintContinuity() const;

    bool WillMovePoints(std::map<int, Coord> const& new_positions, int ref) const;

    // utility
    std::pair<bool, std::vector<size_t>> GetRelabelMapping(const_Sheet_iterator_t source_sheet, const_Sheet_iterator_t target_sheets, CalChart::Coord::units tolerance) const;
    SelectionList MakeSelectAll() const;
    SelectionList MakeUnselectAll() const;
    SelectionList MakeAddToSelection(const SelectionList& sl) const;
    SelectionList MakeRemoveFromSelection(const SelectionList& sl) const;
    SelectionList MakeToggleSelection(const SelectionList& sl) const;
    SelectionList MakeSelectWithLasso(const Lasso& lasso, int ref) const;

    // Point selection
    auto IsSelected(int i) const { return mSelectionList.count(i) != 0; }
    auto GetSelectionList() const { return mSelectionList; }

    /*!
     * @brief Generates a JSONElement that could represent this
     * show in an Online Viewer '.viewer' file.
     * @param compiledShow An up-to-date Animation of the show.
     * @return A JSONElement which could represent this show in
     * a '.viewer' file.
     */
    JSONElement toOnlineViewerJSON(const Animation& compiledShow) const;

    /*!
     * @brief Manipulates dest so that it contains a JSONElement that
     * could represent this show in an Online Viewer '.viewer' file.
     * @param dest A reference to the JSONElement which will be transformed
     * into a JSON representation of this show.
     * @param compiledShow An up-to-date Animation of the show.
     */
    void toOnlineViewerJSON(JSONElement& dest, const Animation& compiledShow) const;

private:
    // modification of show is private, and externally done through create and exeucte commands
    Sheet_container_t RemoveNthSheet(int sheetidx);
    void InsertSheet(const Sheet& nsheet, int sheetidx);
    void InsertSheet(const Sheet_container_t& nsheet, int sheetidx);
    void SetCurrentSheet(int n);
    void SetSelection(const SelectionList& sl);

    void SetNumPoints(std::vector<std::string> const& labels, int columns, Coord const& new_march_position);
    void DeletePoints(SelectionList const& sl);
    void SetPointLabel(const std::vector<std::string>& labels);

    // Descriptions aren't used, but keeping this alive.  See issue #203
    auto GetDescr() const { return mDescr; }
    void SetDescr(std::string const& newdescr) { mDescr = newdescr; }

    auto GetSheetBegin() { return mSheets.begin(); }
    auto GetSheetEnd() { return mSheets.end(); }
    auto GetNthSheet(unsigned n) { return GetSheetBegin() + n; }
    auto GetCurrentSheet() { return GetNthSheet(mSheetNum); }

    // implementation and helper functions
    std::vector<uint8_t> SerializeShowData() const;

    // members
    std::string mDescr;
    Sheet_container_t mSheets;
    std::vector<std::string> mPtLabels;
    SelectionList mSelectionList; // order of selections
    int mSheetNum;

    // unit tests
    friend void Show_UnitTests();
    static void CC_show_round_trip_test();
    static void CC_show_round_trip_test_with_number_label_description();
    static void CC_show_blank_desc_test();
    static void CC_show_future_show_test();
    static void CC_show_wrong_size_throws_exception();
    static void CC_show_wrong_size_number_labels_throws();
    static void CC_show_wrong_size_description();
    static void CC_show_extra_cruft_ok();
    static void CC_show_with_nothing_throws();
};

void Show_UnitTests();
}
