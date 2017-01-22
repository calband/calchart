/*
 * CalChartDoc.cpp
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

#include <fstream>

#include "CalChartDoc.h"

#include "cc_command.h"
#include "confgr.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "math_utils.h"
#include "cc_shapes.h"
#include "platconf.h"
#include "draw.h"
#include "cc_fileformat.h"
#include "modes.h"
#include "print_ps.h"
#include "json.h"
#include "json_export.h"

#include <wx/wfstream.h>
#include <wx/textfile.h>
#include <list>

IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_modified, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_FlushAllViews, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_FinishedLoading, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_setup, wxObject)

IMPLEMENT_DYNAMIC_CLASS(CalChartDoc, wxDocument);

// Create a new show
CalChartDoc::CalChartDoc()
    : mShow(CC_show::Create_CC_show())
    , mMode(wxGetApp().GetMode(kShowModeStrings[0]))
    , mTimer(*this)
{
    mTimer.Start(CalChartConfiguration::GetGlobalConfig().Get_AutosaveInterval() * 1000);
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
        int userchoice = wxMessageBox(
            wxT("CalChart has detected a recovery file (possibly from a previous "
                "crash).  ") wxT("Would you like to use the recovery file "
                                 "(Warning: choosing recover will ")
                wxT("destroy the original file)?"),
            wxT("Recovery File Detected"), wxYES_NO | wxCANCEL);
        if (userchoice == wxYES) {
            // move the recovery file to the filename, destroying the file and using
            // the recovery
            wxCopyFile(recoveryFile, filename);
        }
        if (userchoice == wxNO) {
        }
        if (userchoice == wxCANCEL) {
            return false;
        }
    }
    bool success = wxDocument::OnOpenDocument(filename) && mShow;
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
    bool success = wxDocument::OnCloseDocument();
    // first check to see if there is a recover file:
    wxString recoveryFile = TranslateNameToAutosaveName(GetFilename());
    if (!IsModified() && wxFileExists(recoveryFile)) {
        wxRemoveFile(recoveryFile);
    }
    return success;
}

bool CalChartDoc::OnNewDocument()
{
    bool success = wxDocument::OnNewDocument();
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
bool CalChartDoc::OnSaveDocument(const wxString& filename)
{
    bool result = wxDocument::OnSaveDocument(filename);
    wxString recoveryFile = TranslateNameToAutosaveName(filename);
    if (result && wxFileExists(recoveryFile)) {
        wxRemoveFile(recoveryFile);
    }
    return true;
}

// Destroy a show
CalChartDoc::~CalChartDoc() {}

std::pair<bool, std::map<unsigned, std::pair<std::string, std::string> > > CalChartDoc::ImportPrintableContinuity(
    const std::vector<std::string>& lines) const
{
    std::map<unsigned, std::pair<std::string, std::string> > result;
    // should this first clear out all the continuity?
    if (lines.empty()) {
        return { false, {} }; // done, technically
    }
    try {
        // check to make sure the first line starts with %%
        if ((lines.front().length() < 2) || !((lines.front().at(0) == '%') && (lines.front().at(1) == '%'))) {
            // Continuity doesn't begin with a sheet header
            throw std::runtime_error("Continuity file doesn't begin with header");
        }

        // first, split the lines into groups for each page
        unsigned sheet = 0;
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
    }
    catch (const std::runtime_error& e) {
        wxString message = wxT("Error encountered:\n");
        message += e.what();
        wxMessageBox(message, wxT("Error!"));
        return { false, {} };
    }
    return { true, result };
}

template <typename T>
T& CalChartDoc::SaveObjectGeneric(T& stream)
{
    // flush out the text before we save a file.
    FlushAllTextWindows();
    return SaveObjectInternal(stream);
}

#if wxUSE_STD_IOSTREAM
wxSTD ostream& CalChartDoc::SaveObject(wxSTD ostream& stream)
{
    return SaveObjectGeneric<wxSTD ostream>(stream);
}
#else
wxOutputStream& CalChartDoc::SaveObject(wxOutputStream& stream)
{
    return SaveObjectGeneric<wxOutputStream>(stream);
}
#endif

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
    try {
        mShow = CC_show::Create_CC_show(stream);
    }
    catch (CC_FileException& e) {
        wxString message = wxT("Error encountered:\n");
        message += e.what();
        wxMessageBox(message, wxT("Error!"));
    }
    CalChartDoc_FinishedLoading finishedLoading;
    UpdateAllViews(NULL, &finishedLoading);
    return stream;
}

#if wxUSE_STD_IOSTREAM
wxSTD istream& CalChartDoc::LoadObject(wxSTD istream& stream)
{
    return LoadObjectGeneric<wxSTD istream>(stream);
}
#else
wxInputStream& CalChartDoc::LoadObject(wxInputStream& stream)
{
    return LoadObjectGeneric<wxInputStream>(stream);
}
#endif

bool CalChartDoc::exportViewerFile(const wxString& filepath)
{
    JSONElement mainObject = JSONElement::makeObject();

    JSONDataObjectAccessor mainObjectAccessor = mainObject;

    mainObjectAccessor["meta"] = JSONElement::makeObject();
    mainObjectAccessor["show"] = JSONElement::makeNull();

    JSONDataObjectAccessor metaObjectAccessor = mainObjectAccessor["meta"];
    metaObjectAccessor["version"] = ("1.0.0");
    metaObjectAccessor["index_name"] = "MANUAL"; // TODO; for now, manually add index_name to viewer file after saving
    metaObjectAccessor["type"] = "viewer";

    mShow->toOnlineViewerJSON(mainObjectAccessor["show"], Animation(*mShow, nullptr, nullptr));

    return JSONExporter::exportJSON(filepath.ToStdString(), mainObject);
}

void CalChartDoc::FlushAllTextWindows()
{
    CalChartDoc_FlushAllViews flushMod;
    UpdateAllViews(NULL, &flushMod);
}

const std::string& CalChartDoc::GetDescr() const { return mShow->GetDescr(); }

void CalChartDoc::Modify(bool b)
{
    wxDocument::Modify(b);
    CalChartDoc_modified showMod;
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
            wxMessageBox(wxT("Error creating recovery file.  Take heed, save often!"),
                wxT("Recovery Error"));
        }
    }
}

CalChartDoc::const_CC_sheet_iterator_t
CalChartDoc::GetNthSheet(unsigned n) const
{
    return static_cast<CC_show const&>(*mShow).GetNthSheet(n);
}

void CalChartDoc::SetCurrentSheet(unsigned n)
{
    auto cmd = mShow->Create_SetCurrentSheetCommand(n);
    cmd.first(*mShow);
    UpdateAllViews();
}

CalChartDoc::const_CC_sheet_iterator_t CalChartDoc::GetCurrentSheet() const
{
    return static_cast<CC_show const&>(*mShow).GetCurrentSheet();
}

unsigned short CalChartDoc::GetNumSheets() const
{
    return mShow->GetNumSheets();
}

CalChartDoc::const_CC_sheet_iterator_t CalChartDoc::GetSheetBegin() const
{
    return static_cast<CC_show const&>(*mShow).GetSheetBegin();
}

CalChartDoc::const_CC_sheet_iterator_t CalChartDoc::GetSheetEnd() const
{
    return static_cast<CC_show const&>(*mShow).GetSheetEnd();
}

unsigned CalChartDoc::GetCurrentSheetNum() const
{
    return static_cast<CC_show const&>(*mShow).GetCurrentSheetNum();
}

std::unique_ptr<Animation>
CalChartDoc::NewAnimation(NotifyStatus notifyStatus,
    NotifyErrorList notifyErrorList)
{
    return std::unique_ptr<Animation>(
        new Animation(*mShow, notifyStatus, notifyErrorList));
}

void CalChartDoc::WizardSetupNewShow(unsigned num, unsigned columns, std::vector<std::string> const& labels, std::unique_ptr<ShowMode> newmode)
{
    SetMode(std::move(newmode));
    mShow = CC_show::Create_CC_show(num, columns, labels, mMode->FieldOffset());
    UpdateAllViews();
}

unsigned short CalChartDoc::GetNumPoints() const
{
    return mShow->GetNumPoints();
}

std::pair<bool, std::vector<size_t> > CalChartDoc::GetRelabelMapping(const_CC_sheet_iterator_t source_sheet, const_CC_sheet_iterator_t target_sheets) const
{
    return mShow->GetRelabelMapping(source_sheet, target_sheets);
}

std::string CalChartDoc::GetPointLabel(unsigned i) const
{
    return mShow->GetPointLabel(i);
}

const std::vector<std::string>& CalChartDoc::GetPointLabels() const
{
    return mShow->GetPointLabels();
}

SelectionList CalChartDoc::MakeSelectAll() const
{
    return mShow->MakeSelectAll();
}

SelectionList CalChartDoc::MakeUnselectAll() const
{
    return mShow->MakeUnselectAll();
}

SelectionList CalChartDoc::MakeAddToSelection(const SelectionList& sl) const
{
    return mShow->MakeAddToSelection(sl);
}

SelectionList CalChartDoc::MakeRemoveFromSelection(const SelectionList& sl) const
{
    return mShow->MakeRemoveFromSelection(sl);
}

SelectionList CalChartDoc::MakeToggleSelection(const SelectionList& sl) const
{
    return mShow->MakeToggleSelection(sl);
}

SelectionList CalChartDoc::MakeSelectWithLasso(const CC_lasso& lasso, unsigned ref) const
{
    return mShow->MakeSelectWithLasso(lasso, ref);
}

bool CalChartDoc::IsSelected(unsigned i) const
{
    return mShow->IsSelected(i);
}

void CalChartDoc::SetSelection(const SelectionList& sl)
{
    auto cmd = mShow->Create_SetSelectionCommand(sl);
    cmd.first(*mShow);
    UpdateAllViews();
}

const SelectionList& CalChartDoc::GetSelectionList() const
{
    return mShow->GetSelectionList();
}

const ShowMode& CalChartDoc::GetMode() const { return *mMode; }

void CalChartDoc::SetMode(std::unique_ptr<const ShowMode> m)
{
    if (!m) {
        throw std::runtime_error("Cannot use NULL ShowMode");
    }
    std::swap(mMode, m);
    mShow->SetMode(mMode.get());
    UpdateAllViews();
}

CC_show CalChartDoc::ShowSnapShot() const { return *mShow.get(); }

void CalChartDoc::RestoreSnapShot(const CC_show& snapshot)
{
    mShow.reset(new CC_show(snapshot));
}

bool CalChartDoc::AlreadyHasPrintContinuity() const
{
    return mShow->AlreadyHasPrintContinuity();
}

bool CalChartDoc::WillMovePoints(std::map<unsigned, CC_coord> const& new_positions, unsigned ref) const
{
    return mShow->WillMovePoints(new_positions, ref);
}

int CalChartDoc::PrintToPS(std::ostream& buffer, bool eps, bool overview,
    int min_yards, const std::set<size_t>& isPicked,
    const CalChartConfiguration& config_) const
{
    auto doLandscape = config_.Get_PrintPSLandscape();
    auto doCont = config_.Get_PrintPSDoCont();
    auto doContSheet = config_.Get_PrintPSDoContSheet();

    PrintShowToPS printShowToPS(
        *mShow, doLandscape, doCont, doContSheet, overview, min_yards, GetMode(),
        { { config_.Get_HeadFont().ToStdString(),
            config_.Get_MainFont().ToStdString(),
            config_.Get_NumberFont().ToStdString(),
            config_.Get_ContFont().ToStdString(),
            config_.Get_BoldFont().ToStdString(),
            config_.Get_ItalFont().ToStdString(),
            config_.Get_BoldItalFont().ToStdString() } },
        config_.Get_PageWidth(), config_.Get_PageHeight(),
        config_.Get_PageOffsetX(), config_.Get_PageOffsetY(),
        config_.Get_PaperLength(), config_.Get_HeaderSize(),
        config_.Get_YardsSize(), config_.Get_TextSize(), config_.Get_DotRatio(),
        config_.Get_NumRatio(), config_.Get_PLineRatio(),
        config_.Get_SLineRatio(), config_.Get_ContRatio(),
        [&config_](size_t which) {
            return config_.Get_yard_text(which).ToStdString();
        },
        [&config_](size_t which) {
            return config_.Get_spr_line_text(which).ToStdString();
        });
    return printShowToPS(buffer, eps, mShow->GetCurrentSheetNum(), isPicked,
        GetTitle().ToStdString());
}

// CalChartDocCommand consist of the action to perform, and the reverse action to undo.
// Essentially these are lambdas that capture what needs to be applied.

CalChartDocCommand::CC_doc_command_pair CalChartDoc::Inject_CalChartDocArg(CC_show_command_pair show_cmds)
{
    auto action = [cmd = show_cmds.first](CalChartDoc & doc) { cmd(*doc.mShow); };
    auto reaction = [cmd = show_cmds.second](CalChartDoc & doc) { cmd(*doc.mShow); };
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
    SetSheet_cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetSelectionCommand(mShow->GetSelectionList())));
    return SetSheet_cmds;
}

// This will create an array of actions where the first one is to set the sheet and the second is to set the selection

std::unique_ptr<wxCommand> CalChartDoc::Create_SetDescriptionCommand(const wxString& newdescr)
{
    auto show_cmds = Inject_CalChartDocArg(mShow->Create_SetDescriptionCommand(newdescr.ToStdString()));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Description"), show_cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetCurrentSheetCommand(int n)
{
    auto show_cmds = Inject_CalChartDocArg(mShow->Create_SetCurrentSheetCommand(n));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Current Sheet"), show_cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetSelectionCommand(const SelectionList& sl)
{
    auto show_cmds = Inject_CalChartDocArg(mShow->Create_SetSelectionCommand(sl));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Selection"), show_cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetModeCommand(const wxString& newmode)
{
    auto action = [mode = newmode](CalChartDoc & doc)
    {
        auto newmode = wxGetApp().GetMode(mode);
        if (newmode) {
            doc.SetMode(std::move(newmode));
        }
    };
    auto reaction = [mode = mMode->GetName()](CalChartDoc & doc)
    {
        auto newmode = wxGetApp().GetMode(mode);
        if (newmode) {
            doc.SetMode(std::move(newmode));
        }
    };
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Mode"), CalChartDocCommand::CC_doc_command_pair{ action, reaction });
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetShowInfoCommand(unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels)
{
    auto tlabels = std::vector<std::string>(labels.begin(), labels.end());
    auto show_cmds = Inject_CalChartDocArg(mShow->Create_SetShowInfoCommand(numPoints, numColumns, tlabels, mMode->FieldOffset()));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set show info"), show_cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetSheetTitleCommand(const wxString& newname)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetSheetTitleCommand(std::string(newname))));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set title"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetSheetBeatsCommand(unsigned short beats)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetSheetBeatsCommand(beats)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set beats"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_AddSheetsCommand(const CC_show::CC_sheet_container_t& sheets, unsigned where)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_AddSheetsCommand(sheets, where)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Adding Sheets"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_RemoveSheetCommand(unsigned where)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_RemoveSheetCommand(where)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Removing Sheet"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_ApplyRelabelMapping(unsigned sheet, std::vector<size_t> const& mapping)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_ApplyRelabelMapping(sheet, mapping)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Point Remapping"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_AppendShow(std::unique_ptr<CalChartDoc> other_show)
{
    auto currend = mShow->GetNumSheets();
    auto last_sheet = static_cast<CC_show const&>(*mShow).GetNthSheet(currend - 1);
    auto next_sheet = other_show->GetSheetBegin();
    auto result = mShow->GetRelabelMapping(last_sheet, next_sheet);

    // create a command to run on the other show.  This is how to apply changes
    // yes, this is weird.  Create a command and then immediately run it on the copy.  This is to apply the remap.
    other_show->mShow->Create_ApplyRelabelMapping(0, result.second).first(*other_show->mShow);

    // get the sheets we want to append
    auto sheets = CC_show::CC_sheet_container_t(other_show->GetSheetBegin(), other_show->GetSheetEnd());

    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_AddSheetsCommand(sheets, currend)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Append Show"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetPrintableContinuity(std::map<unsigned, std::pair<std::string, std::string> > const& data)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetPrintableContinuity(data)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Set Continuity"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_MovePointsCommand(std::map<unsigned, CC_coord> const& new_positions, unsigned ref)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_MovePointsCommand(new_positions, ref)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Move Points"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_RotatePointPositionsCommand(unsigned rotateAmount, unsigned ref)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_RotatePointPositionsCommand(rotateAmount, ref)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Rotate Points"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetReferencePointToRef0(unsigned ref)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetReferencePointToRef0(ref)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Reset Reference Point"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetSymbolCommand(SYMBOL_TYPE sym)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetSymbolCommand(sym)));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Setting Continuity Symbol"), cmds);
}

std::unique_ptr<wxCommand> CalChartDoc::Create_SetContinuityTextCommand(SYMBOL_TYPE i, const wxString& text)
{
    auto cmds = Create_SetSheetAndSelectionPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_SetContinuityTextCommand(i, text.ToStdString())));
    return std::make_unique<CalChartDocCommand>(*this, wxT("Setting Continuity Text"), cmds);
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

std::unique_ptr<wxCommand> CalChartDoc::Create_AddNewBackgroundImageCommand(int left, int top, int image_width, int image_height, std::vector<unsigned char> const& data, std::vector<unsigned char> const& alpha)
{
    auto cmds = Create_SetSheetPair();
    cmds.emplace_back(Inject_CalChartDocArg(mShow->Create_AddNewBackgroundImageCommand(calchart_core::ImageData{ left, top, image_width, image_height, image_width, image_height, data, alpha })));
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
