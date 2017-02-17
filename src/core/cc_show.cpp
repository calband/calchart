/*
 * cc_show.cpp
 * Member functions for calchart show classes
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

#include "cc_show.h"

#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "cc_shapes.h"
#include "cc_fileformat.h"
#include "ccvers.h"
#include "json.h"
#include "modes.h"

#include <sstream>
#include <functional>

static const std::string k_nofile_str = "Unable to open file";
static const std::string k_badcont_str = "Error in continuity file";
static const std::string k_contnohead_str = "Continuity file doesn't begin with header";

// you can create a show in two ways, from scratch, or from an input stream
std::unique_ptr<CC_show> CC_show::Create_CC_show()
{
    auto show = std::unique_ptr<CC_show>(new CC_show());
    show->InsertSheet(CC_sheet(show->GetNumPoints(), "1"), 0);
    show->SetCurrentSheet(0);
    return show;
}

std::unique_ptr<CC_show> CC_show::Create_CC_show(unsigned num, unsigned columns, std::vector<std::string> const& labels, const CC_coord& new_march_position)
{
    auto show = Create_CC_show();
    show->SetNumPoints(num, columns, labels, new_march_position);
    return show;
}

std::unique_ptr<CC_show> CC_show::Create_CC_show(std::istream& stream)
{
    ReadAndCheckID(stream, INGL_INGL);
    uint32_t version = ReadGurkSymbolAndGetVersion(stream, INGL_GURK);
    if (version <= 0x303) {
        return std::unique_ptr<CC_show>(
            new CC_show(stream, Version_3_3_and_earlier()));
    }

    // read the whole stream into a block, making sure we don't skip white space
    stream.unsetf(std::ios::skipws);
    std::vector<uint8_t> data(std::istream_iterator<uint8_t>{ stream },
        std::istream_iterator<uint8_t>{});
    // wxWidgets doesn't like it when we've reach the end of file.  Remove flags
    stream.clear();

    // debug purposes, you can uncomment this line to have the show dumped
    //	DoRecursiveParsing("", data.data(), data.data() + data.size());
    return std::unique_ptr<CC_show>(
        new CC_show(data.data(), data.size(), Current_version_and_later()));
}

// Create a new show
CC_show::CC_show()
    : numpoints(0)
    , mSheetNum(0)
{
}

// -=-=-=-=-=- LEGACY CODE -=-=-=-=-=-
// Recommend that you don't touch this unless you know what you are doing.
// Constructor for shows 3.3 and ealier.
CC_show::CC_show(std::istream& stream, Version_3_3_and_earlier ver)
    : numpoints(0)
    , mSheetNum(0)
{
    // caller should have stripped off INGL and GURK headers
    /*
  ReadAndCheckID(stream, INGL_INGL);
  uint32_t version = ReadGurkSymbolAndGetVersion(stream, INGL_GURK);
  */
    ReadAndCheckID(stream, INGL_SHOW);

    // Handle show info
    // read in the size:
    // <INGL_SIZE><4><# points>
    numpoints = ReadCheckIDandSize(stream, INGL_SIZE);
    pt_labels.assign(numpoints, std::string());

    uint32_t name = ReadLong(stream);
    // Optional: read in the point labels
    // <INGL_LABL><SIZE>
    if (INGL_LABL == name) {
        std::vector<uint8_t> data = FillData(stream);
        std::vector<std::string> labels;
        const char* str = (const char*)&data[0];
        for (auto i = 0; i < GetNumPoints(); i++) {
            labels.push_back(str);
            str += strlen(str) + 1;
        }
        SetPointLabel(labels);
        // peek for the next name
        name = ReadLong(stream);
    }
    else {
        // fail?
    }

    // Optional: read in the description
    // <INGL_DESC><SIZE>
    if (INGL_DESC == name) {
        std::vector<uint8_t> data = FillData(stream);
        auto str = (const char*)&data[0];
        SetDescr(std::string(str, strlen(str)));
        // peek for the next name
        name = ReadLong(stream);
    }

    // Read in sheets
    // <INGL_GURK><INGL_SHET>
    while (INGL_GURK == name) {
        ReadAndCheckID(stream, INGL_SHET);

        CC_sheet sheet(GetNumPoints(), stream, ver);
        InsertSheet(sheet, GetNumSheets());

        // ReadAndCheckID(stream, INGL_END);
        ReadAndCheckID(stream, INGL_SHET);
        // peek for the next name
        name = ReadLong(stream);
    }
    // ReadAndCheckID(stream, INGL_END);
    ReadAndCheckID(stream, INGL_SHOW);

    // now set the show to sheet 0
    mSheetNum = 0;
}
// -=-=-=-=-=- LEGACY CODE </end>-=-=-=-=-=-

