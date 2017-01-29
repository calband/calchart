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

#include "animate.h"
#include "cc_show.h"

#include <wx/wx.h> // For basic wx defines
#include <wx/docview.h> // For basic wx defines
#include <wx/cmdproc.h>

#include <memory>
#include <vector>
#include <set>

class CC_sheet;
class ShowMode;
class ShowUndoList;
class CC_show;
class CC_lasso;
class Animation;
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
// This holds the CC_show, the core part of CalChart.
class CalChartDoc : public wxDocument {
    DECLARE_DYNAMIC_CLASS(CalChartDoc)
public:
    typedef std::vector<CC_sheet> CC_sheet_container_t;
    typedef CC_sheet_container_t::iterator CC_sheet_iterator_t;
    typedef CC_sheet_container_t::const_iterator const_CC_sheet_iterator_t;

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
    std::pair<bool, std::map<unsigned, std::pair<std::string, std::string> > > ImportPrintableContinuity(const std::vector<std::string>& lines) const;

    void FlushAllTextWindows();

    std::unique_ptr<Animation> NewAnimation(NotifyStatus notifyStatus,
        NotifyErrorList notifyErrorList);
    void WizardSetupNewShow(unsigned num, unsigned columns, std::vector<std::string> const& labels, std::unique_ptr<ShowMode> newmode);

    std::string GetDescr() const;

    unsigned short GetNumSheets() const;

    const_CC_sheet_iterator_t GetSheetBegin() const;
    const_CC_sheet_iterator_t GetSheetEnd() const;
    const_CC_sheet_iterator_t GetNthSheet(unsigned n) const;
    const_CC_sheet_iterator_t GetCurrentSheet() const;

    unsigned GetCurrentSheetNum() const;
    void SetCurrentSheet(unsigned n);

    unsigned short GetNumPoints() const;
    std::pair<bool, std::vector<size_t> > GetRelabelMapping(const_CC_sheet_iterator_t source_sheet, const_CC_sheet_iterator_t target_sheets) const;

    std::string GetPointLabel(unsigned i) const;
    const std::vector<std::string>& GetPointLabels() const;

    // how to select points
    // Utility functions for constructing new selection lists
    // Then you push the selection list with the Create_SetSelectionCommand
    SelectionList MakeSelectAll() const;
    SelectionList MakeUnselectAll() const;
    SelectionList MakeAddToSelection(const SelectionList& sl) const;
    SelectionList MakeRemoveFromSelection(const SelectionList& sl) const;
    SelectionList MakeToggleSelection(const SelectionList& sl) const;
    SelectionList MakeSelectWithLasso(const CC_lasso& lasso, unsigned ref) const;

    void SetSelection(const SelectionList& sl);

    bool IsSelected(unsigned i) const;
    const SelectionList& GetSelectionList() const;

    const ShowMode& GetMode() const;
    void SetMode(std::unique_ptr<const ShowMode> m);

    bool AlreadyHasPrintContinuity() const;
    bool WillMovePoints(std::map<unsigned, CC_coord> const& new_positions, unsigned ref) const;
    int PrintToPS(std::ostream& buffer, bool eps, bool overview, int min_yards,
        const std::set<size_t>& isPicked,
        const CalChartConfiguration& config_) const;

    // create a set of commands to apply to the document.  This is the best way to interact with the doc.
    std::unique_ptr<wxCommand> Create_SetDescriptionCommand(const wxString& newdescr);
    std::unique_ptr<wxCommand> Create_SetCurrentSheetCommand(int n);
    std::unique_ptr<wxCommand> Create_SetSelectionCommand(const SelectionList& sl);
    std::unique_ptr<wxCommand> Create_SetModeCommand(const wxString& newmode);
    std::unique_ptr<wxCommand> Create_SetShowInfoCommand(unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels);
    std::unique_ptr<wxCommand> Create_SetSheetTitleCommand(const wxString& newname);
    std::unique_ptr<wxCommand> Create_SetSheetBeatsCommand(unsigned short beats);
    std::unique_ptr<wxCommand> Create_AddSheetsCommand(const CC_show::CC_sheet_container_t& sheets, unsigned where);
    std::unique_ptr<wxCommand> Create_RemoveSheetCommand(unsigned where);
    std::unique_ptr<wxCommand> Create_ApplyRelabelMapping(unsigned sheet, std::vector<size_t> const& mapping);
    std::unique_ptr<wxCommand> Create_AppendShow(std::unique_ptr<CalChartDoc> sheets);
    std::unique_ptr<wxCommand> Create_SetPrintableContinuity(std::map<unsigned, std::pair<std::string, std::string> > const& data);
    std::unique_ptr<wxCommand> Create_MovePointsCommand(std::map<unsigned, CC_coord> const& new_positions, unsigned ref);
    std::unique_ptr<wxCommand> Create_RotatePointPositionsCommand(unsigned rotateAmount, unsigned ref);
    std::unique_ptr<wxCommand> Create_SetReferencePointToRef0(unsigned ref);
    std::unique_ptr<wxCommand> Create_SetSymbolCommand(SYMBOL_TYPE sym);
    std::unique_ptr<wxCommand> Create_SetContinuityTextCommand(SYMBOL_TYPE i, const wxString& text);
    std::unique_ptr<wxCommand> Create_SetLabelRightCommand(bool right);
    std::unique_ptr<wxCommand> Create_ToggleLabelFlipCommand();
    std::unique_ptr<wxCommand> Create_SetLabelVisibleCommand(bool isVisible);
    std::unique_ptr<wxCommand> Create_ToggleLabelVisibilityCommand();
    std::unique_ptr<wxCommand> Create_AddNewBackgroundImageCommand(int left, int top, int image_width, int image_height, std::vector<unsigned char> const& data, std::vector<unsigned char> const& alpha);
    std::unique_ptr<wxCommand> Create_RemoveBackgroundImageCommand(int which);
    std::unique_ptr<wxCommand> Create_MoveBackgroundImageCommand(int which, int left, int top, int scaled_width, int scaled_height);

private:
    static CC_doc_command_pair Inject_CalChartDocArg(CC_show_command_pair);
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

    friend class BasicCalChartCommand;
    CC_show ShowSnapShot() const;
    void RestoreSnapShot(const CC_show& snapshot);

    std::unique_ptr<CC_show> mShow;
    std::unique_ptr<const ShowMode> mMode;
    AutoSaveTimer mTimer;
};
