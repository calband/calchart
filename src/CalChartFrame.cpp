/*
 * CalChartFrame.cpp
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

#include <cstring>
#include <fstream>
#include <sstream>

#include "CalChartFrame.h"

#include "AnimationErrorsPanel.h"
#include "AnimationPanel.h"
#include "CalChartApp.h"
#include "CalChartView.h"
#include "ContinuityBrowser.h"
#include "FieldCanvas.h"
#include "FieldThumbnailBrowser.h"
#include "PrintContinuityEditor.h"
#include "TopFrame.h"
#include "cc_coord.h"
#include "cc_fileformat.h"
#include "cc_point.h"
#include "cc_preferences_ui.h"
#include "cc_sheet.h"
#include "cc_show.h"
#include "ccvers.h"
#include "confgr.h"
#include "draw.h"
#include "e7_transition_solver_ui.h"
#include "FieldControlsToolBar.h"
#include "mode_dialog.h"
#include "modes.h"
#include "platconf.h"
#include "print_ps_dialog.h"
#include "show_ui.h"
#include "CalChartToolBar.h"
#include "ui_enums.h"
#include "ColorPalette.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#ifdef __WXMSW__
#include <wx/helpwin.h>
#endif
#include <wx/clipbrd.h>
#include <wx/cmdproc.h>
#include <wx/tglbtn.h>
#include <wx/aui/auibar.h>

const wxString kSheetDataClipboardFormat = wxT("CC_sheet_clipboard_v1");

static const wxChar* file_wild = FILE_WILDCARDS;

extern wxPrintDialogData* gPrintDialogData;

static std::map<int, std::string> kAUIEnumToString = {
    { CALCHART__ViewFieldThumbnail, "Field Thumbnails" },
    { CALCHART__ViewFieldControls, "Controls ToolBar" },
    { CALCHART__ViewContinuityInfo, "Continuities" },
    { CALCHART__ViewAnimationErrors, "Animation Errors" },
    { CALCHART__ViewAnimation, "Animation" },
    { CALCHART__ViewPrintContinuity, "Print Continuity" },
    { CALCHART__ViewToolBar, "ToolBar" },
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
EVT_MENU(CALCHART__SETUP, CalChartFrame::OnCmdSetup)
EVT_MENU(CALCHART__SETMODE, CalChartFrame::OnCmdSetMode)
EVT_MENU(CALCHART__POINTS, CalChartFrame::OnCmdPoints)
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
EVT_MENU(CALCHART__ViewToolBar, CalChartFrame::OnCmd_AdjustViews)
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
EVT_COMBOBOX(CALCHART__slider_zoom, CalChartFrame::zoom_callback)
EVT_TEXT_ENTER(CALCHART__slider_zoom, CalChartFrame::zoom_callback_textenter)
EVT_CHOICE(CALCHART__refnum_callback, CalChartFrame::OnCmd_ReferenceNumber)
EVT_CHOICE(CALCHART__GhostControls, CalChartFrame::OnCmd_GhostOption)
EVT_CHECKBOX(CALCHART__draw_paths, CalChartFrame::OnEnableDrawPaths)
EVT_MENU(CALCHART__ResetReferencePoint, CalChartFrame::OnCmd_ResetReferencePoint)
EVT_BUTTON(CALCHART__ResetReferencePoint, CalChartFrame::OnCmd_ResetReferencePoint)
EVT_BUTTON(CALCHART__ChangedColorPalette, CalChartFrame::OnCmd_ChangedColorPalette)
EVT_MENU(CALCHART__E7TransitionSolver, CalChartFrame::OnCmd_SolveTransition)
EVT_AUI_PANE_CLOSE(CalChartFrame::AUIIsClose)
END_EVENT_TABLE()

class MyPrintout : public wxPrintout {
public:
    MyPrintout(const wxString& title, const CalChartDoc& show,
        const CalChartConfiguration& config_)
        : wxPrintout(title)
        , mShow(show)
        , mConfig(config_)
    {
    }
    virtual ~MyPrintout() {}
    virtual bool HasPage(int pageNum) { return pageNum <= mShow.GetNumSheets(); }
    virtual void GetPageInfo(int* minPage, int* maxPage, int* pageFrom,
        int* pageTo)
    {
        *minPage = 1;
        *maxPage = mShow.GetNumSheets();
        *pageFrom = 1;
        *pageTo = mShow.GetNumSheets();
    }
    virtual bool OnPrintPage(int pageNum)
    {
        wxDC* dc = wxPrintout::GetDC();
        auto sheet = mShow.GetNthSheet(pageNum - 1);

        int size = gPrintDialogData->GetPrintData().GetOrientation();

        DrawForPrinting(dc, mConfig, mShow, *sheet, 0, 2 == size);

        return true;
    }
    const CalChartDoc& mShow;
    const CalChartConfiguration& mConfig;
};

// Main frame constructor
CalChartFrame::CalChartFrame(wxDocument* doc, wxView* view,
    CalChartConfiguration& config_, wxDocParentFrame* frame,
    const wxPoint& pos, const wxSize& size)
    : wxDocChildFrame(doc, view, frame, -1, wxT("CalChart"), pos, size)
    , mConfig(config_)
    , mAUIManager( new wxAuiManager(this) )
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
    // Give it an icon
    SetBandIcon(this);

    // Give it a status line
    CreateStatusBar(3);
    SetStatusText(wxT("Welcome to Calchart v" wxT(STRINGIZE(CC_MAJOR_VERSION))
            wxT(".") wxT(STRINGIZE(CC_MINOR_VERSION)) wxT(".")
                wxT(STRINGIZE(CC_SUB_MINOR_VERSION))));

    // Make a menubar
    wxMenu* file_menu = new wxMenu;
    file_menu->Append(wxID_NEW, wxT("&New Show\tCTRL-N"), wxT("Create a new show"));
    file_menu->Append(wxID_OPEN, wxT("&Open...\tCTRL-O"), wxT("Load a saved show"));
    file_menu->Append(CALCHART__IMPORT_CONT_FILE, wxT("&Import Continuity...\tCTRL-I"), wxT("Import continuity text"));
    file_menu->Append(wxID_SAVE, wxT("&Save\tCTRL-S"), wxT("Save show"));
    file_menu->Append(wxID_SAVEAS, wxT("Save &As...\tCTRL-SHIFT-S"), wxT("Save show as a new name"));
    file_menu->Append(CALCHART__wxID_PRINT, wxT("&Print...\tCTRL-P"), wxT("Print this show"));
    file_menu->Append(CALCHART__wxID_PREVIEW, wxT("Preview...\tCTRL-SHIFT-P"), wxT("Preview this show"));
    file_menu->Append(wxID_PAGE_SETUP, wxT("Page Setup...\tCTRL-SHIFT-ALT-P"), wxT("Setup Pages"));
    file_menu->Append(CALCHART__LEGACY_PRINT, wxT("Print to PS..."), wxT("Print show to PostScript"));
    file_menu->Append(CALCHART__EXPORT_VIEWER_FILE, wxT("Export for Online Viewer..."), wxT("Export show to be viewed using the CalChart Online Viewer"));
    file_menu->Append(wxID_PREFERENCES, wxT("&Preferences\tCTRL-,"));
    file_menu->Append(wxID_CLOSE, wxT("Close Window\tCTRL-W"), wxT("Close this window"));
    file_menu->Append(wxID_EXIT, wxT("&Quit\tCTRL-Q"), wxT("Quit CalChart"));

    // A nice touch: a history of files visited. Use this menu.
    // causes a crash :(
    // view->GetDocumentManager()->FileHistoryUseMenu(file_menu);

    wxMenu* edit_menu = new wxMenu;
    edit_menu->Append(wxID_UNDO, wxT("&Undo\tCTRL-Z"));
    edit_menu->Append(wxID_REDO, wxT("&Redo\tCTRL-SHIFT-Z"));
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__INSERT_BEFORE, wxT("&Insert Sheet Before\tCTRL-["), wxT("Insert a new stuntsheet before this one"));
    edit_menu->Append(CALCHART__INSERT_AFTER, wxT("Insert Sheet &After\tCTRL-]"), wxT("Insert a new stuntsheet after this one"));
    edit_menu->Append(wxID_DELETE, wxT("&Delete Sheet\tCTRL-DEL"), wxT("Delete this stuntsheet"));
    edit_menu->Append(CALCHART__COPY_SHEET, wxT("&Copy Sheet\tCTRL-C"), wxT("Copy the current stuntsheet"));
    edit_menu->Append(CALCHART__PASTE_SHEET, wxT("&Paste Sheet\tCTRL-V"), wxT("Paste the current stuntsheet"));
    edit_menu->Append(CALCHART__INSERT_OTHER_SHOW, wxT("Insert Sheets From Other Show..."), wxT("Insert a saved stuntsheet after this one"));
    edit_menu->Append(CALCHART__RELABEL, wxT("&Relabel Sheets\tCTRL-R"), wxT("Relabel all stuntsheets after this one"));
    edit_menu->Append(CALCHART__APPEND_FILE, wxT("Append Show..."), wxT("Append a show to the end"));
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__SETUP, wxT("Set &Up Marchers...\tCTRL-U"), wxT("Setup number of marchers"));
    edit_menu->Append(CALCHART__SETMODE, wxT("Set Show &Mode..."), wxT("Set the show mode"));
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__POINTS, wxT("&Point Selections..."), wxT("Select Points"));
    edit_menu->Append(CALCHART__SELECT_ALL, wxT("Select &All...\tCTRL-A"), wxT("Select All Points"));
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__SET_SHEET_TITLE, wxT("Set Sheet &Title...\tCTRL-T"), wxT("Change the title of this stuntsheet"));
    edit_menu->Append(CALCHART__SET_BEATS, wxT("Set &Beats...\tCTRL-B"), wxT("Change the number of beats for this stuntsheet"));
    edit_menu->Append(CALCHART__ResetReferencePoint, wxT("Reset reference point..."), wxT("Reset the current reference point"));
    edit_menu->AppendSeparator();
    edit_menu->Append(CALCHART__E7TransitionSolver, wxT("Solve transition"), wxT("Solve the transition to the next sheet automatically"));
    edit_menu->Append(CALCHART__PRINT_EDIT_CONTINUITY, wxT("Edit Print Continuity..."), wxT("Edit Print continuity for this stuntsheet"));

    wxMenu* backgroundimage_menu = new wxMenu;
    backgroundimage_menu->AppendCheckItem(CALCHART__ShowBackgroundImages, wxT("Show Background Images"), wxT("Toggle showing background images"));
    backgroundimage_menu->Append(CALCHART__AddBackgroundImage, wxT("Add Background Image..."), wxT("Add a background image"));
    backgroundimage_menu->AppendCheckItem(CALCHART__AdjustBackgroundImageMode, wxT("Image Adjust Mode..."), wxT("Mode to adjust background images"));
    backgroundimage_menu->Enable(CALCHART__ShowBackgroundImages, true);
    backgroundimage_menu->Enable(CALCHART__AddBackgroundImage, true);
    backgroundimage_menu->Enable(CALCHART__AdjustBackgroundImageMode, true);

    wxMenu* view_menu = new wxMenu;
    view_menu->Append(CALCHART__ViewSwapFieldAndAnimate, wxT("View Animation\tCTRL-RETURN"), wxT("View Animation or Field"));
    view_menu->AppendSeparator();
    for (int i = CALCHART__ViewFieldThumbnail; i <= CALCHART__ViewToolBar; ++i) {
        view_menu->Append(i, std::string("Show ") + kAUIEnumToString[i], std::string("Controls Displaying ") + kAUIEnumToString[i]);
    }
    view_menu->AppendSeparator();
    view_menu->AppendCheckItem(CALCHART__draw_paths, wxT("Draw Paths"), wxT("Draw Paths"));
    view_menu->Check(CALCHART__draw_paths, false);
    view_menu->AppendSeparator();
    view_menu->Append(CALCHART__GhostOff, wxT("Disable Ghost View"), wxT("Turn off ghost view"));
    view_menu->Append(CALCHART__GhostNextSheet, wxT("Ghost Next Sheet"), wxT("Draw a ghost of the next stuntsheet"));
    view_menu->Append(CALCHART__GhostPreviousSheet, wxT("Ghost Previous Sheet"), wxT("Draw a ghost of the previous stuntsheet"));
    view_menu->Append(CALCHART__GhostNthSheet, wxT("Ghost Particular Sheet..."), wxT("Draw a ghost of a particular stuntsheet"));
    view_menu->AppendSeparator();
    view_menu->Append(CALCHART__ViewZoomFit, wxT("Zoom to Fit\tCTRL-0"), wxT("Zoom to fit"));
    view_menu->Append(CALCHART__ViewZoomIn, wxT("Zoom In\tCTRL-+"), wxT("Zoom In"));
    view_menu->Append(CALCHART__ViewZoomOut, wxT("Zoom In\tCTRL--"), wxT("Zoom Out"));

    wxMenu* help_menu = new wxMenu;
    help_menu->Append(wxID_ABOUT, wxT("&About CalChart..."), wxT("Information about the program"));
    help_menu->Append(wxID_HELP, wxT("&Help on CalChart...\tCTRL-H"), wxT("Help on using CalChart"));

    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, wxT("&File"));
    menu_bar->Append(edit_menu, wxT("&Edit"));
    menu_bar->Append(view_menu, wxT("&View"));
    menu_bar->Append(backgroundimage_menu, wxT("&Field Image"));
    menu_bar->Append(help_menu, wxT("&Help"));
    SetMenuBar(menu_bar);

    refreshGhostOptionStates();

    // Add a toolbar
    mToolBar = CreateMainAuiToolBar(this, wxID_ANY, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW);
    mToolBar->SetFont(ResizeFont(mToolBar->GetFont(), GetToolBarFontSize()));

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
    // sanity: Don't let the user zoom too low
    {
        auto zoom_size = mConfig.Get_FieldFrameZoom();
        if (zoom_size < 0.01) {
            mConfig.Set_FieldFrameZoom(0.01);
        }
    }

    mCanvas = new FieldCanvas(mConfig.Get_FieldFrameZoom(), static_cast<CalChartView*>(view), this);
    // set scroll rate 1 to 1, so we can have even scrolling of whole field
    mCanvas->SetScrollRate(1, 1);
    mCanvas->Scroll(mConfig.Get_FieldCanvasScrollX(), mConfig.Get_FieldCanvasScrollY());

    // Create all the other things attached to the frame:
    mControls = FieldControls::CreateToolBar(this, wxID_ANY, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW | wxAUI_TB_TEXT);
    FieldControls::SetZoomAmount(this, mConfig.Get_FieldFrameZoom());

    mContinuityBrowser = new ContinuityBrowser(this, wxID_ANY, wxDefaultPosition, GetContinuityBrowserConstructSize());
    mFieldThumbnailBrowser = new FieldThumbnailBrowser(this, wxID_ANY, wxDefaultPosition, GetFieldThumbnailBrowserConstructSize());
    mAnimationErrorsPanel = new AnimationErrorsPanel(this);
    mAnimationPanel = new AnimationPanel(this);
    mPrintContinuityEditor = new PrintContinuityEditor(this);

    // for doing mini and main panels
    mShadowAnimationPanel = new AnimationPanel(this);

    // update our lookups:
    mLookupEnumToSubWindow[CALCHART__ViewFieldThumbnail] = mFieldThumbnailBrowser;
    mLookupEnumToSubWindow[CALCHART__ViewFieldControls] = mControls;
    mLookupEnumToSubWindow[CALCHART__ViewContinuityInfo] = mContinuityBrowser;
    mLookupEnumToSubWindow[CALCHART__ViewAnimationErrors] = mAnimationErrorsPanel;
    mLookupEnumToSubWindow[CALCHART__ViewAnimation] = mAnimationPanel;
    mLookupEnumToSubWindow[CALCHART__ViewPrintContinuity] = mPrintContinuityEditor;
    mLookupEnumToSubWindow[CALCHART__ViewToolBar] = mToolBar;
    for (auto&& i : mLookupEnumToSubWindow) {
        mLookupSubWindowToEnum[i.second] = i.first;
    }

    mAnimationPanel->SetInMiniMode(true);
    mShadowAnimationPanel->SetInMiniMode(false);

    // now patch up the controls with the views:
    mCanvas->SetView(GetFieldView());
    mContinuityBrowser->SetView(GetFieldView());
    mFieldThumbnailBrowser->SetView(GetFieldView());
    mAnimationErrorsPanel->SetView(GetFieldView());
    mAnimationPanel->SetView(GetFieldView());
    mPrintContinuityEditor->SetView(GetFieldView());

    mShadowAnimationPanel->SetView(GetFieldView());

    // Now determine what to show and not show.
    mAUIManager->AddPane(mCanvas, wxAuiPaneInfo().Name(wxT("Field")).CenterPane().Show());
    mAUIManager->AddPane(mShadowAnimationPanel, wxAuiPaneInfo().Name(wxT("ShadowAnimation")).CenterPane().Hide());

    mAUIManager->AddPane(mFieldThumbnailBrowser, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewFieldThumbnail]).Caption(kAUIEnumToString[CALCHART__ViewFieldThumbnail]).Left().BestSize(GetFieldThumbnailBrowserSize()));
    mAUIManager->AddPane(mAnimationPanel, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewAnimation]).Caption(kAUIEnumToString[CALCHART__ViewAnimation]).Left().BestSize(GetAnimationSize()));
    mAUIManager->AddPane(mContinuityBrowser, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewContinuityInfo]).Caption(kAUIEnumToString[CALCHART__ViewContinuityInfo]).Right().BestSize(GetContinuityBrowserSize()));
    mAUIManager->AddPane(mAnimationErrorsPanel, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewAnimationErrors]).Caption(kAUIEnumToString[CALCHART__ViewAnimationErrors]).Right().BestSize(GetAnimationErrorsSize()));
    mAUIManager->AddPane(mPrintContinuityEditor, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewPrintContinuity]).Caption(kAUIEnumToString[CALCHART__ViewPrintContinuity]).Right().BestSize(GetPrintContinuitySize()));
    mAUIManager->AddPane(mControls, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewFieldControls]).Caption(kAUIEnumToString[CALCHART__ViewFieldControls]).ToolbarPane().Top());
    mAUIManager->AddPane(mToolBar, wxAuiPaneInfo().Name(kAUIEnumToString[CALCHART__ViewToolBar]).Caption(kAUIEnumToString[CALCHART__ViewToolBar]).ToolbarPane().Top());

    mAUIManager->Update();

    // restore the manager with the Current visability
    if (auto lastLayout = mConfig.Get_CalChartFrameAUILayout(); lastLayout != wxT("")) {
        mAUIManager->LoadPerspective(lastLayout, true);
    }

    // adjust the menu items to reflect.
    for (int i = CALCHART__ViewFieldThumbnail; i <= CALCHART__ViewToolBar; ++i) {
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
    SetCurrentLasso(mCanvas->GetCurrentLasso());
    SetCurrentMove(mCanvas->GetCurrentMove());

    // Show the frame
    OnUpdate();
    mCanvas->wxWindow::SetFocus();

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
    mConfig.Set_CalChartFrameAUILayout(mAUIManager->SavePerspective());
}

void CalChartFrame::OnCmdAppend(wxCommandEvent& event) { AppendShow(); }

void CalChartFrame::OnCmdImportCont(wxCommandEvent& event) { ImportContFile(); }

// the default wxView print doesn't handle landscape.  rolling our own
void CalChartFrame::OnCmdPrint(wxCommandEvent& event)
{
    // grab our current page setup.
    wxPrinter printer(gPrintDialogData);
    MyPrintout printout(wxT("My Printout"), *GetShow(), mConfig);
    wxPrintDialogData& printDialog = printer.GetPrintDialogData();

    int minPage, maxPage, pageFrom, pageTo;
    printout.GetPageInfo(&minPage, &maxPage, &pageFrom, &pageTo);
    printDialog.SetMinPage(minPage);
    printDialog.SetMaxPage(maxPage);

    if (!printer.Print(this, &printout, true)) {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
            wxMessageBox(wxT("A problem was encountered when trying to print"),
                wxT("Printing"), wxOK);
        }
    } else {
        *gPrintDialogData = printer.GetPrintDialogData();
    }
}

// the default wxView print doesn't handle landscape.  rolling our own
void CalChartFrame::OnCmdPrintPreview(wxCommandEvent& event)
{
    // grab our current page setup.
    wxPrintPreview* preview = new wxPrintPreview(
        new MyPrintout(wxT("My Printout"), *GetShow(), mConfig),
        new MyPrintout(wxT("My Printout"), *GetShow(), mConfig), gPrintDialogData);
    if (!preview->Ok()) {
        delete preview;
        wxMessageBox(wxT("There was a problem previewing.\nPerhaps your current "
                         "printer is not set correctly?"),
            wxT("Previewing"), wxOK);
        return;
    }
    wxPreviewFrame* frame = new wxPreviewFrame(preview, this, wxT("Show Print Preview"));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show(true);
}

void CalChartFrame::OnCmdPageSetup(wxCommandEvent& event)
{
    wxPageSetupData mPageSetupData;
    mPageSetupData.EnableOrientation(true);

    wxPageSetupDialog pageSetupDialog(this, &mPageSetupData);
    if (pageSetupDialog.ShowModal() == wxID_OK)
        mPageSetupData = pageSetupDialog.GetPageSetupData();
    // pass the print data to our global print dialog
    gPrintDialogData->SetPrintData(mPageSetupData.GetPrintData());
}

void CalChartFrame::OnCmdLegacyPrint(wxCommandEvent& event)
{
    if (GetShow()) {
        PrintPostScriptDialog dialog(static_cast<CalChartDoc*>(GetDocument()), this);
        if (dialog.ShowModal() == wxID_OK) {
            dialog.PrintShow(mConfig);
        }
    }
}

void CalChartFrame::OnCmdExportViewerFile(wxCommandEvent& event)
{
    if (GetShow()) {
        wxFileDialog saveFileDialog(this, _("Save viewer file"), "", "", "viewer files (*.viewer)|*.viewer", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (saveFileDialog.ShowModal() == wxID_CANCEL)
            return;

        if (!GetShow()->exportViewerFile(saveFileDialog.GetPath())) {
            wxMessageBox(wxT("There was a problem exporting the viewer file.\n") + saveFileDialog.GetPath(), wxT("Exporting Viewer File"));
            return;
        }
    }
}

void CalChartFrame::OnCmdPreferences(wxCommandEvent& event)
{
    CalChartPreferences dialog1(this);
    if (dialog1.ShowModal() == wxID_OK) {
        GetFieldView()->OnUpdate(nullptr);
    }
}

void CalChartFrame::OnCmdInsertBefore(wxCommandEvent& event)
{
    CalChart::Show::Sheet_container_t sht(1, *GetShow()->GetCurrentSheet());
    GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum());
    GetFieldView()->GoToPrevSheet();
}

void CalChartFrame::OnCmdInsertAfter(wxCommandEvent& event)
{
    CalChart::Show::Sheet_container_t sht(1, *GetShow()->GetCurrentSheet());
    GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum() + 1);
    GetFieldView()->GoToNextSheet();
}

void CalChartFrame::OnCmdInsertFromOtherShow(wxCommandEvent& event)
{
    wxString s = wxFileSelector(wxT("Add Sheets from Other Shows"), wxEmptyString,
        wxEmptyString, wxEmptyString, file_wild);
    if (s.IsEmpty())
        return;
    CalChartDoc show;
    if (!(&show)->OnOpenDocument(s)) {
        (void)wxMessageBox(wxT("Error Opening show"), wxT("Load Error"));
        return;
    }
    if ((&show)->GetNumPoints() != GetShow()->GetNumPoints()) {
        (void)wxMessageBox(wxT("The blocksize doesn't match"), wxT("Import Error"));
        return;
    }
    wxString prompt = wxT("Enter the %s sheet number (highest possible: %i)");
    wxString begin = wxGetTextFromUser(
        wxString::Format(prompt, "beginning", (&show)->GetNumSheets()),
        "First Sheet Number", "1", this);
    long beginValue;
    if (!begin || !begin.ToLong(&beginValue) || beginValue < 1 || beginValue > (&show)->GetNumSheets()) {
        (void)wxMessageBox(wxT("Not a valid sheet number"), wxT("Insert Failed"));
        return;
    }
    long endValue;
    if (beginValue != (&show)->GetNumSheets()) {
        wxString end = wxGetTextFromUser(
            wxString::Format(prompt, "ending", (&show)->GetNumSheets()),
            "Last Sheet Number", begin, this);
        if (!end || !end.ToLong(&endValue) || endValue < beginValue || endValue > (&show)->GetNumSheets()) {
            (void)wxMessageBox(wxT("Not a valid sheet number"), wxT("Insert Failed"));
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

void CalChartFrame::OnCmdCopySheet(wxCommandEvent& event)
{
    if (wxTheClipboard->Open()) {
        std::unique_ptr<wxCustomDataObject> clipboardObject(
            new wxCustomDataObject(kSheetDataClipboardFormat));
        std::vector<uint8_t> serializedSheet = GetShow()->GetCurrentSheet()->SerializeSheet();

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

void CalChartFrame::OnCmdPasteSheet(wxCommandEvent& event)
{
    if (wxTheClipboard->Open()) {
        if (wxTheClipboard->IsSupported(kSheetDataClipboardFormat)) {
            wxCustomDataObject clipboardObject(kSheetDataClipboardFormat);
            wxTheClipboard->GetData(clipboardObject);

            auto numPoints = GetShow()->GetNumPoints();
            memcpy(&numPoints, clipboardObject.GetData(), sizeof(numPoints));
            if (numPoints != GetShow()->GetNumPoints()) {
                wxMessageBox(wxString::Format(
                    wxT("Cannot paste - number of points in pasted sheet (%i) does not "
                        "match number of points in current show (%i)"),
                    numPoints, GetShow()->GetNumPoints()));
                return;
            }
            std::stringstream sheetStream;
            sheetStream.write((char*)(clipboardObject.GetData()) + sizeof(numPoints),
                clipboardObject.GetDataSize() - sizeof(numPoints));

            CalChart::ReadLong(sheetStream);
            CalChart::ReadLong(sheetStream);

            sheetStream.unsetf(std::ios_base::skipws);
            std::istream_iterator<uint8_t> theBegin(sheetStream);
            std::istream_iterator<uint8_t> theEnd{};
            std::vector<uint8_t> data(theBegin, theEnd);

            CalChart::Show::Sheet_container_t sht(
                1, CalChart::Sheet(numPoints, data.data(), data.size()));
            GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum());
        }
        wxTheClipboard->Close();
    }
}

void CalChartFrame::OnCmdDelete(wxCommandEvent& event)
{
    if (GetFieldView()->GetNumSheets() > 1) {
        GetFieldView()->DoDeleteSheet(GetFieldView()->GetCurrentSheetNum());
    }
}

// grey out if we're on a sheet
void CalChartFrame::OnCmdRelabel(wxCommandEvent& event)
{
    if (GetFieldView()->GetCurrentSheetNum() + 1 < GetFieldView()->GetNumSheets()) {
        if (!GetFieldView()->DoRelabel()) {
            (void)wxMessageBox(wxT("Stuntsheets don't match"), wxT("Relabel sheets"));
        }
    } else {
        (void)wxMessageBox(wxT("This can't used on the last stuntsheet"),
            wxT("Relabel sheets"));
    }
}

void CalChartFrame::OnCmdEditPrintCont(wxCommandEvent& event)
{
    mAUIManager->GetPane("Print Continuity").Show(true);
    mAUIManager->Update();
}

void CalChartFrame::OnCmdSetSheetTitle(wxCommandEvent& event)
{
    wxString s;
    if (GetShow()) {
        s = wxGetTextFromUser(wxT("Enter the sheet title"),
            GetShow()->GetCurrentSheet()->GetName(),
            GetShow()->GetCurrentSheet()->GetName(), this);
        if (!s.IsEmpty()) {
            GetFieldView()->DoSetSheetTitle(s);
        }
    }
}

void CalChartFrame::OnCmdSetBeats(wxCommandEvent& event)
{
    wxString s;
    if (GetShow()) {
        wxString buf;
        buf.sprintf(wxT("%u"), GetShow()->GetCurrentSheet()->GetBeats());
        s = wxGetTextFromUser(wxT("Enter the number of beats"),
            GetShow()->GetCurrentSheet()->GetName(), buf, this);
        if (!s.empty()) {
            long val;
            if (s.ToLong(&val)) {
                GetFieldView()->DoSetSheetBeats(static_cast<int>(val));
            }
        }
    }
}

void CalChartFrame::OnCmdSetup(wxCommandEvent& event) { Setup(); }

void CalChartFrame::OnCmdSetMode(wxCommandEvent& event) { SetMode(); }

void CalChartFrame::OnCmdPoints(wxCommandEvent& event)
{
    if (GetShow()) {
        PointPicker* pp = new PointPicker(*GetShow(), this);
        // make it modeless:
        pp->Show();
    }
}

void CalChartFrame::OnCmdSelectAll(wxCommandEvent& event)
{
    if (GetShow()) {
        GetShow()->SetSelection(GetShow()->MakeSelectAll());
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

void CalChartFrame::OnCmdAbout(wxCommandEvent& event) { TopFrame::About(); }

void CalChartFrame::OnCmdHelp(wxCommandEvent& event) { TopFrame::Help(); }

void CalChartFrame::OnCmd_prev_ss(wxCommandEvent& event)
{
    GetFieldView()->GoToPrevSheet();
}

void CalChartFrame::OnCmd_next_ss(wxCommandEvent& event)
{
    GetFieldView()->GoToNextSheet();
}

void CalChartFrame::OnCmd_box(wxCommandEvent& event)
{
    SetCurrentLasso(CC_DRAG::BOX);
}

void CalChartFrame::OnCmd_poly(wxCommandEvent& event)
{
    SetCurrentLasso(CC_DRAG::POLY);
}

void CalChartFrame::OnCmd_lasso(wxCommandEvent& event)
{
    SetCurrentLasso(CC_DRAG::LASSO);
}

void CalChartFrame::OnCmd_swap(wxCommandEvent& event)
{
    SetCurrentLasso(CC_DRAG::SWAP);
}

void CalChartFrame::OnCmd_move(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_NORMAL);
}

void CalChartFrame::OnCmd_shape_line(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SHAPE_LINE);
}

void CalChartFrame::OnCmd_shape_x(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SHAPE_X);
}

void CalChartFrame::OnCmd_shape_cross(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SHAPE_CROSS);
}

void CalChartFrame::OnCmd_shape_box(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SHAPE_RECTANGLE);
}

void CalChartFrame::OnCmd_shape_ellipse(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SHAPE_ELLIPSE);
}

void CalChartFrame::OnCmd_shape_draw(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SHAPE_DRAW);
}

void CalChartFrame::OnCmd_line(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_LINE);
}

void CalChartFrame::OnCmd_rot(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_ROTATE);
}

void CalChartFrame::OnCmd_shear(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SHEAR);
}

void CalChartFrame::OnCmd_reflect(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_REFL);
}

void CalChartFrame::OnCmd_size(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SIZE);
}

void CalChartFrame::OnCmd_genius(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_GENIUS);
}

void CalChartFrame::OnCmd_label_left(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabel(false);
}

void CalChartFrame::OnCmd_label_right(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabel(true);
}

void CalChartFrame::OnCmd_label_flip(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabelFlip();
}

void CalChartFrame::OnCmd_label_hide(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabelVisibility(false);
}

void CalChartFrame::OnCmd_label_show(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabelVisibility(true);
}

void CalChartFrame::OnCmd_label_visibility_toggle(wxCommandEvent& event)
{
    GetFieldView()->DoTogglePointsLabelVisibility();
}

void CalChartFrame::OnCmd_setsym0(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_PLAIN);
}

void CalChartFrame::OnCmd_setsym1(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SOL);
}

void CalChartFrame::OnCmd_setsym2(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_BKSL);
}

void CalChartFrame::OnCmd_setsym3(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SL);
}

void CalChartFrame::OnCmd_setsym4(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_X);
}

void CalChartFrame::OnCmd_setsym5(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SOLBKSL);
}

void CalChartFrame::OnCmd_setsym6(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SOLSL);
}

void CalChartFrame::OnCmd_setsym7(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SOLX);
}

void CalChartFrame::OnChar(wxKeyEvent& event) { mCanvas->OnChar(event); }

void CalChartFrame::OnCmd_AddBackgroundImage(wxCommandEvent& event)
{
    wxString filename;
    filename = wxLoadFileSelector(wxT("Select a background image"),
        wxT("BMP files (*.bmp)|*.bmp")
#if wxUSE_LIBPNG
            wxT("|PNG files (*.png)|*.png")
#endif
#if wxUSE_LIBJPEG
                wxT("|JPEG files (*.jpg)|*.jpg")
#endif
#if wxUSE_GIF
                    wxT("|GIF files (*.gif)|*.gif")
#endif
#if wxUSE_LIBTIFF
                        wxT("|TIFF files (*.tif)|*.tif")
#endif
#if wxUSE_PCX
                            wxT("|PCX files (*.pcx)|*.pcx")
#endif
    );
    if (!filename.empty()) {
        wxImage image;
        if (!image.LoadFile(filename)) {
            wxLogError(wxT("Couldn't load image from '%s'."), filename.c_str());
            return;
        }
        if (!GetFieldView()->AddBackgroundImage(image)) {
            wxLogError(wxT("Couldn't load image from '%s'."), filename.c_str());
            return;
        }
        GetFieldView()->DoPictureAdjustment(true);
        GetFieldView()->DoDrawBackground(true);
        GetMenuBar()->FindItem(CALCHART__ShowBackgroundImages)->Check(true);
        GetMenuBar()->FindItem(CALCHART__AdjustBackgroundImageMode)->Check(true);
        mCanvas->Refresh();
    }
}

void CalChartFrame::OnCmd_AdjustBackgroundImageMode(wxCommandEvent& event)
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

void CalChartFrame::OnCmd_ShowBackgroundImages(wxCommandEvent& event)
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
    auto which_option = (event.GetId() == CALCHART__GhostControls) ? selection : (event.GetId() == CALCHART__GhostOff) ? 0 : (event.GetId() == CALCHART__GhostNextSheet) ? 1 : (event.GetId() == CALCHART__GhostPreviousSheet) ? 2 : 3;
    switch (which_option) {
    case 0:
        GetFieldView()->getGhostModule().setGhostSource(GhostModule::disabled);
        FieldControls::SetGhostChoice(mControls, 0);
        break;
    case 1:
        GetFieldView()->getGhostModule().setGhostSource(GhostModule::next);
        FieldControls::SetGhostChoice(mControls, 1);
        break;
    case 2:
        GetFieldView()->getGhostModule().setGhostSource(GhostModule::previous);
        FieldControls::SetGhostChoice(mControls, 2);
        break;
    case 3: {
        wxString targetSheet = wxGetTextFromUser("Enter the sheet number to ghost:",
            "Ghost Sheet", "1", this);
        long targetSheetNum = 0;
        if (targetSheet.ToLong(&targetSheetNum)) {
            GetFieldView()->getGhostModule().setGhostSource(GhostModule::specific,
                static_cast<int>(targetSheetNum) - 1);
            FieldControls::SetGhostChoice(mControls, 3);
        } else {
            wxMessageBox(wxT("The input must be a number."),
                wxT("Operation failed."));
        }
    } break;
    }
    refreshGhostOptionStates();
    GetCanvas()->Refresh();
}

void CalChartFrame::refreshGhostOptionStates()
{
    bool active = GetFieldView()->getGhostModule().isActive();
    GetMenuBar()->FindItem(CALCHART__GhostOff)->Enable(active);
}

void CalChartFrame::OnCmd_AdjustViews(wxCommandEvent& event)
{
    ChangeVisibility(!mAUIManager->GetPane(mLookupEnumToSubWindow[event.GetId()]).IsShown(), event.GetId());
}

void CalChartFrame::OnCmd_SwapAnimation(wxCommandEvent& event)
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
    if (itemid < CALCHART__ViewFieldThumbnail || itemid > CALCHART__ViewToolBar) {
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
        GetMenuBar()->FindItem(CALCHART__ViewSwapFieldAndAnimate)->SetItemLabel(wxT("View Animation\tCTRL-RETURN"));
    } else {
        GetMenuBar()->FindItem(CALCHART__ViewSwapFieldAndAnimate)->SetItemLabel(wxT("View Field\tCTRL-RETURN"));
    }
    ShowFieldAndHideAnimation(mMainFieldVisible);
}

void CalChartFrame::OnCmd_ResetReferencePoint(wxCommandEvent& event)
{
    GetFieldView()->DoResetReferencePoint();
}

void CalChartFrame::OnCmd_ChangedColorPalette(wxCommandEvent& event)
{
    GetFieldView()->OnUpdate(nullptr);
}

void CalChartFrame::OnCmd_SolveTransition(wxCommandEvent& event)
{
    if (GetShow()) {
        TransitionSolverFrame* transitionSolver = new TransitionSolverFrame(static_cast<CalChartDoc*>(GetDocument()), this, wxID_ANY, wxT("Transition Solver"));
        transitionSolver->Show();
    }
}

// Append a show with file selector
void CalChartFrame::AppendShow()
{
    auto s = wxFileSelector(wxT("Append show"), wxEmptyString, wxEmptyString, wxEmptyString, file_wild);
    if (s.IsEmpty()) {
        return;
    }
    auto shw = std::make_unique<CalChartDoc>();
    if (!shw->OnOpenDocument(s)) {
        (void)wxMessageBox(wxT("Error Opening show"), wxT("Load Error"));
        return;
    }
    auto result = GetFieldView()->DoAppendShow(std::move(shw));
    if (!result.first) {
        (void)wxMessageBox(result.second, wxT("Append Error"));
        return;
    }
}

// Append a show with file selector
void CalChartFrame::ImportContFile()
{
    wxString s;

    s = wxFileSelector(wxT("Import Continuity"), wxEmptyString, wxEmptyString,
        wxEmptyString, wxT("*.txt"));
    if (!s.empty()) {
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

void CalChartFrame::SetCurrentLasso(CC_DRAG type)
{
    // retoggle the tool because we want it to draw as selected
    int toggleID = (type == CC_DRAG::POLY) ? CALCHART__poly : (type == CC_DRAG::LASSO) ? CALCHART__lasso : (type == CC_DRAG::SWAP) ? CALCHART__swap : CALCHART__box;
    mToolBar->ToggleTool(toggleID, true);

    mCanvas->SetCurrentLasso(type);
}

void CalChartFrame::SetCurrentMove(CC_MOVE_MODES type)
{
    ToolBarSetCurrentMove(type);
    mCanvas->SetCurrentMove(type);
}

// call by the canvas to inform that the move has been set.  Don't call back to canvas
void CalChartFrame::ToolBarSetCurrentMove(CC_MOVE_MODES type)
{
    // retoggle the tool because we want it to draw as selected
    mToolBar->ToggleTool(CALCHART__move + type, true);
}

void CalChartFrame::Setup()
{
    if (GetShow()) {
        ShowInfoReq dialog(*GetShow(), this);
        if (dialog.ShowModal() == wxID_OK) {
            GetFieldView()->DoSetShowInfo(dialog.GetLabels(), dialog.GetNumberColumns());
        }
    }
}

void CalChartFrame::SetMode()
{
    if (GetShow()) {
        ModeSetupDialog dialog(GetShow()->GetShowMode(), this);
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

void CalChartFrame::zoom_callback(wxCommandEvent& event)
{
    auto zoom_amount = FieldControls::GetZoomAmount(this);
    if (zoom_amount == 0) {
        zoom_amount = mCanvas->ZoomToFitFactor();
    }
    do_zoom(zoom_amount);
}

void CalChartFrame::OnCmd_ZoomFit(wxCommandEvent& event)
{
    do_zoom(mCanvas->ZoomToFitFactor());
}

void CalChartFrame::OnCmd_ZoomIn(wxCommandEvent& event)
{
    // because zoom is truncated to 2 decimal places, make sure we at least increment by 1.
    auto original_zoom = FieldControls::GetZoomAmount(this);
    auto new_zoom = original_zoom * 1.20;
    if ((new_zoom - original_zoom) < 0.01) {
        new_zoom += 0.01;
    }
    if (new_zoom < 5) {
        do_zoom(new_zoom);
    }
}

void CalChartFrame::OnCmd_ZoomOut(wxCommandEvent& event)
{
    auto new_zoom = FieldControls::GetZoomAmount(this) * 0.8;
    if (new_zoom > 0.01) {
        do_zoom(new_zoom);
    }
}

void CalChartFrame::zoom_callback_textenter(wxCommandEvent& event)
{
    auto zoomtxt = static_cast<wxComboBox*>(FindWindow(event.GetId()))->GetValue();
    // strip the trailing '%' if it exists
    if (zoomtxt.Length() && (zoomtxt.Last() == wxT('%'))) {
        zoomtxt.RemoveLast();
    }
    double zoom_amount = 1.0;
    if (zoomtxt.ToDouble(&zoom_amount)) {
        zoom_amount /= 100.0;
    } else {
        wxString msg("Please enter a valid number\n");
        wxMessageBox(msg, wxT("Invalid number"), wxICON_INFORMATION | wxOK);
        // return if not valid
        return;
    }
    do_zoom(zoom_amount);
}

void CalChartFrame::do_zoom(float zoom_amount)
{
    zoom_amount = ToolBarSetZoom(zoom_amount);
    mCanvas->SetZoom(zoom_amount);
}

float CalChartFrame::ToolBarSetZoom(float zoom_amount)
{
    zoom_amount = std::max(zoom_amount, 0.01f);
    FieldControls::SetZoomAmount(this, zoom_amount);
    mConfig.Set_FieldFrameZoom(zoom_amount);
    return zoom_amount;
}

void CalChartFrame::OnUpdate()
{
    wxString tempbuf;
    auto sht = GetShow()->GetCurrentSheet();
    unsigned num = GetShow()->GetNumSheets();
    unsigned curr = GetFieldView()->GetCurrentSheetNum() + 1;

    tempbuf.sprintf(wxT("%s%d of %d \"%.32s\" %d beats"),
        GetShow()->IsModified() ? wxT("* ") : wxT(""), curr, num,
        (sht != GetShow()->GetSheetEnd()) ? wxString(sht->GetName())
                                          : wxT(""),
        (sht != GetShow()->GetSheetEnd()) ? sht->GetBeats() : 0);
    SetStatusText(tempbuf, 1);
    tempbuf.Clear();
    tempbuf << GetShow()->GetSelectionList().size() << wxT(" of ")
            << GetShow()->GetNumPoints() << wxT(" selected");
    SetStatusText(tempbuf, 2);

    SetTitle(GetDocument()->GetUserReadableName());
    mCanvas->Refresh();

    mContinuityBrowser->OnUpdate();
    mFieldThumbnailBrowser->OnUpdate();
    mAnimationErrorsPanel->OnUpdate();
    mAnimationPanel->OnUpdate();
    mPrintContinuityEditor->OnUpdate();

    mShadowAnimationPanel->OnUpdate();
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