CC_show::CC_show(const uint8_t* ptr, size_t size, Current_version_and_later ver)
    : numpoints(0)
    , mSheetNum(0)
{
    // caller should have stripped off INGL and GURK headers

    // construct the parser handlers
    // TODO: Why can't I capture this here?
    auto parse_INGL_SIZE = [](CC_show* show, const uint8_t* ptr, size_t size) {
        if (4 != size) {
            throw CC_FileException("Incorrect size", INGL_SIZE);
        }
        show->numpoints = get_big_long(ptr);
        show->pt_labels.assign(show->numpoints, std::string());
    };
    auto parse_INGL_LABL = [](CC_show* show, const uint8_t* ptr, size_t size) {
        std::vector<std::string> labels;
        if (!size && show->GetNumPoints()) {
            throw CC_FileException("Label the wrong size", INGL_LABL);
        }
        // restrict search to the size we're given
        auto str = (const char*)ptr;
        for (auto i = 0; i < show->GetNumPoints(); i++) {
            labels.push_back(str);
            auto length = strlen(str) + 1;
            if (length > size) {
                throw CC_FileException("Label too large", INGL_LABL);
            }
            str += length;
            size -= length;
        }
        if (size != 0) {
            throw CC_FileException("Label the wrong size", INGL_LABL);
        }
        show->SetPointLabel(labels);
    };
    auto parse_INGL_DESC = [](CC_show* show, const uint8_t* ptr, size_t size) {
        auto str = (const char*)ptr;
        if (size != (strlen(str) + 1)) {
            throw CC_FileException("Description the wrong size", INGL_DESC);
        }
        show->SetDescr(std::string(str, strlen(str)));
    };
    auto parse_INGL_SHET = [](CC_show* show, const uint8_t* ptr, size_t size) {
        CC_sheet sheet(show->GetNumPoints(), ptr, size,
            Current_version_and_later());
        show->InsertSheet(sheet, show->GetNumSheets());
    };
    // [=] needed here to pull in the parse functions
    auto parse_INGL_SHOW = [=](CC_show* show, const uint8_t* ptr, size_t size) {
        static const std::map<uint32_t, std::function<void(CC_show * show, const uint8_t*, size_t)> >
            parser = {
                { INGL_SIZE, parse_INGL_SIZE },
                { INGL_LABL, parse_INGL_LABL },
                { INGL_DESC, parse_INGL_DESC },
                { INGL_SHET, parse_INGL_SHET },
            };
        auto table = CalChart::Parser::ParseOutLabels(ptr, ptr + size);
        for (auto& i : table) {
            auto the_parser = parser.find(std::get<0>(i));
            if (the_parser != parser.end()) {
                the_parser->second(show, std::get<1>(i), std::get<2>(i));
            }
        }
    };

    auto table = CalChart::Parser::ParseOutLabels(ptr, ptr + size);
    bool found_show = false;
    for (auto& i : table) {
        if (std::get<0>(i) == INGL_SHOW) {
            parse_INGL_SHOW(this, std::get<1>(i), std::get<2>(i));
            found_show = true;
        }
    }
    if (!found_show) {
        throw CC_FileException("did not find show", INGL_SHOW);
    }

    // now set the show to sheet 0
    mSheetNum = 0;
}

// Destroy a show
CC_show::~CC_show() {}

std::vector<uint8_t> CC_show::SerializeShowData() const
{
    using CalChart::Parser::Append;
    using CalChart::Parser::AppendAndNullTerminate;
    using CalChart::Parser::Construct_block;
    std::vector<uint8_t> result;
    // SHOW_DATA          = NUM_MARCH , LABEL , [ DESCRIPTION ] , { SHEET }* ;
    // Write NUM_MARCH
    Append(result, Construct_block(INGL_SIZE, static_cast<uint32_t>( GetNumPoints() )));

    // Write LABEL
    std::vector<char> labels;
    for (auto& i : pt_labels) {
        AppendAndNullTerminate(labels, i);
    }
    Append(result, Construct_block(INGL_LABL, labels));

    // write Description
    if (!GetDescr().empty()) {
        std::vector<char> descr;
        AppendAndNullTerminate(descr, GetDescr());
        Append(result, Construct_block(INGL_DESC, descr));
    }

    // Handle sheets
    for (auto& sheet : sheets) {
        Append(result, sheet.SerializeSheet());
    }
    return result;
}

std::vector<uint8_t> CC_show::SerializeShow() const
{
    using CalChart::Parser::Append;
    using CalChart::Parser::Construct_block;
    std::vector<uint8_t> result;
    // show               = START , SHOW ;
    // START              = INGL_INGL , INGL_VERS ;
    // SHOW               = INGL_SHOW , BigEndianInt32(DataTill_SHOW_END) ,
    // SHOW_DATA , SHOW_END ;
    // SHOW_END           = INGL_END , INGL_SHOW ;
    Append(result, uint32_t{ INGL_INGL });
    Append(result, uint16_t{ INGL_GURK >> 16 });
    Append(result, uint8_t{ CC_MAJOR_VERSION + '0' });
    Append(result, uint8_t{ CC_MINOR_VERSION + '0' });
    Append(result, Construct_block(INGL_SHOW, SerializeShowData()));
    return result;
}

void CC_show::SetDescr(const std::string& newdescr) { descr = newdescr; }

int CC_show::GetNumSheets() const { return static_cast<int>(sheets.size()); }

