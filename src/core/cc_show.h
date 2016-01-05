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

class CC_sheet;
class ShowMode;
class ShowUndoList;
class CC_lasso;
class CalChartDoc;
class CC_coord;

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
    static std::unique_ptr<CC_show> Create_CC_show(std::istream& stream);

private:
    CC_show();
    // using overloading with structs to determine which constructor to use
    CC_show(std::istream& stream, Version_3_3_and_earlier);
    CC_show(const uint8_t* ptr, size_t size, Current_version_and_later);

public:
    ~CC_show();

private:
    std::vector<uint8_t> SerializeShowData() const;

public:
    // How we save and load a show:
    std::vector<uint8_t> SerializeShow() const;

public:
    const std::string& GetDescr() const;
    void SetDescr(const std::string& newdescr);

    void SetupNewShow();
    size_t GetNumSheets() const;

    CC_sheet_iterator_t GetSheetBegin();
    const_CC_sheet_iterator_t GetSheetBegin() const;
    CC_sheet_iterator_t GetSheetEnd();
    const_CC_sheet_iterator_t GetSheetEnd() const;

    const_CC_sheet_iterator_t GetNthSheet(unsigned n) const;
    CC_sheet_iterator_t GetNthSheet(unsigned n);
    const_CC_sheet_iterator_t GetCurrentSheet() const;
    CC_sheet_iterator_t GetCurrentSheet();
    unsigned GetCurrentSheetNum() const;
    void SetCurrentSheet(unsigned n);

    CC_sheet_container_t RemoveNthSheet(unsigned sheetidx);
    void DeleteNthSheet(unsigned sheetidx);
    void InsertSheetInternal(const CC_sheet& nsheet, unsigned sheetidx);
    void InsertSheetInternal(const CC_sheet_container_t& nsheet,
        unsigned sheetidx);
    void InsertSheet(const CC_sheet& nsheet, unsigned sheetidx);
    inline unsigned short GetNumPoints() const { return numpoints; }
    void SetNumPoints(unsigned num, unsigned columns,
        const CC_coord& new_march_position);
    bool RelabelSheets(unsigned sht);

    std::string GetPointLabel(unsigned i) const;
    void SetPointLabel(const std::vector<std::string>& labels)
    {
        pt_labels = labels;
    }
    inline const std::vector<std::string>& GetPointLabels() const
    {
        return pt_labels;
    }

    bool AlreadyHasPrintContinuity() const;

    // how to select points:
    // Always select or unselect in groups
    bool SelectAll();
    bool UnselectAll();
    void AddToSelection(const SelectionList& sl);
    void SetSelection(const SelectionList& sl);
    void RemoveFromSelection(const SelectionList& sl);
    void ToggleSelection(const SelectionList& sl);
    void SelectWithLasso(const CC_lasso& lasso, bool toggleSelected,
        unsigned ref);
    inline bool IsSelected(unsigned i) const
    {
        return selectionList.count(i) != 0;
    }
    inline const SelectionList& GetSelectionList() const { return selectionList; }

    JSONElement generateOnlineViewerObject(const Animation& compiledShow);
    void sculptOnlineViewerObject(JSONElement& dest, const Animation& compiledShow);
private:
    std::string descr;
    unsigned short numpoints;
    CC_sheet_container_t sheets;
    std::vector<std::string> pt_labels;
    SelectionList selectionList; // order of selections
    unsigned mSheetNum;

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