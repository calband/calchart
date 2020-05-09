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
#include "CalChartApp.h"
#include "CalChartDoc.h"
#include "CalChartDocCommand.h"
#include "CalChartFrame.h"
#include "FieldCanvas.h"
#include "animate.h"
#include "animate_types.h"
#include "animatecommand.h"
#include "animatecompile.h"
#include "background_image.h"
#include "cc_drawcommand.h"
#include "cc_shapes.h"
#include "cc_sheet.h"
#include "confgr.h"
#include "draw.h"
#include "ghost_module.h"
#include "setup_wizards.h"
#include "show_ui.h"

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

        auto ghostSheet = mGhostModule.getGhostSheet(mShow, GetCurrentSheetNum());

        if (ghostSheet != nullptr) {
            DrawGhostSheet(*dc, mConfig, origin, SelectionList(),
                mShow->GetNumPoints(), mShow->GetPointLabels(),
                *ghostSheet, 0);
        }

        auto sheet = mShow->GetCurrentSheet();
        if (sheet != mShow->GetSheetEnd()) {
            if (mCurrentReferencePoint > 0) {
                DrawPoints(*dc, mConfig, origin, mShow->GetSelectionList(),
                    mShow->GetNumPoints(), mShow->GetPointLabels(),
                    *mShow->GetCurrentSheet(), 0, false);
                DrawPoints(*dc, mConfig, origin, mShow->GetSelectionList(),
                    mShow->GetNumPoints(), mShow->GetPointLabels(),
                    *mShow->GetCurrentSheet(), mCurrentReferencePoint, true);
            } else {
                DrawPoints(*dc, mConfig, origin, mShow->GetSelectionList(),
                    mShow->GetNumPoints(), mShow->GetPointLabels(),
                    *mShow->GetCurrentSheet(), mCurrentReferencePoint, true);
            }
            DrawPaths(*dc, *sheet);
        }
    }
}

void CalChartView::DrawOtherPoints(wxDC& dc, std::map<int, CalChart::Coord> const& positions)
{
    DrawPhatomPoints(dc, mConfig, *mShow, *mShow->GetCurrentSheet(), positions);
}

void CalChartView::OnDrawBackground(wxDC& dc)
{
    if (!mDrawBackground) {
        return;
    }
    for (auto i = 0; i < static_cast<int>(mBackgroundImages.size()); ++i) {
        mBackgroundImages[i].OnPaint(dc, mAdjustBackgroundMode, mWhichBackgroundIndex == i);
    }
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
    SetFrame(static_cast<wxFrame*>(nullptr));

    Activate(false);

    if (!GetDocument()->Close()) {
        return false;
    }

    if (deleteWindow) {
        delete mFrame;
    }
    return true;
}

