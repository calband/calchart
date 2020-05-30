#pragma once
/*
 * CalChartFrame.h
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

#include "cc_coord.h"
#include "cc_types.h"

#include <map>
#include <wx/aui/framemanager.h>
#include <wx/docview.h>

// CalChartFrame is the central frame of the CalChart app.  Manipulation of
// the CalChartDoc happens via the CalChartView.

class AnimationErrorsPanel;
class AnimationPanel;
class CalChartConfiguration;
class CalChartDoc;
class CalChartView;
class ContinuityBrowser;
class FieldCanvas;
class FieldFrameControls;
class FieldThumbnailBrowser;
class PrintContinuityEditor;
class wxAuiToolBar;

// Define the main editing frame
class CalChartFrame : public wxDocChildFrame {
    using super = wxDocChildFrame;

public:
    // CalChartFrame will own the show that is passed in
    CalChartFrame(wxDocument* doc, wxView* view, CalChartConfiguration& config_, wxDocParentFrame* frame, const wxPoint& pos, const wxSize& size);
    ~CalChartFrame() override;

    void OnCmdAppend(wxCommandEvent& event);
    void OnCmdImportCont(wxCommandEvent& event);
    void OnCmdSave(wxCommandEvent& event);
    void OnCmdSaveAs(wxCommandEvent& event);
    void OnCmdPrint(wxCommandEvent& event);
    void OnCmdPrintPreview(wxCommandEvent& event);
    void OnCmdLegacyPrint(wxCommandEvent& event);
    void OnCmdPageSetup(wxCommandEvent& event);
    void OnCmdExportViewerFile(wxCommandEvent& event);
    void OnCmdPreferences(wxCommandEvent& event);
    void OnCmdClose(wxCommandEvent& event);
    void OnCmdRedo(wxCommandEvent& event);
    void OnCmdInsertBefore(wxCommandEvent& event);
    void OnCmdInsertAfter(wxCommandEvent& event);
    void OnCmdInsertFromOtherShow(wxCommandEvent& event);
    void OnCmdCopySheet(wxCommandEvent& event);
    void OnCmdPasteSheet(wxCommandEvent& event);
    void OnCmdDelete(wxCommandEvent& event);
    void OnCmdRelabel(wxCommandEvent& event);
    void OnCmdEditPrintCont(wxCommandEvent& event);
    void OnCmdSetSheetTitle(wxCommandEvent& event);
    void OnCmdSetBeats(wxCommandEvent& event);
    void OnCmdSetup(wxCommandEvent& event);
    void OnCmdSetMode(wxCommandEvent& event);
    void OnCmdPoints(wxCommandEvent& event);
    void OnCmdSelectAll(wxCommandEvent& event);
    void OnCmdAbout(wxCommandEvent& event);
    void OnCmdHelp(wxCommandEvent& event);
    void OnCmdAnimate(wxCommandEvent& event);

    void OnCmd_prev_ss(wxCommandEvent& event);
    void OnCmd_next_ss(wxCommandEvent& event);
    void OnCmd_box(wxCommandEvent& event);
    void OnCmd_poly(wxCommandEvent& event);
    void OnCmd_lasso(wxCommandEvent& event);
    void OnCmd_move(wxCommandEvent& event);
    void OnCmd_swap(wxCommandEvent& event);
    void OnCmd_shape_line(wxCommandEvent& event);
    void OnCmd_shape_x(wxCommandEvent& event);
    void OnCmd_shape_cross(wxCommandEvent& event);
    void OnCmd_shape_box(wxCommandEvent& event);
    void OnCmd_shape_ellipse(wxCommandEvent& event);
    void OnCmd_shape_draw(wxCommandEvent& event);
    void OnCmd_line(wxCommandEvent& event);
    void OnCmd_rot(wxCommandEvent& event);
    void OnCmd_shear(wxCommandEvent& event);
    void OnCmd_reflect(wxCommandEvent& event);
    void OnCmd_size(wxCommandEvent& event);
    void OnCmd_genius(wxCommandEvent& event);
    void OnCmd_label_left(wxCommandEvent& event);
    void OnCmd_label_right(wxCommandEvent& event);
    void OnCmd_label_flip(wxCommandEvent& event);
    void OnCmd_label_hide(wxCommandEvent& event);
    void OnCmd_label_show(wxCommandEvent& event);
    void OnCmd_label_visibility_toggle(wxCommandEvent& event);
    void OnCmd_setsym0(wxCommandEvent& event);
    void OnCmd_setsym1(wxCommandEvent& event);
    void OnCmd_setsym2(wxCommandEvent& event);
    void OnCmd_setsym3(wxCommandEvent& event);
    void OnCmd_setsym4(wxCommandEvent& event);
    void OnCmd_setsym5(wxCommandEvent& event);
    void OnCmd_setsym6(wxCommandEvent& event);
    void OnCmd_setsym7(wxCommandEvent& event);
    void OnChar(wxKeyEvent& event);

    void OnCmd_AddBackgroundImage(wxCommandEvent& event);
    void OnCmd_AdjustBackgroundImageMode(wxCommandEvent& event);
    void OnCmd_ShowBackgroundImages(wxCommandEvent& event);

    void OnCmd_GhostOption(wxCommandEvent& event);
    void OnCmd_AdjustViews(wxCommandEvent& event);
    void OnCmd_SwapAnimation(wxCommandEvent& event);
    void OnCmd_DrawPaths(wxCommandEvent& event);

    void OnCmd_ResetReferencePoint(wxCommandEvent& event);

    void OnCmd_ChangedColorPalette(wxCommandEvent& event);

    void OnCmd_SolveTransition(wxCommandEvent& event);

    void OnCmd_ZoomFit(wxCommandEvent&);
    void OnCmd_ZoomIn(wxCommandEvent&);
    void OnCmd_ZoomOut(wxCommandEvent&);

    void OnClose();

    void AppendShow();
    void ImportContFile();

    // Grid chioce distance (distance per beat), Grid placement lock (where to end)
    std::pair<CalChart::Coord::units, CalChart::Coord::units> GridChoice() const;
    std::pair<CalChart::Coord::units, CalChart::Coord::units> ToolGridChoice() const;
    void OnUpdate();

    void SetCurrentLasso(CC_DRAG type);
    void SetCurrentMove(CC_MOVE_MODES type);
    void ToolbarSetCurrentMove(CC_MOVE_MODES type);
    float ToolbarSetZoom(float zoom); // set to an amount, returns what it was set to.
    void zoom_callback(wxCommandEvent&);
    void zoom_callback_textenter(wxCommandEvent&);
    void refnum_callback(wxCommandEvent&);
    void OnEnableDrawPaths(wxCommandEvent&);

    void do_zoom(float zoom_amount);

    void Setup();
    void SetMode();

    const FieldCanvas* GetCanvas() const { return mCanvas; }
    FieldCanvas* GetCanvas() { return mCanvas; }

    const CalChartView* GetFieldView() const;
    CalChartView* GetFieldView();

    const CalChartDoc* GetShow() const;
    CalChartDoc* GetShow();

    void AUIIsClose(wxAuiManagerEvent& event);

private:
    void refreshGhostOptionStates();
    void ChangeFieldThumbnailVisibility(bool show);
    void ChangeVisibility(wxWindow* window, bool show, int itemid, std::string const& name);
    void ChangeVisibility(bool show, int itemid);
    void ChangeAnimationErrorsVisibility(bool show);
    void ChangeAnimationVisibility(bool show);
    void ChangeMainFieldVisibility(bool show);

    void ShowFieldAndHideAnimation(bool showField);

    FieldFrameControls* mControls{};
    FieldCanvas* mCanvas{};
    FieldThumbnailBrowser* mFieldThumbnailBrowser{};
    ContinuityBrowser* mContinuityBrowser{};
    AnimationErrorsPanel* mAnimationErrorsPanel{};
    AnimationPanel* mAnimationPanel{};
    AnimationPanel* mShadowAnimationPanel{};
    PrintContinuityEditor* mPrintContinuityEditor{};
    wxAuiToolBar* mToolbar;

    std::map<int, wxWindow*> mLookupEnumToSubWindow;
    std::map<wxWindow*, int> mLookupSubWindowToEnum;

    CalChartConfiguration& mConfig;
    wxAuiManager* mAUIManager;

    bool mMainFieldVisible = true;

    DECLARE_EVENT_TABLE()
};