CC_show::CC_sheet_container_t CC_show::RemoveNthSheet(int sheetidx)
{
    CC_sheet_iterator_t i = GetNthSheet(sheetidx);
    CC_sheet_container_t shts(1, *i);
    sheets.erase(i);

    if (sheetidx < GetCurrentSheetNum()) {
        SetCurrentSheet(GetCurrentSheetNum() - 1);
    }
    if (GetCurrentSheetNum() >= GetNumSheets()) {
        SetCurrentSheet(GetNumSheets() - 1);
    }

    return shts;
}

void CC_show::SetCurrentSheet(int n) { mSheetNum = n; }

void CC_show::InsertSheet(const CC_sheet& sheet, int sheetidx)
{
    sheets.insert(sheets.begin() + sheetidx, sheet);
    if (sheetidx <= GetCurrentSheetNum())
        SetCurrentSheet(GetCurrentSheetNum() + 1);
}

void CC_show::InsertSheet(const CC_sheet_container_t& sheet,
    int sheetidx)
{
    sheets.insert(sheets.begin() + sheetidx, sheet.begin(), sheet.end());
    if (sheetidx <= GetCurrentSheetNum())
        SetCurrentSheet(GetCurrentSheetNum() + 1);
}

// warning, the labels might not match up
void CC_show::SetNumPoints(int num, int columns, const std::vector<std::string>& labels, const CC_coord& new_march_position)
{
    if (num != static_cast<int>(labels.size())) {
        throw std::runtime_error("set num points with num=" + std::to_string(num) + " doesn't match number of labels=" + std::to_string(labels.size()));
    }
    for (CC_sheet_iterator_t sht = GetSheetBegin(); sht != GetSheetEnd(); ++sht) {
        auto pts = sht->NewNumPointsPositions(num, columns, new_march_position);
        sht->SetPoints(pts);
    }
    numpoints = num;
    SetPointLabel(labels);
}

void CC_show::SetPointLabel(const std::vector<std::string>& labels)
{
    pt_labels = labels;
}

// A relabel mapping is the mapping you would need to apply to sheet_next (and all following sheets)
// so that they match with this current sheet
std::pair<bool, std::vector<size_t> > CC_show::GetRelabelMapping(const_CC_sheet_iterator_t source_sheet, const_CC_sheet_iterator_t target_sheets) const
{
    std::vector<size_t> table(GetNumPoints());
    std::vector<unsigned> used_table(GetNumPoints());

    for (auto i = 0; i < GetNumPoints(); i++) {
        auto j = 0;
        for (; j < GetNumPoints(); j++) {
            if (!used_table[j]) {
                if (source_sheet->GetPosition(i) == target_sheets->GetPosition(j)) {
                    table[i] = j;
                    used_table[j] = true;
                    break;
                }
            }
        }
        if (j == GetNumPoints()) {
            // didn't find a match
            return { false, {} };
        }
    }

    return { true, table };
}

std::string CC_show::GetPointLabel(int i) const
{
    if (i >= static_cast<int>(pt_labels.size()))
        return "";
    return pt_labels.at(i);
}

bool CC_show::AlreadyHasPrintContinuity() const
{
    for (auto& i : sheets) {
        if (i.GetPrintableContinuity().size() > 0) {
            return true;
        }
    }
    return false;
}

bool CC_show::WillMovePoints(std::map<int, CC_coord> const& new_positions, int ref) const
{
    auto sheet = GetCurrentSheet();
    for (auto&& index : new_positions) {
        if (index.second != sheet->GetPosition(index.first, ref)) {
            return true;
        }
    }
    return false;
}

SelectionList CC_show::MakeSelectAll() const
{
    SelectionList sl;
    for (auto i = 0; i < numpoints; ++i)
        sl.insert(i);
    return sl;
}

SelectionList CC_show::MakeUnselectAll() const
{
    return {};
}

void CC_show::SetSelection(const SelectionList& sl)
{
    selectionList = sl;
}

SelectionList CC_show::MakeAddToSelection(const SelectionList& sl) const
{
    SelectionList slcopy = selectionList;
    slcopy.insert(sl.begin(), sl.end());
    return slcopy;
}

SelectionList CC_show::MakeRemoveFromSelection(const SelectionList& sl) const
{
    SelectionList slcopy = selectionList;
    slcopy.erase(sl.begin(), sl.end());
    return slcopy;
}

SelectionList CC_show::MakeToggleSelection(const SelectionList& sl) const
{
    SelectionList slcopy = selectionList;
    for (auto i : sl) {
        if (slcopy.count(i)) {
            slcopy.erase(i);
        }
        else {
            slcopy.insert(i);
        }
    }
    return slcopy;
}

// toggle selection means toggle it as selected to unselected
// otherwise, always select it
SelectionList CC_show::MakeSelectWithLasso(const CC_lasso& lasso, int ref) const
{
    if (!lasso.FirstPoint()) {
        return {};
    }

    SelectionList sl;
    CC_show::const_CC_sheet_iterator_t sheet = GetCurrentSheet();
    for (int i = 0; i < GetNumPoints(); i++) {
        if (lasso.Inside(sheet->GetPosition(i, ref))) {
            sl.insert(i);
        }
    }
    return sl;
}

