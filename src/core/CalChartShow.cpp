/*
 * CalChartShow.cpp
 * Member functions for calchart show classes
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

#include "CalChartShow.h"
#include "CalChartConfiguration.h"
#include "CalChartConstants.h"
#include "CalChartContinuity.h"
#include "CalChartFileFormat.h"
#include "CalChartPoint.h"
#include "CalChartRanges.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "ccvers.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>

namespace CalChart {
static constexpr auto kDefault = "default";
static std::string const k_badcont_str = "Error in continuity file";
static std::string const k_contnohead_str = "Continuity file doesn't begin with header";

// you can create a show in two ways, from scratch, or from an input stream
std::unique_ptr<Show> Show::Create(ShowMode const& mode)
{
    auto show = std::unique_ptr<Show>(new Show(mode));
    show->InsertSheet(Sheet(show->GetNumPoints(), "1"), 0);
    show->SetCurrentSheet(0);
    return show;
}

std::unique_ptr<Show> Show::Create(ShowMode const& mode, std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, unsigned columns)
{
    auto show = Create(mode);
    show->SetNumPoints(labelsAndInstruments, columns, mode.FieldOffset());
    return show;
}

std::unique_ptr<Show> Show::Create(ShowMode const& mode, std::istream& stream, ParseErrorHandlers const* correction)
{
    // read the whole stream into a block, making sure we don't skip white space
    stream.unsetf(std::ios::skipws);
    auto data = std::vector<std::byte>{};
    std::transform(std::istream_iterator<char>{ stream }, std::istream_iterator<char>{}, std::back_inserter(data), [](auto b) { return static_cast<std::byte>(b); });
    // wxWidgets doesn't like it when we've reach the end of file.  Remove flags
    stream.clear();
    auto reader = Reader({ data.data(), data.size() });

    reader.ReadAndCheckID(INGL_INGL);
    auto version = reader.ReadGurkSymbolAndGetVersion(INGL_GURK);
    auto [majorVersion, minorVersion] = Reader::parseVersion(version);

    if (currentVersionCompare(majorVersion, minorVersion) < 0
        && correction
        && correction->mVersionMismatchHandler
        && !correction->mVersionMismatchHandler(majorVersion, minorVersion)) {
        throw std::runtime_error("Not able to parse older shows");
    }
    if (version <= 0x303) {
        return std::unique_ptr<Show>(new Show(Version_3_3_and_earlier{}, mode, reader, correction));
    }

    // debug purposes, you can uncomment this line to have the show dumped
    //	DoRecursiveParsing("", data.data(), data.data() + data.size());
    return std::unique_ptr<Show>(new Show(mode, reader, correction));
}

// Create a new show
Show::Show(ShowMode const& mode)
    : mMode(mode)
{
}

// -=-=-=-=-=- LEGACY CODE -=-=-=-=-=-
// Recommend that you don't touch this unless you know what you are doing.
// Constructor for shows 3.3 and ealier.
Show::Show(Version_3_3_and_earlier, ShowMode const& mode, Reader reader, ParseErrorHandlers const* correction)
    : Show(mode)
{
    // caller should have stripped off INGL and GURK headers
    reader.ReadAndCheckID(INGL_SHOW);

    // Handle show info
    // read in the size:
    // <INGL_SIZE><4><# points>
    auto numpoints = reader.ReadCheckIDandSize(INGL_SIZE);
    mDotLabelAndInstrument.assign(numpoints, std::pair<std::string, std::string>{ "", kDefault });

    uint32_t name = reader.Get<uint32_t>();
    // Optional: read in the point labels
    // <INGL_LABL><SIZE>
    if (INGL_LABL == name) {
        std::vector<uint8_t> data = reader.GetVector<uint8_t>();
        std::vector<std::string> labels;
        auto str = (char const*)&data[0];
        for (auto i = 0; i < GetNumPoints(); i++) {
            labels.push_back(str);
            str += strlen(str) + 1;
        }
        std::vector<std::pair<std::string, std::string>> labelsAndInstruments;
        std::transform(labels.begin(), labels.end(), std::back_inserter(labelsAndInstruments), [](auto&& i) { return std::pair<std::string, std::string>{ i, kDefault }; });
        SetPointLabelAndInstrument(labelsAndInstruments);
        // peek for the next name
        name = reader.Get<uint32_t>();
    } else {
        // fail?
    }

    // Optional: read in the description
    // <INGL_DESC><SIZE>
    if (INGL_DESC == name) {
        std::vector<uint8_t> data = reader.GetVector<uint8_t>();
        auto str = (char const*)&data[0];
        SetDescr(std::string(str, strlen(str)));
        // peek for the next name
        name = reader.Get<uint32_t>();
    }

    // Read in sheets
    // <INGL_GURK><INGL_SHET>
    while (INGL_GURK == name) {
        reader.ReadAndCheckID(INGL_SHET);

        Sheet sheet(Version_3_3_and_earlier{}, GetNumPoints(), reader, correction);
        InsertSheet(sheet, GetNumSheets());

        // ReadAndCheckID(stream, INGL_END);
        reader.ReadAndCheckID(INGL_SHET);
        // peek for the next name
        name = reader.Get<uint32_t>();
    }
    // ReadAndCheckID(stream, INGL_END);
    reader.ReadAndCheckID(INGL_SHOW);

    // now set the show to sheet 0
    mSheetNum = 0;
}
// -=-=-=-=-=- LEGACY CODE </end>-=-=-=-=-=-

Show::Show(ShowMode const& mode, Reader reader, ParseErrorHandlers const* correction)
    : Show(mode)
{
    // caller should have stripped off INGL and GURK headers

    // construct the parser handlers
    // TODO: Why can't I capture this here?
    auto parse_INGL_SIZE = [](Show& show, Reader reader) {
        if (reader.size() != 4) {
            throw CC_FileException("Incorrect size", INGL_SIZE);
        }
        auto numpoints = reader.Get<uint32_t>();
        show.mDotLabelAndInstrument.assign(numpoints, std::pair<std::string, std::string>{ "", kDefault });
    };
    auto parse_INGL_LABL = [](Show& show, Reader reader) {
        std::vector<std::pair<std::string, std::string>> labels;
        if (reader.size() == 0 && show.GetNumPoints()) {
            throw CC_FileException("Label the wrong size", INGL_LABL);
        }
        // restrict search to the size we're given
        for (auto i = 0; i < show.GetNumPoints(); i++) {
            auto str = reader.Get<std::string>();
            labels.push_back({ str, kDefault });
        }
        if (reader.size() != 0) {
            throw CC_FileException("Label the wrong size", INGL_LABL);
        }
        show.SetPointLabelAndInstrument(labels);
    };
    auto parse_INGL_INST = [](Show& show, Reader reader) {
        auto currentLabels = show.mDotLabelAndInstrument;
        if (reader.size() == 0 && show.GetNumPoints()) {
            throw CC_FileException("Label the wrong size", INGL_LABL);
        }
        // restrict search to the size we're given
        for (auto i = 0; i < show.GetNumPoints(); i++) {
            auto thisInst = reader.Get<std::string>();
            currentLabels.at(i).second = thisInst == "" ? kDefault : thisInst;
        }
        if (reader.size() != 0) {
            throw CC_FileException("Label the wrong size", INGL_LABL);
        }
        show.SetPointLabelAndInstrument(currentLabels);
    };
    auto parse_INGL_DESC = [](Show& show, Reader reader) {
        auto str = reader.Get<std::string>();
        if (reader.size() != 0) {
            throw CC_FileException("Description the wrong size", INGL_DESC);
        }
        show.SetDescr(str);
    };
    auto parse_INGL_SHET = [correction](Show& show, Reader reader) {
        Sheet sheet(show.GetNumPoints(), reader, correction);
        auto sheet_num = show.GetCurrentSheetNum();
        show.InsertSheet(sheet, show.GetNumSheets());
        show.SetCurrentSheet(sheet_num);
    };
    auto parse_INGL_SELE = [](Show& show, Reader reader) {
        if ((reader.size() % 4) != 0) {
            throw CC_FileException("Incorrect size", INGL_SIZE);
        }
        while (reader.size()) {
            show.mSelectionList.insert(reader.Get<uint32_t>());
        }
    };
    auto parse_INGL_CURR = [](Show& show, Reader reader) {
        if (reader.size() != 4) {
            throw CC_FileException("Incorrect size", INGL_SIZE);
        }
        show.mSheetNum = reader.Get<uint32_t>();
    };
    auto parse_INGL_MODE = [](Show& show, Reader reader) {
        show.mMode = ShowMode::CreateShowMode(reader);
    };
    // [=] needed here to pull in the parse functions
    auto parse_INGL_SHOW = [=](Show& show, Reader reader) {
        std::map<uint32_t, std::function<void(Show & show, Reader)>> const parser = {
            { INGL_SIZE, parse_INGL_SIZE },
            { INGL_LABL, parse_INGL_LABL },
            { INGL_INST, parse_INGL_INST },
            { INGL_DESC, parse_INGL_DESC },
            { INGL_SHET, parse_INGL_SHET },
            { INGL_SELE, parse_INGL_SELE },
            { INGL_CURR, parse_INGL_CURR },
            { INGL_MODE, parse_INGL_MODE },
        };
        auto table = reader.ParseOutLabels();
        for (auto& i : table) {
            auto the_parser = parser.find(std::get<0>(i));
            if (the_parser != parser.end()) {
                the_parser->second(show, std::get<1>(i));
            }
        }
    };

    auto table = reader.ParseOutLabels();
    bool found_show = false;
    for (auto& i : table) {
        if (std::get<0>(i) == INGL_SHOW) {
            parse_INGL_SHOW(*this, std::get<1>(i));
            found_show = true;
        }
    }
    if (!found_show) {
        throw CC_FileException("did not find show", INGL_SHOW);
    }
}

template <typename T>
auto anyInstrumentsBesidesDefault(T const& all)
{
    auto instruments = std::set(all.begin(), all.end());
    instruments.erase(kDefault);
    return instruments.size();
}

auto Show::SerializeShowData() const -> std::vector<std::byte>
{
    using Parser::Append;
    using Parser::AppendAndNullTerminate;
    using Parser::Construct_block;
    std::vector<std::byte> result;
    // SHOW_DATA          = NUM_MARCH , LABEL , [ DESCRIPTION ] , { SHEET }* ;
    // Write NUM_MARCH
    Append(result, Construct_block(INGL_SIZE, static_cast<uint32_t>(GetNumPoints())));

    // Write LABEL
    std::vector<std::byte> labels;
    for (auto& i : mDotLabelAndInstrument) {
        AppendAndNullTerminate(labels, i.first);
    }
    Append(result, Construct_block(INGL_LABL, labels));

    // Write INSTRUMENTS
    if (anyInstrumentsBesidesDefault(GetPointsInstrument())) {
        std::vector<std::byte> instruments;
        for (auto& i : mDotLabelAndInstrument) {
            AppendAndNullTerminate(instruments, i.second == kDefault ? "" : i.second);
        }
        Append(result, Construct_block(INGL_INST, instruments));
    }

    // write Description
    if (!GetDescr().empty()) {
        std::vector<std::byte> descr;
        AppendAndNullTerminate(descr, GetDescr());
        Append(result, Construct_block(INGL_DESC, descr));
    }

    // Handle sheets
    for (auto& sheet : mSheets) {
        Append(result, sheet.SerializeSheet());
    }

    // add selection
    if (!mSelectionList.empty()) {
        std::vector<std::byte> selections;
        for (auto&& i : mSelectionList) {
            Append(selections, uint32_t(i));
        }
        Append(result, Construct_block(INGL_SELE, selections));
    }

    // add current sheet
    Append(result, Construct_block(INGL_CURR, static_cast<uint32_t>(mSheetNum)));

    // add the mode
    Append(result, Construct_block(INGL_MODE, mMode.Serialize()));

    return result;
}

auto Show::SerializeShow() const -> std::vector<std::byte>
{
    using Parser::Append;
    using Parser::Construct_block;
    std::vector<std::byte> result;
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

auto Show::GenerateSheetElements(CalChart::Configuration const& config, int referencePoint) const -> std::vector<CalChart::Draw::DrawCommand>
{
    return GetCurrentSheet()->GenerateSheetElements(config, mSelectionList, GetPointsLabel(), referencePoint);
}

auto Show::GenerateGhostPointsDrawCommands(
    CalChart::Configuration const& config,
    CalChart::SelectionList const& selection_list,
    CalChart::Sheet const& sheet) const -> std::vector<CalChart::Draw::DrawCommand>
{
    return sheet.GenerateGhostElements(config, selection_list, GetPointsLabel());
}

template <std::ranges::input_range Range>
    requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Point>)
auto GeneratePointDrawCommand(Range&& points) -> std::vector<CalChart::Draw::DrawCommand>
{
    return CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(points | std::views::transform([](auto&& point) {
        auto size = CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(1) };
        return CalChart::Draw::Rectangle{
            point.GetPos() - size / 2, { CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(1) }
        };
    }));
}

auto Show::GenerateFieldWithMarchersDrawCommands(CalChart::Configuration const& config) const -> std::vector<std::vector<CalChart::Draw::DrawCommand>>
{
    auto field = CalChart::Draw::withBrushAndPen(
        config.Get_CalChartBrushAndPen(CalChart::Colors::FIELD), CalChart::CreateModeDrawCommandsWithBorderOffset(config, mMode, CalChart::HowToDraw::Animation));
    return CalChart::Ranges::ToVector<std::vector<CalChart::Draw::DrawCommand>>(
        mSheets
        | std::views::transform(
            [this, field, &config](auto&& sheet) {
                return std::vector<CalChart::Draw::DrawCommand>{
                    field,
                    CalChart::Draw::withBrushAndPen(
                        config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_FRONT),
                        GeneratePointDrawCommand(sheet.GetAllMarchers())),
                }
                + mMode.Offset();
            }));
}

int Show::GetNumSheets() const
{
    return static_cast<int>(mSheets.size());
}

Show::Sheet_container_t Show::RemoveNthSheet(int sheetidx)
{
    auto i = GetNthSheet(sheetidx);
    Sheet_container_t shts(1, *i);
    mSheets.erase(i);

    if (sheetidx < GetCurrentSheetNum()) {
        SetCurrentSheet(GetCurrentSheetNum() - 1);
    }
    if (GetCurrentSheetNum() >= GetNumSheets()) {
        SetCurrentSheet(GetNumSheets() - 1);
    }

    return shts;
}

void Show::SetCurrentSheet(int n) { mSheetNum = n; }

void Show::InsertSheet(const Sheet& sheet, int sheetidx)
{
    mSheets.insert(mSheets.begin() + sheetidx, sheet);
    if (sheetidx <= GetCurrentSheetNum())
        SetCurrentSheet(GetCurrentSheetNum() + 1);
}

void Show::InsertSheet(const Sheet_container_t& sheet,
    int sheetidx)
{
    mSheets.insert(mSheets.begin() + sheetidx, sheet.begin(), sheet.end());
    if (sheetidx <= GetCurrentSheetNum())
        SetCurrentSheet(GetCurrentSheetNum() + 1);
}

// warning, the labels might not match up
void Show::SetNumPoints(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int columns, Coord const& new_march_position)
{
    for (auto sht = GetSheetBegin(); sht != GetSheetEnd(); ++sht) {
        auto pts = sht->NewNumPointsPositions(static_cast<int>(labelsAndInstruments.size()), columns, new_march_position);
        sht->SetPoints(pts);
    }
    SetPointLabelAndInstrument(labelsAndInstruments);
}

void Show::DeletePoints(SelectionList const& sl)
{
    for (auto iter = sl.rbegin(); iter != sl.rend(); ++iter) {
        mDotLabelAndInstrument.erase(mDotLabelAndInstrument.begin() + *iter);
    }
    for (auto&& sht : mSheets) {
        sht.DeletePoints(sl);
    }
}

void Show::SetPointLabelAndInstrument(std::vector<std::pair<std::string, std::string>> const& labels)
{
    mDotLabelAndInstrument = labels;
}

// A relabel mapping is the mapping you would need to apply to sheet_next (and all following sheets)
// so that they match with this current sheet
auto Show::GetRelabelMapping(const_Sheet_iterator_t source_sheet, const_Sheet_iterator_t target_sheets, CalChart::Coord::units tolerance) const -> std::optional<std::vector<size_t>>
{
    std::vector<size_t> table(GetNumPoints());
    std::vector<unsigned> used_table(GetNumPoints());

    for (auto i = 0; i < GetNumPoints(); i++) {
        auto j = 0;
        for (; j < GetNumPoints(); j++) {
            if (!used_table[j]) {
                if (source_sheet->GetPosition(i).Distance(target_sheets->GetPosition(j)) < tolerance) {
                    table[i] = j;
                    used_table[j] = true;
                    break;
                }
            }
        }
        if (j == GetNumPoints()) {
            // didn't find a match
            return std::nullopt;
        }
    }

    return table;
}

std::string Show::GetPointLabel(int i) const
{
    if (i >= static_cast<int>(mDotLabelAndInstrument.size()))
        return "";
    return mDotLabelAndInstrument.at(i).first;
}

std::vector<std::string> Show::GetPointsLabel() const
{
    std::vector<std::string> labels;
    std::transform(mDotLabelAndInstrument.begin(), mDotLabelAndInstrument.end(), std::back_inserter(labels), [](auto&& i) { return i.first; });
    return labels;
}

std::string Show::GetPointInstrument(int i) const
{
    if (i >= static_cast<int>(mDotLabelAndInstrument.size()))
        return "";
    return mDotLabelAndInstrument.at(i).second;
}

std::vector<std::string> Show::GetPointsInstrument() const
{
    std::vector<std::string> instruments;
    std::transform(mDotLabelAndInstrument.begin(), mDotLabelAndInstrument.end(), std::back_inserter(instruments), [](auto&& i) { return i.second; });
    return instruments;
}

SYMBOL_TYPE Show::GetPointSymbol(int i) const
{
    if (i >= static_cast<int>(mDotLabelAndInstrument.size()))
        return SYMBOL_PLAIN;
    return GetCurrentSheet()->GetSymbol(i);
}

std::vector<SYMBOL_TYPE> Show::GetPointsSymbol() const
{
    return GetCurrentSheet()->GetSymbols();
}

bool Show::AlreadyHasPrintContinuity() const
{
    for (auto& i : mSheets) {
        if (i.GetPrintableContinuity().size() > 0) {
            return true;
        }
    }
    return false;
}

auto Show::GetSheetsName() const -> std::vector<std::string>
{
    return CalChart::Ranges::ToVector<std::string>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetName(); }));
}

bool Show::WillMovePoints(std::map<int, Coord> const& new_positions, int ref) const
{
    auto sheet = GetCurrentSheet();
    for (auto&& index : new_positions) {
        if (index.second != sheet->GetPosition(index.first, ref)) {
            return true;
        }
    }
    return false;
}

void Show::SetShowMode(ShowMode const& mode)
{
    mMode = mode;
}

SelectionList Show::MakeSelectAll() const
{
    SelectionList sl;
    for (auto i = 0u; i < mDotLabelAndInstrument.size(); ++i)
        sl.insert(i);
    return sl;
}

SelectionList Show::MakeUnselectAll() const
{
    return {};
}

void Show::SetSelectionList(const SelectionList& sl)
{
    mSelectionList = sl;
}

SelectionList Show::MakeAddToSelection(const SelectionList& sl) const
{
    SelectionList slcopy = mSelectionList;
    slcopy.insert(sl.begin(), sl.end());
    return slcopy;
}

SelectionList Show::MakeRemoveFromSelection(const SelectionList& sl) const
{
    SelectionList slcopy = mSelectionList;
    slcopy.erase(sl.begin(), sl.end());
    return slcopy;
}

SelectionList Show::MakeToggleSelection(const SelectionList& sl) const
{
    SelectionList slcopy = mSelectionList;
    for (auto i : sl) {
        if (slcopy.count(i)) {
            slcopy.erase(i);
        } else {
            slcopy.insert(i);
        }
    }
    return slcopy;
}

// toggle selection means toggle it as selected to unselected
// otherwise, always select it
SelectionList Show::MakeSelectWithinPolygon(CalChart::RawPolygon_t const& polygon, int ref) const
{
    if (polygon.size() < 3) {
        return {};
    }

    SelectionList sl;
    auto sheet = GetCurrentSheet();
    for (int i = 0; i < GetNumPoints(); i++) {
        if (CalChart::Inside(sheet->GetPosition(i, ref), polygon)) {
            sl.insert(i);
        }
    }
    return sl;
}

SelectionList Show::MakeSelectBySymbol(SYMBOL_TYPE symbol) const
{
    return GetCurrentSheet()->MakeSelectPointsBySymbol(symbol);
}

SelectionList Show::MakeSelectByInstrument(std::string const& instrumentName) const
{
    SelectionList sl;
    for (auto i = 0ul; i < mDotLabelAndInstrument.size(); ++i) {
        if (mDotLabelAndInstrument[i].second == instrumentName) {
            sl.insert(i);
        }
    }
    return sl;
}

SelectionList Show::MakeSelectByLabel(std::string const& label) const
{
    SelectionList sl;
    for (auto i = 0ul; i < mDotLabelAndInstrument.size(); ++i) {
        if (mDotLabelAndInstrument[i].first == label) {
            sl.insert(i);
        }
    }
    return sl;
}

namespace {
    // In 'movements', make a series of commands to describe how a point should be animated over time in the Online Viewer
    // This is effectively a reduce, except the dotLabels make this challenging.
    auto GetMovement(std::vector<std::string> dotLabels, std::vector<std::vector<nlohmann::json>> const& allMovements) -> std::map<std::string, std::vector<nlohmann::json>>
    {
        auto movements = std::map<std::string, std::vector<nlohmann::json>>{};
        for (auto ptIndex : std::views::iota(0UL, dotLabels.size())) {
            movements[dotLabels[ptIndex]] = allMovements.at(ptIndex);
        }

        return movements;
    }

    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Sheet>)
    auto GetAllPointPositions(Range&& sheets)
    {
        return sheets | std::views::transform([](auto&& sheet) { return sheet.GetAllMarchers(); });
    }
}

nlohmann::json Show::toOnlineViewerJSON(const Animation& compiledShow) const
{
    nlohmann::json j;

    // Setup the skeleton for the show's JSON representation
    j["title"] = "(MANUAL) the show title that you want people to see goes here"; // TODO; For now, this will be manually added to the exported file
    j["year"] = "(MANUAL) enter show year (e.g. 2017)"; // TODO; Should eventually save automatically
    j["description"] = mDescr;
    std::vector<std::string> ptLabels;
    std::transform(mDotLabelAndInstrument.begin(), mDotLabelAndInstrument.end(), std::back_inserter(ptLabels), [](auto&& i) { return i.first; });
    j["labels"] = ptLabels;

    std::vector<nlohmann::json> sheetData;
    auto allMovements = compiledShow.toOnlineViewerJSON();
    for (auto index : std::views::iota(0UL, mSheets.size())) {
        auto thisMovement = GetMovement(ptLabels, allMovements.at(index));
        sheetData.push_back(mSheets.at(index).toOnlineViewerJSON(index + 1, ptLabels, thisMovement));
    }

    j["sheets"] = sheetData;

    return j;
}

Show_command_pair Show::Create_SetCurrentSheetCommand(int n) const
{
    auto action = [n = n](Show& show) { show.SetCurrentSheet(n); };
    auto reaction = [n = mSheetNum](Show& show) { show.SetCurrentSheet(n); };
    return { action, reaction };
}

Show_command_pair Show::Create_SetSelectionListCommand(const SelectionList& sl) const
{
    auto action = [sl](Show& show) { show.SetSelectionList(sl); };
    auto reaction = [sl = mSelectionList](Show& show) { show.SetSelectionList(sl); };
    return { action, reaction };
}

Show_command_pair Show::Create_SetCurrentSheetAndSelectionCommand(int n, const SelectionList& sl) const
{
    auto action = [n, sl](Show& show) { show.SetCurrentSheet(n); show.SetSelectionList(sl); };
    auto reaction = [n = mSheetNum, sl = mSelectionList](Show& show) { show.SetCurrentSheet(n); show.SetSelectionList(sl); };
    return { action, reaction };
}

Show_command_pair Show::Create_SetShowModeCommand(CalChart::ShowMode const& newmode) const
{
    auto action = [mode = newmode](Show& show) {
        show.SetShowMode(mode);
    };
    auto reaction = [mode = GetShowMode()](Show& show) {
        show.SetShowMode(mode);
    };
    return { action, reaction };
}

Show_command_pair Show::Create_SetupMarchersCommand(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int numColumns, Coord const& new_march_position) const
{
    auto action = [labelsAndInstruments, numColumns, new_march_position](Show& show) { show.SetNumPoints(labelsAndInstruments, numColumns, new_march_position); };
    // need to go through and save all the positions and labels for later
    auto old_labels = mDotLabelAndInstrument;
    std::vector<std::vector<Point>> old_points;
    for (auto&& sheet : mSheets) {
        old_points.emplace_back(sheet.GetAllMarchers());
    }
    auto reaction = [old_labels, old_points](Show& show) {
        for (auto i = 0ul; i < show.mSheets.size(); ++i) {
            show.mSheets.at(i).SetPoints(old_points.at(i));
        }
        show.SetPointLabelAndInstrument(old_labels);
    };
    return { action, reaction };
}

Show_command_pair Show::Create_SetInstrumentsCommand(std::map<int, std::string> const& dotToInstrument) const
{
    auto old_labels = mDotLabelAndInstrument;
    auto new_labels = old_labels;
    std::for_each(dotToInstrument.begin(), dotToInstrument.end(), [&new_labels](auto&& i) {
        new_labels.at(i.first).second = i.second;
    });
    auto action = [new_labels](Show& show) {
        show.SetPointLabelAndInstrument(new_labels);
    };
    auto reaction = [old_labels](Show& show) {
        show.SetPointLabelAndInstrument(old_labels);
    };
    return { action, reaction };
}

Show_command_pair Show::Create_SetSheetTitleCommand(std::string const& newname) const
{
    auto action = [whichSheet = mSheetNum, newname](Show& show) { show.mSheets.at(whichSheet).SetName(newname); };
    auto reaction = [whichSheet = mSheetNum, newname = mSheets.at(mSheetNum).GetName()](Show& show) { show.mSheets.at(whichSheet).SetName(newname); };
    return { action, reaction };
}

Show_command_pair Show::Create_SetSheetBeatsCommand(int beats) const
{
    auto action = [whichSheet = mSheetNum, beats](Show& show) { show.mSheets.at(whichSheet).SetBeats(beats); };
    auto reaction = [whichSheet = mSheetNum, beats = mSheets.at(mSheetNum).GetBeats()](Show& show) { show.mSheets.at(whichSheet).SetBeats(beats); };
    return { action, reaction };
}

Show_command_pair Show::Create_AddSheetsCommand(const Show::Sheet_container_t& sheets, int where) const
{
    auto action = [sheets, where](Show& show) { show.InsertSheet(sheets, where); };
    auto reaction = [sheets, where](Show& show) { auto num_times = sheets.size(); while (num_times--) show.RemoveNthSheet(where); };
    return { action, reaction };
}

Show_command_pair Show::Create_RemoveSheetCommand(int where) const
{
    Sheet_container_t old_shts(1, *GetNthSheet(where));
    auto action = [where](Show& show) { show.RemoveNthSheet(where); };
    auto reaction = [old_shts, where](Show& show) { show.InsertSheet(old_shts, where); };
    return { action, reaction };
}

// remapping gets applied on this sheet till the last one
Show_command_pair Show::Create_ApplyRelabelMapping(int sheet_num_first, std::vector<size_t> const& mapping) const
{
    std::vector<std::vector<Point>> current_pos;
    for (auto s = GetNthSheet(sheet_num_first); s != GetSheetEnd(); ++s) {
        current_pos.emplace_back(s->GetAllMarchers());
    }
    // first gather where all the points are;
    auto action = [sheet_num_first, mapping](Show& show) {
        auto s = show.GetNthSheet(sheet_num_first);
        while (s != show.GetSheetEnd()) {
            s->SetPoints(s->RemapPoints(mapping));
            ++s;
        }
    };
    auto reaction = [sheet_num_first, current_pos](Show& show) {
        auto i = 0;
        auto s = show.GetNthSheet(sheet_num_first);
        while (s != show.GetSheetEnd()) {
            s->SetPoints(current_pos.at(i++));
            ++s;
        }
    };
    return { action, reaction };
}

Show_command_pair Show::Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string>> const& data) const
{
    std::map<unsigned, std::pair<std::string, std::string>> undo_data;
    for (auto&& i : data) {
        undo_data[i.first] = { GetNthSheet(i.first)->GetPrintNumber(), GetNthSheet(i.first)->GetRawPrintContinuity() };
    }
    auto action = [data](Show& show) {
        for (auto&& i : data) {
            show.GetNthSheet(i.first)->SetPrintableContinuity(i.second.first, i.second.second);
        }
    };
    auto reaction = [undo_data](Show& show) {
        for (auto&& i : undo_data) {
            show.GetNthSheet(i.first)->SetPrintableContinuity(i.second.first, i.second.second);
        }
    };
    return { action, reaction };
}

Show_command_pair Show::Create_MovePointsCommand(std::map<int, Coord> const& new_positions, int ref) const
{
    return Create_MovePointsCommand(GetCurrentSheetNum(), new_positions, ref);
}

Show_command_pair Show::Create_MovePointsCommand(int whichSheet, std::map<int, Coord> const& new_positions, int ref) const
{
    auto sheet = GetNthSheet(whichSheet);
    std::map<unsigned, Coord> original_positions;
    for (auto&& index : new_positions) {
        original_positions[index.first] = sheet->GetPosition(index.first, ref);
    }
    auto action = [sheet_num = whichSheet, new_positions, ref](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : new_positions) {
            sheet->SetPosition(i.second, i.first, ref);
        }
    };
    auto reaction = [sheet_num = whichSheet, original_positions, ref](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : original_positions) {
            sheet->SetPosition(i.second, i.first, ref);
        }
    };
    return { action, reaction };
}

Show_command_pair Show::Create_DeletePointsCommand() const
{
    auto action = [selectionList = mSelectionList](Show& show) {
        show.DeletePoints(selectionList);
        show.SetSelectionList({});
    };
    // need to go through and save all the positions and labels for later
    auto old_labels = mDotLabelAndInstrument;
    std::vector<std::vector<Point>> old_points;
    for (auto&& sheet : mSheets) {
        old_points.emplace_back(sheet.GetAllMarchers());
    }
    auto reaction = [old_labels, old_points](Show& show) {
        for (auto i = 0ul; i < show.mSheets.size(); ++i) {
            show.mSheets.at(i).SetPoints(old_points.at(i));
        }
        show.SetPointLabelAndInstrument(old_labels);
    };
    return { action, reaction };
}

Show_command_pair Show::Create_RotatePointPositionsCommand(int rotateAmount, int ref) const
{
    // construct a vector of point indices in order
    std::vector<unsigned> pointIndices;
    std::copy(mSelectionList.begin(), mSelectionList.end(), std::back_inserter(pointIndices));

    // construct a vector of point positions, rotated by rotate amount
    std::vector<Coord> finalPositions;
    auto sheet = GetCurrentSheet();
    std::transform(mSelectionList.begin(), mSelectionList.end(),
        std::back_inserter(finalPositions),
        [=](unsigned i) { return sheet->GetPosition(i, ref); });
    rotateAmount %= mSelectionList.size();
    std::rotate(finalPositions.begin(), finalPositions.begin() + rotateAmount,
        finalPositions.end());

    // put things into place.
    std::map<int, Coord> positions;
    for (int index = static_cast<int>(pointIndices.size()) - 1; index >= 0; index--) {
        positions[pointIndices[index]] = finalPositions[index];
    }
    return Create_MovePointsCommand(positions, ref);
}

Show_command_pair Show::Create_ResetReferencePointToRef0(int ref) const
{
    std::map<int, Coord> positions;
    auto sheet = GetCurrentSheet();
    // for selected points, set the reference point to ref0
    // this should be per selection list defect #179
    for (auto&& i : mSelectionList) {
        positions[i] = sheet->GetPosition(i, 0);
    }
    return Create_MovePointsCommand(positions, ref);
}

Show_command_pair Show::Create_SetSymbolCommand(SYMBOL_TYPE sym) const
{
    return Create_SetSymbolCommand(mSelectionList, sym);
}

Show_command_pair Show::Create_SetSymbolCommand(const SelectionList& selectionList, SYMBOL_TYPE sym) const
{
    std::map<unsigned, SYMBOL_TYPE> new_sym;
    std::map<unsigned, SYMBOL_TYPE> original_sym;
    auto sheet = GetCurrentSheet();
    for (auto&& i : selectionList) {
        // Only do work on points that have different symbols
        if (sym != sheet->GetSymbol(i)) {
            new_sym[i] = sym;
            original_sym[i] = sheet->GetSymbol(i);
        }
    }
    auto action = [sheet_num = mSheetNum, new_sym](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : new_sym) {
            sheet->SetSymbol(i.first, i.second);
        }
    };
    auto reaction = [sheet_num = mSheetNum, original_sym](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : original_sym) {
            sheet->SetSymbol(i.first, i.second);
        }
    };
    return { action, reaction };
}

Show_command_pair Show::Create_SetContinuityCommand(SYMBOL_TYPE which_sym, CalChart::Continuity const& new_cont) const
{
    auto original_cont = GetCurrentSheet()->GetContinuityBySymbol(which_sym);
    auto action = [sheet_num = mSheetNum, which_sym, new_cont](Show& show) {
        show.GetNthSheet(sheet_num)->SetContinuity(which_sym, new_cont);
    };
    auto reaction = [sheet_num = mSheetNum, which_sym, original_cont](Show& show) {
        show.GetNthSheet(sheet_num)->SetContinuity(which_sym, original_cont);
    };
    return { action, reaction };
}

Show_command_pair Show::Create_SetLabelFlipCommand(std::map<int, bool> const& new_flip) const
{
    auto sheet = GetCurrentSheet();
    std::map<unsigned, bool> original_flip;
    for (auto&& index : new_flip) {
        original_flip[index.first] = sheet->GetMarcher(index.first).GetFlip();
    }
    auto action = [sheet_num = mSheetNum, new_flip](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : new_flip) {
            sheet->SetMarcherFlip(i.first, i.second);
        }
    };
    auto reaction = [sheet_num = mSheetNum, original_flip](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : original_flip) {
            sheet->SetMarcherFlip(i.first, i.second);
        }
    };
    return { action, reaction };
}

Show_command_pair Show::Create_SetLabelRightCommand(bool right) const
{
    std::map<int, bool> flips;
    for (auto&& i : mSelectionList) {
        flips[i] = right;
    }
    return Create_SetLabelFlipCommand(flips);
}

Show_command_pair Show::Create_ToggleLabelFlipCommand() const
{
    std::map<int, bool> flips;
    auto sheet = GetCurrentSheet();
    for (auto&& i : mSelectionList) {
        flips[i] = !sheet->GetMarcher(i).GetFlip();
    }
    return Create_SetLabelFlipCommand(flips);
}

Show_command_pair Show::Create_SetLabelVisiblityCommand(std::map<int, bool> const& new_visibility) const
{
    auto sheet = GetCurrentSheet();
    std::map<int, bool> original_visibility;
    for (auto&& index : new_visibility) {
        original_visibility[index.first] = sheet->GetMarcher(index.first).LabelIsVisible();
    }
    auto action = [sheet_num = mSheetNum, new_visibility](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : new_visibility) {
            sheet->SetMarcherLabelVisibility(i.first, i.second);
        }
    };
    auto reaction = [sheet_num = mSheetNum, original_visibility](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        for (auto&& i : original_visibility) {
            sheet->SetMarcherLabelVisibility(i.first, i.second);
        }
    };
    return { action, reaction };
}

Show_command_pair Show::Create_SetLabelVisibleCommand(bool isVisible) const
{
    std::map<int, bool> visible;
    for (auto&& i : mSelectionList) {
        visible[i] = isVisible;
    }
    return Create_SetLabelVisiblityCommand(visible);
}

Show_command_pair Show::Create_ToggleLabelVisibilityCommand() const
{
    std::map<int, bool> visible;
    auto sheet = GetCurrentSheet();
    for (auto&& i : mSelectionList) {
        visible[i] = !sheet->GetMarcher(i).LabelIsVisible();
    }
    return Create_SetLabelVisiblityCommand(visible);
}

Show_command_pair Show::Create_AddNewBackgroundImageCommand(ImageInfo const& image) const
{
    auto sheet = GetCurrentSheet();
    auto action = [sheet_num = mSheetNum, image, where = sheet->GetNumberBackgroundImages()](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->AddBackgroundImage(image, where);
    };
    auto reaction = [sheet_num = mSheetNum, where = sheet->GetNumberBackgroundImages()](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->RemoveBackgroundImage(where);
    };
    return { action, reaction };
}

Show_command_pair Show::Create_RemoveBackgroundImageCommand(int which) const
{
    auto sheet = GetCurrentSheet();
    if (static_cast<size_t>(which) >= sheet->GetNumberBackgroundImages()) {
        return { [](Show&) {}, [](Show&) {} };
    }
    auto action = [sheet_num = mSheetNum, which](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->RemoveBackgroundImage(which);
    };
    auto reaction = [sheet_num = mSheetNum, image = sheet->GetBackgroundImages().at(which), which](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->AddBackgroundImage(image, which);
    };
    return { action, reaction };
}

Show_command_pair Show::Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height) const
{
    auto sheet = GetCurrentSheet();
    auto [current_left, current_top, current_scaled_width, current_scaled_height] = sheet->GetBackgroundImageInfo(which);
    auto action = [sheet_num = mSheetNum, which, left, top, scaled_width, scaled_height](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->MoveBackgroundImage(which, left, top, scaled_width, scaled_height);
    };
    auto reaction = [sheet_num = mSheetNum, which, current_left, current_top, current_scaled_width, current_scaled_height](Show& show) {
        auto sheet = show.GetNthSheet(sheet_num);
        sheet->MoveBackgroundImage(which, current_left, current_top, current_scaled_width, current_scaled_height);
    };
    return { action, reaction };
}

auto Show::Create_AddSheetCurveCommand(CalChart::Curve const& curve) const -> Show_command_pair
{
    auto newIndex = GetCurrentSheet()->GetCurvesSize();
    auto action = [sheet_num = mSheetNum, curve, newIndex](Show& show) {
        show.GetNthSheet(sheet_num)->AddCurve(curve, newIndex);
    };
    auto reaction = [sheet_num = mSheetNum, newIndex](Show& show) {
        show.GetNthSheet(sheet_num)->RemoveCurve(newIndex);
    };
    return { action, reaction };
}

auto Show::Create_ReplaceSheetCurveCommand(CalChart::Curve const& curve, int whichCurve) const -> Show_command_pair
{
    auto oldCurve = GetCurrentSheet()->GetCurve(whichCurve);
    auto action = [sheet_num = mSheetNum, curve, whichCurve](Show& show) {
        show.GetNthSheet(sheet_num)->ReplaceCurve(curve, whichCurve);
    };
    auto reaction = [sheet_num = mSheetNum, oldCurve, whichCurve](Show& show) {
        show.GetNthSheet(sheet_num)->ReplaceCurve(oldCurve, whichCurve);
    };
    return { action, reaction };
}

auto Show::Create_RemoveSheetCurveCommand(int whichCurve) const -> Show_command_pair
{
    auto oldCurve = GetCurrentSheet()->GetCurve(whichCurve);
    auto action = [sheet_num = mSheetNum, whichCurve](Show& show) {
        show.GetNthSheet(sheet_num)->RemoveCurve(whichCurve);
    };
    auto reaction = [sheet_num = mSheetNum, oldCurve, whichCurve](Show& show) {
        show.GetNthSheet(sheet_num)->AddCurve(oldCurve, whichCurve);
    };
    return { action, reaction };
}

// -=-=-=-=-=-=- Unit Tests -=-=-=-=-=-=-=-
#include <cassert>
using namespace Parser;

static auto
Construct_show_zero_points_zero_labels_zero_description()
{
    std::vector<std::byte> show_data;
    Append(show_data, Construct_block(INGL_SIZE, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_LABL, std::vector<uint8_t>{}));
    Append(show_data, Construct_block(INGL_DESC, std::vector<uint8_t>(1)));
    Append(show_data, Construct_block(INGL_CURR, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_MODE, ShowMode::GetDefaultShowMode().Serialize()));
    return Construct_block(INGL_SHOW, show_data);
}

static auto Construct_show_zero_points_zero_labels()
{
    std::vector<std::byte> show_data;
    Append(show_data, Construct_block(INGL_SIZE, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_LABL, std::vector<uint8_t>{}));
    Append(show_data, Construct_block(INGL_CURR, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_MODE, ShowMode::GetDefaultShowMode().Serialize()));
    return Construct_block(INGL_SHOW, show_data);
}

static auto Construct_show_zero_points_zero_labels_1_sheet_and_random()
{
    std::vector<std::byte> show_data;
    Append(show_data, Construct_block(0x12345678, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_SIZE, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(0x87654321, std::vector<uint8_t>(13)));
    Append(show_data, Construct_block(INGL_LABL, std::vector<uint8_t>{}));
    Append(show_data, Construct_block(0xDEADBEEF, std::vector<uint8_t>(1)));

    std::vector<std::byte> sheet_data;
    Append(sheet_data, Construct_block(INGL_NAME, std::vector<uint8_t>{ '1', '\0' }));
    Append(sheet_data, Construct_block(INGL_DURA, std::vector<uint8_t>{ 0, 0, 0, 1 }));
    Append(sheet_data, Construct_block(INGL_PNTS, std::vector<uint8_t>{}));
    Append(sheet_data, Construct_block(INGL_CONT, std::vector<uint8_t>{}));
    Append(sheet_data, Construct_block(INGL_PCNT, std::vector<uint8_t>{ '\0', '\0' }));
    Append(show_data, Construct_block(INGL_SHET, sheet_data));

    Append(show_data, Construct_block(INGL_CURR, std::vector<uint8_t>(4)));

    return Construct_block(INGL_SHOW, show_data);
}

void Show::CC_show_round_trip_test()
{
    auto blank_show = Show::Create(ShowMode::GetDefaultShowMode());
    auto blank_show_data = blank_show->SerializeShow();
    auto char_data = std::string{};
    std::transform(blank_show_data.begin(), blank_show_data.end(), std::back_inserter(char_data), [](auto a) { return std::to_integer<char>(a); });
    std::istringstream is(char_data);
    auto re_read_show = Show::Create(ShowMode::GetDefaultShowMode(), is);
    auto re_read_show_data = re_read_show->SerializeShow();
    bool is_equal = blank_show_data.size() == re_read_show_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(), re_read_show_data.begin());
    (void)is_equal;
    assert(is_equal);
}

void Show::CC_show_round_trip_test_with_number_label_description()
{
    std::vector<std::byte> point_data;
    Append(point_data, uint32_t{ 1 });
    std::vector<std::byte> data;
    Append(data, Construct_block(INGL_SIZE, point_data));
    Append(data, Construct_block(INGL_LABL, std::vector<char>{ 'p', 'o', 'i', 'n', 't', '\0' }));
    Append(data, Construct_block(INGL_DESC, std::vector<char>{ 'd', 'e', 's', 'c', 'r', 'i', 'p', 't', 'i', 'o', 'n', '\0' }));
    std::vector<std::byte> curr_data;
    Append(curr_data, uint32_t{ 0 });
    Append(data, Construct_block(INGL_CURR, curr_data));
    Append(data, Construct_block(INGL_MODE, ShowMode::GetDefaultShowMode().Serialize()));
    auto show_data = Construct_block(INGL_SHOW, data);

    Show show1(ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }));
    auto show1_data = show1.SerializeShow();
    // eat header
    show1_data.erase(show1_data.begin(), show1_data.begin() + 8);
    for (auto i = 0llu; i < show1_data.size(); ++i) {
        if (show1_data.at(i) != show_data.at(i))
            std::cout << "Wrong at " << i << ", " << std::to_integer<char>(show1_data.at(i)) << "\n";
    }
    auto is_equal = show1_data.size() == show_data.size() && std::equal(show1_data.begin(), show1_data.end(), show_data.begin());
    (void)is_equal;
    assert(is_equal);

    // now check that things loaded correctly
    assert(show1.GetNumPoints() == 1);
    assert(show1.GetNumSheets() == 0);
    assert(show1.GetPointLabel(0) == "point");
    assert(show1.GetDescr() == "description");
}

void Show::CC_show_round_trip_test_with_different_show_modes()
{
    Show show1(ShowMode::CreateShowMode({ { 36, 52, 8, 8, 8, 8, -80, -42, 160, 84 } }, kDefaultYardLines));
    assert(show1.GetShowMode().HashW() == 36);
    auto show1_data = show1.SerializeShow();
    // eat header
    show1_data.erase(show1_data.begin(), show1_data.begin() + 8);

    Show show2(ShowMode::GetDefaultShowMode(), Reader({ show1_data.data(), show1_data.size() }));
    assert(show2.GetShowMode().HashW() == 36);
}

void Show::CC_show_blank_desc_test()
{
    auto show_zero_points_zero_labels_zero_description = Construct_show_zero_points_zero_labels_zero_description();
    Show show1(ShowMode::GetDefaultShowMode(),
        Reader({ show_zero_points_zero_labels_zero_description.data(),
            show_zero_points_zero_labels_zero_description.size() }));
    auto show1_data = show1.SerializeShow();
    // eat header
    show1_data.erase(show1_data.begin(), show1_data.begin() + 8);
    bool is_equal = show1_data.size() == show_zero_points_zero_labels_zero_description.size() && std::equal(show1_data.begin(), show1_data.end(), show_zero_points_zero_labels_zero_description.begin());
    (void)is_equal;
    assert(!is_equal);
    assert(show1.GetNumPoints() == 0);
    assert(show1.GetNumSheets() == 0);

    // now remove the description and they should be equal
    auto show_zero_points_zero_labels = Construct_show_zero_points_zero_labels();
    Show show2(ShowMode::GetDefaultShowMode(), Reader({ show_zero_points_zero_labels.data(), show_zero_points_zero_labels.size() }));
    auto show2_data = show2.SerializeShow();
    show2_data.erase(show2_data.begin(), show2_data.begin() + 8);
    is_equal = show2_data.size() == show_zero_points_zero_labels.size() && std::equal(show2_data.begin(), show2_data.end(), show_zero_points_zero_labels.begin());
    assert(is_equal);
    assert(show1.GetNumPoints() == 0);
    assert(show1.GetNumSheets() == 0);
}

// confirm we try to handle shows from the future
void Show::CC_show_future_show_test()
{
    // how?  By creating a show from scratch, then modifying the version; make
    // sure that we load it, and it looks the same
    // except the data gets reverted
    auto blank_show = Show::Create(ShowMode::GetDefaultShowMode());
    auto blank_show_data = blank_show->SerializeShow();
    auto char_data = std::vector<char>{};
    std::transform(blank_show_data.begin(), blank_show_data.end(), std::back_inserter(char_data), [](auto a) { return std::to_integer<char>(a); });
    assert(char_data.at(6) - '0' == CC_MAJOR_VERSION && char_data.at(7) - '0' == CC_MINOR_VERSION);
    ++char_data.at(6);
    ++char_data.at(7);
    std::istringstream is(std::string{ char_data.data(), char_data.size() });
    auto re_read_show = Show::Create(ShowMode::GetDefaultShowMode(), is);
    auto re_read_show_data = blank_show->SerializeShow();
    --char_data.at(6);
    --char_data.at(7);
    bool is_equal = blank_show_data.size() == re_read_show_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(), re_read_show_data.begin());
    (void)is_equal;
    assert(is_equal);
}

void Show::CC_show_wrong_size_throws_exception()
{
    auto points_3(Construct_block(INGL_SIZE, std::vector<uint8_t>(3)));
    auto show_data = Construct_block(INGL_SHOW, points_3);
    bool hit_exception = false;
    try {
        Show show1(ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }));
    } catch (CC_FileException const&) {
        hit_exception = true;
    }
    (void)hit_exception;
    assert(hit_exception);
}

// too large, and too small
void Show::CC_show_wrong_size_number_labels_throws()
{
    {
        std::vector<std::byte> point_data(4);
        details::put_big_long(point_data.data(), 1);
        auto points(Construct_block(INGL_SIZE, point_data));
        auto no_labels(Construct_block(INGL_LABL, std::vector<uint8_t>{}));
        auto t_show_data = points;
        t_show_data.insert(t_show_data.end(), no_labels.begin(), no_labels.end());
        auto show_data = Construct_block(INGL_SHOW, t_show_data);
        bool hit_exception = false;
        try {
            Show show1(ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }));
        } catch (CC_FileException const&) {
            hit_exception = true;
        }
        (void)hit_exception;
        assert(hit_exception);
    }
    {
        std::vector<std::byte> point_data(4);
        details::put_big_long(point_data.data(), 1);
        auto points(Construct_block(INGL_SIZE, point_data));
        auto labels(Construct_block(INGL_LABL, std::vector<char>{ 'a', '\0', 'b', '\0' }));
        auto t_show_data = points;
        t_show_data.insert(t_show_data.end(), labels.begin(), labels.end());
        auto show_data = Construct_block(INGL_SHOW, t_show_data);
        bool hit_exception = false;
        try {
            Show show1(ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }));
        } catch (CC_FileException const&) {
            hit_exception = true;
        }
        (void)hit_exception;
        assert(hit_exception);
    }
}

// too large, and too small
void Show::CC_show_wrong_size_description()
{
    auto no_points = Construct_block(INGL_SIZE, std::vector<uint8_t>(4));
    auto no_labels = Construct_block(INGL_LABL, std::vector<uint8_t>{});
    auto descr = Construct_block(INGL_DESC, std::vector<char>{ 'a', 'b', 'c', '\0' });
    descr.at(9) = std::byte{};
    auto t_show_data = no_points;
    t_show_data.insert(t_show_data.end(), no_labels.begin(), no_labels.end());
    t_show_data.insert(t_show_data.end(), descr.begin(), descr.end());
    auto show_data = Construct_block(INGL_SHOW, t_show_data);
    bool hit_exception = false;
    try {
        auto show1 = Show{ ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }) };
    } catch (CC_FileException const&) {
        hit_exception = true;
    }
    (void)hit_exception;
    assert(hit_exception);
}

// extra cruft ok
void Show::CC_show_extra_cruft_ok()
{
    // now remove the description and they should be equal
    auto extra_cruft = Construct_show_zero_points_zero_labels_1_sheet_and_random();
    auto show1 = Show{ ShowMode::GetDefaultShowMode(), Reader({ extra_cruft.data(), extra_cruft.size() }) };
    auto show1_data = show1.SerializeShow();

    auto blank_show = Show::Create(ShowMode::GetDefaultShowMode());
    auto blank_show_data = blank_show->SerializeShow();
    auto is_equal = blank_show_data.size() == show1_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(), show1_data.begin());
    (void)is_equal;
    assert(is_equal);
}

// show with nothing should fail:
void Show::CC_show_with_nothing_throws()
{
    std::vector<std::byte> empty{};
    bool hit_exception = false;
    try {
        auto show = Show{ ShowMode::GetDefaultShowMode(), Reader({ empty.data(), empty.size() }) };
    } catch (CC_FileException const&) {
        hit_exception = true;
    }
    (void)hit_exception;
    assert(hit_exception);
}

void Show_UnitTests()
{
    Show::CC_show_round_trip_test();
    Show::CC_show_round_trip_test_with_number_label_description();
    Show::CC_show_round_trip_test_with_different_show_modes();
    Show::CC_show_blank_desc_test();
    Show::CC_show_future_show_test();
    Show::CC_show_wrong_size_throws_exception();
    Show::CC_show_wrong_size_number_labels_throws();
    Show::CC_show_wrong_size_description();
    Show::CC_show_extra_cruft_ok();
    Show::CC_show_with_nothing_throws();
}
}
