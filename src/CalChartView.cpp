/*
 * CalChartView.h
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

#include "CalChartView.h"
#include "BackgroundImages.h"
#include "CalChartAnimation.h"
#include "CalChartAnimationCommand.h"
#include "CalChartAnimationErrors.h"
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartDocCommand.h"
#include "CalChartDrawCommand.h"
#include "CalChartFrame.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "FieldCanvas.h"
#include "SetupInstruments.h"
#include "SetupMarchers.h"
#include "ShowModeWizard.h"
#include "draw.h"

#include <memory>
#include <wx/docview.h>
#include <wx/textfile.h>
#include <wx/wizard.h>

IMPLEMENT_DYNAMIC_CLASS(CalChartView, wxView)

CalChartView::CalChartView()
    : mConfig(CalChartConfiguration::GetGlobalConfig())
{
}

// What to do when a view is created. Creates actual windows for displaying the view.
bool CalChartView::OnCreate(wxDocument* doc, long WXUNUSED(flags))
{
    mShow = static_cast<CalChartDoc*>(doc);
    mShow->SetCurrentSheet(0);
    mFrame = new CalChartFrame(doc, this, mConfig,
        wxStaticCast(wxGetApp().GetTopWindow(), wxDocParentFrame),
        wxPoint(mConfig.Get_FieldFramePositionX(), mConfig.Get_FieldFramePositionY()), wxSize(static_cast<int>(mConfig.Get_FieldFrameWidth()), static_cast<int>(mConfig.Get_FieldFrameHeight())));

    UpdateBackgroundImages();
    mFrame->Show(true);
    Activate(true);
    return true;
}

// Sneakily gets used for default print/preview as well as drawing on the screen.
void CalChartView::OnDraw(wxDC* dc)
{
    if (mShow) {
        // draw the field
        auto origin = mShow->GetShowMode().Offset();
        DrawMode(*dc, mConfig, mShow->GetShowMode(), ShowMode_kFieldView);

        auto ghostSheet = mShow->GetGhostSheet(GetCurrentSheetNum());

        if (ghostSheet != nullptr) {
            DrawGhostSheet(*dc, mConfig, origin, CalChart::SelectionList(),
                mShow->GetNumPoints(), mShow->GetPointsLabel(),
                *ghostSheet, 0);
        }

        auto sheet = mShow->GetCurrentSheet();
        if (sheet != mShow->GetSheetEnd()) {
            if (mShow->GetCurrentReferencePoint() > 0) {
                DrawPoints(*dc, mConfig, origin, mShow->GetSelectionList(),
                    mShow->GetNumPoints(), mShow->GetPointsLabel(),
                    *mShow->GetCurrentSheet(), 0, false);
                DrawPoints(*dc, mConfig, origin, mShow->GetSelectionList(),
                    mShow->GetNumPoints(), mShow->GetPointsLabel(),
                    *mShow->GetCurrentSheet(), mShow->GetCurrentReferencePoint(), true);
            } else {
                DrawPoints(*dc, mConfig, origin, mShow->GetSelectionList(),
                    mShow->GetNumPoints(), mShow->GetPointsLabel(),
                    *mShow->GetCurrentSheet(), mShow->GetCurrentReferencePoint(), true);
            }
            DrawPaths(*dc);
        }
    }
}

void CalChartView::DrawUncommitedMovePoints(wxDC& dc, std::map<int, CalChart::Coord> const& positions)
{
    DrawPhatomPoints(dc, mConfig, *mShow, *mShow->GetCurrentSheet(), positions);
}

void CalChartView::OnDrawBackground(wxDC& dc)
{
    if (!mShow->GetDrawBackground()) {
        return;
    }
    mBackgroundImages.OnPaint(dc);
}

void CalChartView::OnUpdate(wxView* WXUNUSED(sender), wxObject* hint)
{
    if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_setup))) {
        // give our show a first page
        CalChartDoc* show = static_cast<CalChartDoc*>(GetDocument());

        // Set up everything else
        OnWizardSetup(*show);

        // make the show modified so it gets repainted
        show->Modify(true);
    }
    UpdateBackgroundImages();

    if (mFrame) {
        mFrame->OnUpdate();
        mFrame->Refresh();
    }
}

// Clean up windows used for displaying the view.
bool CalChartView::OnClose(bool deleteWindow)
{
    if (!wxView::OnClose(deleteWindow)) {
        return false;
    }
    if (!GetDocument()->Close()) {
        return false;
    }

    Activate(false);

    // notify the frame to clean things up.
    mFrame->OnClose();

    if (deleteWindow) {
        GetFrame()->Destroy();
        SetFrame(NULL);
    }
    return true;
}

void CalChartView::OnWizardSetup(CalChartDoc& show)
{
    auto wizard = new wxWizard(mFrame, wxID_ANY, wxT("New Show Setup Wizard"));
    // page 1:
    // set the number of points and the labels
    auto page1 = new SetupMarchersWizard(wizard);

    // page 2:
    // choose the show mode
    auto page2 = new ShowModeWizard(wizard);

    // page 3:
    wxWizardPageSimple::Chain(page1, page2);

    wizard->GetPageAreaSizer()->Add(page1);
    if (wizard->RunWizard(page1)) {
        auto labels = page1->GetLabelsAndInstruments();
        auto columns = page1->GetNumberColumns();
        auto newmode = GetConfigShowMode(page2->GetValue());

        show.WizardSetupNewShow(labels, columns, newmode);
        SetupInstruments dialog(show, mFrame);
        if (dialog.ShowModal() == wxID_OK) {
            auto instrumentMapping = dialog.GetInstruments();
            for (auto& i : instrumentMapping) {
                labels.at(i.first).second = i.second;
            }
            show.WizardSetupNewShow(labels, columns, newmode);
        }
    } else {
        wxMessageBox(wxT("Show setup not completed.\n")
                         wxT("You can change the number of marchers\n")
                             wxT("and show mode via the menu options"),
            wxT("Show not setup"), wxICON_INFORMATION | wxOK);
    }
    wizard->Destroy();
}

bool CalChartView::DoRotatePointPositions(int rotateAmount)
{
    if (mShow->GetSelectionList().size() == 0) {
        return false;
    }
    auto cmd = mShow->Create_RotatePointPositionsCommand(rotateAmount);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoMovePoints(const std::map<int, CalChart::Coord>& newPositions)
{
    if (mShow->GetSelectionList().size() == 0 || !mShow->WillMovePoints(newPositions)) {
        return false;
    }
    auto cmd = mShow->Create_MovePointsCommand(newPositions);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoDeletePoints()
{
    if (mShow->GetSelectionList().size() == 0) {
        return false;
    }
    auto cmd = mShow->Create_DeletePointsCommand();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoResetReferencePoint()
{
    if (mShow->GetSelectionList().size() == 0) {
        return false;
    }
    auto cmd = mShow->Create_ResetReferencePointToRef0();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoSetPointsSymbol(CalChart::SYMBOL_TYPE sym)
{
    if (mShow->GetSelectionList().size() == 0) {
        SetSelectionList(mShow->MakeSelectBySymbol(sym));
        return true;
    }
    auto cmd = mShow->Create_SetSymbolCommand(sym);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

void CalChartView::DoSetMode(CalChart::ShowMode const& mode)
{
    auto cmd = mShow->Create_SetShowModeCommand(mode);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoSetupMarchers(const std::vector<std::pair<std::string, std::string>>& labelsAndInstruments, int numColumns)
{
    auto cmd = mShow->Create_SetupMarchersCommand(labelsAndInstruments, numColumns);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoSetInstruments(std::map<int, std::string> const& dotToInstrument)
{
    auto cmd = mShow->Create_SetInstrumentsCommand(dotToInstrument);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoSetSheetTitle(const wxString& descr)
{
    auto cmd = mShow->Create_SetSheetTitleCommand(descr);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

bool CalChartView::DoSetSheetBeats(int beats)
{
    auto cmd = mShow->Create_SetSheetBeatsCommand(beats);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoSetPointsLabel(bool right)
{
    if (mShow->GetSelectionList().size() == 0) {
        return false;
    }
    auto cmd = mShow->Create_SetLabelRightCommand(right);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoSetPointsLabelFlip()
{
    if (mShow->GetSelectionList().size() == 0) {
        return false;
    }
    auto cmd = mShow->Create_ToggleLabelFlipCommand();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoSetPointsLabelVisibility(bool isVisible)
{
    if (mShow->GetSelectionList().size() == 0) {
        return false;
    }
    auto cmd = mShow->Create_SetLabelVisibleCommand(isVisible);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoTogglePointsLabelVisibility()
{
    if (mShow->GetSelectionList().size() == 0) {
        return false;
    }
    auto cmd = mShow->Create_ToggleLabelVisibilityCommand();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

void CalChartView::DoInsertSheets(const CalChart::Show::Sheet_container_t& sht,
    int where)
{
    auto cmd = mShow->Create_AddSheetsCommand(sht, where);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

bool CalChartView::DoDeleteSheet(int where)
{
    auto cmd = mShow->Create_RemoveSheetCommand(where);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoImportPrintableContinuity(const wxString& file)
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

void CalChartView::DoSetPrintContinuity(int which_sheet, const wxString& number, const wxString& cont)
{
    std::map<int, std::pair<std::string, std::string>> data{ { which_sheet, { number.ToStdString(), cont.ToStdString() } } };
    auto cmd = static_cast<CalChartDoc*>(GetDocument())->Create_SetPrintableContinuity(data);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

bool CalChartView::DoRelabel()
{
    auto sheet_num = GetCurrentSheetNum();
    auto current_sheet = mShow->GetNthSheet(GetCurrentSheetNum());
    auto next_sheet = current_sheet + 1;
    // get a relabel mapping based on the current sheet.
    auto result = mShow->GetRelabelMapping(current_sheet, next_sheet, CalChart::Float2CoordUnits(mConfig.Get_DotRatio()));
    // check to see if there's a valid remapping
    if (!result.first) {
        return false;
    }
    // Apply remapping to the rest
    auto cmd = mShow->Create_ApplyRelabelMapping(sheet_num + 1, result.second);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

// append is an insert with a relabel
std::pair<bool, std::string> CalChartView::DoAppendShow(std::unique_ptr<CalChartDoc> other_show)
{
    if (other_show->GetNumPoints() != mShow->GetNumPoints()) {
        return { false, "The blocksize doesn't match" };
    }
    auto last_sheet = mShow->GetNthSheet(GetNumSheets() - 1);
    auto next_sheet = other_show->GetSheetBegin();
    auto result = mShow->GetRelabelMapping(last_sheet, next_sheet, CalChart::Float2CoordUnits(mConfig.Get_DotRatio()));
    // check to see if there's a valid remapping
    if (!result.first) {
        return { false, "Last sheet doesn't match first sheet of other show" };
    }
    auto cmd = mShow->Create_AppendShow(std::move(other_show), CalChart::Float2CoordUnits(mConfig.Get_DotRatio()));
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return { true, "" };
}

// append is an insert with a relabel
bool CalChartView::DoSetContinuityCommand(CalChart::SYMBOL_TYPE sym, CalChart::Continuity const& new_cont)
{
    auto cmd = mShow->Create_SetContinuityCommand(sym, new_cont);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

int CalChartView::FindPoint(CalChart::Coord pos) const
{
    return mShow->GetCurrentSheet()->FindPoint(pos, CalChart::Float2CoordUnits(mConfig.Get_DotRatio()), mShow->GetCurrentReferencePoint());
}

CalChart::Coord CalChartView::PointPosition(int which) const
{
    return mShow->GetCurrentSheet()->GetPosition(which, mShow->GetCurrentReferencePoint());
}

std::vector<CalChart::AnimationErrors> CalChartView::GetAnimationErrors() const
{
    if (!mShow) {
        return {};
    }
    auto animation = mShow->GetAnimation();
    return animation ? animation->GetAnimationErrors() : std::vector<CalChart::AnimationErrors>{};
}

std::map<int, CalChart::SelectionList> CalChartView::GetAnimationCollisions() const
{
    auto result = std::map<int, CalChart::SelectionList>{};
    if (!mShow) {
        return result;
    }
    if (!mShow->GetAnimation()) {
        return result;
    }
    // first map all the collisions to a sheet with a point group.
    for (auto&& i : mShow->GetAnimation()->GetCollisions()) {
        result[std::get<1>(i.first)].insert(std::get<0>(i.first));
    }
    return result;
}

std::unique_ptr<CalChart::Animation> CalChartView::GetAnimationInstance() const
{
    if (!mShow) {
        return {};
    }
    auto animation = mShow->GetAnimation();
    return animation ? std::make_unique<CalChart::Animation>(*animation) : std::unique_ptr<CalChart::Animation>{};
}

void CalChartView::GoToSheet(int which)
{
    if (which >= 0 && which < mShow->GetNumSheets()) {
        // This *could* be run through a command or run directly...
        if (mConfig.Get_CommandUndoSetSheet()) {
            auto cmd = mShow->Create_SetCurrentSheetCommand(which);
            GetDocument()->GetCommandProcessor()->Submit(cmd.release());
        } else {
            mShow->SetCurrentSheet(which);
        }
    }
}

void CalChartView::SetActiveReferencePoint(int which)
{
    mShow->SetCurrentReferencePoint(which);
}

// toggle selection means toggle it as selected to unselected
// otherwise, always select it
void CalChartView::SelectWithinPolygon(CalChart::RawPolygon_t const& lasso, bool toggleSelected)
{
    auto select = mShow->MakeSelectWithinPolygon(lasso);
    if (toggleSelected) {
        select = mShow->MakeToggleSelection(select);
    } else {
        select = mShow->MakeAddToSelection(select);
    }
    SetSelectionList(select);
}

void CalChartView::SetSelectionList(const CalChart::SelectionList& sl)
{
    auto current_sl = mShow->GetSelectionList();
    if (std::equal(current_sl.begin(), current_sl.end(), sl.begin(), sl.end()))
        return;
    // This *could* be run through a command or run directly...
    if (mConfig.Get_CommandUndoSelection()) {
        auto cmd = mShow->Create_SetSelectionListCommand(sl);
        GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    } else {
        mShow->SetSelectionList(sl);
    }
}

void CalChartView::SetSelect(CalChart::Select select)
{
    if (select == mShow->GetSelect()) {
        return;
    }
    // select is special, directly manipulates the show without going through undo system...
    mShow->SetSelect(select);
}

void CalChartView::GoToSheetAndSetSelectionList(int which, const CalChart::SelectionList& sl)
{
    if (which < 0 || which >= mShow->GetNumSheets()) {
        return;
    }
    auto current_sl = mShow->GetSelectionList();
    if (std::equal(current_sl.begin(), current_sl.end(), sl.begin(), sl.end())) {
        return;
    }
    // This *could* be run through a command or run directly...
    if (mConfig.Get_CommandUndoSelection()) {
        auto cmd = mShow->Create_SetCurrentSheetAndSelectionCommand(which, sl);
        GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    } else {
        mShow->SetCurrentSheet(which);
        mShow->SetSelectionList(sl);
    }
}

void CalChartView::OnEnableDrawPaths(bool enable)
{
    mShow->SetDrawPaths(enable);
}

void CalChartView::DrawPaths(wxDC& dc)
{
    auto animation = mShow->GetAnimation();
    if (mShow->GetDrawPaths() && animation && animation->GetNumberSheets() && (animation->GetNumberSheets() > mShow->GetCurrentSheetNum())) {
        auto origin = GetShowFieldOffset();
        for (auto&& point : mShow->GetSelectionList()) {
            DrawPath(dc, mConfig, animation->GenPathToDraw(mShow->GetCurrentSheetNum(), point, origin), animation->EndPosition(mShow->GetCurrentSheetNum(), point, origin));
        }
    }
}

void CalChartView::DoDrawBackground(bool enable)
{
    mShow->SetDrawBackground(enable);
}

bool CalChartView::DoingDrawBackground() const
{
    return mShow->GetDrawBackground();
}

void CalChartView::DoPictureAdjustment(bool enable)
{
    mBackgroundImages.SetAdjustBackgroundMode(enable);
}

bool CalChartView::DoingPictureAdjustment() const
{
    return mBackgroundImages.GetAdjustBackgroundMode();
}

bool CalChartView::AddBackgroundImage(const wxImage& image)
{
    if (!image.IsOk()) {
        return false;
    }
    auto x = 100;
    auto y = 100;

    auto width = image.GetWidth();
    auto height = image.GetHeight();
    std::vector<unsigned char> data(width * height * 3);
    auto d = image.GetData();
    std::copy(d, d + width * height * 3, data.data());
    std::vector<unsigned char> alpha;
    auto a = image.GetAlpha();
    if (a) {
        alpha.resize(width * height);
        std::copy(a, a + width * height, alpha.data());
    }

    auto cmd = mShow->Create_AddNewBackgroundImageCommand(x, y, width, height, data, alpha);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

void CalChartView::OnBackgroundMouseLeftDown(wxMouseEvent& event, wxDC& dc)
{
    mBackgroundImages.OnMouseLeftDown(event, dc);
}

void CalChartView::OnBackgroundMouseLeftUp(wxMouseEvent& event, wxDC& dc)
{
    if (auto result = mBackgroundImages.OnMouseLeftUp(event, dc); result) {
        auto [index, resultArray] = *result;
        auto cmd = mShow->Create_MoveBackgroundImageCommand(index, std::get<0>(resultArray), std::get<1>(resultArray), std::get<2>(resultArray), std::get<3>(resultArray));
        GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    }
}

void CalChartView::OnBackgroundMouseMove(wxMouseEvent& event, wxDC& dc)
{
    mBackgroundImages.OnMouseMove(event, dc);
}

void CalChartView::OnBackgroundImageDelete()
{
    auto currentIndex = mBackgroundImages.GetCurrentIndex();
    if (!mBackgroundImages.GetAdjustBackgroundMode() || !currentIndex) {
        return;
    }
    // let the doc know we've removed a picture.
    auto cmd = mShow->Create_RemoveBackgroundImageCommand(*currentIndex);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::UpdateBackgroundImages()
{
    if (mShow && mShow->GetNumSheets()) {
        mBackgroundImages.SetBackgroundImages(mShow->GetCurrentSheet()->GetBackgroundImages());
    }
}