JSONElement CC_show::toOnlineViewerJSON(const Animation& compiledShow) const
{
    JSONElement newViewerObject = JSONElement::makeNull();
    toOnlineViewerJSON(newViewerObject, compiledShow);
    return newViewerObject;
}

void CC_show::toOnlineViewerJSON(JSONElement& dest, const Animation& compiledShow) const
{
    JSONDataObjectAccessor showObjectAccessor = dest = JSONElement::makeObject();

    // Setup the skeleton for the show's JSON representation
    showObjectAccessor["title"] = "(MANUAL) the show title that you want people to see goes here"; // TODO; For now, this will be manually added to the exported file
    showObjectAccessor["year"] = "(MANUAL) enter show year (e.g. 2017)"; // TODO; Should eventually save automatically
    showObjectAccessor["description"] = descr;
    showObjectAccessor["labels"] = JSONElement::makeArray();
    showObjectAccessor["sheets"] = JSONElement::makeArray();

    // Fill in 'dot_labels' with the labels for each dot (e.g. A0, A1, A2, ...)
    JSONDataArrayAccessor dotLabelsAccessor = showObjectAccessor["labels"];
    for (unsigned i = 0; i < pt_labels.size(); i++) {
        dotLabelsAccessor->pushBack(pt_labels[i]);
    }

    // Fill in 'sheets' with the JSON representation of each sheet
    JSONDataArrayAccessor sheetsAccessor = showObjectAccessor["sheets"];
    unsigned i = 0;
    auto animateSheetIter = compiledShow.sheetsBegin();
    for (auto iter = GetSheetBegin(); iter != GetSheetEnd(); iter++, i++, animateSheetIter++) {
        sheetsAccessor->pushBack(JSONElement::makeNull());
        (*iter).toOnlineViewerJSON(sheetsAccessor[i], i + 1, pt_labels, *animateSheetIter);
    }
}

