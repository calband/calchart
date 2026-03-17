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

#define _LIBCPP_ENABLE_EXPERIMENTAL 1
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
#include "e7_transition_solver.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>

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
        for (auto i = 0UL; i < GetNumPoints(); i++) {
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
        for (auto i = 0UL; i < show.GetNumPoints(); i++) {
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
        for (auto i = 0UL; i < show.GetNumPoints(); i++) {
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
    auto parse_INGL_MEDIA = [](Show& show, Reader reader) {
        show.mMedia = ToFileData(reader);
        ++show.mMediaVersion;
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
            { INGL_MEDIA, parse_INGL_MEDIA },
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

    // add media
    if (!std::get<0>(mMedia).empty()) {
        Append(result, Construct_block(INGL_MEDIA, SerializeFileData(mMedia)));
    }

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
    return mSheets.at(mSheetNum).GenerateSheetElements(config, mSelectionList, GetPointsLabel(), referencePoint);
}

auto Show::GeneratePhatomPointsDrawCommands(
    Configuration const& config,
    MarcherToPosition const& positions) const -> std::vector<Draw::DrawCommand>
{
    auto& sheet = mSheets.at(mSheetNum);
    auto pointLabelFont = CalChart::Font{ Float2CoordUnits(config.Get_DotRatio() * config.Get_NumRatio()) };

    auto drawCmds = positions
        | std::views::transform([this, &sheet, &config](auto&& whichPosition) {
              auto [which, position] = whichPosition;
              // because points draw their position, we remove it then add the new position.
              return sheet.GetMarcher(which).GetDrawCommands(
                         GetPointLabel(which),
                         config)
                  + position
                  - sheet.GetMarcher(which).GetPos();
          })
        | std::views::join;
    return {
        CalChart::Draw::withFont(
            pointLabelFont,
            CalChart::Draw::withBrushAndPen(
                config.Get_CalChartBrushAndPen(CalChart::Colors::GHOST_POINT),
                CalChart::Draw::withTextForeground(
                    config.Get_CalChartBrushAndPen(CalChart::Colors::GHOST_POINT_TEXT),
                    drawCmds)))
    };
}

auto Show::GenerateGhostPointsDrawCommands(
    CalChart::Configuration const& config,
    CalChart::SelectionList const& selection_list,
    CalChart::Sheet const& sheet) const -> std::vector<CalChart::Draw::DrawCommand>
{
    return sheet.GenerateGhostElements(config, selection_list, GetPointsLabel());
}

auto Show::GenerateGhostPointsDrawCommands(
    int sheet,
    CalChart::Configuration const& config,
    CalChart::SelectionList const& selection_list) const -> std::vector<CalChart::Draw::DrawCommand>
{
    return GenerateGhostPointsDrawCommands(config, selection_list, mSheets.at(sheet));
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

auto Show::RemoveNthSheet(size_t sheetidx) -> Sheet_container_t
{
    if (sheetidx >= mSheets.size()) {
        return {};
    }
    auto i = mSheets.begin() + sheetidx;
    Sheet_container_t shts(1, *i);
    mSheets.erase(i);

    if (sheetidx > 0 && sheetidx < GetCurrentSheetNum()) {
        SetCurrentSheet(GetCurrentSheetNum() - 1);
    }
    if (GetCurrentSheetNum() >= GetNumSheets()) {
        SetCurrentSheet(GetNumSheets() - 1);
    }

    return shts;
}

void Show::InsertSheet(Sheet const& sheet, size_t sheetidx)
{
    if (sheetidx > mSheets.size()) {
        return;
    }
    mSheets.insert(mSheets.begin() + sheetidx, sheet);
    if (sheetidx <= GetCurrentSheetNum()) {
        SetCurrentSheet(GetCurrentSheetNum() + 1);
    }
}

void Show::InsertSheet(Sheet_container_t const& sheet, size_t sheetidx)
{
    if (sheetidx > mSheets.size()) {
        return;
    }
    mSheets.insert(mSheets.begin() + sheetidx, sheet.begin(), sheet.end());
    if (sheetidx <= GetCurrentSheetNum()) {
        SetCurrentSheet(GetCurrentSheetNum() + 1);
    }
}

// warning, the labels might not match up
void Show::SetNumPoints(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int columns, Coord const& new_march_position)
{
    for (auto& sheet : mSheets) {
        sheet.SetMarchers(sheet.NewNumPointsPositions(static_cast<int>(labelsAndInstruments.size()), columns, new_march_position));
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
auto Show::GetRelabelMapping(std::vector<Coord> const& source_marchers, std::vector<Coord> const& target_marchers, CalChart::Coord::units tolerance) -> std::optional<std::vector<MarcherIndex>>
{
    if (source_marchers.size() != target_marchers.size()) {
        return std::nullopt;
    }
    std::vector<MarcherIndex> table(source_marchers.size());
    std::vector<bool> used_table(source_marchers.size());

    for (auto i = 0UL; i < source_marchers.size(); i++) {
        auto j = 0UL;
        for (; j < source_marchers.size(); j++) {
            if (!used_table[j]) {
                if (source_marchers.at(i).Distance(target_marchers.at(j)) < tolerance) {
                    table[i] = j;
                    used_table[j] = true;
                    break;
                }
            }
        }
        if (j == source_marchers.size()) {
            // didn't find a match
            return std::nullopt;
        }
    }

    return table;
}

auto Show::WillMovePoints(MarcherToPosition const& new_positions, int ref) const -> bool
{
    auto& sheet = mSheets.at(mSheetNum);
    for (auto&& [index, position] : new_positions) {
        if (position != sheet.GetMarcherPosition(index, ref)) {
            return true;
        }
    }
    return false;
}

// Transition Solver
auto Show::validateCurrentSheetForTransitionSolver() const -> std::vector<std::string>
{
    return validateSheetForTransitionSolver(mSheets.at(mSheetNum));
}

auto Show::validateNextSheetForTransitionSolver() const -> std::vector<std::string>
{
    return validateSheetForTransitionSolver(mSheets.at(mSheetNum + 1));
}

void Show::runTransitionSolver(TransitionSolverParams const& params, TransitionSolverDelegate* delegate) const
{
    CalChart::runTransitionSolver(mSheets.at(mSheetNum), mSheets.at(mSheetNum + 1), params, delegate);
}

void Show::SetShowMode(ShowMode const& mode)
{
    mMode = mode;
}

auto Show::MakeSelectAll() const -> SelectionList
{
    SelectionList sl;
    for (auto i = 0u; i < mDotLabelAndInstrument.size(); ++i)
        sl.insert(i);
    return sl;
}

auto Show::MakeUnselectAll() const -> SelectionList
{
    return {};
}

void Show::SetSelectionList(SelectionList const& sl)
{
    mSelectionList = sl;
}

auto Show::MakeAddToSelection(SelectionList const& sl) const -> SelectionList
{
    SelectionList slcopy = mSelectionList;
    slcopy.insert(sl.begin(), sl.end());
    return slcopy;
}

auto Show::MakeRemoveFromSelection(SelectionList const& sl) const -> SelectionList
{
    SelectionList slcopy = mSelectionList;
    slcopy.erase(sl.begin(), sl.end());
    return slcopy;
}

auto Show::MakeToggleSelection(SelectionList const& sl) const -> SelectionList
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
auto Show::MakeSelectWithinPolygon(CalChart::RawPolygon_t const& polygon, int ref) const -> SelectionList
{
    if (polygon.size() < 3) {
        return {};
    }

    SelectionList sl;
    auto& sheet = mSheets.at(mSheetNum);
    for (auto i = 0UL; i < GetNumPoints(); i++) {
        if (CalChart::Inside(sheet.GetMarcherPosition(i, ref), polygon)) {
            sl.insert(i);
        }
    }
    return sl;
}

auto Show::MakeSelectBySymbol(SYMBOL_TYPE symbol) const -> SelectionList
{
    return mSheets.at(mSheetNum).MakeSelectPointsBySymbol(symbol);
}

auto Show::MakeSelectByInstrument(std::string const& instrumentName) const -> SelectionList
{
    SelectionList sl;
    for (auto i = 0ul; i < mDotLabelAndInstrument.size(); ++i) {
        if (mDotLabelAndInstrument[i].second == instrumentName) {
            sl.insert(i);
        }
    }
    return sl;
}

auto Show::MakeSelectByLabel(std::string const& label) const -> SelectionList
{
    SelectionList sl;
    for (auto i = 0ul; i < mDotLabelAndInstrument.size(); ++i) {
        if (mDotLabelAndInstrument[i].first == label) {
            sl.insert(i);
        }
    }
    return sl;
}

auto Show::MakeSelectByLabels(std::vector<std::string> const& labels) const -> SelectionList
{
    SelectionList sl;
    for (auto&& label : labels) {
        auto this_sl = MakeSelectByLabel(label);
        sl.insert(this_sl.begin(), this_sl.end());
    }
    return sl;
}

// Print continuity utility
auto Show::AlreadyHasPrintContinuity() const -> bool
{
    for (auto& i : mSheets) {
        if (i.GetPrintableContinuity().size() > 0) {
            return true;
        }
    }
    return false;
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

auto Show::toOnlineViewerJSON(Animation const& compiledShow) const -> nlohmann::json
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

auto Show::Create_SetCurrentSheetCommand(size_t n) const -> Show_command_pair
{
    auto action = [n = n](Show& show) { show.SetCurrentSheet(n); };
    auto reaction = [n = mSheetNum](Show& show) { show.SetCurrentSheet(n); };
    return { action, reaction };
}

auto Show::Create_SetSelectionListCommand(SelectionList const& sl) const -> Show_command_pair
{
    auto action = [sl](Show& show) { show.SetSelectionList(sl); };
    auto reaction = [sl = mSelectionList](Show& show) { show.SetSelectionList(sl); };
    return { action, reaction };
}

auto Show::Create_SetCurrentSheetAndSelectionCommand(size_t n, SelectionList const& sl) const -> Show_command_pair
{
    auto action = [n, sl](Show& show) { show.SetCurrentSheet(n); show.SetSelectionList(sl); };
    auto reaction = [n = mSheetNum, sl = mSelectionList](Show& show) { show.SetCurrentSheet(n); show.SetSelectionList(sl); };
    return { action, reaction };
}

auto Show::Create_SetShowModeCommand(CalChart::ShowMode const& newmode) const -> Show_command_pair
{
    auto action = [mode = newmode](Show& show) {
        show.SetShowMode(mode);
    };
    auto reaction = [mode = GetShowMode()](Show& show) {
        show.SetShowMode(mode);
    };
    return { action, reaction };
}

auto Show::Create_SetupMarchersCommand(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int numColumns, Coord const& new_march_position) const -> Show_command_pair
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
            show.mSheets.at(i).SetMarchers(old_points.at(i));
        }
        show.SetPointLabelAndInstrument(old_labels);
    };
    return { action, reaction };
}

auto Show::Create_SetInstrumentsCommand(std::map<MarcherIndex, std::string> const& dotToInstrument) const -> Show_command_pair
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

auto Show::Create_SetSheetTitleCommand(std::string const& newname) const -> Show_command_pair
{
    auto action = [whichSheet = mSheetNum, newname](Show& show) { show.mSheets.at(whichSheet).SetName(newname); };
    auto reaction = [whichSheet = mSheetNum, newname = mSheets.at(mSheetNum).GetName()](Show& show) { show.mSheets.at(whichSheet).SetName(newname); };
    return { action, reaction };
}

auto Show::Create_SetSheetBeatsCommand(Beats beats) const -> Show_command_pair
{
    auto action = [whichSheet = mSheetNum, beats](Show& show) { show.mSheets.at(whichSheet).SetBeats(beats); };
    auto reaction = [whichSheet = mSheetNum, beats = mSheets.at(mSheetNum).GetBeats()](Show& show) { show.mSheets.at(whichSheet).SetBeats(beats); };
    return { action, reaction };
}

auto Show::Create_SetSheetTempoCommand(Tempo tempo) const -> Show_command_pair
{
    auto action = [whichSheet = mSheetNum, tempo](Show& show) { show.mSheets.at(whichSheet).SetTempo(tempo); };
    auto reaction = [whichSheet = mSheetNum, tempo = mSheets.at(mSheetNum).GetTempo()](Show& show) { show.mSheets.at(whichSheet).SetTempo(tempo); };
    return { action, reaction };
}

auto Show::Create_SetSheetsBeatInfoCommand(std::vector<SheetBeatInfo> const& beatInfos) const -> Show_command_pair
{
    auto action = [beatInfos](Show& show) {
        for (auto&& [whichSheet, beatInfo] : CalChart::Ranges::enumerate_view(beatInfos)) {
            show.mSheets.at(whichSheet).SetSheetBeatInfo(beatInfo);
        }
    };
    auto reaction = [beatInfos = GetSheetsBeatInfo()](Show& show) {
        for (auto&& [whichSheet, beatInfo] : CalChart::Ranges::enumerate_view(beatInfos)) {
            show.mSheets.at(whichSheet).SetSheetBeatInfo(beatInfo);
        }
    };
    return { action, reaction };
}

auto Show::Create_SetMediaCommand(FileData const& media) const -> Show_command_pair
{
    auto action = [media](Show& show) { show.mMedia = media; ++show.mMediaVersion; };
    auto reaction = [media = mMedia, version = mMediaVersion](Show& show) { show.mMedia = media; show.mMediaVersion = version; };
    return { action, reaction };
}

auto Show::Create_AddSheetsCommand(const Show::Sheet_container_t& sheets, size_t where) const -> Show_command_pair
{
    auto action = [sheets, where](Show& show) { show.InsertSheet(sheets, where); };
    auto reaction = [sheets, where](Show& show) { auto num_times = sheets.size(); while (num_times--) show.RemoveNthSheet(where); };
    return { action, reaction };
}

auto Show::Create_RemoveSheetCommand(size_t where) const -> Show_command_pair
{
    Sheet_container_t old_shts(1, mSheets.at(where));
    auto action = [where](Show& show) { show.RemoveNthSheet(where); };
    auto reaction = [old_shts, where](Show& show) { show.InsertSheet(old_shts, where); };
    return { action, reaction };
}

// remapping gets applied on this sheet till the last one
auto Show::Create_ApplyRelabelMapping(int sheet_num_first, std::vector<MarcherIndex> const& mapping) const -> Show_command_pair
{
    auto current_pos = CalChart::Ranges::ToVector<std::vector<Point>>(
        mSheets | std::views::drop(sheet_num_first) | std::views::transform([](auto&& sheet) { return sheet.GetAllMarchers(); }));
    // first gather where all the points are;
    auto action = [sheet_num_first, mapping](Show& show) {
        for (auto index = static_cast<size_t>(sheet_num_first); index < show.mSheets.size(); ++index) {
            auto& sheet = show.mSheets.at(index);
            sheet.SetMarchers(sheet.RemapPoints(mapping));
        }
    };
    auto reaction = [sheet_num_first, current_pos](Show& show) {
        for (auto index = 0U; index < current_pos.size(); ++index) {
            show.mSheets.at(index + sheet_num_first).SetMarchers(current_pos.at(index));
        }
    };
    return { action, reaction };
}

auto Show::Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string>> const& data) const -> Show_command_pair
{
    std::map<unsigned, std::pair<std::string, std::string>> undo_data;
    for (auto&& i : data) {
        undo_data[i.first] = { mSheets.at(i.first).GetPrintNumber(), mSheets.at(i.first).GetRawPrintContinuity() };
    }
    auto action = [data](Show& show) {
        for (auto&& i : data) {
            show.mSheets.at(i.first).SetPrintableContinuity(i.second.first, i.second.second);
        }
    };
    auto reaction = [undo_data](Show& show) {
        for (auto&& i : undo_data) {
            show.mSheets.at(i.first).SetPrintableContinuity(i.second.first, i.second.second);
        }
    };
    return { action, reaction };
}

auto Show::Create_MovePointsCommand(MarcherToPosition const& new_positions, int ref) const -> Show_command_pair
{
    return Create_MovePointsCommand(GetCurrentSheetNum(), new_positions, ref);
}

auto Show::Create_MovePointsCommand(int whichSheet, MarcherToPosition const& new_positions, int ref) const -> Show_command_pair
{
    auto const& sheet = mSheets.at(whichSheet);
    MarcherToPosition original_positions;
    for (auto&& index : new_positions) {
        original_positions[index.first] = sheet.GetMarcherPosition(index.first, ref);
    }
    auto originalCurves = sheet.GetCurveAssignments();

    auto action = [sheet_num = whichSheet, new_positions, ref](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        for (auto&& i : new_positions) {
            sheet.SetPosition(i.second, i.first, ref);
        }
    };
    auto reaction = [sheet_num = whichSheet, original_positions, ref, originalCurves](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        for (auto&& i : original_positions) {
            sheet.SetPosition(i.second, i.first, ref);
        }
        sheet.SetCurveAssignment(originalCurves);
    };
    return { action, reaction };
}

namespace {
    auto found(MarcherIndex marcher, std::vector<std::vector<MarcherIndex>> locations)
    {
        for (auto&& location : locations) {
            if (std::find(location.begin(), location.end(), marcher) != location.end()) {
                return true;
            }
        }
        return false;
    }
}

Show_command_pair Show::Create_AssignPointsToCurve(size_t whichCurve, std::vector<MarcherIndex> whichMarchers)
{
    auto whichSheet = GetCurrentSheetNum();
    auto const& sheet = mSheets.at(whichSheet);
    std::map<MarcherIndex, Coord> originalPositions;
    auto originalCurves = sheet.GetCurveAssignments();

    for (auto&& index : whichMarchers) {
        if (!found(index, originalCurves)) {
            originalPositions[index] = sheet.GetMarcherPosition(index);
        }
    }
    auto newAssignments = sheet.GetCurveAssignmentsWithNewAssignments(whichCurve, whichMarchers);

    auto action = [whichSheet, newAssignments](Show& show) {
        show.mSheets.at(whichSheet).SetCurveAssignment(newAssignments);
    };
    auto reaction = [whichSheet, originalPositions, originalCurves](Show& show) {
        auto& sheet = show.mSheets.at(whichSheet);
        for (auto&& [whichMarcher, pos] : originalPositions) {
            sheet.SetPosition(pos, whichMarcher);
        }
        sheet.SetCurveAssignment(originalCurves);
    };
    return { action, reaction };
}

auto Show::Create_DeletePointsCommand() const -> Show_command_pair
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
            show.mSheets.at(i).SetMarchers(old_points.at(i));
        }
        show.SetPointLabelAndInstrument(old_labels);
    };
    return { action, reaction };
}

