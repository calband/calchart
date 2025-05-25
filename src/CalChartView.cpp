/*
 * CalChartView.h
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
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "CalChartFrame.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "FieldCanvas.h"
#include "SetupInstruments.h"
#include "SetupMarchers.h"
#include "ShowModeWizard.h"
#include "SystemConfiguration.h"

#include <memory>
#include <ranges>
#include <wx/docview.h>
#include <wx/textfile.h>
#include <wx/wizard.h>

namespace {

void OnWizardSetup(CalChartDoc& show, wxWindow* parent)
{
    auto& config = show.GetConfiguration();
    auto* wizard = new wxWizard(parent, wxID_ANY, "New Show Setup Wizard");
    // page 1:
    // set the number of points and the labels
    auto* page1 = new SetupMarchersWizard(wizard);

    // page 2:
    // choose the show mode
    auto* page2 = new ShowModeWizard(wizard);

    // page 3:
    wxWizardPageSimple::Chain(page1, page2);

    wizard->GetPageAreaSizer()->Add(page1);
    if (wizard->RunWizard(page1)) {
        auto labels = page1->GetLabelsAndInstruments();
        auto columns = page1->GetNumberColumns();
        auto newmode = GetConfigShowMode(config, page2->GetValue());

        show.WizardSetupNewShow(labels, columns, newmode);
        SetupInstruments dialog(show, parent);
        if (dialog.ShowModal() == wxID_OK) {
            auto instrumentMapping = dialog.GetInstruments();
            for (auto& i : instrumentMapping) {
                labels.at(i.first).second = i.second;
            }
            show.WizardSetupNewShow(labels, columns, newmode);
        }
    } else {
        wxMessageBox(
            "Show setup not completed.\n"
            "You can change the number of marchers\n"
            "and show mode via the menu options",
            "Show not setup",
            wxICON_INFORMATION | wxOK);
    }
    wizard->Destroy();
}

}

IMPLEMENT_DYNAMIC_CLASS(CalChartView, wxView)

// What to do when a view is created. Creates actual windows for displaying the view.
bool CalChartView::OnCreate(wxDocument* doc, long WXUNUSED(flags))
{
    mShow = static_cast<CalChartDoc*>(doc);
    mShow->SetCurrentSheet(0);
    auto& config = mShow->GetConfiguration();
    mFrame = new CalChartFrame(doc, this, config,
        wxStaticCast(wxGetApp().GetTopWindow(), wxDocParentFrame),
        wxPoint(config.Get_FieldFramePositionX(), config.Get_FieldFramePositionY()), wxSize(static_cast<int>(config.Get_FieldFrameWidth()), static_cast<int>(config.Get_FieldFrameHeight())));

    UpdateBackgroundImages();
    mFrame->Show(true);
    Activate(true);
    return true;
}

// Sneakily gets used for default print/preview as well as drawing on the screen.
void CalChartView::OnDraw(wxDC* dc)
{
    if (mShow == nullptr) {
        return;
    }
    wxCalChart::Draw::DrawCommandList(*dc, mShow->GenerateCurrentSheetPointsDrawCommands());
}

auto CalChartView::GeneratePhatomPointsDrawCommands(CalChart::MarcherToPosition const& positions) const -> std::vector<CalChart::Draw::DrawCommand>
{
    return mShow->GeneratePhatomPointsDrawCommands(positions);
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
        OnWizardSetup(*show, mFrame);

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
auto CalChartView::OnClose(bool deleteWindow) -> bool
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

void CalChartView::DoRotatePointPositions(int rotateAmount)
{
    if (mShow->GetSelectionList().size() == 0) {
        return;
    }
    auto cmd = mShow->Create_RotatePointPositionsCommand(rotateAmount);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoMovePoints(CalChart::MarcherToPosition const& newPositions)
{
    if (mShow->GetSelectionList().size() == 0 || !mShow->WillMovePoints(newPositions)) {
        return;
    }
    auto cmd = mShow->Create_MovePointsCommand(newPositions);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoDeletePoints()
{
    if (mShow->GetSelectionList().size() == 0) {
        return;
    }
    auto cmd = mShow->Create_DeletePointsCommand();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoResetReferencePoint()
{
    if (mShow->GetSelectionList().size() == 0) {
        return;
    }
    auto cmd = mShow->Create_ResetReferencePointToRef0();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoSetPointsSymbol(CalChart::SYMBOL_TYPE sym)
{
    if (mShow->GetSelectionList().size() == 0) {
        SetSelectionList(mShow->MakeSelectBySymbol(sym));
        return;
    }
    auto cmd = mShow->Create_SetSymbolCommand(sym);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
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

void CalChartView::DoSetInstruments(std::map<CalChart::MarcherIndex, std::string> const& dotToInstrument)
{
    auto cmd = mShow->Create_SetInstrumentsCommand(dotToInstrument);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoSetSheetTitle(const wxString& descr)
{
    auto cmd = mShow->Create_SetSheetTitleCommand(descr);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoSetSheetBeats(CalChart::Beats beats)
{
    auto cmd = mShow->Create_SetSheetBeatsCommand(beats);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoSetPointsLabel(bool right)
{
    if (mShow->GetSelectionList().size() == 0) {
        return;
    }
    auto cmd = mShow->Create_SetLabelRightCommand(right);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoSetPointsLabelFlip()
{
    if (mShow->GetSelectionList().size() == 0) {
        return;
    }
    auto cmd = mShow->Create_ToggleLabelFlipCommand();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoSetPointsLabelVisibility(bool isVisible)
{
    if (mShow->GetSelectionList().size() == 0) {
        return;
    }
    auto cmd = mShow->Create_SetLabelVisibleCommand(isVisible);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoTogglePointsLabelVisibility()
{
    if (mShow->GetSelectionList().size() == 0) {
        return;
    }
    auto cmd = mShow->Create_ToggleLabelVisibilityCommand();
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoInsertSheets(CalChart::Show::Sheet_container_t const& sht,
    int where)
{
    auto cmd = mShow->Create_AddSheetsCommand(sht, where);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoDeleteSheet(int where)
{
    auto cmd = mShow->Create_RemoveSheetCommand(where);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoImportPrintableContinuity(std::string const& file)
{
    wxTextFile fp;
    fp.Open(file);
    if (!fp.IsOpened()) {
        wxMessageBox("Unable to open file", "Unable to open file", wxOK);
        return;
    }
    // read the file into a vector
    std::vector<std::string> lines;
    for (size_t line = 0; line < fp.GetLineCount(); ++line) {
        lines.push_back(fp.GetLine(line).ToStdString());
    }
    auto hasCont = mShow->AlreadyHasPrintContinuity();
    if (hasCont) {
        // prompt the user to find out if they would like to continue
        auto userchoice = wxMessageBox(
            "This show already has some Printable Continuity.  "
            "Would you like to continue Importing Printable Continuity and "
            "overwrite it?",
            "Overwrite Printable Continuity?", wxYES_NO | wxCANCEL);
        if (userchoice != wxYES) {
            return;
        }
    }
    if (auto data = mShow->ImportPrintableContinuity(lines);
        data) {
        auto cmd = mShow->Create_SetPrintableContinuity(*data);
        GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    }
}

void CalChartView::DoSetPrintContinuity(int which_sheet, wxString const& number, wxString const& cont)
{
    std::map<int, std::pair<std::string, std::string>> data{ { which_sheet, { number.ToStdString(), cont.ToStdString() } } };
    auto cmd = static_cast<CalChartDoc*>(GetDocument())->Create_SetPrintableContinuity(data);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

auto CalChartView::DoRelabel() -> std::optional<std::string>
{
    auto& config = mShow->GetConfiguration();
    auto sheet_num = GetCurrentSheetNum();
    auto current_sheet = mShow->GetNthSheet(GetCurrentSheetNum());
    auto next_sheet = current_sheet + 1;
    // get a relabel mapping based on the current sheet.
    auto result = mShow->GetRelabelMapping(current_sheet, next_sheet, CalChart::Float2CoordUnits(config.Get_DotRatio()));
    // check to see if there's a valid remapping
    if (!result.has_value()) {
        return "Stuntsheets don't match";
    }
    // Apply remapping to the rest
    auto cmd = mShow->Create_ApplyRelabelMapping(sheet_num + 1, *result);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return std::nullopt;
}

// append is an insert with a relabel
auto CalChartView::DoAppendShow(std::unique_ptr<CalChartDoc> other_show) -> std::optional<std::string>
{
    auto& config = mShow->GetConfiguration();
    if (other_show->GetNumPoints() != mShow->GetNumPoints()) {
        return "The blocksize doesn't match";
    }
    auto last_sheet = mShow->GetNthSheet(GetNumSheets() - 1);
    auto next_sheet = other_show->GetSheetBegin();
    auto result = mShow->GetRelabelMapping(last_sheet, next_sheet, CalChart::Float2CoordUnits(config.Get_DotRatio()));
    // check to see if there's a valid remapping
    if (!result.has_value()) {
        return "Last sheet doesn't match first sheet of other show";
    }
    auto cmd = mShow->Create_AppendShow(std::move(other_show), CalChart::Float2CoordUnits(config.Get_DotRatio()));
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
    return std::nullopt;
}

// append is an insert with a relabel
void CalChartView::DoSetContinuityCommand(CalChart::SYMBOL_TYPE sym, CalChart::Continuity const& new_cont)
{
    auto cmd = mShow->Create_SetContinuityCommand(sym, new_cont);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoAddSheetCurveCommand(CalChart::Curve const& curve)
{
    auto cmd = mShow->Create_AddSheetCurveCommand(curve);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoReplaceSheetCurveCommand(CalChart::Curve const& curve, int whichCurve)
{
    auto cmd = mShow->Create_ReplaceSheetCurveCommand(curve, whichCurve);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

void CalChartView::DoRemoveSheetCurveCommand(int whichCurve)
{
    auto cmd = mShow->Create_RemoveSheetCurveCommand(whichCurve);
    GetDocument()->GetCommandProcessor()->Submit(cmd.release());
}

std::vector<CalChart::Animate::Errors> CalChartView::GetAnimationErrors() const
{
    if (!mShow) {
        return {};
    }
    return mShow->GetAnimationErrors();
}

std::map<int, CalChart::SelectionList> CalChartView::GetAnimationCollisions() const
{
    if (!mShow) {
        return {};
    }
    return mShow->GetAnimationCollisions();
}

auto CalChartView::GenerateAnimationDrawCommands(
    CalChart::Beats whichBeat,
    bool drawCollisionWarning,
    std::optional<bool> onBeat,
    CalChart::Animation::AngleStepToImageFunction imageFunction) const -> std::vector<CalChart::Draw::DrawCommand>
{
    return mShow->GenerateAnimationDrawCommands(
        whichBeat,
        drawCollisionWarning,
        onBeat,
        imageFunction);
}

auto CalChartView::GetAnimationInfo(CalChart::Beats whichBeat, CalChart::MarcherIndex which) const -> std::optional<CalChart::Animate::Info>
{
    return mShow->GetAnimationInfo(whichBeat, which);
}

auto CalChartView::GetSelectedAnimationInfoWithDistanceFromPoint(CalChart::Beats whichBeat, CalChart::Coord origin) const -> std::multimap<double, CalChart::Animate::Info>
{
    return mShow->GetSelectedAnimationInfoWithDistanceFromPoint(whichBeat, origin);
}

auto CalChartView::GetTotalNumberAnimationBeats() const -> std::optional<CalChart::Beats>
{
    return mShow->GetTotalNumberAnimationBeats();
}

// Return a bounding box of the show of where the marchers are.  If they are
// outside the show, we don't see them.
auto CalChartView::GetAnimationBoundingBox(bool zoomInOnMarchers, CalChart::Beats whichBeat) const -> std::pair<CalChart::Coord, CalChart::Coord>
{
    return mShow->GetAnimationBoundingBox(zoomInOnMarchers, whichBeat);
}

auto CalChartView::BeatHasCollision(CalChart::Beats whichBeat) const -> bool
{
    return mShow->BeatHasCollision(whichBeat);
}

auto CalChartView::GetAnimationBeatForCurrentSheet() const -> CalChart::Beats
{
    return mShow->GetAnimationBeatForCurrentSheet();
}

void CalChartView::GoToSheet(int which)
{
    auto& config = mShow->GetConfiguration();
    if (which >= 0 && which < mShow->GetNumSheets()) {
        // This *could* be run through a command or run directly...
        if (config.Get_CommandUndoSetSheet()) {
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
    auto& config = mShow->GetConfiguration();
    auto current_sl = mShow->GetSelectionList();
    if (std::equal(current_sl.begin(), current_sl.end(), sl.begin(), sl.end()))
        return;
    // This *could* be run through a command or run directly...
    if (config.Get_CommandUndoSelection()) {
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
    auto& config = mShow->GetConfiguration();
    if (which < 0 || which >= mShow->GetNumSheets()) {
        return;
    }
    auto current_sl = mShow->GetSelectionList();
    if (std::equal(current_sl.begin(), current_sl.end(), sl.begin(), sl.end())) {
        return;
    }
    // This *could* be run through a command or run directly...
    if (config.Get_CommandUndoSelection()) {
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
    auto cmd = mShow->Create_AddNewBackgroundImageCommand(wxCalChart::ConvertToImageInfo(image, 100, 100));
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
