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

#pragma once

#include "cc_types.h"
#include "cc_fileformat.h"
#include "json.h"

#include <vector>
#include <string>
#include <set>
#include <map>
#include <memory>
#include "animate.h"
#include "cc_sheet.h"

class CC_sheet;
class ShowMode;
class ShowUndoList;
class CC_lasso;
class CalChartDoc;
class CC_coord;

using CC_show_command = std::function<void(CC_show&)>;
using CC_show_command_pair = std::pair<CC_show_command, CC_show_command>;

// CalChart Show
// The CalChart show object is what most parts of the system interact with
// It is essentially a collection of CC_sheets that you iterate through to
// either look at, or change.
// A show can be loaded from a input stream

class CC_show {
public:
    typedef std::vector<CC_sheet> CC_sheet_container_t;
    typedef CC_sheet_container_t::iterator CC_sheet_iterator_t;
    typedef CC_sheet_container_t::const_iterator const_CC_sheet_iterator_t;

    // you can create a show in two ways, from nothing, or from an input stream
    static std::unique_ptr<CC_show> Create_CC_show();
    static std::unique_ptr<CC_show> Create_CC_show(unsigned num, unsigned columns, std::vector<std::string> const& labels, const CC_coord& new_march_position);
    static std::unique_ptr<CC_show> Create_CC_show(std::istream& stream);

private:
    CC_show();
    // using overloading with structs to determine which constructor to use
    CC_show(std::istream& stream, Version_3_3_and_earlier);
    CC_show(const uint8_t* ptr, size_t size, Current_version_and_later);

public:
    ~CC_show();

public:
    // How we save and load a show:
    std::vector<uint8_t> SerializeShow() const;

    // Create command, consists of an action and undo action
    CC_show_command_pair Create_SetDescriptionCommand(std::string const& descr) const;
    CC_show_command_pair Create_SetCurrentSheetCommand(int n) const;
    CC_show_command_pair Create_SetSelectionCommand(const SelectionList& sl) const;
    CC_show_command_pair Create_SetShowInfoCommand(int numPoints, int numColumns, const std::vector<std::string>& labels, const CC_coord& new_march_position) const;
    CC_show_command_pair Create_SetSheetTitleCommand(std::string const& newname) const;
    CC_show_command_pair Create_SetSheetBeatsCommand(int beats) const;
    CC_show_command_pair Create_AddSheetsCommand(const CC_show::CC_sheet_container_t& sheets, int where) const;
    CC_show_command_pair Create_RemoveSheetCommand(int where) const;
    CC_show_command_pair Create_ApplyRelabelMapping(int sheet_num_first, std::vector<size_t> const& mapping) const;
    CC_show_command_pair Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string> > const& data) const;
    CC_show_command_pair Create_MovePointsCommand(std::map<int, CC_coord> const& new_positions, int ref) const;
    CC_show_command_pair Create_RotatePointPositionsCommand(int rotateAmount, int ref) const;
    CC_show_command_pair Create_SetReferencePointToRef0(int ref) const;
    CC_show_command_pair Create_SetSymbolCommand(SYMBOL_TYPE sym) const;
    CC_show_command_pair Create_SetContinuityTextCommand(SYMBOL_TYPE which_sym, std::string const& text) const;
    CC_show_command_pair Create_SetLabelFlipCommand(std::map<int, bool> const& new_flip) const;
    CC_show_command_pair Create_SetLabelRightCommand(bool right) const;
    CC_show_command_pair Create_ToggleLabelFlipCommand() const;
    CC_show_command_pair Create_SetLabelVisiblityCommand(std::map<int, bool> const& new_visibility) const;
    CC_show_command_pair Create_SetLabelVisibleCommand(bool isVisible) const;
    CC_show_command_pair Create_ToggleLabelVisibilityCommand() const;
    CC_show_command_pair Create_AddNewBackgroundImageCommand(calchart_core::ImageData const& image) const;
    CC_show_command_pair Create_RemoveBackgroundImageCommand(int which) const;
    CC_show_command_pair Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height) const;

    // Accessors
    auto GetDescr() const { return descr; }

    auto GetSheetBegin() const { return sheets.begin(); }
    auto GetSheetEnd() const { return sheets.end(); }
    auto GetNthSheet(unsigned n) const { return GetSheetBegin() + n; }
    auto GetCurrentSheet() const { return GetNthSheet(mSheetNum); }
    int GetNumSheets() const;
    auto GetCurrentSheetNum() const { return mSheetNum; }
    auto GetNumPoints() const { return numpoints; }
    std::string GetPointLabel(int i) const;
    auto GetPointLabels() const { return pt_labels; }

    bool AlreadyHasPrintContinuity() const;

    bool WillMovePoints(std::map<int, CC_coord> const& new_positions, int ref) const;

    // utility
    std::pair<bool, std::vector<size_t> > GetRelabelMapping(const_CC_sheet_iterator_t source_sheet, const_CC_sheet_iterator_t target_sheets) const;
    SelectionList MakeSelectAll() const;
    SelectionList MakeUnselectAll() const;
    SelectionList MakeAddToSelection(const SelectionList& sl) const;
    SelectionList MakeRemoveFromSelection(const SelectionList& sl) const;
    SelectionList MakeToggleSelection(const SelectionList& sl) const;
    SelectionList MakeSelectWithLasso(const CC_lasso& lasso, int ref) const;

    // Point selection
    auto IsSelected(int i) const { return selectionList.count(i) != 0; }
    auto GetSelectionList() const { return selectionList; }

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

    // This is something we should try to get rid of.
    void SetMode(ShowMode const* mode) { mMode = mode; }

private:
    // modification of show is private, and externally done through create and exeucte commands
    CC_sheet_container_t RemoveNthSheet(int sheetidx);
    void InsertSheet(const CC_sheet& nsheet, int sheetidx);
    void InsertSheet(const CC_sheet_container_t& nsheet, int sheetidx);
    void SetCurrentSheet(int n);
    void SetSelection(const SelectionList& sl);

    void SetNumPoints(int num, int columns, const std::vector<std::string>& labels, const CC_coord& new_march_position);
    void SetPointLabel(const std::vector<std::string>& labels);

    void SetDescr(const std::string& newdescr);

    auto GetSheetBegin() { return sheets.begin(); }
    auto GetSheetEnd() { return sheets.end(); }
    auto GetNthSheet(unsigned n) { return GetSheetBegin() + n; }
    auto GetCurrentSheet() { return GetNthSheet(mSheetNum); }

    // implementation and helper functions
    std::vector<uint8_t> SerializeShowData() const;

    // members
    std::string descr;
    int numpoints;
    CC_sheet_container_t sheets;
    std::vector<std::string> pt_labels;
    SelectionList selectionList; // order of selections
    int mSheetNum;
    ShowMode const* mMode = nullptr;

    // unit tests
    friend void CC_show_UnitTests();
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

void CC_show_UnitTests();