auto Show::Create_RotatePointPositionsCommand(int rotateAmount, int ref) const -> Show_command_pair
{
    // construct a vector of point indices in order
    std::vector<unsigned> pointIndices;
    std::copy(mSelectionList.begin(), mSelectionList.end(), std::back_inserter(pointIndices));

    // construct a vector of point positions, rotated by rotate amount
    std::vector<Coord> finalPositions;
    auto& sheet = mSheets.at(mSheetNum);
    std::transform(mSelectionList.begin(), mSelectionList.end(),
        std::back_inserter(finalPositions),
        [&sheet, ref](unsigned i) { return sheet.GetMarcherPosition(i, ref); });
    rotateAmount %= mSelectionList.size();
    std::rotate(finalPositions.begin(), finalPositions.begin() + rotateAmount,
        finalPositions.end());

    // put things into place.
    MarcherToPosition positions;
    for (int index = static_cast<int>(pointIndices.size()) - 1; index >= 0; index--) {
        positions[pointIndices[index]] = finalPositions[index];
    }
    return Create_MovePointsCommand(positions, ref);
}

auto Show::Create_ResetReferencePointToRef0(int ref) const -> Show_command_pair
{
    MarcherToPosition positions;
    auto& sheet = mSheets.at(mSheetNum);
    // for selected points, set the reference point to ref0
    // this should be per selection list defect #179
    for (auto&& i : mSelectionList) {
        positions[i] = sheet.GetMarcherPosition(i, 0);
    }
    return Create_MovePointsCommand(positions, ref);
}