void CalChartView::OnWizardSetup(CalChartDoc& show)
{
    wxWizard* wizard = new wxWizard(mFrame, wxID_ANY, wxT("New Show Setup Wizard"));
    // page 1:
    // set the number of points and the labels
    ShowInfoReqWizard* page1 = new ShowInfoReqWizard(wizard);

    // page 2:
    // choose the show mode
    ChooseShowModeWizard* page2 = new ChooseShowModeWizard(wizard);

    // page 3:
    wxWizardPageSimple::Chain(page1, page2);

    wizard->GetPageAreaSizer()->Add(page1);
    if (wizard->RunWizard(page1)) {
        auto labels = page1->GetLabels();
        auto columns = page1->GetNumberColumns();
        std::vector<std::string> tlabels(labels.begin(), labels.end());
        auto newmode = wxGetApp().GetShowMode(page2->GetValue());

        show.WizardSetupNewShow(tlabels, columns, newmode);
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
    auto cmd = mShow->Create_RotatePointPositionsCommand(rotateAmount, mCurrentReferencePoint);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoMovePoints(const std::map<int, CalChart::Coord>& newPositions)
{
    if (mShow->GetSelectionList().size() == 0 || !mShow->WillMovePoints(newPositions, mCurrentReferencePoint)) {
        return false;
    }
    auto cmd = mShow->Create_MovePointsCommand(newPositions, mCurrentReferencePoint);
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
    auto cmd = mShow->Create_ResetReferencePointToRef0(mCurrentReferencePoint);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

bool CalChartView::DoSetPointsSymbol(SYMBOL_TYPE sym)
{
    if (mShow->GetSelectionList().size() == 0) {
        return false;
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

void CalChartView::DoSetShowInfo(const std::vector<wxString>& labels, int numColumns)
{
    auto cmd = mShow->Create_SetShowInfoCommand(labels, numColumns);
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

bool CalChartView::DoRelabel()
{
    auto sheet_num = GetCurrentSheetNum();
    auto current_sheet = mShow->GetNthSheet(GetCurrentSheetNum());
    auto next_sheet = current_sheet + 1;
    // get a relabel mapping based on the current sheet.
    auto result = mShow->GetRelabelMapping(current_sheet, next_sheet, Float2CoordUnits(mConfig.Get_DotRatio()));
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
    auto result = mShow->GetRelabelMapping(last_sheet, next_sheet, Float2CoordUnits(mConfig.Get_DotRatio()));
    // check to see if there's a valid remapping
    if (!result.first) {
        return { false, "Last sheet doesn't match first sheet of other show" };
    }
    auto cmd = mShow->Create_AppendShow(std::move(other_show), Float2CoordUnits(mConfig.Get_DotRatio()));
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return { true, "" };
}

// append is an insert with a relabel
bool CalChartView::DoSetContinuityCommand(SYMBOL_TYPE sym, CalChart::Continuity const& new_cont)
{
    auto cmd = mShow->Create_SetContinuityCommand(sym, new_cont);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return true;
}

int CalChartView::FindPoint(CalChart::Coord pos) const
{
    return mShow->GetCurrentSheet()->FindPoint(
        pos, Float2CoordUnits(mConfig.Get_DotRatio()),
        mCurrentReferencePoint);
}

CalChart::Coord CalChartView::PointPosition(int which) const
{
    return mShow->GetCurrentSheet()->GetPosition(which, mCurrentReferencePoint);
}

std::vector<CalChart::AnimationErrors> CalChartView::GetAnimationErrors() const
{
    if (!mShow) {
        return {};
    }
    auto animation = mShow->GetAnimation();
    return animation ? animation->GetAnimationErrors() : std::vector<CalChart::AnimationErrors>{};
}

std::map<int, SelectionList> CalChartView::GetAnimationCollisions() const
{
    auto result = std::map<int, SelectionList>{};
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
    mCurrentReferencePoint = which;
    OnUpdate(this);
}

// toggle selection means toggle it as selected to unselected
// otherwise, always select it
void CalChartView::SelectWithLasso(const CalChart::Lasso* lasso, bool toggleSelected)
{
    auto select = mShow->MakeSelectWithLasso(*lasso, mCurrentReferencePoint);
    if (toggleSelected) {
        select = mShow->MakeToggleSelection(select);
    } else {
        select = mShow->MakeAddToSelection(select);
    }
    SetSelection(select);
}

// Select points within rectangle
void CalChartView::SelectPointsInRect(const CalChart::Coord& c1, const CalChart::Coord& c2,
    bool toggleSelected)
{
    CalChart::Lasso lasso(c1);
    lasso.Append(CalChart::Coord(c1.x, c2.y));
    lasso.Append(c2);
    lasso.Append(CalChart::Coord(c2.x, c1.y));
    lasso.End();
    SelectWithLasso(&lasso, toggleSelected);
}

void CalChartView::SetSelection(const SelectionList& sl)
{
    auto current_sl = mShow->GetSelectionList();
    if (std::equal(current_sl.begin(), current_sl.end(), sl.begin(), sl.end()))
        return;
    // This *could* be run through a command or run directly...
    if (mConfig.Get_CommandUndoSelection()) {
        auto cmd = mShow->Create_SetSelectionCommand(sl);
        GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    } else {
        mShow->SetSelection(sl);
    }
}

void CalChartView::GoToSheetAndSetSelection(int which, const SelectionList& sl)
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
        mShow->SetSelection(sl);
    }
}

void CalChartView::OnEnableDrawPaths(bool enable)
{
    mDrawPaths = enable;
    mFrame->Refresh();
}

void CalChartView::DrawPaths(wxDC& dc, const CalChart::Sheet& sheet)
{
    auto animation = mShow->GetAnimation();
    if (mDrawPaths && animation && animation->GetNumberSheets() && (animation->GetNumberSheets() > mShow->GetCurrentSheetNum())) {
        auto origin = GetShowFieldOffset();
        for (auto&& point : mShow->GetSelectionList()) {
            DrawPath(dc, mConfig, animation->GenPathToDraw(mShow->GetCurrentSheetNum(), point, origin), animation->EndPosition(mShow->GetCurrentSheetNum(), point, origin));
        }
    }
}

void CalChartView::DoDrawBackground(bool enable)
{
    mDrawBackground = enable;
}

bool CalChartView::DoingDrawBackground() const
{
    return mDrawBackground;
}

void CalChartView::DoPictureAdjustment(bool enable)
{
    mAdjustBackgroundMode = enable;
}

bool CalChartView::DoingPictureAdjustment() const
{
    return mAdjustBackgroundMode;
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
    if (!mAdjustBackgroundMode) {
        return;
    }
    mWhichBackgroundIndex = -1;
    for (auto i = 0; i < static_cast<int>(mBackgroundImages.size()); ++i) {
        if (mBackgroundImages[i].MouseClickIsHit(event, dc)) {
            mWhichBackgroundIndex = i;
        }
    }
    if (mWhichBackgroundIndex != -1) {
        mBackgroundImages[mWhichBackgroundIndex].OnMouseLeftDown(event, dc);
    }
}

void CalChartView::OnBackgroundMouseLeftUp(wxMouseEvent& event, wxDC& dc)
{
    if (!mAdjustBackgroundMode) {
        return;
    }
    if (mWhichBackgroundIndex >= 0 && mWhichBackgroundIndex < static_cast<int>(mBackgroundImages.size())) {
        auto result = mBackgroundImages[mWhichBackgroundIndex].OnMouseLeftUp(event, dc);
        auto cmd = mShow->Create_MoveBackgroundImageCommand(mWhichBackgroundIndex, std::get<0>(result), std::get<1>(result), std::get<2>(result), std::get<3>(result));
        GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    }
}

void CalChartView::OnBackgroundMouseMove(wxMouseEvent& event, wxDC& dc)
{
    if (!mAdjustBackgroundMode) {
        return;
    }
    if (mWhichBackgroundIndex >= 0 && mWhichBackgroundIndex < static_cast<int>(mBackgroundImages.size())) {
        mBackgroundImages[mWhichBackgroundIndex].OnMouseMove(event, dc);
    }
}

void CalChartView::OnBackgroundImageDelete()
{
    if (!mAdjustBackgroundMode || !(mWhichBackgroundIndex >= 0 && mWhichBackgroundIndex < static_cast<int>(mBackgroundImages.size()))) {
        return;
    }
    // let the doc know we've removed a picture.
    auto cmd = mShow->Create_RemoveBackgroundImageCommand(mWhichBackgroundIndex);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::UpdateBackgroundImages()
{
    mBackgroundImages.clear();
    if (mShow && mShow->GetNumSheets()) {
        auto images = mShow->GetCurrentSheet()->GetBackgroundImages();
        for (auto&& image : images) {
            // ugh...  not sure if there's a better way to pass data to image.
            auto d = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * image.image_width * image.image_height * 3));
            std::copy(image.data.begin(), image.data.end(), d);
            auto a = static_cast<unsigned char*>(nullptr);
            if (image.alpha.size()) {
                a = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * image.image_width * image.image_height));
                std::copy(image.alpha.begin(), image.alpha.end(), a);
                wxImage img(image.image_width, image.image_height, d, a);
                mBackgroundImages.emplace_back(img, image.left, image.top, image.scaled_width, image.scaled_height);
            } else {
                wxImage img(image.image_width, image.image_height, d);
                mBackgroundImages.emplace_back(img, image.left, image.top, image.scaled_width, image.scaled_height);
            }
        }
    }
}
