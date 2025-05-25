/*
 * CalChartFrame.cpp
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

#include <cstring>
#include <fstream>
#include <sstream>

#include "CalChartFrame.h"

#include "AnimationErrorsPanel.h"
#include "AnimationPanel.h"
#include "CalChartAnimationErrors.h"
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartCoord.h"
#include "CalChartDrawing.h"
#include "CalChartPoint.h"
#include "CalChartPreferences.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "CalChartShowMode.h"
#include "CalChartSplash.h"
#include "CalChartToolBar.h"
#include "CalChartView.h"
#include "ColorPalette.h"
#include "ContinuityBrowser.h"
#include "FieldCanvas.h"
#include "FieldControlsToolBar.h"
#include "FieldThumbnailBrowser.h"
#include "ModeSetupDialog.h"
#include "PointPicker.h"
#include "PrintContinuityEditor.h"
#include "PrintPostScriptDialog.h"
#include "SetupInstruments.h"
#include "SetupMarchers.h"
#include "SystemConfiguration.h"
#include "TransitionSolverFrame.h"
#include "TransitionSolverView.h"
#include "ccvers.h"
#include "platconf.h"
#include "ui_enums.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#ifdef __WXMSW__
#include <wx/helpwin.h>
#endif
#include <wx/aui/auibar.h>
#include <wx/clipbrd.h>
#include <wx/cmdproc.h>
#include <wx/tglbtn.h>

const wxString kSheetDataClipboardFormat = "CC_sheet_clipboard_v1";

static const wxChar* file_wild = FILE_WILDCARDS;

static std::map<int, std::string> kAUIEnumToString = {
    { CALCHART__ViewFieldThumbnail, "Field Thumbnails" },
    { CALCHART__ViewFieldControls, "Controls ToolBar" },
    { CALCHART__ViewContinuityInfo, "Continuities" },
    { CALCHART__ViewAnimationErrors, "Marching Errors" },
    { CALCHART__ViewAnimation, "Animation" },
    { CALCHART__ViewPrintContinuity, "Print Continuity" },
    { CALCHART__ViewSelectAndMoveToolBar, "Select and Move ToolBar" },
    { CALCHART__ViewMarcherToolBar, "Marcher ToolBar" },
};

BEGIN_EVENT_TABLE(CalChartFrame, wxDocChildFrame)
EVT_CHAR(CalChartFrame::OnChar)
EVT_MENU(CALCHART__APPEND_FILE, CalChartFrame::OnCmdAppend)
EVT_MENU(CALCHART__IMPORT_CONT_FILE, CalChartFrame::OnCmdImportCont)
EVT_MENU(CALCHART__wxID_PRINT, CalChartFrame::OnCmdPrint)
EVT_MENU(CALCHART__wxID_PREVIEW, CalChartFrame::OnCmdPrintPreview)
EVT_MENU(wxID_PAGE_SETUP, CalChartFrame::OnCmdPageSetup)
EVT_MENU(CALCHART__LEGACY_PRINT, CalChartFrame::OnCmdLegacyPrint)
EVT_MENU(CALCHART__COPY_SHEET, CalChartFrame::OnCmdCopySheet)
EVT_MENU(CALCHART__PASTE_SHEET, CalChartFrame::OnCmdPasteSheet)
EVT_MENU(CALCHART__INSERT_BEFORE, CalChartFrame::OnCmdInsertBefore)
EVT_MENU(CALCHART__INSERT_AFTER, CalChartFrame::OnCmdInsertAfter)
EVT_MENU(CALCHART__INSERT_OTHER_SHOW, CalChartFrame::OnCmdInsertFromOtherShow)
EVT_MENU(wxID_DELETE, CalChartFrame::OnCmdDelete)
EVT_MENU(CALCHART__RELABEL, CalChartFrame::OnCmdRelabel)
EVT_MENU(CALCHART__PRINT_EDIT_CONTINUITY, CalChartFrame::OnCmdEditPrintCont)
EVT_MENU(CALCHART__SET_SHEET_TITLE, CalChartFrame::OnCmdSetSheetTitle)
EVT_MENU(CALCHART__SET_BEATS, CalChartFrame::OnCmdSetBeats)
EVT_MENU(CALCHART__SETUPMARCHERS, CalChartFrame::OnCmdSetupMarchers)
EVT_MENU(CALCHART__SETUPINSTRUMENTS, CalChartFrame::OnCmdSetupInstruments)
EVT_MENU(CALCHART__SETMODE, CalChartFrame::OnCmdSetMode)
EVT_MENU(CALCHART__POINTPICKER, CalChartFrame::OnCmdPointPicker)
EVT_MENU(CALCHART__SELECT_ALL, CalChartFrame::OnCmdSelectAll)
EVT_MENU(wxID_ABOUT, CalChartFrame::OnCmdAbout)
EVT_MENU(wxID_HELP, CalChartFrame::OnCmdHelp)
EVT_MENU(CALCHART__ShowBackgroundImages, CalChartFrame::OnCmd_ShowBackgroundImages)
EVT_MENU(CALCHART__AddBackgroundImage, CalChartFrame::OnCmd_AddBackgroundImage)
EVT_MENU(CALCHART__AdjustBackgroundImageMode, CalChartFrame::OnCmd_AdjustBackgroundImageMode)
EVT_MENU(CALCHART__GhostOff, CalChartFrame::OnCmd_GhostOption)
EVT_MENU(CALCHART__GhostNextSheet, CalChartFrame::OnCmd_GhostOption)
EVT_MENU(CALCHART__GhostPreviousSheet, CalChartFrame::OnCmd_GhostOption)
EVT_MENU(CALCHART__GhostNthSheet, CalChartFrame::OnCmd_GhostOption)
EVT_MENU(CALCHART__ViewFieldThumbnail, CalChartFrame::OnCmd_AdjustViews)
EVT_MENU(CALCHART__ViewFieldControls, CalChartFrame::OnCmd_AdjustViews)
EVT_MENU(CALCHART__ViewContinuityInfo, CalChartFrame::OnCmd_AdjustViews)
EVT_MENU(CALCHART__ViewAnimationErrors, CalChartFrame::OnCmd_AdjustViews)
EVT_MENU(CALCHART__ViewAnimation, CalChartFrame::OnCmd_AdjustViews)
EVT_MENU(CALCHART__ViewPrintContinuity, CalChartFrame::OnCmd_AdjustViews)
EVT_MENU(CALCHART__ViewSelectAndMoveToolBar, CalChartFrame::OnCmd_AdjustViews)
EVT_MENU(CALCHART__ViewMarcherToolBar, CalChartFrame::OnCmd_AdjustViews)
EVT_MENU(CALCHART__ViewSwapFieldAndAnimate, CalChartFrame::OnCmd_SwapAnimation)
EVT_MENU(CALCHART__ViewZoomFit, CalChartFrame::OnCmd_ZoomFit)
EVT_MENU(CALCHART__ViewZoomIn, CalChartFrame::OnCmd_ZoomIn)
EVT_BUTTON(CALCHART__ViewZoomIn, CalChartFrame::OnCmd_ZoomIn)
EVT_MENU(CALCHART__ViewZoomOut, CalChartFrame::OnCmd_ZoomOut)
EVT_BUTTON(CALCHART__ViewZoomOut, CalChartFrame::OnCmd_ZoomIn)
EVT_MENU(CALCHART__prev_ss, CalChartFrame::OnCmd_prev_ss)
EVT_MENU(CALCHART__next_ss, CalChartFrame::OnCmd_next_ss)
EVT_MENU(CALCHART__box, CalChartFrame::OnCmd_box)
EVT_MENU(CALCHART__poly, CalChartFrame::OnCmd_poly)
EVT_MENU(CALCHART__lasso, CalChartFrame::OnCmd_lasso)
EVT_MENU(CALCHART__curve, CalChartFrame::OnCmd_curve)
EVT_MENU(CALCHART__move, CalChartFrame::OnCmd_move)
EVT_MENU(CALCHART__swap, CalChartFrame::OnCmd_swap)
EVT_MENU(CALCHART__shape_line, CalChartFrame::OnCmd_shape_line)
EVT_MENU(CALCHART__shape_x, CalChartFrame::OnCmd_shape_x)
EVT_MENU(CALCHART__shape_cross, CalChartFrame::OnCmd_shape_cross)
EVT_MENU(CALCHART__shape_box, CalChartFrame::OnCmd_shape_box)
EVT_MENU(CALCHART__shape_ellipse, CalChartFrame::OnCmd_shape_ellipse)
EVT_MENU(CALCHART__shape_draw, CalChartFrame::OnCmd_shape_draw)
EVT_MENU(CALCHART__line, CalChartFrame::OnCmd_line)
EVT_MENU(CALCHART__rot, CalChartFrame::OnCmd_rot)
EVT_MENU(CALCHART__shear, CalChartFrame::OnCmd_shear)
EVT_MENU(CALCHART__reflect, CalChartFrame::OnCmd_reflect)
EVT_MENU(CALCHART__size, CalChartFrame::OnCmd_size)
EVT_MENU(CALCHART__genius, CalChartFrame::OnCmd_genius)
EVT_MENU(CALCHART__label_left, CalChartFrame::OnCmd_label_left)
EVT_MENU(CALCHART__label_right, CalChartFrame::OnCmd_label_right)
EVT_MENU(CALCHART__label_flip, CalChartFrame::OnCmd_label_flip)
EVT_MENU(CALCHART__label_hide, CalChartFrame::OnCmd_label_hide)
EVT_MENU(CALCHART__label_show, CalChartFrame::OnCmd_label_show)
EVT_MENU(CALCHART__label_visibility_toggle, CalChartFrame::OnCmd_label_visibility_toggle)
EVT_MENU(CALCHART__setsym0, CalChartFrame::OnCmd_setsym0)
EVT_MENU(CALCHART__setsym1, CalChartFrame::OnCmd_setsym1)
EVT_MENU(CALCHART__setsym2, CalChartFrame::OnCmd_setsym2)
EVT_MENU(CALCHART__setsym3, CalChartFrame::OnCmd_setsym3)
EVT_MENU(CALCHART__setsym4, CalChartFrame::OnCmd_setsym4)
EVT_MENU(CALCHART__setsym5, CalChartFrame::OnCmd_setsym5)
EVT_MENU(CALCHART__setsym6, CalChartFrame::OnCmd_setsym6)
EVT_MENU(CALCHART__setsym7, CalChartFrame::OnCmd_setsym7)
EVT_MENU(CALCHART__EXPORT_VIEWER_FILE, CalChartFrame::OnCmdExportViewerFile)
EVT_MENU(wxID_PREFERENCES, CalChartFrame::OnCmdPreferences)
EVT_MENU(CALCHART__draw_paths, CalChartFrame::OnCmd_DrawPaths)
EVT_SLIDER(CALCHART__slider_zoom, CalChartFrame::zoom_callback)
EVT_CHOICE(CALCHART__refnum_callback, CalChartFrame::OnCmd_ReferenceNumber)
EVT_CHOICE(CALCHART__GhostControls, CalChartFrame::OnCmd_GhostOption)
EVT_CHOICE(CALCHART__InstrumentChoice, CalChartFrame::OnCmd_InstrumentSelection)
EVT_CHOICE(CALCHART__SymbolChoice, CalChartFrame::OnCmd_SymbolSelection)
EVT_CHOICE(CALCHART__MarcherChoice, CalChartFrame::OnCmd_MarcherSelection)
EVT_CHECKBOX(CALCHART__draw_paths, CalChartFrame::OnEnableDrawPaths)
EVT_MENU(CALCHART__ResetReferencePoint, CalChartFrame::OnCmd_ResetReferencePoint)
EVT_BUTTON(CALCHART__ResetReferencePoint, CalChartFrame::OnCmd_ResetReferencePoint)
EVT_BUTTON(CALCHART__ChangedColorPalette, CalChartFrame::OnCmd_ChangedColorPalette)
EVT_MENU(CALCHART__E7TransitionSolver, CalChartFrame::OnCmd_SolveTransition)
EVT_AUI_PANE_CLOSE(CalChartFrame::AUIIsClose)
END_EVENT_TABLE()

class CalChartPrintout : public wxPrintout {
public:
    CalChartPrintout(wxString const& title, CalChartDoc const& show, CalChart::Configuration const& config_)
        : wxPrintout(title)
        , mShow(show)
        , mConfig(config_)
    {
    }
    ~CalChartPrintout() override = default;
    bool HasPage(int pageNum) override { return pageNum <= mShow.GetNumSheets(); }
    void GetPageInfo(int* minPage, int* maxPage, int* pageFrom, int* pageTo) override
    {
        *minPage = 1;
        *maxPage = mShow.GetNumSheets();
        *pageFrom = 1;
        *pageTo = mShow.GetNumSheets();
    }
    bool OnPrintPage(int pageNum) override
    {
        auto dc = wxPrintout::GetDC();
        auto sheet = mShow.GetNthSheet(pageNum - 1);
        auto size = wxGetApp().GetGlobalPrintDialog().GetPrintData().GetOrientation();

        CalChartDraw::DrawForPrinting(dc, mConfig, mShow, *sheet, 0, 2 == size);
        return true;
    }
    CalChartDoc const& mShow;
    CalChart::Configuration const& mConfig;
};

// Main frame constructor
CalChartFrame::CalChartFrame(wxDocument* doc, wxView* view, CalChart::Configuration& config, wxDocParentFrame* frame, wxPoint const& pos, wxSize const& size)
    : wxDocChildFrame(doc, view, frame, -1, "CalChart", pos, size)
    , mConfig(config)
    , mAUIManager(new wxAuiManager(this))
{
    // Give it an icon
    SetBandIcon(this);

    // Give it a status line
    CreateStatusBar(3);
    SetStatusText("Welcome to Calchart " CC_VERSION);

    // Make a menubar
    auto file_menu = new wxMenu;
    file_menu->Append(wxID_NEW, "&New Show\tCTRL-N", "Create a new show");
    file_menu->Append(wxID_OPEN, "&Open...\tCTRL-O", "Load a saved show");
    file_menu->Append(CALCHART__IMPORT_CONT_FILE, "&Import Continuity...\tCTRL-SHIFT-I", "Import continuity text");
    file_menu->Append(wxID_SAVE, "&Save\tCTRL-S", "Save show");
    file_menu->Append(wxID_SAVEAS, "Save &As...\tCTRL-SHIFT-S", "Save show as a new name");
    file_menu->Append(CALCHART__wxID_PRINT, "&Print...\tCTRL-P", "Print this show");
    file_menu->Append(CALCHART__wxID_PREVIEW, "Preview...\tCTRL-SHIFT-P", "Preview this show");
    file_menu->Append(wxID_PAGE_SETUP, "Page Setup...\tCTRL-SHIFT-ALT-P", "Setup Pages");
    file_menu->Append(CALCHART__LEGACY_PRINT, "Print to PS...", "Print show to PostScript");
    file_menu->Append(CALCHART__EXPORT_VIEWER_FILE, "Export for Online Viewer...", "Export show to be viewed using the CalChart Online Viewer");
    file_menu->Append(wxID_PREFERENCES, "&Preferences\tCTRL-,");
    file_menu->Append(wxID_CLOSE, "Close Window\tCTRL-W", "Close this window");
    file_menu->Append(wxID_EXIT, "&Quit\tCTRL-Q", "Quit CalChart");

    // A nice touch: a history of files visited. Use this menu.
    // causes a crash :(
    // view->GetDocumentManager()->FileHistoryUseMenu(file_menu);

    auto edit_menu = new wxMenu;
    edit_menu->Append(wxID_UNDO, "&Undo\tCTRL-Z");
    edit_menu->Append(wxID_REDO, "&Redo\tCTRL-SHIFT-Z");
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__INSERT_BEFORE, "&Insert Sheet Before\tCTRL-[", "Insert a new stuntsheet before this one");
    edit_menu->Append(CALCHART__INSERT_AFTER, "Insert Sheet &After\tCTRL-]", "Insert a new stuntsheet after this one");
    edit_menu->Append(wxID_DELETE, "&Delete Sheet\tCTRL-DEL", "Delete this stuntsheet");
    edit_menu->Append(CALCHART__COPY_SHEET, "&Copy Sheet\tCTRL-C", "Copy the current stuntsheet");
    edit_menu->Append(CALCHART__PASTE_SHEET, "&Paste Sheet\tCTRL-V", "Paste the current stuntsheet");
    edit_menu->Append(CALCHART__INSERT_OTHER_SHOW, "Insert Sheets From Other Show...", "Insert a saved stuntsheet after this one");
    edit_menu->Append(CALCHART__RELABEL, "&Relabel Sheets\tCTRL-R", "Relabel all stuntsheets after this one");
    edit_menu->Append(CALCHART__APPEND_FILE, "Append Show...", "Append a show to the end");
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__SETUPMARCHERS, "Set &Up Marchers...\tCTRL-U", "Setup number of marchers");
    edit_menu->Append(CALCHART__SETUPINSTRUMENTS, "Set &Instruments...\tCTRL-I", "Set instruments");
    edit_menu->Append(CALCHART__SETMODE, "Set Show &Mode...", "Set the show mode");
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__POINTPICKER, "Point Picker...\tCTRL-SHIFT-A", "Point Picker");
    edit_menu->Append(CALCHART__SELECT_ALL, "Select &All...\tCTRL-A", "Select All Points");
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__SET_SHEET_TITLE, "Set Sheet &Title...\tCTRL-T", "Change the title of this stuntsheet");
    edit_menu->Append(CALCHART__SET_BEATS, "Set &Beats...\tCTRL-B", "Change the number of beats for this stuntsheet");
    edit_menu->Append(CALCHART__ResetReferencePoint, "Reset reference point...", "Reset the current reference point");
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__E7TransitionSolver, "Solve transition", "Solve the transition to the next sheet automatically");
    edit_menu->Append(CALCHART__PRINT_EDIT_CONTINUITY, "Edit Print Continuity...", "Edit Print continuity for this stuntsheet");

    auto backgroundimage_menu = new wxMenu;
    backgroundimage_menu->AppendCheckItem(CALCHART__ShowBackgroundImages, "Show Background Images", "Toggle showing background images");
    backgroundimage_menu->Append(CALCHART__AddBackgroundImage, "Add Background Image...", "Add a background image");
    backgroundimage_menu->AppendCheckItem(CALCHART__AdjustBackgroundImageMode, "Image Adjust Mode...", "Mode to adjust background images");
    backgroundimage_menu->Enable(CALCHART__ShowBackgroundImages, true);
    backgroundimage_menu->Enable(CALCHART__AddBackgroundImage, true);
    backgroundimage_menu->Enable(CALCHART__AdjustBackgroundImageMode, true);

    auto view_menu = new wxMenu;
    view_menu->Append(CALCHART__ViewSwapFieldAndAnimate, "View Animation\tCTRL-RETURN", "View Animation or Field");
    view_menu->AppendSeparator();
    for (int i = CALCHART__ViewFieldThumbnail; i <= CALCHART__ViewMarcherToolBar; ++i) {
        view_menu->Append(i, std::string("Show ") + kAUIEnumToString[i], std::string("Controls Displaying ") + kAUIEnumToString[i]);
    }
    view_menu->AppendSeparator();
    view_menu->AppendCheckItem(CALCHART__draw_paths, "Draw Paths", "Draw Paths");
    view_menu->Check(CALCHART__draw_paths, false);
    view_menu->AppendSeparator();
    view_menu->Append(CALCHART__GhostOff, "Disable Ghost View", "Turn off ghost view");
    view_menu->Append(CALCHART__GhostNextSheet, "Ghost Next Sheet", "Draw a ghost of the next stuntsheet");
    view_menu->Append(CALCHART__GhostPreviousSheet, "Ghost Previous Sheet", "Draw a ghost of the previous stuntsheet");
    view_menu->Append(CALCHART__GhostNthSheet, "Ghost Particular Sheet...", "Draw a ghost of a particular stuntsheet");
    view_menu->AppendSeparator();
    view_menu->Append(CALCHART__ViewZoomFit, "Zoom to Fit\tCTRL-0", "Zoom to fit");
    view_menu->Append(CALCHART__ViewZoomIn, "Zoom In\tCTRL-+", "Zoom In");
    view_menu->Append(CALCHART__ViewZoomOut, "Zoom In\tCTRL--", "Zoom Out");

    auto help_menu = new wxMenu;
    help_menu->Append(wxID_ABOUT, "&About CalChart...", "Information about the program");
    help_menu->Append(wxID_HELP, "&Help on CalChart...\tCTRL-H", "Help on using CalChart");

    auto menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, "&File");
    menu_bar->Append(edit_menu, "&Edit");
    menu_bar->Append(view_menu, "&View");
    menu_bar->Append(backgroundimage_menu, "&Field Image");
    menu_bar->Append(help_menu, "&Help");
    SetMenuBar(menu_bar);

    // Add a toolbar
    mSelectAndMoveToolBar = CreateSelectAndMoves(this, mConfig.Get_FeatureCurves(), wxID_ANY, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW);
    mSelectAndMoveToolBar->SetFont(ResizeFont(mSelectAndMoveToolBar->GetFont(), GetToolBarFontSize()));
    mMarcherToolBar = CreateDotModifiers(this, wxID_ANY, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW);
    mMarcherToolBar->SetFont(ResizeFont(mMarcherToolBar->GetFont(), GetToolBarFontSize()));

    // sanity: Don't let the user zoom too low
    {
        auto zoom_size = mConfig.Get_FieldFrameZoom_3_6_0();
        if (zoom_size < 0.01) {
            mConfig.Set_FieldFrameZoom_3_6_0(0.01);
        }
    }

    mCanvas = new FieldCanvas(this, static_cast<CalChartView*>(view), mConfig.Get_FieldFrameZoom_3_6_0(), mConfig);
    // set scroll rate 1 to 1, so we can have even scrolling of whole field
    mCanvas->SetScrollRate(1, 1);

    // Create all the other things attached to the frame:
    mControls = FieldControls::CreateToolBar(this, wxID_ANY, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW | wxAUI_TB_TEXT, mConfig);
    FieldControls::SetZoomAmount(this, mConfig.Get_FieldFrameZoom_3_6_0());

    mContinuityBrowser = new ContinuityBrowser(this, GetContinuityBrowserConstructSize(), mConfig);
    mFieldThumbnailBrowser = new FieldThumbnailBrowser(mConfig, this, wxID_ANY, wxDefaultPosition, GetFieldThumbnailBrowserConstructSize());
    mAnimationErrorsPanel = new AnimationErrorsPanel(this);
    mAnimationPanel = new AnimationPanel(mConfig, this);
    mPrintContinuityEditor = new PrintContinuityEditor(this, mConfig);

    // for doing mini and main panels
    mShadowAnimationPanel = new AnimationPanel(mConfig, this);

    // update our lookups:
    mLookupEnumToSubWindow[CALCHART__ViewFieldThumbnail] = mFieldThumbnailBrowser;
    mLookupEnumToSubWindow[CALCHART__ViewFieldControls] = mControls;
    mLookupEnumToSubWindow[CALCHART__ViewContinuityInfo] = mContinuityBrowser;
    mLookupEnumToSubWindow[CALCHART__ViewAnimationErrors] = mAnimationErrorsPanel;
    mLookupEnumToSubWindow[CALCHART__ViewAnimation] = mAnimationPanel;
    mLookupEnumToSubWindow[CALCHART__ViewPrintContinuity] = mPrintContinuityEditor;
    mLookupEnumToSubWindow[CALCHART__ViewSelectAndMoveToolBar] = mSelectAndMoveToolBar;
    mLookupEnumToSubWindow[CALCHART__ViewMarcherToolBar] = mMarcherToolBar;

    for (auto&& i : mLookupEnumToSubWindow) {
        mLookupSubWindowToEnum[i.second] = i.first;
    }

    mAnimationPanel->SetInMiniMode(true);
    mShadowAnimationPanel->SetInMiniMode(false);

    // now patch up the controls with the views:
    SetViewsOnComponents(GetFieldView());

    // Now determine what to show and not show.
    mAUIManager->AddPane(mCanvas, wxAuiPaneInfo().Name("Field").CenterPane().Show());
    mAUIManager->AddPane(mShadowAnimationPanel, wxAuiPaneInfo().Name("ShadowAnimation").CenterPane().Hide());

    mAUIManager->AddPane(mFieldThumbnailBrowser, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewFieldThumbnail]).Caption(kAUIEnumToString[CALCHART__ViewFieldThumbnail]).Left().BestSize(GetFieldThumbnailBrowserSize()));
    mAUIManager->AddPane(mAnimationPanel, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewAnimation]).Caption(kAUIEnumToString[CALCHART__ViewAnimation]).Left().BestSize(GetAnimationSize()));
    mAUIManager->AddPane(mContinuityBrowser, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewContinuityInfo]).Caption(kAUIEnumToString[CALCHART__ViewContinuityInfo]).Right().BestSize(GetContinuityBrowserSize()));
    mAUIManager->AddPane(mAnimationErrorsPanel, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewAnimationErrors]).Caption(kAUIEnumToString[CALCHART__ViewAnimationErrors]).Right().BestSize(GetAnimationErrorsSize()));
    mAUIManager->AddPane(mPrintContinuityEditor, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewPrintContinuity]).Caption(kAUIEnumToString[CALCHART__ViewPrintContinuity]).Right().BestSize(GetPrintContinuitySize()));
    mAUIManager->AddPane(mControls, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewFieldControls]).Caption(kAUIEnumToString[CALCHART__ViewFieldControls]).ToolbarPane().Top());
    mAUIManager->AddPane(mSelectAndMoveToolBar, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewSelectAndMoveToolBar]).Caption(kAUIEnumToString[CALCHART__ViewSelectAndMoveToolBar]).ToolbarPane().Left());
    mAUIManager->AddPane(mMarcherToolBar, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewMarcherToolBar]).Caption(kAUIEnumToString[CALCHART__ViewMarcherToolBar]).ToolbarPane().Top());

    mAUIManager->Update();

    // restore the manager with the Current visability
    if (auto lastLayout = mConfig.Get_CalChartFrameAUILayout_3_6_1(); lastLayout != "") {
        mAUIManager->LoadPerspective(lastLayout, true);
    }

    // adjust the menu items to reflect.
    for (int i = CALCHART__ViewFieldThumbnail; i <= CALCHART__ViewMarcherToolBar; ++i) {
        ChangeVisibility(mAUIManager->GetPane(mLookupEnumToSubWindow[i]).IsShown(), i);
    }

    ChangeMainFieldVisibility(mMainFieldVisible);

    SetTitle(static_cast<CalChartDoc*>(doc)->GetTitle());

    // Update the command processor with the undo/redo menu items
    edit_menu->FindItem(wxID_UNDO)->Enable(false);
    edit_menu->FindItem(wxID_REDO)->Enable(false);
    doc->GetCommandProcessor()->SetEditMenu(edit_menu);
    doc->GetCommandProcessor()->Initialize();

    // Update the tool bar
    SetCurrentSelect(mCanvas->GetCurrentSelect());
    SetCurrentMove(mCanvas->GetCurrentMove());
    refreshGhostOptionStates();

    // Show the frame
    OnUpdate();
    mCanvas->wxWindow::SetFocus();
    // make sure we restore the scroll to the last place we were.
    mCanvas->Scroll(mConfig.Get_FieldCanvasScrollX(), mConfig.Get_FieldCanvasScrollY());

    mCanvas->Refresh();

    // re-set the size
    SetSize(size);
    Show(true);
}

CalChartFrame::~CalChartFrame()
{
    mAUIManager->UnInit();
}

// View has closed us, clean things up for next time.
void CalChartFrame::OnClose()
{
    mConfig.Set_FieldFramePositionX(GetPosition().x);
    mConfig.Set_FieldFramePositionY(GetPosition().y);
    mConfig.Set_FieldFrameWidth(GetSize().x);
    mConfig.Set_FieldFrameHeight(GetSize().y);
    mConfig.Set_FieldCanvasScrollX(mCanvas->GetViewStart().x);
    mConfig.Set_FieldCanvasScrollY(mCanvas->GetViewStart().y);

    // just to make sure we never end up hiding the Field
    ShowFieldAndHideAnimation(true);
    mConfig.Set_CalChartFrameAUILayout_3_6_1(mAUIManager->SavePerspective());
    SetViewsOnComponents(nullptr);
}

void CalChartFrame::OnCmdAppend(wxCommandEvent&) { AppendShow(); }

void CalChartFrame::OnCmdImportCont(wxCommandEvent&) { ImportContFile(); }

// the default wxView print doesn't handle landscape.  rolling our own
void CalChartFrame::OnCmdPrint(wxCommandEvent&)
{
    // grab our current page setup.
    wxPrinter printer(&(wxGetApp().GetGlobalPrintDialog()));
    CalChartPrintout printout("My Printout", *GetShow(), mConfig);
    wxPrintDialogData& printDialog = printer.GetPrintDialogData();

    int minPage, maxPage, pageFrom, pageTo;
    printout.GetPageInfo(&minPage, &maxPage, &pageFrom, &pageTo);
    printDialog.SetMinPage(minPage);
    printDialog.SetMaxPage(maxPage);

    if (!printer.Print(this, &printout, true)) {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
            wxMessageBox("A problem was encountered when trying to print", "Printing", wxOK);
        }
    } else {
        wxGetApp().GetGlobalPrintDialog() = printer.GetPrintDialogData();
    }
}

// the default wxView print doesn't handle landscape.  rolling our own
void CalChartFrame::OnCmdPrintPreview(wxCommandEvent&)
{
    // grab our current page setup.
    auto preview = new wxPrintPreview(
        new CalChartPrintout("My Printout", *GetShow(), mConfig),
        new CalChartPrintout("My Printout", *GetShow(), mConfig),
        &wxGetApp().GetGlobalPrintDialog());
    if (!preview->Ok()) {
        delete preview;
        wxMessageBox("There was a problem previewing.\nPerhaps your current printer is not set correctly?", "Previewing", wxOK);
        return;
    }
    auto frame = new wxPreviewFrame(preview, this, "Show Print Preview");
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show(true);
}

void CalChartFrame::OnCmdPageSetup(wxCommandEvent&)
{
    wxPageSetupData mPageSetupData;
    mPageSetupData.EnableOrientation(true);

    wxPageSetupDialog pageSetupDialog(this, &mPageSetupData);
    if (pageSetupDialog.ShowModal() == wxID_OK)
        mPageSetupData = pageSetupDialog.GetPageSetupData();
    // pass the print data to our global print dialog
    wxGetApp().GetGlobalPrintDialog().SetPrintData(mPageSetupData.GetPrintData());
}

void CalChartFrame::OnCmdLegacyPrint(wxCommandEvent&)
{
    if (GetShow()) {
        auto localConfig = mConfig.Copy();
        PrintPostScriptDialog dialog(static_cast<CalChartDoc*>(GetDocument()), localConfig, this);
        if (dialog.ShowModal() == wxID_OK) {
            wxCalChart::AssignConfig(localConfig);
            dialog.PrintShow();
        }
    }
}

void CalChartFrame::OnCmdExportViewerFile(wxCommandEvent&)
{
    if (GetShow()) {
        wxFileDialog saveFileDialog(this, _("Save viewer file"), "", "", "viewer files (*.viewer)|*.viewer", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (saveFileDialog.ShowModal() == wxID_CANCEL)
            return;

        if (!GetShow()->exportViewerFile(saveFileDialog.GetPath())) {
            wxMessageBox("There was a problem exporting the viewer file.\n" + saveFileDialog.GetPath(), "Exporting Viewer File");
            return;
        }
    }
}

void CalChartFrame::OnCmdPreferences(wxCommandEvent&)
{
    auto localConfig = mConfig.Copy();
    CalChartPreferences dialog1(this, localConfig);
    if (dialog1.ShowModal() == wxID_OK) {
        // here's where we flush out the configuration.
        wxCalChart::AssignConfig(localConfig);
        GetFieldView()->OnUpdate(nullptr);
    }
}

void CalChartFrame::OnCmdInsertBefore(wxCommandEvent&)
{
    CalChart::Show::Sheet_container_t sht(1, *GetShow()->GetCurrentSheet());
    GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum());
    GetFieldView()->GoToPrevSheet();
}

void CalChartFrame::OnCmdInsertAfter(wxCommandEvent&)
{
    CalChart::Show::Sheet_container_t sht(1, *GetShow()->GetCurrentSheet());
    GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum() + 1);
    GetFieldView()->GoToNextSheet();
}

void CalChartFrame::OnCmdInsertFromOtherShow(wxCommandEvent&)
{
    auto s = wxFileSelector("Add Sheets from Other Shows", wxEmptyString, wxEmptyString, wxEmptyString, file_wild);
    if (s.IsEmpty()) {
        return;
    }
    CalChartDoc show;
    if (!(&show)->OnOpenDocument(s)) {
        (void)wxMessageBox("Error Opening show", "Load Error");
        return;
    }
    if ((&show)->GetNumPoints() != GetShow()->GetNumPoints()) {
        (void)wxMessageBox("The blocksize doesn't match", "Import Error");
        return;
    }
    auto prompt = "Enter the %s sheet number (highest possible: %i)";
    auto begin = wxGetTextFromUser(
        wxString::Format(prompt, "beginning", (&show)->GetNumSheets()),
        "First Sheet Number", "1", this);
    long beginValue;
    if (!begin || !begin.ToLong(&beginValue) || beginValue < 1 || beginValue > (&show)->GetNumSheets()) {
        (void)wxMessageBox("Not a valid sheet number", "Insert Failed");
        return;
    }
    long endValue;
    if (beginValue != (&show)->GetNumSheets()) {
        wxString end = wxGetTextFromUser(
            wxString::Format(prompt, "ending", (&show)->GetNumSheets()),
            "Last Sheet Number", begin, this);
        if (!end || !end.ToLong(&endValue) || endValue < beginValue || endValue > (&show)->GetNumSheets()) {
            (void)wxMessageBox("Not a valid sheet number", "Insert Failed");
            return;
        }
    } else {
        // don't ask user if the first sheet they want to copy over is the last
        // sheet in the show being copied from
        endValue = beginValue;
    }

    CalChart::Show::Sheet_container_t sheets((&show)->GetNthSheet(static_cast<int>(beginValue) - 1),
        (&show)->GetNthSheet(static_cast<int>(endValue)));
    GetFieldView()->DoInsertSheets(
        sheets, GetFieldView()->GetCurrentSheetNum() + 1);
}

void CalChartFrame::OnCmdCopySheet(wxCommandEvent&)
{
    if (wxTheClipboard->Open()) {
        std::unique_ptr<wxCustomDataObject> clipboardObject(
            new wxCustomDataObject(kSheetDataClipboardFormat));
        auto serializedSheet = GetShow()->GetCurrentSheet()->SerializeSheet();

        auto numPoints = GetShow()->GetNumPoints();

        auto bytesForNumPoints = sizeof(numPoints);
        auto bytesForSheetData = serializedSheet.size() * sizeof(uint8_t);
        auto totalBytes = bytesForNumPoints + bytesForSheetData;
        std::vector<char> clipboardData(totalBytes);
        memcpy(clipboardData.data(), &numPoints, bytesForNumPoints);
        memcpy(clipboardData.data() + bytesForNumPoints, serializedSheet.data(),
            bytesForSheetData);

        clipboardObject->SetData(totalBytes, clipboardData.data());

        wxTheClipboard->SetData(clipboardObject.release());
        wxTheClipboard->Close();
    }
}

void CalChartFrame::OnCmdPasteSheet(wxCommandEvent&)
{
    if (wxTheClipboard->Open()) {
        if (wxTheClipboard->IsSupported(kSheetDataClipboardFormat)) {
            wxCustomDataObject clipboardObject(kSheetDataClipboardFormat);
            wxTheClipboard->GetData(clipboardObject);

            auto numPoints = GetShow()->GetNumPoints();
            memcpy(&numPoints, clipboardObject.GetData(), sizeof(numPoints));
            if (numPoints != GetShow()->GetNumPoints()) {
                wxMessageBox(wxString::Format("Cannot paste - number of points in pasted sheet (%i) does not match number of points in current show (%i)",
                    numPoints, GetShow()->GetNumPoints()));
                wxTheClipboard->Close();
                return;
            }
            auto reader = CalChart::Reader({ static_cast<std::byte const*>(clipboardObject.GetData()) + sizeof(numPoints),
                clipboardObject.GetDataSize() - sizeof(numPoints) });
            reader.Get<uint32_t>();
            reader.Get<uint32_t>();

            CalChart::Show::Sheet_container_t sht(1, CalChart::Sheet(numPoints, reader));
            GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum());
        }
        wxTheClipboard->Close();
    }
}

void CalChartFrame::OnCmdDelete(wxCommandEvent&)
{
    if (GetFieldView()->GetNumSheets() > 1) {
        GetFieldView()->DoDeleteSheet(GetFieldView()->GetCurrentSheetNum());
    }
}

// grey out if we're on a sheet
void CalChartFrame::OnCmdRelabel(wxCommandEvent&)
{
    if (GetFieldView()->GetCurrentSheetNum() + 1 >= GetFieldView()->GetNumSheets()) {
        (void)wxMessageBox("This can't used on the last stuntsheet", "Relabel sheets");
        return;
    }
    auto failMessage = GetFieldView()->DoRelabel();
    if (failMessage.has_value()) {
        (void)wxMessageBox(*failMessage, "Relabel sheets");
    }
}

void CalChartFrame::OnCmdEditPrintCont(wxCommandEvent&)
{
    mAUIManager->GetPane("Print Continuity").Show(true);
    mAUIManager->Update();
}

void CalChartFrame::OnCmdSetSheetTitle(wxCommandEvent&)
{
    wxString s;
    if (GetShow()) {
        s = wxGetTextFromUser("Enter the sheet title",
            GetShow()->GetCurrentSheet()->GetName(),
            GetShow()->GetCurrentSheet()->GetName(), this);
        if (!s.IsEmpty()) {
            GetFieldView()->DoSetSheetTitle(s);
        }
    }
}

void CalChartFrame::OnCmdSetBeats(wxCommandEvent&)
{
    wxString s;
    if (GetShow()) {
        wxString buf;
        buf.sprintf("%u", GetShow()->GetCurrentSheet()->GetBeats());
        s = wxGetTextFromUser("Enter the number of beats",
            GetShow()->GetCurrentSheet()->GetName(), buf, this);
        if (!s.empty()) {
            long val;
            if (s.ToLong(&val)) {
                GetFieldView()->DoSetSheetBeats(static_cast<int>(val));
            }
        }
    }
}

void CalChartFrame::OnCmdSetupMarchers(wxCommandEvent&)
{
    if (GetShow()) {
        SetupMarchers dialog(*GetShow(), this);
        if (dialog.ShowModal() == wxID_OK) {
            GetFieldView()->DoSetupMarchers(dialog.GetLabelsAndInstruments(), dialog.GetNumberColumns());
        }
    }
}

void CalChartFrame::OnCmdSetupInstruments(wxCommandEvent&)
{
    if (GetShow()) {
        SetupInstruments dialog(*GetShow(), this);
        if (dialog.ShowModal() == wxID_OK) {
            GetFieldView()->DoSetInstruments(dialog.GetInstruments());
        }
    }
}

void CalChartFrame::OnCmdSetMode(wxCommandEvent&) { SetMode(); }

void CalChartFrame::OnCmdPointPicker(wxCommandEvent&)
{
    if (!GetShow()) {
        return;
    }
    PointPicker dialog(this, *GetShow());
    if (dialog.ShowModal() == wxID_OK) {
        auto selections = dialog.GetMarchersSelected();
        GetShow()->SetSelectionList(GetShow()->MakeSelectByLabels(selections));
    }
}

void CalChartFrame::OnCmdSelectAll(wxCommandEvent&)
{
    if (GetShow()) {
        GetShow()->SetSelectionList(GetShow()->MakeSelectAll());
    }
}

void CalChartFrame::ShowFieldAndHideAnimation(bool showField)
{
    // when we are going to show field, pause the animation
    if (showField) {
        mShadowAnimationPanel->SetPlayState(false);
    }
    mAUIManager->GetPane("Field").Show(showField);
    mAUIManager->GetPane("ShadowAnimation").Show(!showField);
    mAUIManager->Update();
}

void CalChartFrame::OnCmdAbout(wxCommandEvent&) { CalChartSplash::About(); }

void CalChartFrame::OnCmdHelp(wxCommandEvent&) { CalChartSplash::Help(); }

void CalChartFrame::OnCmd_prev_ss(wxCommandEvent&)
{
    GetFieldView()->GoToPrevSheet();
}

void CalChartFrame::OnCmd_next_ss(wxCommandEvent&)
{
    GetFieldView()->GoToNextSheet();
}

void CalChartFrame::OnCmd_box(wxCommandEvent&)
{
    SetCurrentSelect(CalChart::Select::Box);
}

void CalChartFrame::OnCmd_poly(wxCommandEvent&)
{
    SetCurrentSelect(CalChart::Select::Poly);
}

void CalChartFrame::OnCmd_lasso(wxCommandEvent&)
{
    SetCurrentSelect(CalChart::Select::Lasso);
}

void CalChartFrame::OnCmd_swap(wxCommandEvent&)
{
    SetCurrentSelect(CalChart::Select::Swap);
}

void CalChartFrame::OnCmd_curve(wxCommandEvent&)
{
    mSelectAndMoveToolBar->ToggleTool(CALCHART__curve, true);
    mCanvas->SetDrawingCurve(true);
}

void CalChartFrame::OnCmd_move(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::Normal);
}

void CalChartFrame::OnCmd_shape_line(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::ShapeLine);
}

void CalChartFrame::OnCmd_shape_x(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::ShapeX);
}

void CalChartFrame::OnCmd_shape_cross(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::ShapeCross);
}

void CalChartFrame::OnCmd_shape_box(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::ShapeRectange);
}

void CalChartFrame::OnCmd_shape_ellipse(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::ShapeEllipse);
}

void CalChartFrame::OnCmd_shape_draw(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::ShapeDraw);
}

void CalChartFrame::OnCmd_line(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::MoveLine);
}

void CalChartFrame::OnCmd_rot(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::MoveRotate);
}

void CalChartFrame::OnCmd_shear(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::MoveShear);
}

void CalChartFrame::OnCmd_reflect(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::MoveReflect);
}

void CalChartFrame::OnCmd_size(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::MoveSize);
}

void CalChartFrame::OnCmd_genius(wxCommandEvent&)
{
    SetCurrentMove(CalChart::MoveMode::MoveGenius);
}

void CalChartFrame::OnCmd_label_left(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsLabel(false);
}

void CalChartFrame::OnCmd_label_right(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsLabel(true);
}

void CalChartFrame::OnCmd_label_flip(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsLabelFlip();
}

void CalChartFrame::OnCmd_label_hide(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsLabelVisibility(false);
}

void CalChartFrame::OnCmd_label_show(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsLabelVisibility(true);
}

void CalChartFrame::OnCmd_label_visibility_toggle(wxCommandEvent&)
{
    GetFieldView()->DoTogglePointsLabelVisibility();
}

void CalChartFrame::OnCmd_setsym0(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsSymbol(CalChart::SYMBOL_PLAIN);
}

void CalChartFrame::OnCmd_setsym1(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsSymbol(CalChart::SYMBOL_SOL);
}

void CalChartFrame::OnCmd_setsym2(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsSymbol(CalChart::SYMBOL_BKSL);
}

void CalChartFrame::OnCmd_setsym3(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsSymbol(CalChart::SYMBOL_SL);
}

void CalChartFrame::OnCmd_setsym4(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsSymbol(CalChart::SYMBOL_X);
}

void CalChartFrame::OnCmd_setsym5(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsSymbol(CalChart::SYMBOL_SOLBKSL);
}

void CalChartFrame::OnCmd_setsym6(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsSymbol(CalChart::SYMBOL_SOLSL);
}

void CalChartFrame::OnCmd_setsym7(wxCommandEvent&)
{
    GetFieldView()->DoSetPointsSymbol(CalChart::SYMBOL_SOLX);
}

void CalChartFrame::OnChar(wxKeyEvent& event) { mCanvas->OnChar(event); }

void CalChartFrame::OnCmd_AddBackgroundImage(wxCommandEvent&)
{
    wxString filename;
    filename = wxLoadFileSelector("Select a background image",
        "BMP files (*.bmp)|*.bmp"
#if wxUSE_LIBPNG
        "|PNG files (*.png)|*.png"
#endif
#if wxUSE_LIBJPEG
        "|JPEG files (*.jpg)|*.jpg"
#endif
#if wxUSE_GIF
        "|GIF files (*.gif)|*.gif"
#endif
#if wxUSE_LIBTIFF
        "|TIFF files (*.tif)|*.tif"
#endif
#if wxUSE_PCX
        "|PCX files (*.pcx)|*.pcx"
#endif
    );
    if (!filename.empty()) {
        wxImage image;
        if (!image.LoadFile(filename)) {
            wxLogError("Couldn't load image from '%s'.", filename.c_str());
            return;
        }
        if (!GetFieldView()->AddBackgroundImage(image)) {
            wxLogError("Couldn't load image from '%s'.", filename.c_str());
            return;
        }
        GetFieldView()->DoPictureAdjustment(true);
        GetFieldView()->DoDrawBackground(true);
        GetMenuBar()->FindItem(CALCHART__ShowBackgroundImages)->Check(true);
        GetMenuBar()->FindItem(CALCHART__AdjustBackgroundImageMode)->Check(true);
        mCanvas->Refresh();
    }
}

void CalChartFrame::OnCmd_AdjustBackgroundImageMode(wxCommandEvent&)
{
    bool toggle = !GetFieldView()->DoingPictureAdjustment();
    GetFieldView()->DoPictureAdjustment(toggle);
    GetMenuBar()->FindItem(CALCHART__AdjustBackgroundImageMode)->Check(toggle);
    if (toggle) {
        GetFieldView()->DoDrawBackground(toggle);
        GetMenuBar()->FindItem(CALCHART__ShowBackgroundImages)->Check(toggle);
    }
    mCanvas->Refresh();
}

void CalChartFrame::OnCmd_ShowBackgroundImages(wxCommandEvent&)
{
    bool toggle = !GetFieldView()->DoingDrawBackground();
    GetFieldView()->DoDrawBackground(toggle);
    GetMenuBar()->FindItem(CALCHART__ShowBackgroundImages)->Check(toggle);
    if (!toggle) {
        GetFieldView()->DoPictureAdjustment(toggle);
        GetMenuBar()->FindItem(CALCHART__AdjustBackgroundImageMode)->Check(toggle);
    }
    mCanvas->Refresh();
}

void CalChartFrame::OnCmd_GhostOption(wxCommandEvent& event)
{
    auto selection = static_cast<wxChoice*>(FindWindow(CALCHART__GhostControls))->GetSelection();
    auto which_option = (event.GetId() == CALCHART__GhostControls) ? selection
        : (event.GetId() == CALCHART__GhostOff)                    ? 0
        : (event.GetId() == CALCHART__GhostNextSheet)              ? 1
        : (event.GetId() == CALCHART__GhostPreviousSheet)          ? 2
                                                                   : 3;
    switch (which_option) {
    case 0:
        GetFieldView()->SetGhostSource(GhostSource::disabled);
        break;
    case 1:
        GetFieldView()->SetGhostSource(GhostSource::next);
        break;
    case 2:
        GetFieldView()->SetGhostSource(GhostSource::previous);
        break;
    case 3: {
        wxString targetSheet = wxGetTextFromUser("Enter the sheet number to ghost:", "Ghost Sheet", "1", this);
        long targetSheetNum = 0;
        if (targetSheet.ToLong(&targetSheetNum)) {
            GetFieldView()->SetGhostSource(GhostSource::specific, static_cast<int>(targetSheetNum) - 1);
        } else {
            wxMessageBox("The input must be a number.", "Operation failed.");
        }
    } break;
    }
    refreshGhostOptionStates();
    GetCanvas()->Refresh();
}

void CalChartFrame::OnCmd_InstrumentSelection(wxCommandEvent&)
{
    auto choice = static_cast<wxChoice*>(FindWindow(CALCHART__InstrumentChoice));
    auto selection = choice->GetString(choice->GetSelection());
    if (selection != "" && GetShow()) {
        GetShow()->SetSelectionList(GetShow()->MakeSelectByInstrument(selection));
    }
}

void CalChartFrame::OnCmd_SymbolSelection(wxCommandEvent&)
{
    auto choice = static_cast<wxChoice*>(FindWindow(CALCHART__SymbolChoice));
    auto selection = choice->GetString(choice->GetSelection());
    if (selection != "" && GetShow()) {
        GetShow()->SetSelectionList(GetShow()->MakeSelectByInstrument(selection));
    }
}

void CalChartFrame::OnCmd_MarcherSelection(wxCommandEvent&)
{
    auto choice = static_cast<wxChoice*>(FindWindow(CALCHART__MarcherChoice));
    auto selection = choice->GetString(choice->GetSelection());
    if (selection != "" && GetShow()) {
        GetShow()->SetSelectionList(GetShow()->MakeSelectByLabel(selection));
    }
}

void CalChartFrame::refreshGhostOptionStates()
{
    bool active = GetFieldView()->GetGhostModuleIsActive();
    GetMenuBar()->FindItem(CALCHART__GhostOff)->Enable(active);
    FieldControls::SetGhostChoice(mControls, static_cast<int>(GetFieldView()->GetGhostSource()));
}

void CalChartFrame::refreshInUse()
{
    // handle any changes to the instrument, and update the instrument selector
    auto instruments = GetShow()->GetPointsInstrument();
    auto currentInstruments = std::set(instruments.begin(), instruments.end());
    FieldControls::SetInstrumentsInUse(this, { currentInstruments.begin(), currentInstruments.end() });
    auto symbols = GetShow()->GetPointsSymbol();
    auto currentsymbols = std::set(symbols.begin(), symbols.end());
    FieldControls::SetLabelsInUse(this, GetShow()->GetPointsLabel());
}

void CalChartFrame::OnCmd_AdjustViews(wxCommandEvent& event)
{
    ChangeVisibility(!mAUIManager->GetPane(mLookupEnumToSubWindow[event.GetId()]).IsShown(), event.GetId());
}

void CalChartFrame::OnCmd_SwapAnimation(wxCommandEvent&)
{
    ChangeMainFieldVisibility(!mMainFieldVisible);
}

void CalChartFrame::AUIIsClose(wxAuiManagerEvent& event)
{
    auto id = mLookupSubWindowToEnum[event.GetPane()->window];
    ChangeVisibility(!mAUIManager->GetPane(mLookupEnumToSubWindow[id]).IsShown(), id);
}

void CalChartFrame::ChangeVisibility(bool show, int itemid)
{
    if (itemid < CALCHART__ViewFieldThumbnail || itemid > CALCHART__ViewMarcherToolBar) {
        return;
    }
    auto name = kAUIEnumToString[itemid];
    if (show) {
        GetMenuBar()->FindItem(itemid)->SetItemLabel(std::string("Hide ") + name);
    } else {
        GetMenuBar()->FindItem(itemid)->SetItemLabel(std::string("Show ") + name);
    }
    mAUIManager->GetPane(mLookupEnumToSubWindow[itemid]).Show(show);
    mAUIManager->Update();
}

void CalChartFrame::ChangeMainFieldVisibility(bool show)
{
    mMainFieldVisible = show;
    if (mMainFieldVisible) {
        GetMenuBar()->FindItem(CALCHART__ViewSwapFieldAndAnimate)->SetItemLabel("View Animation\tCTRL-RETURN");
    } else {
        GetMenuBar()->FindItem(CALCHART__ViewSwapFieldAndAnimate)->SetItemLabel("View Field\tCTRL-RETURN");
    }
    ShowFieldAndHideAnimation(mMainFieldVisible);
}

void CalChartFrame::OnCmd_ResetReferencePoint(wxCommandEvent&)
{
    GetFieldView()->DoResetReferencePoint();
}

void CalChartFrame::OnCmd_ChangedColorPalette(wxCommandEvent&)
{
    GetFieldView()->OnUpdate(nullptr);
}

void CalChartFrame::OnCmd_SolveTransition(wxCommandEvent&)
{
    if (GetShow()) {
        TransitionSolverFrame* transitionSolver = new TransitionSolverFrame(static_cast<CalChartDoc*>(GetDocument()), this, wxID_ANY, "Transition Solver");
        transitionSolver->Show();
    }
}

// Append a show with file selector
void CalChartFrame::AppendShow()
{
    auto s = wxFileSelector("Append show", wxEmptyString, wxEmptyString, wxEmptyString, file_wild);
    if (s.IsEmpty()) {
        return;
    }
    auto shw = std::make_unique<CalChartDoc>();
    if (!shw->OnOpenDocument(s)) {
        (void)wxMessageBox("Error Opening show", "Load Error");
        return;
    }
    auto result = GetFieldView()->DoAppendShow(std::move(shw));
    if (result.has_value()) {
        (void)wxMessageBox(*result, "Append Error");
        return;
    }
}

// Append a show with file selector
void CalChartFrame::ImportContFile()
{
    if (std::string s = wxFileSelector("Import Continuity", wxEmptyString, wxEmptyString, wxEmptyString, "*.txt");
        !s.empty()) {
        GetFieldView()->DoImportPrintableContinuity(s);
    }
}

std::pair<CalChart::Coord::units, CalChart::Coord::units> CalChartFrame::GridChoice() const
{
    return FieldControls::GridChoice(mControls);
}

std::pair<CalChart::Coord::units, CalChart::Coord::units> CalChartFrame::ToolGridChoice() const
{
    return FieldControls::ToolGridChoice(mControls);
}

void CalChartFrame::SetCurrentSelect(CalChart::Select select)
{
    // retoggle the tool because we want it to draw as selected
    int toggleID = (select == CalChart::Select::Poly) ? CALCHART__poly
        : (select == CalChart::Select::Lasso)         ? CALCHART__lasso
        : (select == CalChart::Select::Swap)          ? CALCHART__swap
                                                      : CALCHART__box;
    mSelectAndMoveToolBar->ToggleTool(toggleID, true);

    mCanvas->SetCurrentSelect(select);
}

void CalChartFrame::SetCurrentMove(CalChart::MoveMode type)
{
    ToolBarSetCurrentMove(type);
    mCanvas->SetCurrentMove(type);
}

// call by the canvas to inform that the move has been set.  Don't call back to canvas
void CalChartFrame::ToolBarSetCurrentMove(CalChart::MoveMode type)
{
    // retoggle the tool because we want it to draw as selected
    mSelectAndMoveToolBar->ToggleTool(CALCHART__move + toUType(type), true);
}

// call by the canvas to inform that the curve draw is done.  Don't call back to canvas
void CalChartFrame::ToolBarUnsetDrawingCurve()
{
    mSelectAndMoveToolBar->ToggleTool(CALCHART__curve, false);
}

void CalChartFrame::SetMode()
{
    if (GetShow()) {
        ModeSetupDialog dialog(this, GetShow()->GetShowMode(), mConfig);
        if (dialog.ShowModal() == wxID_OK) {
            GetFieldView()->DoSetMode(dialog.GetShowMode());
        }
    }
}

void CalChartFrame::OnCmd_ReferenceNumber(wxCommandEvent& event)
{
    GetFieldView()->SetActiveReferencePoint(static_cast<wxChoice*>(FindWindow(event.GetId()))->GetSelection());
}

void CalChartFrame::OnEnableDrawPaths(wxCommandEvent& event)
{
    GetFieldView()->OnEnableDrawPaths(event.IsChecked());
    GetMenuBar()->FindItem(CALCHART__draw_paths)->Check(event.IsChecked());
}

void CalChartFrame::OnCmd_DrawPaths(wxCommandEvent& event)
{
    GetFieldView()->OnEnableDrawPaths(event.IsChecked());
}

void CalChartFrame::zoom_callback(wxCommandEvent&)
{
    auto zoom_amount = FieldControls::GetZoomAmount(this);
    if (zoom_amount == 0) {
        zoom_amount = mCanvas->ZoomToFitFactor();
    }
    do_zoom(zoom_amount);
}

void CalChartFrame::OnCmd_ZoomFit(wxCommandEvent&)
{
    do_zoom(mCanvas->ZoomToFitFactor());
}

void CalChartFrame::OnCmd_ZoomIn(wxCommandEvent&)
{
    // because zoom is truncated to 2 decimal places, make sure we at least increment by 1.
    auto original_zoom = mCanvas->GetZoom();
    auto new_zoom = original_zoom * 1.20;
    if ((new_zoom - original_zoom) < 0.01) {
        new_zoom += 0.01;
    }
    if (new_zoom < 10) {
        do_zoom(new_zoom);
    }
}

void CalChartFrame::OnCmd_ZoomOut(wxCommandEvent&)
{
    auto new_zoom = mCanvas->GetZoom() * 0.8;
    if (new_zoom > 0.01) {
        do_zoom(new_zoom);
    }
}

void CalChartFrame::zoom_callback_textenter(wxCommandEvent& event)
{
    auto zoomtxt = static_cast<wxComboBox*>(FindWindow(event.GetId()))->GetValue();
    // strip the trailing '%' if it exists
    if (zoomtxt.Length() && (zoomtxt.Last() == '%')) {
        zoomtxt.RemoveLast();
    }
    double zoom_amount = 1.0;
    if (zoomtxt.ToDouble(&zoom_amount)) {
        zoom_amount /= 100.0;
    } else {
        wxString msg("Please enter a valid number\n");
        wxMessageBox(msg, "Invalid number", wxICON_INFORMATION | wxOK);
        // return if not valid
        return;
    }
    do_zoom(zoom_amount);
}

void CalChartFrame::do_zoom(float zoom_amount)
{
    zoom_amount = ToolBarSetZoom(zoom_amount);
    mCanvas->SetZoomAroundCenter(zoom_amount);
}

float CalChartFrame::ToolBarSetZoom(float zoom_amount)
{
    zoom_amount = std::max(zoom_amount, 0.01f);
    FieldControls::SetZoomAmount(this, zoom_amount);
    mConfig.Set_FieldFrameZoom_3_6_0(zoom_amount);
    return zoom_amount;
}

wxString CalChartFrame::BeatStatusText() const
{
    wxString result;
    auto sht = GetShow()->GetCurrentSheet();
    auto num = GetShow()->GetNumSheets();
    auto curr = GetFieldView()->GetCurrentSheetNum() + 1;

    result.sprintf("%s%d of %d \"%.32s\" %d beats",
        GetShow()->IsModified() ? "* " : "", curr, num,
        (sht != GetShow()->GetSheetEnd()) ? wxString(sht->GetName())
                                          : "",
        (sht != GetShow()->GetSheetEnd()) ? sht->GetBeats() : 0);
    return result;
}

wxString CalChartFrame::PointStatusText() const
{
    auto const* show = GetShow();
    wxString result;
    auto sl = show->GetSelectionList();
    result << show->GetSelectionList().size() << " of "
           << show->GetNumPoints() << " selected";
    std::set<std::string> instruments;
    std::transform(sl.begin(), sl.end(), std::inserter(instruments, instruments.begin()), [&show](auto&& i) {
        return show->GetPointInstrument(i);
    });
    if (instruments.size()) {
        result << " [ ";
        auto firstTime = true;
        for (auto&& i : instruments) {
            if (!firstTime) {
                result << ", ";
            }
            firstTime = false;
            result << i;
        }
        result << " ]";
    }
    return result;
}

void CalChartFrame::OnUpdate()
{
    SetStatusText(BeatStatusText(), 1);
    SetStatusText(PointStatusText(), 2);

    SetTitle(GetDocument()->GetUserReadableName());
    mCanvas->Refresh();

    mContinuityBrowser->OnUpdate();
    mFieldThumbnailBrowser->OnUpdate();
    mAnimationErrorsPanel->OnUpdate();
    mAnimationPanel->OnUpdate();
    mPrintContinuityEditor->OnUpdate();

    mShadowAnimationPanel->OnUpdate();

    refreshInUse();
}

const CalChartView* CalChartFrame::GetFieldView() const
{
    return static_cast<const CalChartView*>(GetView());
}

CalChartView* CalChartFrame::GetFieldView()
{
    return static_cast<CalChartView*>(GetView());
}

const CalChartDoc* CalChartFrame::GetShow() const
{
    return static_cast<const CalChartDoc*>(GetDocument());
}

CalChartDoc* CalChartFrame::GetShow()
{
    return static_cast<CalChartDoc*>(GetDocument());
}

void CalChartFrame::SetViewsOnComponents(CalChartView* view)
{
    mCanvas->SetView(view);
    mContinuityBrowser->SetView(view);
    mFieldThumbnailBrowser->SetView(view);
    mAnimationErrorsPanel->SetView(view);
    mAnimationPanel->SetView(view);
    mPrintContinuityEditor->SetView(view);
    mShadowAnimationPanel->SetView(view);
}