auto Show::Create_SetSymbolCommand(SYMBOL_TYPE sym) const -> Show_command_pair
{
    return Create_SetSymbolCommand(mSelectionList, sym);
}

auto Show::Create_SetSymbolCommand(SelectionList const& selectionList, SYMBOL_TYPE sym) const -> Show_command_pair
{
    std::map<MarcherIndex, SYMBOL_TYPE> new_sym;
    std::map<MarcherIndex, SYMBOL_TYPE> original_sym;
    auto& sheet = mSheets.at(mSheetNum);
    for (auto&& i : selectionList) {
        // Only do work on points that have different symbols
        if (sym != sheet.GetSymbol(i)) {
            new_sym[i] = sym;
            original_sym[i] = sheet.GetSymbol(i);
        }
    }
    auto action = [sheet_num = mSheetNum, new_sym](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        for (auto&& i : new_sym) {
            sheet.SetSymbol(i.first, i.second);
        }
    };
    auto reaction = [sheet_num = mSheetNum, original_sym](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        for (auto&& i : original_sym) {
            sheet.SetSymbol(i.first, i.second);
        }
    };
    return { action, reaction };
}

auto Show::Create_SetContinuityCommand(SYMBOL_TYPE which_sym, CalChart::Continuity const& new_cont) const -> Show_command_pair
{
    auto original_cont = mSheets.at(mSheetNum).GetContinuityBySymbol(which_sym);
    auto action = [sheet_num = mSheetNum, which_sym, new_cont](Show& show) {
        show.mSheets.at(sheet_num).SetContinuity(which_sym, new_cont);
    };
    auto reaction = [sheet_num = mSheetNum, which_sym, original_cont](Show& show) {
        show.mSheets.at(sheet_num).SetContinuity(which_sym, original_cont);
    };
    return { action, reaction };
}

