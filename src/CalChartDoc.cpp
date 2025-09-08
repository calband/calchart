/*
 * CalChartDoc.cpp
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
#include "CalChartDoc.h"
#include "CalChartAnimationErrors.h"
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartConstants.h"
#include "CalChartContinuity.h"
#include "CalChartDocCommand.h"
#include "CalChartPoint.h"
#include "CalChartPrintShowToPS.hpp"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "CalChartShowMode.h"
#include "CalChartUtils.h"
#include "ContinuityEditorPopup.h"
#include "SystemConfiguration.h"
#include "platconf.h"

#include <fstream>
#include <iomanip>
#include <wx/textfile.h>
#include <wx/wfstream.h>

using namespace CalChart;

IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_modified, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_FlushAllViews, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_FinishedLoading, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_setup, wxObject)

IMPLEMENT_DYNAMIC_CLASS(CalChartDoc, CalChartDoc::super);

// Create a new show
CalChartDoc::CalChartDoc()
    : mConfig{ wxCalChart::GetGlobalConfig() }
    , mShow(Show::Create(GetConfigShowMode(mConfig, std::get<0>(CalChart::kShowModeDefaultValues[0]))))
    , mTimer(*this)
{
    mTimer.Start(static_cast<int>(mConfig.Get_AutosaveInterval()) * 1000);
}

// When a file is opened, we first check to see if there is a temporary
// file, and if there is, prompt the user to see if they would like use
// that file instead.
bool CalChartDoc::OnOpenDocument(const wxString& filename)
{
    // first check to see if there is a recover file:
    wxString recoveryFile = TranslateNameToAutosaveName(filename);
    if (wxFileExists(recoveryFile)) {
        // prompt the user to find out if they would like to use the recovery file
        auto userchoice = wxMessageBox("CalChart has detected a recovery file (possibly from a previous crash).  Would you like to use the recovery file (Warning: choosing recover will destroy the original file)?", "Recovery File Detected", wxYES_NO | wxCANCEL);
        if (userchoice == wxYES) {
            // move the recovery file to the filename, destroying the file and using
            // the recovery
            wxCopyFile(recoveryFile, filename);
        }
        if (userchoice == wxCANCEL) {
            return false;
        }
    }
    bool success = super::OnOpenDocument(filename) && mShow;
    if (success) {
        // at this point the recover file is no longer useful.
        if (wxFileExists(recoveryFile)) {
            wxRemoveFile(recoveryFile);
        }
    }
    return success;
}

// If we close a file and decide not to save the changes, don't create a
// recovery
// file, it may confuse the user.
bool CalChartDoc::OnCloseDocument()
{
    bool success = super::OnCloseDocument();
    // first check to see if there is a recover file:
    wxString recoveryFile = TranslateNameToAutosaveName(GetFilename());
    if (!IsModified() && wxFileExists(recoveryFile)) {
        wxRemoveFile(recoveryFile);
    }
    return success;
}

bool CalChartDoc::OnNewDocument()
{
    bool success = super::OnNewDocument();
    if (success) {
        // notify the views that we are a new document.  That should prompt a wizard
        // to set up the show
        CalChartDoc_setup show_setup;
        UpdateAllViews(NULL, &show_setup);
    }
    return success;
}

// When we save a file, the recovery file should be removed to prevent
// a false detection that the file writing failed.
bool CalChartDoc::OnSaveDocument(wxString const& filename)
{
    bool result = super::OnSaveDocument(filename);
    wxString recoveryFile = TranslateNameToAutosaveName(filename);
    if (result && wxFileExists(recoveryFile)) {
        wxRemoveFile(recoveryFile);
    }
    return true;
}

auto CalChartDoc::ImportPrintableContinuity(std::vector<std::string> const& lines) const -> std::optional<std::map<int, std::pair<std::string, std::string>>>
{
    std::map<int, std::pair<std::string, std::string>> result;
    // should this first clear out all the continuity?
    if (lines.empty()) {
        return std::nullopt;
    }
    try {
        // check to make sure the first line starts with %%
        if ((lines.front().length() < 2) || !((lines.front().at(0) == '%') && (lines.front().at(1) == '%'))) {
            // Continuity doesn't begin with a sheet header
            throw std::runtime_error("Continuity file doesn't begin with header");
        }

        // first, split the lines into groups for each page
        auto sheet = 0;
        std::string number(lines.front(), 2);
        std::string current_print_cont;
        bool first_line = true;
        for (auto line = lines.begin() + 1; line != lines.end(); ++line) {
            // new sheet; push the current one into the map and reset it for the next
            // time
            if ((line->length() >= 2) && (line->at(0) == '%') && (line->at(1) == '%')) {
                result[sheet] = { number, current_print_cont };
                number = std::string(*line, 2);
                current_print_cont.clear();
                ++sheet;
                if (sheet >= GetNumSheets()) {
                    throw std::runtime_error("More print continuity than sheets!");
                }
                continue;
            }
            // we need to concatinate all the lines together, making sure to put a new
            // line between them.
            if (!first_line) {
                current_print_cont += "\n";
            }
            first_line = false;
            current_print_cont += *line;
        }
    } catch (const std::runtime_error& e) {
        wxString message = wxT("Error encountered:\n");
        message += e.what();
        wxMessageBox(message, wxT("Error!"));
        return std::nullopt;
    }
    return result;
}

template <typename T>
T& CalChartDoc::SaveObjectGeneric(T& stream)
{
    // flush out the text before we save a file.
    FlushAllTextWindows();
    return SaveObjectInternal(stream);
}

wxSTD ostream& CalChartDoc::SaveObject(wxSTD ostream& stream)
{
    return SaveObjectGeneric<wxSTD ostream>(stream);
}

template <typename T>
T& CalChartDoc::SaveObjectInternal(T& stream)
{
    auto data = mShow->SerializeShow();
    stream.write(reinterpret_cast<const char*>(&data[0]), data.size());
    return stream;
}

template <>
wxFFileOutputStream& CalChartDoc::SaveObjectInternal<wxFFileOutputStream>(
    wxFFileOutputStream& stream)
{
    auto data = mShow->SerializeShow();
    stream.Write(&data[0], data.size());
    return stream;
}

template <typename T>
T& CalChartDoc::LoadObjectGeneric(T& stream)
{
    // here's where we would put up the correction box
    bool modified = false;
    try {
        ParseErrorHandlers handlers = {
            [this, &modified](std::string const& description, std::string const& what, int line, int column) {
                wxString message = wxT("Error encountered when importing on parsing continuity:\n");
                message += description + "\n";
                message += "Please correct\n";
                wxMessageBox(message, wxT("Error!"));
                // if we got here, then the user is forced to make a change to their show.  We set it as modified
                modified = true;
                return ContinuityEditorPopup::ProcessEditContinuity(GetDocumentWindow(), description, what, line, column).ToStdString();
            },
            [](int majorVersion, int minorVersion) {
                wxString message;
                message.Printf(
                    "Warning: Current version of CalChart is older than show file.\n"
                    "Current Version %d.%d, show file: %d.%d.\n"
                    "Please consider upgrade to a newer version of CalChart by checking https://sourceforge.net/projects/calchart.\n"
                    "Continue trying to open this file?",
                    CC_MAJOR_VERSION,
                    CC_MINOR_VERSION,
                    majorVersion,
                    minorVersion);
                auto userchoice = wxMessageBox(message, wxT("Warning!"), wxYES_NO);
                // if we got here, then the user is forced to make a change to their show.  We set it as modified
                return userchoice == wxYES;
            },
        };
        mShow = Show::Create(GetConfigShowMode(mConfig, CalChart::GetShowModeNames()[0]), stream, &handlers);
    } catch (std::exception const& e) {
        wxString message = wxT("Error encountered:\n");
        message += e.what();
        wxMessageBox(message, wxT("Error!"));
        // if we got here, then the user is did not do edits, and the show is in weird state.  Don't force a save on exit.
        modified = false;
    }
    super::Modify(modified);
    mAnimation = Animation{ *mShow };
    CalChartDoc_FinishedLoading finishedLoading;
    UpdateAllViews(NULL, &finishedLoading);
    return stream;
}

wxSTD istream& CalChartDoc::LoadObject(wxSTD istream& stream)
{
    return LoadObjectGeneric<wxSTD istream>(stream);
}

bool CalChartDoc::exportViewerFile(wxString const& filepath)
{
    nlohmann::json j;

    j["meta"] = {
        { "version", "1.0.0" },
        { "index_name", "(MANUAL) Give a unique name for this show; this is effectively a filename, and won't be displayed to CalChart Online Viewer users (recommended format: show-name-year, e.g. taylor-swift-2016)" }, // TODO; for now, manually add index_name to viewer file after saving
        { "type", "viewer" },
    };

    j["show"] = mShow->toOnlineViewerJSON(Animation(*mShow));

    auto o = std::ofstream(filepath.ToStdString());
    o << std::setw(4) << j << std::endl;
    return true;
}

void CalChartDoc::FlushAllTextWindows()
{
    CalChartDoc_FlushAllViews flushMod;
    UpdateAllViews(NULL, &flushMod);
}

void CalChartDoc::Modify(bool b)
{
    super::Modify(b);
    CalChartDoc_modified showMod;
    // generate a new animation
    // uncomment below to see how long it takes to print
    //    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    mAnimation = Animation{ *mShow };
    //    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //    std::cout << "generation "
    //             << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    //             << "us.\n";

    UpdateAllViews(NULL, &showMod);
}

void CalChartDoc::AutoSaveTimer::Notify() { mShow.Autosave(); }

wxString CalChartDoc::TranslateNameToAutosaveName(const wxString& name)
{
    return name + wxT("~");
}

// When the timer goes off, and if the show has a name and is modified,
// we will write the file to a version of the file that the same
// but with the extension .shw~, to indicate that there is a recovery
// file at that location.
void CalChartDoc::Autosave()
{
    if (GetFilename() != wxT("") && IsModified()) {
        wxFFileOutputStream outputStream(
            TranslateNameToAutosaveName(GetFilename()));
        if (outputStream.IsOk()) {
            SaveObjectInternal(outputStream);
        }
        if (!outputStream.IsOk()) {
            wxMessageBox(wxT("Error creating recovery file.  Take heed, save often!"), wxT("Recovery Error"));
        }
    }
}

void CalChartDoc::SetCurrentSheet(int n)
{
    auto cmd = mShow->Create_SetCurrentSheetCommand(n);
    cmd.first(*mShow);
    UpdateAllViews();
}

auto CalChartDoc::GetAnimationErrors() const -> std::vector<CalChart::Animate::Errors>
{
    if (!mAnimation) {
        return {};
    }
    return mAnimation->GetErrors();
}

auto CalChartDoc::GetAnimationCollisions() const -> std::map<int, CalChart::SelectionList>
{
    if (!mAnimation) {
        return {};
    }
    return mAnimation->GetCollisions();
}

auto CalChartDoc::GenerateAnimationDrawCommands(
    CalChart::Beats whichBeat,
    bool drawCollisionWarning,
    std::optional<bool> onBeat,
    CalChart::Animation::AngleStepToImageFunction imageFunction) const -> std::vector<CalChart::Draw::DrawCommand>
{
    if (!mAnimation) {
        return {};
    }
    return mAnimation->GenerateDrawCommands(
        whichBeat,
        mShow->GetSelectionList(),
        mShow->GetShowMode(),
        GetConfiguration(),
        drawCollisionWarning,
        onBeat,
        imageFunction);
}

auto CalChartDoc::GetAnimationInfo(CalChart::Beats whichBeat, int which) const -> std::optional<CalChart::Animate::Info>
{
    if (!mAnimation) {
        return std::nullopt;
    }
    return mAnimation->GetAnimateInfo(whichBeat, which);
}

auto CalChartDoc::GetSelectedAnimationInfoWithDistanceFromPoint(CalChart::Beats whichBeat, CalChart::Coord origin) const -> std::multimap<double, CalChart::Animate::Info>
{
    if (!mAnimation) {
        return {};
    }
    if (mShow->GetSelectionList().empty()) {
        return mAnimation->GetAnimateInfoWithDistanceFromPoint(whichBeat, origin);
    }
    return mAnimation->GetAnimateInfoWithDistanceFromPoint(whichBeat, mShow->GetSelectionList(), origin);
}

auto CalChartDoc::GetTotalNumberAnimationBeats() const -> std::optional<CalChart::Beats>
{
    if (!mAnimation) {
        return std::nullopt;
    }
    return mAnimation->GetTotalNumberBeats();
}

// Return a bounding box of where the marchers are or the entire show.  If they are
// outside the show, we don't see them.
auto CalChartDoc::GetAnimationBoundingBox(bool zoomInOnMarchers, CalChart::Beats whichBeat) const -> std::pair<CalChart::Coord, CalChart::Coord>
{
    auto modeSize = mShow->GetShowMode().Size();
    if (!zoomInOnMarchers || !mAnimation) {
        return { modeSize, { 0, 0 } };
    }
    auto [bounding_box_upper_left, bounding_box_low_right] = mAnimation->GetBoundingBox(whichBeat);
    return { bounding_box_low_right - bounding_box_upper_left, (modeSize / 2) + bounding_box_upper_left };
}

auto CalChartDoc::BeatHasCollision(CalChart::Beats whichBeat) const -> bool
{
    if (!mAnimation) {
        return false;
    }
    return mAnimation->BeatHasCollision(whichBeat);
}

auto CalChartDoc::GetAnimationBeatForCurrentSheet() const -> CalChart::Beats
{
    if (!mAnimation) {
        return {};
    }
    return mAnimation->GetBeatForShowSheet(GetCurrentSheetNum());
}

namespace {
// Returns a view adaptor that will transform a range of point indices to the Path DrawCommands.
auto TransformIndexToDrawPathCommands(CalChart::Animation const& animation, unsigned whichSheet, CalChart::Coord::units endRadius)
{
    return std::views::transform([&animation, whichSheet, endRadius](int i) {
        return animation.GenPathToDraw(whichSheet, i, endRadius);
    })
        | std::views::join;
}

}

auto CalChartDoc::GenerateGhostPointsDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>
{
    if (const auto* ghostSheet = GetGhostSheet(); ghostSheet != nullptr) {
        return mShow->GenerateGhostPointsDrawCommands(
            GetConfiguration(),
            CalChart::SelectionList(),
            *ghostSheet);
    }
    return {};
}

auto CalChartDoc::GenerateCurrentSheetPointsDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>
{
    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};
    auto& config = GetConfiguration();
    auto origin = GetShowFieldOffset();
    CalChart::append(drawCmds, CalChart::CreateModeDrawCommandsWithBorderOffset(config, GetShowMode(), CalChart::HowToDraw::FieldView));
    CalChart::append(drawCmds, GenerateGhostPointsDrawCommands());
    auto sheet = GetCurrentSheet();
    if (sheet == GetSheetEnd()) {
        return drawCmds;
    }
    CalChart::append(drawCmds, mShow->GenerateSheetElements(config, GetCurrentReferencePoint()));
    CalChart::append(drawCmds, GeneratePathsDrawCommands());
    return drawCmds + origin;
}

auto CalChartDoc::GeneratePhatomPointsDrawCommands(CalChart::MarcherToPosition const& positions) const -> std::vector<CalChart::Draw::DrawCommand>
{
    auto sheet = GetCurrentSheet();
    auto pointLabelFont = CalChart::Font{ CalChart::Float2CoordUnits(mConfig.Get_DotRatio() * mConfig.Get_NumRatio()) };

    auto drawCmds = positions
        | std::views::transform([this, sheet](auto&& whichPosition) {
              auto [which, position] = whichPosition;
              // because points draw their position, we remove it then add the new position.
              return sheet->GetMarcher(which).GetDrawCommands(
                         GetPointLabel(which),
                         mConfig)
                  + position
                  - sheet->GetMarcher(which).GetPos();
          })
        | std::views::join;
    return {
        CalChart::Draw::withFont(
            pointLabelFont,
            CalChart::Draw::withBrushAndPen(
                mConfig.Get_CalChartBrushAndPen(CalChart::Colors::GHOST_POINT),
                CalChart::Draw::withTextForeground(
                    mConfig.Get_CalChartBrushAndPen(CalChart::Colors::GHOST_POINT_TEXT),
                    drawCmds)))
    };
}

auto CalChartDoc::GeneratePathsDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>
{
    auto& config = GetConfiguration();
    if (!GetDrawPaths()) {
        return {};
    }
    if (!mAnimation || mAnimation->GetNumberSheets() == 0 || (static_cast<int>(mAnimation->GetNumberSheets()) <= GetCurrentSheetNum())) {
        return {};
    }
    auto endRadius = CalChart::Float2CoordUnits(config.Get_DotRatio()) / 2;
    auto currentSheet = GetCurrentSheetNum();
    return {
        CalChart::Draw::withBrushAndPen(
            config.Get_CalChartBrushAndPen(CalChart::Colors::PATHS),
            mShow->GetSelectionList()
                | TransformIndexToDrawPathCommands(*mAnimation, currentSheet, endRadius))
    };
}

void CalChartDoc::WizardSetupNewShow(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int columns, ShowMode const& newmode)
{
    mShow = Show::Create(newmode, labelsAndInstruments, columns);
    UpdateAllViews();
}

auto CalChartDoc::GetRelabelMapping(std::vector<CalChart::Coord> const& source_marchers, std::vector<CalChart::Coord> const& target_marchers) const -> std::optional<std::vector<CalChart::MarcherIndex>>
{
    return mShow->GetRelabelMapping(source_marchers, target_marchers, CalChart::Float2CoordUnits(mConfig.Get_DotRatio()));
}

void CalChartDoc::SetSelectionList(SelectionList const& sl)
{
    auto cmd = mShow->Create_SetSelectionListCommand(sl);
    cmd.first(*mShow);
    UpdateAllViews();
}

auto CalChartDoc::GetSelectedPoints() const -> CalChart::MarcherToPosition
{
    CalChart::MarcherToPosition result;
    for (auto i : GetSelectionList()) {
        result[i] = GetCurrentSheet()->GetMarcherPosition(i, GetCurrentReferencePoint());
    }
    return result;
}

void CalChartDoc::SetSelect(CalChart::Select select)
{
    mSelect = select;
    UpdateAllViews();
}

void CalChartDoc::SetCurrentReferencePoint(int currentReferencePoint)
{
    mCurrentReferencePoint = currentReferencePoint;
    UpdateAllViews();
}

auto CalChartDoc::FindMarcher(CalChart::Coord pos) const -> std::optional<CalChart::MarcherIndex>
{
    return GetCurrentSheet()->FindMarcher(pos, CalChart::Float2CoordUnits(mConfig.Get_DotRatio()), GetCurrentReferencePoint());
}

auto CalChartDoc::FindCurveControlPoint(CalChart::Coord pos) const -> std::optional<std::tuple<size_t, size_t>>
{
    return GetCurrentSheet()->FindCurveControlPoint(pos, CalChart::Float2CoordUnits(mConfig.Get_ControlPointRatio()));
}

auto CalChartDoc::FindCurve(CalChart::Coord pos) const -> std::optional<std::tuple<size_t, size_t, double>>
{
    return GetCurrentSheet()->FindCurve(pos, CalChart::Float2CoordUnits(mConfig.Get_ControlPointRatio()));
}

void CalChartDoc::SetDrawPaths(bool drawPaths)
{
    mDrawPaths = drawPaths;
    UpdateAllViews();
}

void CalChartDoc::SetDrawBackground(bool drawBackground)
{
    mDrawBackground = drawBackground;
    UpdateAllViews();
}

void CalChartDoc::SetCurrentMove(CalChart::MoveMode move)
{
    mCurrentMove = move;
    UpdateAllViews();
}

auto CalChartDoc::GetGhostSheet() const -> CalChart::Sheet const*
{
    auto currentSheet = GetCurrentSheetNum();
    if (!GetGhostModuleIsActive()) {
        return nullptr;
    }
    auto targetSheet = (mGhostSource == GhostSource::next) ? currentSheet + 1 : (mGhostSource == GhostSource::previous) ? currentSheet - 1
                                                                                                                        : mGhostSheet;
    if (targetSheet >= 0 && targetSheet < GetNumSheets()) {
        return &(*(GetNthSheet(targetSheet)));
    }
    return nullptr;
}

void CalChartDoc::SetGhostSource(GhostSource source, int which)
{
    mGhostSource = source;
    mGhostSheet = which;
    UpdateAllViews();
}

const ShowMode& CalChartDoc::GetShowMode() const { return mShow->GetShowMode(); }

auto CalChartDoc::PrintToPS(bool overview, int min_yards, std::set<size_t> const& isPicked, CalChart::Configuration const& config_) const -> std::tuple<std::string, int>
{
    auto doLandscape = config_.Get_PrintPSLandscape();
    auto doCont = config_.Get_PrintPSDoCont();
    auto doContSheet = config_.Get_PrintPSDoContSheet();

    PrintShowToPS printShowToPS(*mShow, doLandscape, doCont, doContSheet, overview, min_yards, GetShowMode(), config_);
    return printShowToPS(isPicked, GetTitle().ToStdString());
}

// CalChartDocCommand consist of the action to perform, and the reverse action to undo.
// Essentially these are lambdas that capture what needs to be applied.

CalChartDocCommand::CC_doc_command_pair CalChartDoc::Inject_CalChartDocArg(Show_command_pair show_cmds)
{
    auto action = [cmd = show_cmds.first](CalChartDoc& doc) { cmd(*doc.mShow); };
    auto reaction = [cmd = show_cmds.second](CalChartDoc& doc) { cmd(*doc.mShow); };
    return { action, reaction };
}

// This will create an array of actions where the first one is to set the sheet
std::vector<CalChartDocCommand::CC_doc_command_pair> CalChartDoc::Create_SetSheetPair() const
{
    return { Inject_CalChartDocArg(mShow->Create_SetCurrentSheetCommand(mShow->GetCurrentSheetNum())) };
}

// This will create an array of actions where the first one is to set the sheet
std::vector<CalChartDocCommand::CC_doc_command_pair> CalChartDoc::Create_SetSheetAndSelectionPair() const
{
    auto SetSheet_cmds = Create_SetSheetPair();
    SetSheet_cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetSelectionListCommand(mShow->GetSelectionList())));
    return SetSheet_cmds;
}

// This will create an array of actions where the first one is to set the sheet and the second is to set the selection

std::unique_ptr<wxCommand> CalChartDoc::Create_SetCurrentSheetCommand(int n)
{
    auto show_cmds = Inject_CalChartDocArg(mShow->Create_SetCurrentSheetCommand(n));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Current Sheet"), show_cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetSelectionListCommand(const SelectionList& sl)
{
    auto show_cmds = Inject_CalChartDocArg(mShow->Create_SetSelectionListCommand(sl));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Selection"), show_cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetCurrentSheetAndSelectionCommand(int n, const SelectionList& sl)
{
    auto show_cmds = Inject_CalChartDocArg(mShow->Create_SetCurrentSheetAndSelectionCommand(n, sl));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Current Sheet and Selection"), show_cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetShowModeCommand(CalChart::ShowMode const& newmode)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetShowModeCommand(newmode)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Mode"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetupMarchersCommand(std::vector<std::pair<std::string, std::string>> const& labelsAndInstruments, int numColumns)
{
    auto tlabels = std::vector(labelsAndInstruments.begin(), labelsAndInstruments.end());
    auto show_cmds = Inject_CalChartDocArg(mShow->Create_SetupMarchersCommand(tlabels, numColumns, GetShowMode().FieldOffset()));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set show info"), show_cmds);
}

auto CalChartDoc::Create_SetInstrumentsCommand(std::map<CalChart::MarcherIndex, std::string> const& dotToInstrument) -> std::unique_ptr<wxCommand>
{
    auto show_cmds = Inject_CalChartDocArg(mShow->Create_SetInstrumentsCommand(dotToInstrument));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set instruments"), show_cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetSheetTitleCommand(const wxString& newname)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetSheetTitleCommand(std::string(newname))));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set title"), cmds);
}

auto CalChartDoc::Create_SetSheetBeatsCommand(CalChart::Beats beats) -> std::unique_ptr<wxCommand>
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetSheetBeatsCommand(beats)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set beats"), cmds);
}

auto CalChartDoc::Create_AddSheetsCommand(Show::Sheet_container_t const& sheets, int where) -> std::unique_ptr<wxCommand>
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_AddSheetsCommand(sheets, where)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Adding Sheets"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_RemoveSheetCommand(int where)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_RemoveSheetCommand(where)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Removing Sheet"), cmds);
}

auto CalChartDoc::Create_ApplyRelabelMapping(int sheet, std::vector<MarcherIndex> const& mapping) -> std::unique_ptr<wxCommand>
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_ApplyRelabelMapping(sheet, mapping)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Point Remapping"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_AppendShow(std::unique_ptr<CalChartDoc> other_show)
{
    auto currend = mShow->GetNumSheets();
    auto source_marchers = mShow->GetAllMarcherPositions(currend - 1);
    auto target_marchers = other_show->GetAllMarcherPositions(0);
    auto result = GetRelabelMapping(source_marchers, target_marchers);
    if (!result.has_value()) {
        // No way to remap, throw an error
        throw std::runtime_error("No remap information");
    }

    // create a command to run on the other show.  This is how to apply changes
    // yes, this is weird.  Create a command and then immediately run it on the copy.  This is to apply the remap.
    other_show->mShow->Create_ApplyRelabelMapping(0, *result).first(*other_show->mShow);

    // get the sheets we want to append
    auto sheets = Show::Sheet_container_t(other_show->GetSheetBegin(), other_show->GetSheetEnd());

    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_AddSheetsCommand(sheets, currend)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Append Show"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string>> const& data)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetPrintableContinuity(data)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Continuity"), cmds);
}

auto CalChartDoc::Create_MovePointsCommand(CalChart::MarcherToPosition const& new_positions) -> std::unique_ptr<wxCommand>
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_MovePointsCommand(new_positions, mCurrentReferencePoint)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Move Points"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_MovePointsCommand(unsigned whichSheet, CalChart::MarcherToPosition const& new_positions)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_MovePointsCommand(whichSheet, new_positions, mCurrentReferencePoint)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Move Points"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_AssignPointsToCurve(size_t whichCurve, std::vector<MarcherIndex> whichMarchers)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_AssignPointsToCurve(whichCurve, whichMarchers)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Assign to Curve"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_DeletePointsCommand()
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_DeletePointsCommand()));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Delete Points"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_RotatePointPositionsCommand(int rotateAmount)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_RotatePointPositionsCommand(rotateAmount, mCurrentReferencePoint)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Rotate Points"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_ResetReferencePointToRef0()
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_ResetReferencePointToRef0(mCurrentReferencePoint)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Reset Reference Point"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetSymbolCommand(SYMBOL_TYPE sym)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetSymbolCommand(sym)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Setting Continuity Symbol"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetContinuityCommand(SYMBOL_TYPE i, CalChart::Continuity const& new_cont)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetContinuityCommand(i, new_cont)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Setting Continuity"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetLabelRightCommand(bool right)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetLabelRightCommand(right)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Setting Labels"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_ToggleLabelFlipCommand()
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_ToggleLabelFlipCommand()));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Setting Labels"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetLabelVisibleCommand(bool isVisible)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetLabelVisibleCommand(isVisible)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Setting Label Visibility"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_ToggleLabelVisibilityCommand()
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_ToggleLabelVisibilityCommand()));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Setting Label Visibility"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_AddNewBackgroundImageCommand(ImageInfo const& image)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_AddNewBackgroundImageCommand(image)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Adding Background Image"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_RemoveBackgroundImageCommand(int which)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_RemoveBackgroundImageCommand(which)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Removing Background Image"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_MoveBackgroundImageCommand(which, left, top, scaled_width, scaled_height)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Moving Background Image"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetTransitionCommand(const std::vector<Coord>& finalPositions, const std::map<SYMBOL_TYPE, std::string>& continuities, const std::vector<SYMBOL_TYPE>& marcherDotTypes)
{
    CalChart::MarcherToPosition positionAssignments;

    for (MarcherIndex marcher = 0; marcher < finalPositions.size(); marcher++) {
        positionAssignments[marcher] = finalPositions.at(marcher);
    }

    auto cmds = Create_SetSheetAndSelectionPair();

    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_MovePointsCommand(GetCurrentSheetNum() + 1, positionAssignments, 0)));

    for (auto contIter = continuities.begin(); contIter != continuities.end(); contIter++) {
        cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetContinuityCommand(contIter->first, CalChart::Continuity{ contIter->second })));
    }

    std::set<SYMBOL_TYPE> processedSymbols;
    for (unsigned firstMarcherWithSymbol = 0; firstMarcherWithSymbol < marcherDotTypes.size(); firstMarcherWithSymbol++) {
        SelectionList marchersWithSymbol;
        SYMBOL_TYPE symbolToProcess;

        symbolToProcess = marcherDotTypes.at(firstMarcherWithSymbol);

        if (processedSymbols.find(symbolToProcess) != processedSymbols.end()) {
            continue; // Skip any symbols that we've already processed
        } else {
            processedSymbols.insert(symbolToProcess);
        }

        for (unsigned marcher = firstMarcherWithSymbol; marcher < marcherDotTypes.size(); marcher++) {
            if (marcherDotTypes.at(marcher) == symbolToProcess) {
                marchersWithSymbol.insert(marcher);
            }
        }

        cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetSymbolCommand(marchersWithSymbol, symbolToProcess)));
    }

    return std::make_unique<CalChartDocCommand>(*this, wxT("Setting Transition"), cmds);
}

auto CalChartDoc::Create_AddSheetCurveCommand(CalChart::Curve const& curve) -> std::unique_ptr<wxCommand>
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_AddSheetCurveCommand(curve)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Adding Curve"), cmds);
}

auto CalChartDoc::Create_ReplaceSheetCurveCommand(CalChart::Curve const& curve, int whichCurve) -> std::unique_ptr<wxCommand>
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_ReplaceSheetCurveCommand(curve, whichCurve)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Updating Curve"), cmds);
}

auto CalChartDoc::Create_RemoveSheetCurveCommand(int whichCurve) -> std::unique_ptr<wxCommand>
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_RemoveSheetCurveCommand(whichCurve)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Removing Curve"), cmds);
}
