#pragma once
/*
 * CalChartFrame.h
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

#include "CalChartCoord.h"
#include "CalChartDoc.h"
#include "CalChartSelectTool.h"
#include "CalChartTypes.h"

#include <map>
#include <wx/aui/framemanager.h>
#include <wx/docview.h>
#include <wxUI/wxUI.hpp>

// CalChartFrame is the central frame of the CalChart app.  Manipulation of
// the CalChartDoc happens via the CalChartView.

class AnimationErrorsPanel;
class AnimationPanel;
class CalChartDoc;
class CalChartView;
class ContinuityBrowser;
class FieldCanvas;
class FieldFrameControls;
class FieldThumbnailBrowser;
class PrintContinuityEditor;
class wxAuiToolBar;
namespace CalChart {
class Configuration;
}

// Define the main editing frame
class CalChartFrame : public wxDocChildFrame {
    using super = wxDocChildFrame;

public:
    // CalChartFrame will own the show that is passed in
    CalChartFrame(wxDocument* doc, wxView* view, CalChart::Configuration& config_, wxDocParentFrame* frame, const wxPoint& pos, const wxSize& size);
    ~CalChartFrame() override;

    void OnPrint();
    void OnPrintPreview();
    void OnLegacyPrint();
    void OnCmdPageSetup(wxCommandEvent& event);
    void OnExportViewerFile();
    void OnCmdPreferences(wxCommandEvent& event);
    void OnInsertBefore();
    void OnInsertAfter();
    void OnInsertFromOtherShow();
    void OnCopySheet();
    void OnPasteSheet();
    void OnCmdDelete(wxCommandEvent& event);
    void OnRelabel();
    void OnEditPrintContinuity();
    void OnSetSheetTitle();
    void OnSetBeats();
    void OnSetupMarchers();
    void OnSetupInstruments();
    void OnMarcherPicker();
    void OnSelectAll();
    void OnEditCurveAssignments();
    void OnCmdAbout(wxCommandEvent& event);
    void OnCmdHelp(wxCommandEvent& event);

    void OnCmd_prev_ss(wxCommandEvent& event);
    void OnCmd_next_ss(wxCommandEvent& event);
    void OnCmd_box(wxCommandEvent& event);
    void OnCmd_poly(wxCommandEvent& event);
    void OnCmd_lasso(wxCommandEvent& event);
    void OnCmd_move(wxCommandEvent& event);
    void OnCmd_curve(wxCommandEvent& event);
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

    void OnAddBackgroundImage();
    void OnAdjustBackgroundImageMode();
    void OnShowBackgroundImages();

    void OnCmd_GhostOption(wxCommandEvent& event);
    void OnGhostOption(GhostSource option);
    void OnCmd_InstrumentSelection(wxCommandEvent& event);
    void OnCmd_SymbolSelection(wxCommandEvent& event);
    void OnCmd_MarcherSelection(wxCommandEvent& event);
    void OnAdjustViews(size_t which);
    void OnSwapAnimation();

    void OnResetReferencePoint();

    void OnCmd_ChangedColorPalette(wxCommandEvent& event);

    void OnSolveTransition();

    void OnZoomFit();
    void OnZoomIn();
    void OnCmd_ZoomIn(wxCommandEvent&);
    void OnZoomOut();
    void OnCmd_ZoomOut(wxCommandEvent&);

    void OnClose();

    void AppendShow();
    void ImportContFile();

    // Grid chioce distance (distance per beat), Grid placement lock (where to end)
    std::pair<CalChart::Coord::units, CalChart::Coord::units> GridChoice() const;
    std::pair<CalChart::Coord::units, CalChart::Coord::units> ToolGridChoice() const;
    void OnUpdate();

    void SetCurrentSelect(CalChart::Select type);
    void SetCurrentMove(CalChart::MoveMode type);
    void ToolBarSetCurrentMove(CalChart::MoveMode type);
    float ToolBarSetZoom(float zoom); // set to an amount, returns what it was set to.
    void zoom_callback(wxCommandEvent&);
    void zoom_callback_textenter(wxCommandEvent&);
    void OnCmd_ReferenceNumber(wxCommandEvent&);
    void OnEnableDrawPaths(wxCommandEvent&);

    void do_zoom(float zoom_amount);

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
    void refreshInUse();
    void ChangePaneVisibility(bool show, size_t itemid);
    void ChangeMainFieldVisibility(bool show);
    void ShowFieldAndHideAnimation(bool showField);
    void SetViewsOnComponents(CalChartView* showField);
    std::string BeatStatusText() const;
    std::string PointStatusText() const;

    // the components
    FieldCanvas* mCanvas{};
    FieldThumbnailBrowser* mFieldThumbnailBrowser{};
    ContinuityBrowser* mContinuityBrowser{};
    AnimationErrorsPanel* mAnimationErrorsPanel{};
    AnimationPanel* mAnimationPanel{};
    AnimationPanel* mShadowAnimationPanel{};
    PrintContinuityEditor* mPrintContinuityEditor{};
    wxAuiToolBar* mControls;
    wxAuiToolBar* mSelectAndMoveToolBar;
    wxAuiToolBar* mMarcherToolBar;

    std::vector<wxWindow*> mLookupSubWindows;
    std::map<wxWindow*, size_t> mLookupSubWindowToIndex;

    CalChart::Configuration& mConfig;
    wxAuiManager* mAUIManager;

    bool mMainFieldVisible = true;

    wxUI::MenuItemProxy mShowBackgroundImages;
    wxUI::MenuItemProxy mAdjustBackgroundImageMode;
    wxUI::MenuItemProxy mGhostOff;
    wxUI::MenuItemProxy mViewSwapFieldAndAnimate;
    wxUI::MenuItemProxy mDrawPaths;
    std::vector<wxUI::MenuItemProxy> mAdjustPaneIndex;

    DECLARE_EVENT_TABLE()
};