auto Show::Create_SetLabelFlipCommand(std::map<MarcherIndex, bool> const& new_flip) const -> Show_command_pair
{
    auto& sheet = mSheets.at(mSheetNum);
    std::map<unsigned, bool> original_flip;
    for (auto&& index : new_flip) {
        original_flip[index.first] = sheet.GetMarcher(index.first).GetFlip();
    }
    auto action = [sheet_num = mSheetNum, new_flip](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        for (auto&& i : new_flip) {
            sheet.SetMarcherFlip(i.first, i.second);
        }
    };
    auto reaction = [sheet_num = mSheetNum, original_flip](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        for (auto&& i : original_flip) {
            sheet.SetMarcherFlip(i.first, i.second);
        }
    };
    return { action, reaction };
}

auto Show::Create_SetLabelRightCommand(bool right) const -> Show_command_pair
{
    std::map<MarcherIndex, bool> flips;
    for (auto&& i : mSelectionList) {
        flips[i] = right;
    }
    return Create_SetLabelFlipCommand(flips);
}

auto Show::Create_ToggleLabelFlipCommand() const -> Show_command_pair
{
    std::map<MarcherIndex, bool> flips;
    auto& sheet = mSheets.at(mSheetNum);
    for (auto&& i : mSelectionList) {
        flips[i] = !sheet.GetMarcher(i).GetFlip();
    }
    return Create_SetLabelFlipCommand(flips);
}

