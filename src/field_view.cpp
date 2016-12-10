/*
 * field_view.cpp
 * Header for field view
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "field_view.h"
#include "field_canvas.h"
#include "field_frame.h"

#include "calchartapp.h"
#include "confgr.h"
#include "top_frame.h"
#include "animate.h"
#include "show_ui.h"
#include "cc_command.h"
#include "cc_shapes.h"
#include "setup_wizards.h"
#include "animation_frame.h"
#include "draw.h"
#include "animatecommand.h"
#include "cc_sheet.h"
#include "cc_drawcommand.h"
#include "modes.h"

#include <wx/wizard.h>
#include <wx/textfile.h>

IMPLEMENT_DYNAMIC_CLASS(FieldView, wxView)

FieldView::FieldView()
    : mFrame(NULL)
    , mDrawPaths(false)
    , mCurrentReferencePoint(0)
    , mConfig(CalChartConfiguration::GetGlobalConfig())
{
}

FieldView::~FieldView() {}

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool FieldView::OnCreate(wxDocument* doc, long WXUNUSED(flags))
{
    mShow = static_cast<CalChartDoc*>(doc);
    mShow->SetCurrentSheet(0);
#if defined(BUILD_FOR_VIEWER) && (BUILD_FOR_VIEWER != 0)
    mFrame = new AnimationFrame(
        NULL, doc, this,
        wxStaticCast(wxGetApp().GetTopWindow(), wxDocParentFrame));
#else
    mFrame = new FieldFrame(doc, this, mConfig,
        wxStaticCast(wxGetApp().GetTopWindow(), wxDocParentFrame),
        wxPoint(50, 50), wxSize(mConfig.Get_FieldFrameWidth(),
                                mConfig.Get_FieldFrameHeight()));
#endif

    mFrame->Show(true);
    Activate(true);
    return true;
}

// Sneakily gets used for default print/preview
// as well as drawing on the screen.
void FieldView::OnDraw(wxDC* dc)
{
    if (mShow) {
        // draw the field
        CC_coord origin = mShow->GetMode().Offset();
        DrawMode(*dc, mConfig, mShow->GetMode(), ShowMode_kFieldView);

        auto ghostSheet = mGhostModule.getGhostSheet(mShow, GetCurrentSheetNum());

        if (ghostSheet != nullptr) {
            DrawGhostSheet(*dc, mConfig, origin, SelectionList(),
                mShow->GetNumPoints(), mShow->GetPointLabels(),
                *ghostSheet, 0);
        }

        CC_show::const_CC_sheet_iterator_t sheet = mShow->GetCurrentSheet();
        if (sheet != mShow->GetSheetEnd()) {
            if (mCurrentReferencePoint > 0) {
                DrawPoints(*dc, mConfig, origin, mShow->GetSelectionList(),
                    mShow->GetNumPoints(), mShow->GetPointLabels(),
                    *mShow->GetCurrentSheet(), 0, false);
                DrawPoints(*dc, mConfig, origin, mShow->GetSelectionList(),
                    mShow->GetNumPoints(), mShow->GetPointLabels(),
                    *mShow->GetCurrentSheet(), mCurrentReferencePoint, true);
            }
            else {
                DrawPoints(*dc, mConfig, origin, mShow->GetSelectionList(),
                    mShow->GetNumPoints(), mShow->GetPointLabels(),
                    *mShow->GetCurrentSheet(), mCurrentReferencePoint, true);
            }
            DrawPaths(*dc, *sheet);
        }
    }
}

// Sneakily gets used for default print/preview
// as well as drawing on the screen.
void FieldView::DrawOtherPoints(wxDC& dc,
    const std::map<unsigned, CC_coord>& positions)
{
    DrawPhatomPoints(dc, mConfig, *mShow, *mShow->GetCurrentSheet(), positions);
}

void FieldView::OnUpdate(wxView* WXUNUSED(sender), wxObject* hint)
{
    if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_setup))) {
        // give our show a first page
        CalChartDoc* show = static_cast<CalChartDoc*>(GetDocument());

        // Set up everything else
        OnWizardSetup(*show);

        // make the show modified so it gets repainted
        show->Modify(true);
    }
    else if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_modified))) {
        GeneratePaths();
    }

    if (mFrame) {
        mFrame->UpdatePanel();
        mFrame->Refresh();
    }
}

// Clean up windows used for displaying the view.
bool FieldView::OnClose(bool deleteWindow)
{
    SetFrame((wxFrame*)NULL);

    Activate(false);

    if (!GetDocument()->Close())
        return false;

    if (deleteWindow) {
        delete mFrame;
    }
    return true;
}

void FieldView::OnWizardSetup(CalChartDoc& show)
{
    wxWizard* wizard = new wxWizard(mFrame, wxID_ANY, wxT("New Show Setup Wizard"));
    // page 1:
    // set the number of points and the labels
    ShowInfoReqWizard* page1 = new ShowInfoReqWizard(wizard);

    // page 2:
    // choose the show mode
    ChooseShowModeWizard* page2 = new ChooseShowModeWizard(wizard);

    // page 3:
    // and maybe a description
    SetDescriptionWizard* page3 = new SetDescriptionWizard(wizard);
    // page 4:

    wxWizardPageSimple::Chain(page1, page2);
    wxWizardPageSimple::Chain(page2, page3);

    wizard->GetPageAreaSizer()->Add(page1);
    if (wizard->RunWizard(page1)) {
        auto labels = page1->GetLabels();
		auto num = page1->GetNumberPoints();
		auto columns = page1->GetNumberColumns();
        std::vector<std::string> tlabels(labels.begin(), labels.end());
        auto newmode = wxGetApp().GetMode(page2->GetValue());
        auto descr = page3->GetValue().ToStdString();

        show.WizardSetupNewShow(num, columns, tlabels, std::move(newmode), descr);
    }
    else {
        wxMessageBox(wxT("Show setup not completed.\n")
                         wxT("You can change the number of marchers\n")
                             wxT("and show mode via the menu options"),
            wxT("Show not setup"), wxICON_INFORMATION | wxOK);
    }
    wizard->Destroy();
}

bool FieldView::DoRotatePointPositions(unsigned rotateAmount)
{
    if (mShow->GetSelectionList().size() == 0)
        return false;
	auto cmd = mShow->Create_RotatePointPositionsCommand(rotateAmount, mCurrentReferencePoint);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool FieldView::DoMovePoints(const std::map<unsigned, CC_coord>& newPositions)
{
    if (mShow->GetSelectionList().size() == 0 || !mShow->WillMovePoints(newPositions, mCurrentReferencePoint))
        return false;
	auto cmd = mShow->Create_MovePointsCommand(newPositions, mCurrentReferencePoint);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool FieldView::DoResetReferencePoint()
{
    if (mShow->GetSelectionList().size() == 0)
        return false;
	auto cmd = mShow->Create_SetReferencePointToRef0(mCurrentReferencePoint);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool FieldView::DoSetPointsSymbol(SYMBOL_TYPE sym)
{
    if (mShow->GetSelectionList().size() == 0)
        return false;
	auto cmd = mShow->Create_SetSymbolCommand(sym);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool FieldView::DoSetDescription(const wxString& descr)
{
	auto cmd = mShow->Create_SetDescriptionCommand(descr);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

void FieldView::DoSetMode(const wxString& mode)
{
	auto cmd = mShow->Create_SetModeCommand(mode);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void FieldView::DoSetShowInfo(unsigned numPoints, unsigned numColumns,
    const std::vector<wxString>& labels)
{
	auto cmd = mShow->Create_SetShowInfoCommand(numPoints, numColumns, labels);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void FieldView::DoSetSheetTitle(const wxString& descr)
{
	auto cmd = mShow->Create_SetSheetTitleCommand(descr);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

bool FieldView::DoSetSheetBeats(unsigned short beats)
{
	auto cmd = mShow->Create_SetSheetBeatsCommand(beats);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool FieldView::DoSetPointsLabel(bool right)
{
    if (mShow->GetSelectionList().size() == 0)
        return false;
	auto cmd = mShow->Create_SetLabelRightCommand(right);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool FieldView::DoSetPointsLabelFlip()
{
    if (mShow->GetSelectionList().size() == 0)
        return false;
	auto cmd = mShow->Create_ToggleLabelFlipCommand();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool FieldView::DoSetPointsLabelVisibility(bool isVisible)
{
    if (mShow->GetSelectionList().size() == 0)
        return false;
	auto cmd = mShow->Create_SetLabelVisibleCommand(isVisible);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool FieldView::DoTogglePointsLabelVisibility()
{
    if (mShow->GetSelectionList().size() == 0)
        return false;
	auto cmd = mShow->Create_ToggleLabelVisibilityCommand();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

void FieldView::DoInsertSheets(const CC_show::CC_sheet_container_t& sht,
    unsigned where)
{
	auto cmd = mShow->Create_AddSheetsCommand(sht, where);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

bool FieldView::DoDeleteSheet(unsigned where)
{
	auto cmd = mShow->Create_RemoveSheetCommand(where);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool FieldView::DoImportPrintableContinuity(const wxString& file)
{
    wxTextFile fp;
    fp.Open(file);
    if (!fp.IsOpened()) {
        return wxT("Unable to open file");
    }
    // read the file into a vector
    std::vector<std::string> lines;
    for (size_t line = 0; line < fp.GetLineCount(); ++line) {
        lines.push_back(fp.GetLine(line).ToStdString());
    }
    auto hasCont = mShow->AlreadyHasPrintContinuity();
    if (hasCont) {
        // prompt the user to find out if they would like to continue
        int userchoice = wxMessageBox(
            wxT("This show already has some Printable Continuity.")
                wxT("Would you like to continue Importing Printable Continuity and "
                    "overwrite it?"),
            wxT("Overwrite Printable Continuity?"), wxYES_NO | wxCANCEL);
        if (userchoice != wxYES) {
            return true;
        }
    }
    auto data = mShow->ImportPrintableContinuity(lines);
    if (data.first) {
        auto cmd = mShow->Create_SetPrintableContinuity(data.second);
        GetDocument()->GetCommandProcessor()->Submit(cmd.release());
        return true;
    }
    return false;
}

bool FieldView::DoRelabel()
{
    auto sheet_num = GetCurrentSheetNum();
    auto current_sheet = mShow->GetNthSheet(GetCurrentSheetNum());
    auto next_sheet = current_sheet + 1;
    // get a relabel mapping based on the current sheet.
    auto result = mShow->GetRelabelMapping(current_sheet, next_sheet);
    // check to see if there's a valid remapping
    if (!result.first) {
        return false;
    }
    // Apply remapping to the rest
    auto cmd = mShow->Create_ApplyRelabelMapping(sheet_num+1, result.second);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

// append is an insert with a relabel
std::pair<bool, std::string> FieldView::DoAppendShow(std::unique_ptr<CalChartDoc> other_show)
{
    if (other_show->GetNumPoints() != mShow->GetNumPoints()) {
        return { false, "The blocksize doesn't match" };
    }
    auto last_sheet = mShow->GetNthSheet(GetNumSheets()-1);
    auto next_sheet = other_show->GetSheetBegin();
    auto result = mShow->GetRelabelMapping(last_sheet, next_sheet);
    // check to see if there's a valid remapping
    if (!result.first) {
        return { false, "Last sheet doesn't match first sheet of other show" };
    }
	auto cmd = mShow->Create_AppendShow(std::move(other_show));
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return { true, "" };
}

int FieldView::FindPoint(CC_coord pos) const
{
    return mShow->GetCurrentSheet()->FindPoint(
        pos.x, pos.y, Float2Coord(mConfig.Get_DotRatio()),
        mCurrentReferencePoint);
}

CC_coord FieldView::PointPosition(int which) const
{
    return mShow->GetCurrentSheet()->GetPosition(which, mCurrentReferencePoint);
}

CC_coord FieldView::GetShowFieldOffset() const
{
    return mShow->GetMode().Offset();
}

CC_coord FieldView::GetShowFieldSize() const { return mShow->GetMode().Size(); }

void FieldView::GoToSheet(size_t which)
{
    if (which < mShow->GetNumSheets()) {
        // This *could* be run through a command or run directly...
        if (mConfig.Get_CommandUndoSetSheet()) {
            auto cmd = mShow->Create_SetCurrentSheetCommand(which);
            GetDocument()->GetCommandProcessor()->Submit(cmd.release());
        }
        else {
            mShow->SetCurrentSheet(which);
        }
    }
}

void FieldView::GoToNextSheet() { GoToSheet(mShow->GetCurrentSheetNum() + 1); }

void FieldView::GoToPrevSheet() { GoToSheet(mShow->GetCurrentSheetNum() - 1); }

void FieldView::SetReferencePoint(unsigned which)
{
    mCurrentReferencePoint = which;
    OnUpdate(this);
}

void FieldView::UnselectAll()
{
    SetSelection(mShow->MakeUnselectAll());
}

void FieldView::AddToSelection(const SelectionList& sl)
{
    SetSelection(mShow->MakeAddToSelection(sl));
}

void FieldView::ToggleSelection(const SelectionList& sl)
{
    SetSelection(mShow->MakeToggleSelection(sl));
}

// toggle selection means toggle it as selected to unselected
// otherwise, always select it
void FieldView::SelectWithLasso(const CC_lasso* lasso, bool toggleSelected)
{
    auto select = mShow->MakeSelectWithLasso(*lasso, mCurrentReferencePoint);
    if (toggleSelected) {
        select = mShow->MakeToggleSelection(select);
    }
    else {
        select = mShow->MakeAddToSelection(select);
    }
    SetSelection(select);
}

// Select points within rectangle
void FieldView::SelectPointsInRect(const CC_coord& c1, const CC_coord& c2,
    bool toggleSelected)
{
    CC_lasso lasso(c1);
    lasso.Append(CC_coord(c1.x, c2.y));
    lasso.Append(c2);
    lasso.Append(CC_coord(c2.x, c1.y));
    lasso.End();
    SelectWithLasso(&lasso, toggleSelected);
}

const SelectionList& FieldView::GetSelectionList()
{
    return mShow->GetSelectionList();
}

void FieldView::SetSelection(const SelectionList& sl)
{
    if (std::equal(mShow->GetSelectionList().begin(), mShow->GetSelectionList().end(), sl.begin(), sl.end()))
        return;
    // This *could* be run through a command or run directly...
    if (mConfig.Get_CommandUndoSelection()) {
        auto cmd = mShow->Create_SetSelectionCommand(sl);
        GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    }
    else {
        mShow->SetSelection(sl);
    }
}

void FieldView::OnEnableDrawPaths(bool enable)
{
    mDrawPaths = enable;
    if (mDrawPaths) {
        GeneratePaths();
    }
    mFrame->Refresh();
}

void FieldView::DrawPaths(wxDC& dc, const CC_sheet& sheet)
{
    if (mDrawPaths && mAnimation && mAnimation->GetNumberSheets() && (static_cast<unsigned>(mAnimation->GetNumberSheets()) > mShow->GetCurrentSheetNum())) {
        CC_coord origin = GetShowFieldOffset();
        mAnimation->GotoSheet(mShow->GetCurrentSheetNum());
        for (auto point = mShow->GetSelectionList().begin();
             point != mShow->GetSelectionList().end(); ++point) {
            DrawPath(dc, mConfig, mAnimation->GenPathToDraw(*point, origin),
                mAnimation->EndPosition(*point, origin));
        }
    }
}

void FieldView::GeneratePaths()
{
    mAnimation = mShow->NewAnimation(NotifyStatus(), NotifyErrorList());
}