CC_show_command_pair CC_show::Create_SetDescriptionCommand(std::string const& descr) const
{
    auto action = [mDescr = descr](CC_show & show) { show.SetDescr(mDescr); };
    auto reaction = [mDescr = GetDescr()](CC_show & show) { show.SetDescr(mDescr); };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetCurrentSheetCommand(int n) const
{
    auto action = [n = n](CC_show & show) { show.SetCurrentSheet(n); };
    auto reaction = [n = mSheetNum](CC_show & show) { show.SetCurrentSheet(n); };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetSelectionCommand(const SelectionList& sl) const
{
    auto action = [sl](CC_show& show) { show.SetSelection(sl); };
    auto reaction = [sl = selectionList](CC_show & show) { show.SetSelection(sl); };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetShowInfoCommand(int numPoints, int numColumns, const std::vector<std::string>& labels, const CC_coord& new_march_position) const
{
    auto action = [numPoints, numColumns, labels, new_march_position](CC_show& show) { show.SetNumPoints(numPoints, numColumns, labels, new_march_position); };
    // need to go through and save all the positions and labels for later
    auto old_numpoints = numpoints;
    auto old_labels = pt_labels;
    std::vector<std::vector<CC_point> > old_points;
    for (auto&& sheet : sheets) {
        old_points.emplace_back(sheet.GetPoints());
    }
    auto reaction = [old_numpoints, old_labels, old_points](CC_show& show) {
        for (auto i = 0ul; i < show.sheets.size(); ++i) {
            show.sheets.at(i).SetPoints(old_points.at(i));
        }
        show.numpoints = old_numpoints;
        show.SetPointLabel(old_labels);
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetSheetTitleCommand(std::string const& newname) const
{
    auto action = [ whichSheet = mSheetNum, newname ](CC_show & show) { show.sheets.at(whichSheet).SetName(newname); };
    auto reaction = [ whichSheet = mSheetNum, newname = sheets.at(mSheetNum).GetName() ](CC_show & show) { show.sheets.at(whichSheet).SetName(newname); };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetSheetBeatsCommand(int beats) const
{
    auto action = [ whichSheet = mSheetNum, beats ](CC_show & show) { show.sheets.at(whichSheet).SetBeats(beats); };
    auto reaction = [ whichSheet = mSheetNum, beats = sheets.at(mSheetNum).GetBeats() ](CC_show & show) { show.sheets.at(whichSheet).SetBeats(beats); };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_AddSheetsCommand(const CC_show::CC_sheet_container_t& sheets, int where) const
{
    auto action = [sheets, where](CC_show& show) { show.InsertSheet(sheets, where); };
    auto reaction = [sheets, where](CC_show& show) { auto num_times = sheets.size(); while (num_times--) show.RemoveNthSheet(where); };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_RemoveSheetCommand(int where) const
{
    CC_sheet_container_t old_shts(1, *GetNthSheet(where));
    auto action = [where](CC_show& show) { show.RemoveNthSheet(where); };
    auto reaction = [old_shts, where](CC_show& show) { show.InsertSheet(old_shts, where); };
    return CC_show_command_pair{ action, reaction };
}

// remapping gets applied on this sheet till the last one
CC_show_command_pair CC_show::Create_ApplyRelabelMapping(int sheet_num_first, std::vector<size_t> const& mapping) const
{
    std::vector<std::vector<CC_point> > current_pos;
    for (auto s = GetNthSheet(sheet_num_first); s != GetSheetEnd(); ++s) {
        current_pos.emplace_back(s->GetPoints());
    }
    // first gather where all the points are;
    auto action = [sheet_num_first, mapping](CC_show& show) {
        auto s = show.GetNthSheet(sheet_num_first);
        while (s != show.GetSheetEnd()) {
            s->SetPoints(s->RemapPoints(mapping));
            ++s;
        }
    };
    auto reaction = [sheet_num_first, current_pos](CC_show& show) {
        auto i = 0;
        auto s = show.GetNthSheet(sheet_num_first);
        while (s != show.GetSheetEnd()) {
            s->SetPoints(current_pos.at(i++));
            ++s;
        }
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string> > const& data) const
{
    std::map<unsigned, std::pair<std::string, std::string> > undo_data;
    for (auto&& i : data) {
        undo_data[i.first] = { GetNthSheet(i.first)->GetNumber(), GetNthSheet(i.first)->GetRawPrintContinuity() };
    }
    auto action = [data](CC_show& show) {
        for (auto&& i : data) {
            show.GetNthSheet(i.first)->SetPrintableContinuity(i.second.first, i.second.second);
        }
    };
    auto reaction = [undo_data](CC_show& show) {
        for (auto&& i : undo_data) {
            show.GetNthSheet(i.first)->SetPrintableContinuity(i.second.first, i.second.second);
        }
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_MovePointsCommand(std::map<int, CC_coord> const& new_positions, int ref) const
{
    auto sheet = GetCurrentSheet();
    std::map<unsigned, CC_coord> original_positions;
    for (auto&& index : new_positions) {
        original_positions[index.first] = sheet->GetPosition(index.first, ref);
    }
    auto action = [ sheet_num = mSheetNum, new_positions, ref ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : new_positions) {
            auto position = (show.mMode ? show.mMode->ClipPosition(i.second) : i.second);
            sheet->SetPosition(position, i.first, ref);
        }
    };
    auto reaction = [ sheet_num = mSheetNum, original_positions, ref ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : original_positions) {
            auto position = (show.mMode ? show.mMode->ClipPosition(i.second) : i.second);
            sheet->SetPosition(position, i.first, ref);
        }
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_RotatePointPositionsCommand(int rotateAmount, int ref) const
{
    // construct a vector of point indices in order
    std::vector<unsigned> pointIndices;
    std::copy(selectionList.begin(), selectionList.end(), std::back_inserter(pointIndices));

    // construct a vector of point positions, rotated by rotate amount
    std::vector<CC_coord> finalPositions;
    auto sheet = GetCurrentSheet();
    std::transform(selectionList.begin(), selectionList.end(),
        std::back_inserter(finalPositions),
        [=](unsigned i) { return sheet->GetPosition(i, ref); });
    rotateAmount %= selectionList.size();
    std::rotate(finalPositions.begin(), finalPositions.begin() + rotateAmount,
        finalPositions.end());

    // put things into place.
    std::map<int, CC_coord> positions;
    for (int index = static_cast<int>(pointIndices.size()) - 1; index >= 0; index--) {
        positions[pointIndices[index]] = finalPositions[index];
    }
    return Create_MovePointsCommand(positions, ref);
}

CC_show_command_pair CC_show::Create_SetReferencePointToRef0(int ref) const
{
    std::map<int, CC_coord> positions;
    auto sheet = GetCurrentSheet();
    // for selected points, set the reference point to ref0
    // this should be per selection list defect #179
    //for (auto&& i : selectionList) {
    for (auto i = 0; i != GetNumPoints(); ++i) {
        positions[i] = sheet->GetPosition(i, 0);
    }
    return Create_MovePointsCommand(positions, ref);
}

CC_show_command_pair CC_show::Create_SetSymbolCommand(SYMBOL_TYPE sym) const
{
    std::map<unsigned, SYMBOL_TYPE> new_sym;
    std::map<unsigned, SYMBOL_TYPE> original_sym;
    auto sheet = GetCurrentSheet();
    for (auto&& i : selectionList) {
        // Only do work on points that have different symbols
        if (sym != sheet->GetPoint(i).GetSymbol()) {
            new_sym[i] = sym;
            original_sym[i] = sheet->GetPoint(i).GetSymbol();
        }
    }
    auto action = [ sheet_num = mSheetNum, new_sym ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : new_sym) {
            sheet->GetPoint(i.first).SetSymbol(i.second);
        }
    };
    auto reaction = [ sheet_num = mSheetNum, original_sym ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : original_sym) {
            sheet->GetPoint(i.first).SetSymbol(i.second);
        }
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetContinuityTextCommand(SYMBOL_TYPE which_sym, std::string const& text) const
{
    std::string original_cont = GetCurrentSheet()->GetContinuityBySymbol(which_sym).GetText();
    auto action = [ sheet_num = mSheetNum, which_sym, text ](CC_show & show)
    {
        show.GetNthSheet(sheet_num)->SetContinuityText(which_sym, text);
    };
    auto reaction = [ sheet_num = mSheetNum, which_sym, original_cont ](CC_show & show)
    {
        show.GetNthSheet(sheet_num)->SetContinuityText(which_sym, original_cont);
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetLabelFlipCommand(std::map<int, bool> const& new_flip) const
{
    auto sheet = GetCurrentSheet();
    std::map<unsigned, bool> original_flip;
    for (auto&& index : new_flip) {
        original_flip[index.first] = sheet->GetPoint(index.first).GetFlip();
    }
    auto action = [ sheet_num = mSheetNum, new_flip ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : new_flip) {
            sheet->GetPoint(i.first).Flip(i.second);
        }
    };
    auto reaction = [ sheet_num = mSheetNum, original_flip ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : original_flip) {
            sheet->GetPoint(i.first).Flip(i.second);
        }
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetLabelRightCommand(bool right) const
{
    std::map<int, bool> flips;
    for (auto&& i : selectionList) {
        flips[i] = right;
    }
    return Create_SetLabelFlipCommand(flips);
}

CC_show_command_pair CC_show::Create_ToggleLabelFlipCommand() const
{
    std::map<int, bool> flips;
    auto sheet = GetCurrentSheet();
    for (auto&& i : selectionList) {
        flips[i] = !sheet->GetPoint(i).GetFlip();
    }
    return Create_SetLabelFlipCommand(flips);
}

CC_show_command_pair CC_show::Create_SetLabelVisiblityCommand(std::map<int, bool> const& new_visibility) const
{
    auto sheet = GetCurrentSheet();
    std::map<int, bool> original_visibility;
    for (auto&& index : new_visibility) {
        original_visibility[index.first] = sheet->GetPoint(index.first).LabelIsVisible();
    }
    auto action = [ sheet_num = mSheetNum, new_visibility ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : new_visibility) {
            sheet->GetPoint(i.first).SetLabelVisibility(i.second);
        }
    };
    auto reaction = [ sheet_num = mSheetNum, original_visibility ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : original_visibility) {
            sheet->GetPoint(i.first).SetLabelVisibility(i.second);
        }
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_SetLabelVisibleCommand(bool isVisible) const
{
    std::map<int, bool> visible;
    for (auto&& i : selectionList) {
        visible[i] = isVisible;
    }
    return Create_SetLabelVisiblityCommand(visible);
}

CC_show_command_pair CC_show::Create_ToggleLabelVisibilityCommand() const
{
    std::map<int, bool> visible;
    auto sheet = GetCurrentSheet();
    for (auto&& i : selectionList) {
        visible[i] = !sheet->GetPoint(i).LabelIsVisible();
    }
    return Create_SetLabelVisiblityCommand(visible);
}

CC_show_command_pair CC_show::Create_AddNewBackgroundImageCommand(calchart_core::ImageData const& image) const
{
    auto sheet = GetCurrentSheet();
    auto action = [ sheet_num = mSheetNum, image, where = sheet->GetBackgroundImages().size() ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->AddBackgroundImage(image, where);
    };
    auto reaction = [ sheet_num = mSheetNum, where = sheet->GetBackgroundImages().size() ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->RemoveBackgroundImage(where);
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_RemoveBackgroundImageCommand(int which) const
{
    auto sheet = GetCurrentSheet();
    if (static_cast<size_t>(which) >= sheet->GetBackgroundImages().size()) {
        return { [](CC_show&) {}, [](CC_show&) {} };
    }
    auto action = [ sheet_num = mSheetNum, which ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->RemoveBackgroundImage(which);
    };
    auto reaction = [ sheet_num = mSheetNum, image = sheet->GetBackgroundImages().at(which), which ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->AddBackgroundImage(image, which);
    };
    return CC_show_command_pair{ action, reaction };
}

CC_show_command_pair CC_show::Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height) const
{
    auto sheet = GetCurrentSheet();
    auto current_left = sheet->GetBackgroundImages().at(which).left;
    auto current_top = sheet->GetBackgroundImages().at(which).top;
    auto current_scaled_width = sheet->GetBackgroundImages().at(which).scaled_width;
    auto current_scaled_height = sheet->GetBackgroundImages().at(which).scaled_height;
    auto action = [ sheet_num = mSheetNum, which, left, top, scaled_width, scaled_height ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->MoveBackgroundImage(which, left, top, scaled_width, scaled_height);
    };
    auto reaction = [ sheet_num = mSheetNum, which, current_left, current_top, current_scaled_width, current_scaled_height ](CC_show & show)
    {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->MoveBackgroundImage(which, current_left, current_top, current_scaled_width, current_scaled_height);
    };
    return CC_show_command_pair{ action, reaction };
}

// -=-=-=-=-=-=- Unit Tests -=-=-=-=-=-=-=-
#include <assert.h>
using namespace CalChart::Parser;

static std::vector<char>
Construct_show_zero_points_zero_labels_zero_description()
{
    std::vector<char> show_data;
    Append(show_data, Construct_block(INGL_SIZE, std::vector<char>(4)));
    Append(show_data, Construct_block(INGL_LABL, std::vector<char>{}));
    Append(show_data, Construct_block(INGL_DESC, std::vector<char>(1)));
    return Construct_block(INGL_SHOW, show_data);
}

static std::vector<char> Construct_show_zero_points_zero_labels()
{
    std::vector<char> show_data;
    Append(show_data, Construct_block(INGL_SIZE, std::vector<char>(4)));
    Append(show_data, Construct_block(INGL_LABL, std::vector<char>{}));
    return Construct_block(INGL_SHOW, show_data);
}

static std::vector<char> Construct_show_zero_points_zero_labels_1_sheet_and_random()
{
    std::vector<char> show_data;
    Append(show_data, Construct_block(0x12345678, std::vector<char>(4)));
    Append(show_data, Construct_block(INGL_SIZE, std::vector<char>(4)));
    Append(show_data, Construct_block(0x87654321, std::vector<char>(13)));
    Append(show_data, Construct_block(INGL_LABL, std::vector<char>{}));
    Append(show_data, Construct_block(0xDEADBEEF, std::vector<char>(1)));

    std::vector<char> sheet_data;
    Append(sheet_data, Construct_block(INGL_NAME, std::vector<char>{ '1', '\0' }));
    Append(sheet_data, Construct_block(INGL_DURA, std::vector<char>{ 0, 0, 0, 1 }));
    Append(sheet_data, Construct_block(INGL_PNTS, std::vector<char>{}));
    Append(sheet_data, Construct_block(INGL_CONT, std::vector<char>{}));
    Append(sheet_data, Construct_block(INGL_PCNT, std::vector<char>{ '\0', '\0' }));
    Append(show_data, Construct_block(INGL_SHET, sheet_data));

    return Construct_block(INGL_SHOW, show_data);
}

void CC_show::CC_show_round_trip_test()
{
    auto blank_show = CC_show::Create_CC_show();
    auto blank_show_data = blank_show->SerializeShow();
    std::vector<char> char_data{ blank_show_data.begin(), blank_show_data.end() };
    std::istringstream is(std::string{ char_data.data(), char_data.size() });
    auto re_read_show = CC_show::Create_CC_show(is);
    auto re_read_show_data = re_read_show->SerializeShow();
    bool is_equal = blank_show_data.size() == re_read_show_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(),
                                                                              re_read_show_data.begin());
    assert(is_equal);
}

void CC_show::CC_show_round_trip_test_with_number_label_description()
{
    std::vector<char> point_data;
    Append(point_data, uint32_t{ 1 });
    std::vector<char> data;
    Append(data, Construct_block(INGL_SIZE, point_data));
    Append(data, Construct_block(INGL_LABL, std::vector<char>{ 'p', 'o', 'i', 'n',
                                                't', '\0' }));
    Append(data, Construct_block(INGL_DESC, std::vector<char>{
                                                'd', 'e', 's', 'c', 'r', 'i', 'p',
                                                't', 'i', 'o', 'n', '\0' }));
    auto show_data = Construct_block(INGL_SHOW, data);

    CC_show show1((const uint8_t*)show_data.data(), show_data.size(),
        Current_version_and_later());
    auto show1_data = show1.SerializeShow();
    // eat header
    show1_data.erase(show1_data.begin(), show1_data.begin() + 8);
    auto is_equal = show1_data.size() == show_data.size() && std::equal(show1_data.begin(), show1_data.end(), show_data.begin());
    assert(is_equal);

    // now check that things loaded correctly
    assert(show1.GetNumPoints() == 1);
    assert(show1.GetNumSheets() == 0);
    assert(show1.GetPointLabel(0) == "point");
    assert(show1.GetDescr() == "description");
}

void CC_show::CC_show_blank_desc_test()
{
    auto show_zero_points_zero_labels_zero_description = Construct_show_zero_points_zero_labels_zero_description();
    CC_show show1(
        (const uint8_t*)show_zero_points_zero_labels_zero_description.data(),
        show_zero_points_zero_labels_zero_description.size(),
        Current_version_and_later());
    auto show1_data = show1.SerializeShow();
    // eat header
    show1_data.erase(show1_data.begin(), show1_data.begin() + 8);
    bool is_equal = show1_data.size() == show_zero_points_zero_labels_zero_description.size() && std::equal(show1_data.begin(), show1_data.end(),
                                                                                                     show_zero_points_zero_labels_zero_description.begin());
    assert(!is_equal);

    // now remove the description and they should be equal
    auto show_zero_points_zero_labels = Construct_show_zero_points_zero_labels();
    CC_show show2((const uint8_t*)show_zero_points_zero_labels.data(),
        show_zero_points_zero_labels.size(),
        Current_version_and_later());
    auto show2_data = show2.SerializeShow();
    show2_data.erase(show2_data.begin(), show2_data.begin() + 8);
    is_equal = show2_data.size() == show_zero_points_zero_labels.size() && std::equal(show2_data.begin(), show2_data.end(),
                                                                               show_zero_points_zero_labels.begin());
    assert(is_equal);
}

// confirm we try to handle shows from the future
void CC_show::CC_show_future_show_test()
{
    // how?  By creating a show from scratch, then modifying the version; make
    // sure that we load it, and it looks the same
    // except the data gets reverted
    auto blank_show = CC_show::Create_CC_show();
    auto blank_show_data = blank_show->SerializeShow();
    std::vector<char> char_data{ blank_show_data.begin(), blank_show_data.end() };
    assert(char_data.at(6) - '0' == CC_MAJOR_VERSION && char_data.at(7) - '0' == CC_MINOR_VERSION);
    ++char_data.at(6);
    ++char_data.at(7);
    std::istringstream is(std::string{ char_data.data(), char_data.size() });
    auto re_read_show = CC_show::Create_CC_show(is);
    auto re_read_show_data = blank_show->SerializeShow();
    --char_data.at(6);
    --char_data.at(7);
    bool is_equal = blank_show_data.size() == re_read_show_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(),
                                                                              re_read_show_data.begin());
    assert(is_equal);
}

void CC_show::CC_show_wrong_size_throws_exception()
{
    auto points_3(Construct_block(INGL_SIZE, std::vector<char>(3)));
    auto show_data = Construct_block(INGL_SHOW, points_3);
    bool hit_exception = false;
    try {
        CC_show show1((const uint8_t*)show_data.data(), show_data.size(),
            Current_version_and_later());
    }
    catch (const CC_FileException&) {
        hit_exception = true;
    }
    assert(hit_exception);
}

// too large, and too small
void CC_show::CC_show_wrong_size_number_labels_throws()
{
    {
        std::vector<char> point_data(4);
        put_big_long(point_data.data(), 1);
        auto points(Construct_block(INGL_SIZE, point_data));
        auto no_labels(Construct_block(INGL_LABL, std::vector<char>{}));
        auto t_show_data = points;
        t_show_data.insert(t_show_data.end(), no_labels.begin(), no_labels.end());
        auto show_data = Construct_block(INGL_SHOW, t_show_data);
        bool hit_exception = false;
        try {
            CC_show show1((const uint8_t*)show_data.data(), show_data.size(),
                Current_version_and_later());
        }
        catch (const CC_FileException&) {
            hit_exception = true;
        }
        assert(hit_exception);
    }
    {
        std::vector<char> point_data(4);
        put_big_long(point_data.data(), 1);
        auto points(Construct_block(INGL_SIZE, point_data));
        auto labels(
            Construct_block(INGL_LABL, std::vector<char>{ 'a', '\0', 'b', '\0' }));
        auto t_show_data = points;
        t_show_data.insert(t_show_data.end(), labels.begin(), labels.end());
        auto show_data = Construct_block(INGL_SHOW, t_show_data);
        bool hit_exception = false;
        try {
            CC_show show1((const uint8_t*)show_data.data(), show_data.size(),
                Current_version_and_later());
        }
        catch (const CC_FileException&) {
            hit_exception = true;
        }
        assert(hit_exception);
    }
}

// too large, and too small
void CC_show::CC_show_wrong_size_description()
{
    {
        auto no_points(Construct_block(INGL_SIZE, std::vector<char>(4)));
        auto no_labels(Construct_block(INGL_LABL, std::vector<char>{}));
        auto descr(
            Construct_block(INGL_DESC, std::vector<char>{ 'a', 'b', 'c', '\0' }));
        descr.at(9) = '\0';
        auto t_show_data = no_points;
        t_show_data.insert(t_show_data.end(), no_labels.begin(), no_labels.end());
        t_show_data.insert(t_show_data.end(), descr.begin(), descr.end());
        auto show_data = Construct_block(INGL_SHOW, t_show_data);
        bool hit_exception = false;
        try {
            CC_show show1((const uint8_t*)show_data.data(), show_data.size(),
                Current_version_and_later());
        }
        catch (const CC_FileException&) {
            hit_exception = true;
        }
        assert(hit_exception);
    }
}

// extra cruft ok
void CC_show::CC_show_extra_cruft_ok()
{
    // now remove the description and they should be equal
    auto extra_cruft = Construct_show_zero_points_zero_labels_1_sheet_and_random();
    CC_show show1((const uint8_t*)extra_cruft.data(), extra_cruft.size(),
        Current_version_and_later());
    auto show1_data = show1.SerializeShow();

    auto blank_show = CC_show::Create_CC_show();
    auto blank_show_data = blank_show->SerializeShow();
    auto is_equal = blank_show_data.size() == show1_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(),
                                                                       show1_data.begin());
    assert(is_equal);
}

// show with nothing should fail:
void CC_show::CC_show_with_nothing_throws()
{
    std::vector<char> empty{};
    bool hit_exception = false;
    try {
        CC_show show1((const uint8_t*)empty.data(), empty.size(),
            Current_version_and_later());
    }
    catch (const CC_FileException&) {
        hit_exception = true;
    }
    assert(hit_exception);
}

void CC_show_UnitTests()
{
    CC_show::CC_show_round_trip_test();
    CC_show::CC_show_round_trip_test_with_number_label_description();
    CC_show::CC_show_blank_desc_test();
    CC_show::CC_show_future_show_test();
    CC_show::CC_show_wrong_size_throws_exception();
    CC_show::CC_show_wrong_size_number_labels_throws();
    CC_show::CC_show_wrong_size_description();
    CC_show::CC_show_extra_cruft_ok();
    CC_show::CC_show_with_nothing_throws();
}