auto Show::Create_SetLabelVisiblityCommand(std::map<MarcherIndex, bool> const& new_visibility) const -> Show_command_pair
{
    auto& sheet = mSheets.at(mSheetNum);
    std::map<MarcherIndex, bool> original_visibility;
    for (auto&& index : new_visibility) {
        original_visibility[index.first] = sheet.GetMarcher(index.first).LabelIsVisible();
    }
    auto action = [sheet_num = mSheetNum, new_visibility](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        for (auto&& i : new_visibility) {
            sheet.SetMarcherLabelVisibility(i.first, i.second);
        }
    };
    auto reaction = [sheet_num = mSheetNum, original_visibility](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        for (auto&& i : original_visibility) {
            sheet.SetMarcherLabelVisibility(i.first, i.second);
        }
    };
    return { action, reaction };
}

auto Show::Create_SetLabelVisibleCommand(bool isVisible) const -> Show_command_pair
{
    std::map<MarcherIndex, bool> visible;
    for (auto&& i : mSelectionList) {
        visible[i] = isVisible;
    }
    return Create_SetLabelVisiblityCommand(visible);
}

auto Show::Create_ToggleLabelVisibilityCommand() const -> Show_command_pair
{
    std::map<MarcherIndex, bool> visible;
    auto& sheet = mSheets.at(mSheetNum);
    for (auto&& i : mSelectionList) {
        visible[i] = !sheet.GetMarcher(i).LabelIsVisible();
    }
    return Create_SetLabelVisiblityCommand(visible);
}

auto Show::Create_AddNewBackgroundImageCommand(ImageInfo const& image) const -> Show_command_pair
{
    auto& sheet = mSheets.at(mSheetNum);
    auto action = [sheet_num = mSheetNum, image, where = sheet.GetNumberBackgroundImages()](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        sheet.AddBackgroundImage(image, where);
    };
    auto reaction = [sheet_num = mSheetNum, where = sheet.GetNumberBackgroundImages()](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        sheet.RemoveBackgroundImage(where);
    };
    return { action, reaction };
}

auto Show::Create_RemoveBackgroundImageCommand(int which) const -> Show_command_pair
{
    auto& sheet = mSheets.at(mSheetNum);
    if (static_cast<size_t>(which) >= sheet.GetNumberBackgroundImages()) {
        return { [](Show&) { }, [](Show&) { } };
    }
    auto action = [sheet_num = mSheetNum, which](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        sheet.RemoveBackgroundImage(which);
    };
    auto reaction = [sheet_num = mSheetNum, image = sheet.GetBackgroundImages().at(which), which](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        sheet.AddBackgroundImage(image, which);
    };
    return { action, reaction };
}

auto Show::Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height) const -> Show_command_pair
{
    auto& sheet = mSheets.at(mSheetNum);
    auto [current_left, current_top, current_scaled_width, current_scaled_height] = sheet.GetBackgroundImageInfo(which);
    auto action = [sheet_num = mSheetNum, which, left, top, scaled_width, scaled_height](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        sheet.MoveBackgroundImage(which, left, top, scaled_width, scaled_height);
    };
    auto reaction = [sheet_num = mSheetNum, which, current_left, current_top, current_scaled_width, current_scaled_height](Show& show) {
        auto& sheet = show.mSheets.at(sheet_num);
        sheet.MoveBackgroundImage(which, current_left, current_top, current_scaled_width, current_scaled_height);
    };
    return { action, reaction };
}

auto Show::Create_AddSheetCurveCommand(CalChart::Curve const& curve) const -> Show_command_pair
{
    auto newIndex = mSheets.at(mSheetNum).GetNumberCurves();
    auto action = [sheet_num = mSheetNum, curve, newIndex](Show& show) {
        show.mSheets.at(sheet_num).AddCurve(curve, newIndex);
    };
    auto reaction = [sheet_num = mSheetNum, newIndex](Show& show) {
        show.mSheets.at(sheet_num).RemoveCurve(newIndex);
    };
    return { action, reaction };
}

auto Show::Create_ReplaceSheetCurveCommand(CalChart::Curve const& curve, int whichCurve) const -> Show_command_pair
{
    auto oldCurve = mSheets.at(mSheetNum).GetCurve(whichCurve);
    auto action = [sheet_num = mSheetNum, curve, whichCurve](Show& show) {
        show.mSheets.at(sheet_num).ReplaceCurve(curve, whichCurve);
    };
    auto reaction = [sheet_num = mSheetNum, oldCurve, whichCurve](Show& show) {
        show.mSheets.at(sheet_num).ReplaceCurve(oldCurve, whichCurve);
    };
    return { action, reaction };
}

auto Show::Create_RemoveSheetCurveCommand(int whichCurve) const -> Show_command_pair
{
    auto oldCurve = mSheets.at(mSheetNum).GetCurve(whichCurve);
    auto oldAssignments = mSheets.at(mSheetNum).GetCurveAssignments();
    auto action = [sheet_num = mSheetNum, whichCurve](Show& show) {
        show.mSheets.at(sheet_num).RemoveCurve(whichCurve);
    };
    auto reaction = [sheet_num = mSheetNum, oldCurve, whichCurve, oldAssignments](Show& show) {
        show.mSheets.at(sheet_num).AddCurve(oldCurve, whichCurve);
        show.mSheets.at(sheet_num).SetCurveAssignment(oldAssignments);
    };
    return { action, reaction };
}

