/*
 * CalChartDoc.h
 * Definitions for the wxDoc for calchart shows
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

#include "cc_coord.h"
#include "cc_show.h"

#include <wx/cmdproc.h>
#include <wx/docview.h> // For basic wx defines
#include <wx/wx.h> // For basic wx defines

#include <memory>
#include <set>
#include <vector>

class CalChartDoc;
namespace CalChart {
class ShowMode;
class Show;
class Sheet;
class Lasso;
class Animation;
}
class CalChartConfiguration;

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
    DECLARE_DYNAMIC_CLASS(CalChartDoc)
public:
    CalChartDoc();
    virtual ~CalChartDoc();

    // Override the wxDocument functions:
    // Need to override OnOpenDoc so we can report errors, handle recovery file
    virtual bool OnOpenDocument(const wxString& filename);
    // Need to override OnOpenDoc so we can report errors, handle recovery file
    virtual bool OnCloseDocument();
    // Need to override OnNewDoc so we can start the setup wizard
    virtual bool OnNewDocument();
    // Need to override OnSaveDoc so we can handle recovery files
    virtual bool OnSaveDocument(const wxString& filename);
    // Update the views that the doc been modified
    virtual void Modify(bool b);

// How we save and load a show:
#if wxUSE_STD_IOSTREAM
    virtual wxSTD ostream& SaveObject(wxSTD ostream& stream);
    virtual wxSTD istream& LoadObject(wxSTD istream& stream);
#else
    virtual wxOutputStream& SaveObject(wxOutputStream& stream);
    virtual wxInputStream& LoadObject(wxInputStream& stream);
#endif

    /*!
     * @brief Exports the show to a file that can be animated by
     * the CalChart Online Viewer.
     * @param filepath The filepath (relative or absolute) to which the
     * Viewer file will be saved. Note that the CalChart Online Viewer
     * expects a viewer file to end with the '.viewer' extension.
     * @return True if the file was saved successfully; false otherwise.
     */
    bool exportViewerFile(const wxString& filepath);

private:
    template <typename T>
    T& LoadObjectGeneric(T& stream);
    template <typename T>
    T& SaveObjectGeneric(T& stream);
    template <typename T>
    T& SaveObjectInternal(T& stream);