// Accessors
// General show info
auto Show::GetNumSheets() const -> size_t { return mSheets.size(); }
auto Show::GetCurrentSheetNum() const -> size_t { return mSheetNum; }
auto Show::GetShowMode() const -> ShowMode const& { return mMode; }
auto Show::GetNumPoints() const -> size_t { return mDotLabelAndInstrument.size(); }
auto Show::GetCurrentReferencePoint() const -> int { return mCurrentReferencePoint; }

// Sheet copying
auto Show::CopySheet(size_t sheet) const -> Sheet { return mSheets.at(sheet); }
auto Show::CopySheets() const -> Sheet_container_t { return mSheets; }
auto Show::CopyCurrentSheet() const -> Sheet { return CopySheet(GetCurrentSheetNum()); }

// Sheet name
auto Show::GetSheetName(size_t sheet) const -> std::string
{
    return sheet < mSheets.size() ? mSheets.at(sheet).GetName() : "";
}
auto Show::GetSheetsName() const -> std::vector<std::string>
{
    return CalChart::Ranges::ToVector<std::string>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetName(); }));
}
auto Show::GetSheetNameOnCurrentSheet() const -> std::string { return GetSheetName(GetCurrentSheetNum()); }

// Sheet beats
auto Show::GetSheetBeats(size_t sheet) const -> CalChart::Beats
{
    return sheet < mSheets.size() ? mSheets.at(sheet).GetBeats() : 0;
}
auto Show::GetSheetsBeats() const -> std::vector<CalChart::Beats>
{
    return CalChart::Ranges::ToVector<CalChart::Beats>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetBeats(); }));
}
auto Show::GetSheetBeatsOnCurrentSheet() const -> CalChart::Beats { return GetSheetBeats(GetCurrentSheetNum()); }

// Sheet tempo
auto Show::GetSheetTempo(size_t sheet) const -> CalChart::Tempo
{
    return sheet < mSheets.size() ? mSheets.at(sheet).GetTempo() : 0;
}
auto Show::GetSheetsTempo() const -> std::vector<CalChart::Tempo>
{
    return CalChart::Ranges::ToVector<CalChart::Tempo>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetTempo(); }));
}
auto Show::GetSheetTempoOnCurrentSheet() const -> CalChart::Tempo { return GetSheetTempo(GetCurrentSheetNum()); }

// Sheet beat info
auto Show::GetSheetBeatInfo(size_t sheet) const -> CalChart::SheetBeatInfo
{
    return sheet < mSheets.size() ? mSheets.at(sheet).GetSheetBeatInfo() : CalChart::SheetBeatInfo{};
}
auto Show::GetSheetsBeatInfo() const -> std::vector<CalChart::SheetBeatInfo>
{
    return CalChart::Ranges::ToVector<CalChart::SheetBeatInfo>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetSheetBeatInfo(); }));
}
auto Show::GetSheetBeatInfoOnCurrentSheet() const -> CalChart::SheetBeatInfo { return GetSheetBeatInfo(GetCurrentSheetNum()); }

// Calculate downbeat times for all beats in the show
auto Show::GetDownbeatTimes() const -> std::vector<CalChart::Seconds>
{
    std::vector<CalChart::Seconds> downbeatTimes;
    CalChart::Seconds currentTime{ 0.0f };

    for (auto const& sheet : mSheets) {
        auto [tempo, fermatas] = sheet.GetSheetBeatInfo();
        auto numBeats = sheet.GetBeats();

        // Calculate the base duration per beat from tempo (BPM)
        auto baseBeatDuration = CalChart::Seconds{ 60.0f / tempo };

        for (CalChart::Beats beat = 0; beat < numBeats; ++beat) {
            // Record the start time of this beat
            downbeatTimes.push_back(currentTime);

            // Calculate the duration of this beat
            auto beatDuration = baseBeatDuration;

            // Add fermata if one exists for this beat
            if (fermatas.count(beat)) {
                beatDuration += fermatas.at(beat);
            }

            // Advance the current time
            currentTime += beatDuration;
        }
    }

    return downbeatTimes;
}

// Sheet symbols
auto Show::GetSheetSymbols(size_t sheet) const -> std::vector<SYMBOL_TYPE>
{
    return sheet < mSheets.size() ? mSheets.at(sheet).GetSymbols() : std::vector<SYMBOL_TYPE>{};
}
auto Show::GetSheetsSymbols() const -> std::vector<std::vector<SYMBOL_TYPE>>
{
    return CalChart::Ranges::ToVector<std::vector<SYMBOL_TYPE>>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetSymbols(); }));
}
auto Show::GetSheetSymbolsOnCurrentSheet() const -> std::vector<SYMBOL_TYPE> { return GetSheetSymbols(GetCurrentSheetNum()); }

// Sheet print number
auto Show::GetSheetPrintNumber(size_t sheet) const -> std::string
{
    return sheet < mSheets.size() ? mSheets.at(sheet).GetPrintNumber() : "";
}
auto Show::GetSheetsPrintNumber() const -> std::vector<std::string>
{
    return CalChart::Ranges::ToVector<std::string>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetPrintNumber(); }));
}
auto Show::GetSheetPrintNumberOnCurrentSheet() const -> std::string { return GetSheetPrintNumber(GetCurrentSheetNum()); }

// Sheet background images
auto Show::GetSheetBackgroundImages(size_t sheet) const -> std::vector<ImageInfo>
{
    return sheet < mSheets.size() ? mSheets.at(sheet).GetBackgroundImages() : std::vector<ImageInfo>{};
}
auto Show::GetSheetsBackgroundImages() const -> std::vector<std::vector<ImageInfo>>
{
    return CalChart::Ranges::ToVector<std::vector<ImageInfo>>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetBackgroundImages(); }));
}
auto Show::GetSheetBackgroundImagesOnCurrentSheet() const -> std::vector<ImageInfo> { return GetSheetBackgroundImages(GetCurrentSheetNum()); }

// Sheet serialized
auto Show::GetSheetSerialized(size_t sheet) const -> std::vector<std::byte>
{
    return sheet < mSheets.size() ? mSheets.at(sheet).SerializeSheet() : std::vector<std::byte>{};
}
auto Show::GetSheetsSerialized() const -> std::vector<std::vector<std::byte>>
{
    return CalChart::Ranges::ToVector<std::vector<std::byte>>(mSheets | std::views::transform([](auto&& sheet) { return sheet.SerializeSheet(); }));
}
auto Show::GetSheetSerializedOnCurrentSheet() const -> std::vector<std::byte> { return GetSheetSerialized(GetCurrentSheetNum()); }

// Continuities
auto Show::GetContinuities(size_t sheet) const -> std::vector<Continuity>
{
    return Ranges::ToVector<Continuity>(mSheets.at(sheet).GetContinuities());
}
auto Show::GetAllContinuities() const -> std::vector<std::vector<Continuity>>
{
    return CalChart::Ranges::ToVector<std::vector<Continuity>>(std::views::iota(size_t{ 0 }, mSheets.size()) | std::views::transform([this](auto&& sheet) { return GetContinuities(sheet); }));
}
auto Show::GetContinuitiesOnCurrentSheet() const -> std::vector<Continuity> { return GetContinuities(GetCurrentSheetNum()); }

// Continuities in use
auto Show::GetContinuitiesInUse(size_t sheet) const -> std::vector<bool>
{
    return Ranges::ToVector<bool>(mSheets.at(sheet).ContinuitiesInUse());
}
auto Show::GetAllContinuitiesInUse() const -> std::vector<std::vector<bool>>
{
    return CalChart::Ranges::ToVector<std::vector<bool>>(std::views::iota(size_t{ 0 }, mSheets.size()) | std::views::transform([this](auto&& sheet) { return GetContinuitiesInUse(sheet); }));
}
auto Show::GetContinuitiesInUseOnCurrentSheet() const -> std::vector<bool> { return GetContinuitiesInUse(GetCurrentSheetNum()); }

// Raw print continuity
auto Show::GetSheetRawPrintContinuity(size_t sheet) const -> std::string
{
    return mSheets.at(sheet).GetRawPrintContinuity();
}
auto Show::GetAllRawPrintContinuity() const -> std::vector<std::string>
{
    return CalChart::Ranges::ToVector<std::string>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetRawPrintContinuity(); }));
}
auto Show::GetSheetRawPrintContinuityOnCurrentSheet() const -> std::string { return GetSheetRawPrintContinuity(GetCurrentSheetNum()); }

// Print continuity
auto Show::GetSheetPrintContinuity(size_t sheet) const -> PrintContinuity
{
    return mSheets.at(sheet).GetPrintContinuity();
}
auto Show::GetAllPrintContinuity() const -> std::vector<PrintContinuity>
{
    return CalChart::Ranges::ToVector<PrintContinuity>(mSheets | std::views::transform([](auto&& sheet) { return sheet.GetPrintContinuity(); }));
}
auto Show::GetSheetPrintContinuityOnCurrentSheet() const -> PrintContinuity { return GetSheetPrintContinuity(GetCurrentSheetNum()); }

// Point label
auto Show::GetPointLabel(MarcherIndex i) const -> std::string
{
    if (i >= static_cast<MarcherIndex>(mDotLabelAndInstrument.size()))
        return "";
    return mDotLabelAndInstrument.at(i).first;
}
auto Show::GetPointsLabel() const -> std::vector<std::string>
{
    return CalChart::Ranges::ToVector<std::string>(mDotLabelAndInstrument | std::views::transform([](auto&& i) { return i.first; }));
}
auto Show::GetPointsLabel(const CalChart::SelectionList& sl) const -> std::vector<std::string>
{
    return CalChart::Ranges::ToVector<std::string>(sl | std::views::transform([this](auto&& i) { return GetPointLabel(i); }));
}

// Point instrument
auto Show::GetPointInstrument(MarcherIndex i) const -> std::string
{
    return static_cast<size_t>(i) < mDotLabelAndInstrument.size() ? mDotLabelAndInstrument.at(i).second : "";
}
auto Show::GetPointInstrument(std::string const& label) const -> std::optional<std::string>
{
    auto selection = MakeSelectByLabel(label);
    if (selection.size() == 0) {
        return std::nullopt;
    }
    return GetPointInstrument(*selection.begin());
}
auto Show::GetPointsInstrument() const -> std::vector<std::string>
{
    return CalChart::Ranges::ToVector<std::string>(mDotLabelAndInstrument | std::views::transform([](auto&& i) { return i.second; }));
}
auto Show::GetPointsInstrument(CalChart::SelectionList const& sl) const -> std::vector<std::string>
{
    return CalChart::Ranges::ToVector<std::string>(sl | std::views::transform([this](auto&& i) { return GetPointInstrument(i); }));
}

// Point symbol
auto Show::GetPointSymbol(size_t sheet, MarcherIndex i) const -> SYMBOL_TYPE
{
    return static_cast<size_t>(i) < mDotLabelAndInstrument.size() && sheet < mSheets.size() ? mSheets.at(sheet).GetSymbol(i) : SYMBOL_PLAIN;
}
auto Show::GetPointSymbolOnCurrentSheet(MarcherIndex i) const -> SYMBOL_TYPE { return GetPointSymbol(GetCurrentSheetNum(), i); }
auto Show::GetPointsSymbol(size_t sheet) const -> std::vector<SYMBOL_TYPE> { return mSheets.at(sheet).GetSymbols(); }
auto Show::GetPointsSymbolOnCurrentSheet() const -> std::vector<SYMBOL_TYPE> { return GetPointsSymbol(GetCurrentSheetNum()); }
auto Show::GetPointSymbol(size_t sheet, std::string const& label) const -> std::optional<SYMBOL_TYPE>
{
    auto selection = MakeSelectByLabel(label);
    if (selection.size() == 0) {
        return std::nullopt;
    }
    return GetPointSymbol(sheet, *selection.begin());
}
auto Show::GetPointSymbolOnCurrentSheet(std::string const& label) const -> std::optional<SYMBOL_TYPE> { return GetPointSymbol(GetCurrentSheetNum(), label); }
auto Show::GetPointsSymbol(size_t sheet, CalChart::SelectionList const& sl) const -> std::vector<SYMBOL_TYPE>
{
    return CalChart::Ranges::ToVector<SYMBOL_TYPE>(sl | std::views::transform([this, sheet](auto&& i) { return GetPointSymbol(sheet, i); }));
}
auto Show::GetPointsSymbolOnCurrentSheet(CalChart::SelectionList const& sl) const -> std::vector<SYMBOL_TYPE> { return GetPointsSymbol(GetCurrentSheetNum(), sl); }