public:
    // translates input into a mapping of which sheet to number,lines pair.
    std::pair<bool, std::map<int, std::pair<std::string, std::string>>> ImportPrintableContinuity(const std::vector<std::string>& lines) const;

    void FlushAllTextWindows();

    std::unique_ptr<CalChart::Animation> NewAnimation(CalChart::NotifyStatus notifyStatus,
        CalChart::NotifyErrorList notifyErrorList);
    void WizardSetupNewShow(std::vector<std::string> const& labels, int columns, CalChart::ShowMode const& newmode);

    auto GetNumSheets() const { return mShow->GetNumSheets(); }

    auto GetSheetBegin() const { return static_cast<CalChart::Show const&>(*mShow).GetSheetBegin(); }
    auto GetSheetEnd() const { return static_cast<CalChart::Show const&>(*mShow).GetSheetEnd(); }
    auto GetNthSheet(int n) const { return static_cast<CalChart::Show const&>(*mShow).GetNthSheet(n); }
    auto GetCurrentSheet() const { return static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet(); }

    auto GetCurrentSheetNum() const { return static_cast<CalChart::Show const&>(*mShow).GetCurrentSheetNum(); }
    void SetCurrentSheet(int n);

    auto GetNumPoints() const { return mShow->GetNumPoints(); }
    std::pair<bool, std::vector<size_t>> GetRelabelMapping(CalChart::Show::const_Sheet_iterator_t source_sheet, CalChart::Show::const_Sheet_iterator_t target_sheets, CalChart::Coord::units tolerance) const;

    auto GetPointLabel(int i) const { return mShow->GetPointLabel(i); }

    auto GetPointLabels() const { return mShow->GetPointLabels(); }

    // how to select points
    // Utility functions for constructing new selection lists
    // Then you push the selection list with the Create_SetSelectionCommand
    auto MakeSelectAll() const { return mShow->MakeSelectAll(); }
    auto MakeUnselectAll() const { return mShow->MakeUnselectAll(); }
    auto MakeAddToSelection(const SelectionList& sl) const { return mShow->MakeAddToSelection(sl); }
    auto MakeRemoveFromSelection(const SelectionList& sl) const { return mShow->MakeRemoveFromSelection(sl); }
    auto MakeToggleSelection(const SelectionList& sl) const { return mShow->MakeToggleSelection(sl); }
    auto MakeSelectWithLasso(const CalChart::Lasso& lasso, int ref) const { return mShow->MakeSelectWithLasso(lasso, ref); }

    void SetSelection(const SelectionList& sl);

    auto IsSelected(int i) const { return mShow->IsSelected(i); }
    auto GetSelectionList() const { return mShow->GetSelectionList(); }

    const CalChart::ShowMode& GetShowMode() const;

    auto AlreadyHasPrintContinuity() const { return mShow->AlreadyHasPrintContinuity(); }
    auto WillMovePoints(std::map<int, CalChart::Coord> const& new_positions, int ref) const { return mShow->WillMovePoints(new_positions, ref); }
    int PrintToPS(std::ostream& buffer, bool overview, int min_yards,
        const std::set<size_t>& isPicked,
        const CalChartConfiguration& config_) const;

    // create a set of commands to apply to the document.  This is the best way to interact with the doc.
    std::unique_ptr<wxCommand> Create_SetCurrentSheetCommand(int n);
    std::unique_ptr<wxCommand> Create_SetSelectionCommand(const SelectionList& sl);
    std::unique_ptr<wxCommand> Create_SetShowModeCommand(CalChart::ShowMode const& newmode);
    std::unique_ptr<wxCommand> Create_SetShowInfoCommand(std::vector<wxString> const& labels, int numColumns);
    std::unique_ptr<wxCommand> Create_SetSheetTitleCommand(const wxString& newname);
    std::unique_ptr<wxCommand> Create_SetSheetBeatsCommand(int beats);
    std::unique_ptr<wxCommand> Create_AddSheetsCommand(const CalChart::Show::Sheet_container_t& sheets, int where);
    std::unique_ptr<wxCommand> Create_RemoveSheetCommand(int where);
    std::unique_ptr<wxCommand> Create_ApplyRelabelMapping(int sheet, std::vector<size_t> const& mapping);
    std::unique_ptr<wxCommand> Create_AppendShow(std::unique_ptr<CalChartDoc> sheets, CalChart::Coord::units tolerance);
    std::unique_ptr<wxCommand> Create_SetPrintableContinuity(std::map<int, std::pair<std::string, std::string>> const& data);
    std::unique_ptr<wxCommand> Create_MovePointsCommand(std::map<int, CalChart::Coord> const& new_positions, int ref);
    std::unique_ptr<wxCommand> Create_MovePointsCommand(unsigned whichSheet, std::map<int, CalChart::Coord> const& new_positions, int ref);
    std::unique_ptr<wxCommand> Create_DeletePointsCommand();
    std::unique_ptr<wxCommand> Create_RotatePointPositionsCommand(int rotateAmount, int ref);
    std::unique_ptr<wxCommand> Create_SetReferencePointToRef0(int ref);
    std::unique_ptr<wxCommand> Create_SetSymbolCommand(SYMBOL_TYPE sym);
    std::unique_ptr<wxCommand> Create_SetContinuityCommand(SYMBOL_TYPE i, CalChart::Continuity const& new_cont);
    std::unique_ptr<wxCommand> Create_SetLabelRightCommand(bool right);
    std::unique_ptr<wxCommand> Create_ToggleLabelFlipCommand();
    std::unique_ptr<wxCommand> Create_SetLabelVisibleCommand(bool isVisible);
    std::unique_ptr<wxCommand> Create_ToggleLabelVisibilityCommand();
    std::unique_ptr<wxCommand> Create_AddNewBackgroundImageCommand(int left, int top, int image_width, int image_height, std::vector<unsigned char> const& data, std::vector<unsigned char> const& alpha);
    std::unique_ptr<wxCommand> Create_RemoveBackgroundImageCommand(int which);
    std::unique_ptr<wxCommand> Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height);
    std::unique_ptr<wxCommand> Create_SetTransitionCommand(const std::vector<CalChart::Coord>& finalPositions, const std::map<SYMBOL_TYPE, std::string>& continuities, const std::vector<SYMBOL_TYPE>& marcherDotTypes);

private:
    static CC_doc_command_pair Inject_CalChartDocArg(CalChart::Show_command_pair);
    std::vector<CC_doc_command_pair> Create_SetSheetPair() const;
    std::vector<CC_doc_command_pair> Create_SetSheetAndSelectionPair() const;

private:
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
        void Notify();

    private:
        CalChartDoc& mShow;
    };

    std::unique_ptr<CalChart::Show> mShow;
    AutoSaveTimer mTimer;
};