// Point lookup
auto Show::GetPointFromLabel(std::string const& label) const -> std::optional<CalChart::MarcherIndex>
{
    if (auto it = std::find_if(
            mDotLabelAndInstrument.begin(),
            mDotLabelAndInstrument.end(),
            [&label](const auto& pair) { return pair.first == label; });
        it != mDotLabelAndInstrument.end()) {
        return static_cast<CalChart::MarcherIndex>(std::distance(mDotLabelAndInstrument.begin(), it));
    }
    return std::nullopt;
}
auto Show::GetPointsFromLabels(std::vector<std::string> const& labels) const -> std::vector<CalChart::MarcherIndex>
{
    return CalChart::Ranges::ToVector<CalChart::MarcherIndex>(
        labels
        | std::views::transform([this](const std::string& label) { return GetPointFromLabel(label); })
        | std::views::filter([](const std::optional<int>& opt) { return opt.has_value(); })
        | std::views::transform([](const std::optional<int>& opt) { return *opt; }));
}

// Marcher position
auto Show::GetMarcherPosition(size_t sheet, MarcherIndex i, unsigned ref) const -> Coord
{
    return mSheets.at(sheet).GetMarcherPosition(i, ref);
}
auto Show::GetMarcherPositionOnCurrentSheet(MarcherIndex i, unsigned ref) const -> Coord { return GetMarcherPosition(GetCurrentSheetNum(), i, ref); }
auto Show::GetAllMarcherPositions(size_t sheet, unsigned ref) const -> std::vector<Coord>
{
    return mSheets.at(sheet).GetAllMarcherPositions(ref);
}
auto Show::GetAllMarcherPositionsOnCurrentSheet(unsigned ref) const -> std::vector<Coord> { return GetAllMarcherPositions(GetCurrentSheetNum(), ref); }

// Find marcher
auto Show::FindMarcher(size_t sheet, Coord where, Coord::units searchBounds) const -> std::optional<MarcherIndex>
{
    return mSheets.at(sheet).FindMarcher(where, searchBounds);
}
auto Show::FindMarcherOnCurrentSheet(Coord where, Coord::units searchBounds) const -> std::optional<MarcherIndex> { return FindMarcher(GetCurrentSheetNum(), where, searchBounds); }

// Curve
auto Show::GetCurve(size_t sheet, size_t index) const -> Curve
{
    return mSheets.at(sheet).GetCurve(index);
}
auto Show::GetCurveOnCurrentSheet(size_t index) const -> Curve { return GetCurve(GetCurrentSheetNum(), index); }
auto Show::GetAllCurves(int sheet) const -> std::vector<Curve>
{
    return CalChart::Ranges::ToVector<Curve>(std::views::iota(size_t{ 0 }, GetNumberCurves(sheet)) | std::views::transform([this, sheet](auto i) { return GetCurve(sheet, i); }));
}
auto Show::GetAllCurvesOnCurrentSheet() const -> std::vector<Curve>
{
    auto sheetNum = GetCurrentSheetNum();
    return CalChart::Ranges::ToVector<Curve>(std::views::iota(size_t{ 0 }, GetNumberCurves(sheetNum)) | std::views::transform([this, sheetNum](auto i) { return GetCurve(sheetNum, i); }));
}

// Number of curves
auto Show::GetNumberCurves(size_t sheet) const -> size_t
{
    return mSheets.at(sheet).GetNumberCurves();
}
auto Show::GetNumberCurvesOnCurrentSheet() const -> size_t { return GetNumberCurves(GetCurrentSheetNum()); }

// Curve assignments
auto Show::GetCurveAssignments(size_t sheet) const -> std::vector<std::vector<MarcherIndex>>
{
    return mSheets.at(sheet).GetCurveAssignments();
}
auto Show::GetCurveAssignmentsOnCurrentSheet() const -> std::vector<std::vector<MarcherIndex>> { return GetCurveAssignments(GetCurrentSheetNum()); }

// Find curve control point
auto Show::FindCurveControlPoint(size_t sheet, CalChart::Coord pos, Coord::units searchBounds) const -> std::optional<std::tuple<size_t, size_t>>
{
    return mSheets.at(sheet).FindCurveControlPoint(pos, searchBounds);
}
auto Show::FindCurveControlPointOnCurrentSheet(CalChart::Coord pos, Coord::units searchBounds) const -> std::optional<std::tuple<size_t, size_t>> { return FindCurveControlPoint(GetCurrentSheetNum(), pos, searchBounds); }

// Find curve
auto Show::FindCurve(size_t sheet, CalChart::Coord pos, Coord::units searchBounds) const -> std::optional<std::tuple<size_t, size_t, double>>
{
    return mSheets.at(sheet).FindCurve(pos, searchBounds);
}
auto Show::FindCurveOnCurrentSheet(CalChart::Coord pos, Coord::units searchBounds) const -> std::optional<std::tuple<size_t, size_t, double>> { return FindCurve(GetCurrentSheetNum(), pos, searchBounds); }

}
