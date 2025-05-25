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

static const auto kAUINames = std::array{
    "Field Thumbnails",
    "Controls ToolBar",
    "Continuities",
    "Marching Errors",
    "Animation",
    "Print Continuity",
    "Select and Move ToolBar",
    "Marcher ToolBar",
};

BEGIN_EVENT_TABLE(CalChartFrame, wxDocChildFrame)
EVT_CHAR(CalChartFrame::OnChar)
EVT_MENU(wxID_PAGE_SETUP, CalChartFrame::OnCmdPageSetup)
EVT_MENU(wxID_DELETE, CalChartFrame::OnCmdDelete)
EVT_MENU(wxID_ABOUT, CalChartFrame::OnCmdAbout)
EVT_MENU(wxID_HELP, CalChartFrame::OnCmdHelp)
EVT_BUTTON(CALCHART__ViewZoomIn, CalChartFrame::OnCmd_ZoomIn)
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
EVT_MENU(wxID_PREFERENCES, CalChartFrame::OnCmdPreferences)
EVT_SLIDER(CALCHART__slider_zoom, CalChartFrame::zoom_callback)
EVT_CHOICE(CALCHART__refnum_callback, CalChartFrame::OnCmd_ReferenceNumber)
EVT_CHOICE(CALCHART__GhostControls, CalChartFrame::OnCmd_GhostOption)
EVT_CHOICE(CALCHART__InstrumentChoice, CalChartFrame::OnCmd_InstrumentSelection)
EVT_CHOICE(CALCHART__SymbolChoice, CalChartFrame::OnCmd_SymbolSelection)
EVT_CHOICE(CALCHART__MarcherChoice, CalChartFrame::OnCmd_MarcherSelection)
EVT_CHECKBOX(CALCHART__draw_paths, CalChartFrame::OnEnableDrawPaths)
EVT_BUTTON(CALCHART__ChangedColorPalette, CalChartFrame::OnCmd_ChangedColorPalette)
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
    , mAdjustPaneIndex(kAUINames.size())
{
    // Give it an icon
    SetBandIcon(this);

    // Give it a status line
    CreateStatusBar(3);
    SetStatusText("Welcome to Calchart " CC_VERSION);

    wxUI::MenuProxy editMenu;
    wxUI::MenuBar{
        wxUI::Menu{
            "&File",
            wxUI::Item{ wxID_NEW, "&New Show\tCTRL-N", "Create a new show" },
            wxUI::Item{ wxID_OPEN, "&Open...\tCTRL-O", "Load a saved show" },
            wxUI::Item{ "&Import Continuity...\tCTRL-SHIFT-I", "Import continuity text", [this] {
                           ImportContFile();
                       } },
            wxUI::Item{ wxID_SAVE, "&Save\tCTRL-S", "Save show" },
            wxUI::Item{ wxID_SAVEAS, "Save &As...\tCTRL-SHIFT-S", "Save show as a new name" },
            wxUI::Item{ "&Print...\tCTRL-P", "Print this show", [this] {
                           OnPrint();
                       } },
            wxUI::Item{ "Preview...\tCTRL-SHIFT-P", "Preview this show", [this] {
                           OnPrintPreview();
                       } },
            wxUI::Item{ wxID_PAGE_SETUP, "Page Setup...\tCTRL-SHIFT-ALT-P", "Setup Pages" },
            wxUI::Item{ "Print to PS...", "Print show to PostScript", [this] {
                           OnLegacyPrint();
                       } },
            wxUI::Item{ "Export for Online Viewer...", "Export show to be viewed using the CalChart Online Viewer", [this] {
                           OnExportViewerFile();
                       } },
            wxUI::Item{ wxID_PREFERENCES, "&Preferences\tCTRL-," },
            wxUI::Item{ wxID_CLOSE, "Close Window\tCTRL-W", "Close this window" },
            wxUI::Item{ wxID_EXIT, "&Quit\tCTRL-Q", "Quit CalChart" },
        },
        wxUI::Menu{
            "&Edit",
            wxUI::Item{ wxID_UNDO, "&Undo\tCTRL-Z" },
            wxUI::Item{ wxID_REDO, "&Redo\tCTRL-SHIFT-Z" },
            wxUI::Separator{},
            wxUI::Item{ "&Insert Sheet Before\tCTRL-[", "Insert a new stuntsheet before this one", [this] {
                           OnInsertBefore();
                       } },
            wxUI::Item{ "Insert Sheet &After\tCTRL-]", "Insert a new stuntsheet after this one", [this] {
                           OnInsertAfter();
                       } },
            wxUI::Item{ wxID_DELETE, "&Delete Sheet\tCTRL-DEL", "Delete this stuntsheet" },
            wxUI::Item{ "&Copy Sheet\tCTRL-C", "Copy the current stuntsheet", [this] {
                           OnCopySheet();
                       } },
            wxUI::Item{ "&Paste Sheet\tCTRL-V", "Paste the current stuntsheet", [this] {
                           OnPasteSheet();
                       } },
            wxUI::Item{ "Insert Sheets From Other Show...", "Insert a saved stuntsheet after this one", [this] {
                           OnInsertFromOtherShow();
                       } },
            wxUI::Item{ "&Relabel Sheets\tCTRL-R", "Relabel all stuntsheets after this one", [this] {
                           OnRelabel();
                       } },
            wxUI::Item{ "Append Show...", "Append a show to the end", [this] {
                           AppendShow();
                       } },
            wxUI::Separator{},
            wxUI::Item{ "Set &Up Marchers...\tCTRL-U", "Setup number of marchers", [this] {
                           OnSetupMarchers();
                       } },
            wxUI::Item{ "Set &Instruments...\tCTRL-I", "Set instruments", [this] {
                           OnSetupInstruments();
                       } },
            wxUI::Item{ "Set Show &Mode...", "Set the show mode", [this] {
                           SetMode();
                       } },
            wxUI::Separator{},
            wxUI::Item{ "Point Picker...\tCTRL-SHIFT-A", "Point Picker", [this] {
                           OnPointPicker();
                       } },
            wxUI::Item{ "Select &All...\tCTRL-A", "Select All Points", [this] {
                           OnSelectAll();
                       } },
            wxUI::Separator{},
            wxUI::Item{ "Set Sheet &Title...\tCTRL-T", "Change the title of this stuntsheet", [this] {
                           OnSetSheetTitle();
                       } },
            wxUI::Item{ "Set &Beats...\tCTRL-B", "Change the number of beats for this stuntsheet", [this] {
                           OnSetBeats();
                       } },
            wxUI::Item{ "Reset reference point...", "Reset the current reference point", [this] {
                           OnResetReferencePoint();
                       } },
            wxUI::Separator{},
            wxUI::Item{ "Solve transition", "Solve the transition to the next sheet automatically", [this] {
                           OnSolveTransition();
                       } },
            wxUI::Item{ "Edit Print Continuity...", "Edit Print continuity for this stuntsheet", [this] {
                           OnEditPrintContinuity();
                       } },
        }
            .withProxy(editMenu),
        wxUI::Menu{
            "&View",
            wxUI::Item{ "View Animation\tCTRL-RETURN", "View Animation or Field", [this] {
                           OnSwapAnimation();
                       } }
                .withProxy(mViewSwapFieldAndAnimate),
            wxUI::Separator{},
            wxUI::MenuForEach{ CalChart::Ranges::enumerate_view(kAUINames), [this](auto&& whichAndName) {
                                  auto&& [which, name] = whichAndName;
                                  return wxUI::Item{ std::string("Show ") + name, std::string("Controls Displaying ") + name, [this, which] {
                                                        OnAdjustViews(which);
                                                    } }
                                      .withProxy(mAdjustPaneIndex.at(which));
                              } },
            wxUI::Separator{},
            wxUI::CheckItem{ "Draw Paths", "Draw Paths", [this](wxCommandEvent& event) {
                                GetFieldView()->OnEnableDrawPaths(event.IsChecked());
                            } }
                .withProxy(mDrawPaths),
            wxUI::Separator{},
            wxUI::Item{ "Disable Ghost View", "Turn off ghost view", [this] {
                           OnGhostOption(GhostSource::disabled);
                       } }
                .withProxy(mGhostOff),
            wxUI::Item{ "Ghost Next Sheet", "Draw a ghost of the next stuntsheet", [this] {
                           OnGhostOption(GhostSource::next);
                       } },
            wxUI::Item{ "Ghost Previous Sheet", "Draw a ghost of the previous stuntsheet", [this] {
                           OnGhostOption(GhostSource::previous);
                       } },
            wxUI::Item{ "Ghost Particular Sheet...", "Draw a ghost of a particular stuntsheet", [this] {
                           OnGhostOption(GhostSource::specific);
                       } },
            wxUI::Separator{},
            wxUI::Item{ "Zoom to Fit\tCTRL-0", "Zoom to fit", [this] {
                           OnZoomFit();
                       } },
            wxUI::Item{ "Zoom In\tCTRL-+", "Zoom In", [this] {
                           OnZoomIn();
                       } },
            wxUI::Item{ "Zoom In\tCTRL--", "Zoom Out", [this] {
                           OnZoomOut();
                       } },
        },
        wxUI::Menu{
            "&Field Image",
            wxUI::CheckItem{ "Show Background Images", "Toggle showing background images", [this] {
                                OnShowBackgroundImages();
                            } }
                .withProxy(mShowBackgroundImages),
            wxUI::Item{ "Add Background Image...", "Add a background image", [this] {
                           OnAddBackgroundImage();
                       } },
            wxUI::CheckItem{ "Image Adjust Mode...", "Mode to adjust background images", [this] {
                                OnAdjustBackgroundImageMode();
                            } }
                .withProxy(mAdjustBackgroundImageMode),
        },
        wxUI::Menu{
            "&Help",
            wxUI::Item{ wxID_ABOUT, "&About CalChart...", "Information about the program" },
            wxUI::Item{ wxID_HELP, "&Help on CalChart...\tCTRL-H", "Help on using CalChart" },
        }
    }.fitTo(this);
    // A nice touch: a history of files visited. Use this menu.
    // causes a crash :(
    // view->GetDocumentManager()->FileHistoryUseMenu(file_menu);

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
    mLookupSubWindows = {
        mFieldThumbnailBrowser,
        mControls,
        mContinuityBrowser,
        mAnimationErrorsPanel,
        mAnimationPanel,
        mPrintContinuityEditor,
        mSelectAndMoveToolBar,
        mMarcherToolBar,
    };

    for (auto&& [which, window] : CalChart::Ranges::enumerate_view(mLookupSubWindows)) {
        mLookupSubWindowToIndex[window] = which;
    }

    mAnimationPanel->SetInMiniMode(true);
    mShadowAnimationPanel->SetInMiniMode(false);

    // now patch up the controls with the views:
    SetViewsOnComponents(GetFieldView());

    // Now determine what to show and not show.
    mAUIManager->AddPane(mCanvas, wxAuiPaneInfo().Name("Field").CenterPane().Show());
    mAUIManager->AddPane(mShadowAnimationPanel, wxAuiPaneInfo().Name("ShadowAnimation").CenterPane().Hide());

    // And layout the default way things should look
    mAUIManager->AddPane(mFieldThumbnailBrowser, wxAuiPaneInfo().Name(kAUINames[0]).Caption(kAUINames[0]).Left().BestSize(GetFieldThumbnailBrowserSize()));
    mAUIManager->AddPane(mAnimationPanel, wxAuiPaneInfo().Name(kAUINames[4]).Caption(kAUINames[4]).Left().BestSize(GetAnimationSize()));
    mAUIManager->AddPane(mContinuityBrowser, wxAuiPaneInfo().Name(kAUINames[2]).Caption(kAUINames[2]).Right().BestSize(GetContinuityBrowserSize()));
    mAUIManager->AddPane(mAnimationErrorsPanel, wxAuiPaneInfo().Name(kAUINames[3]).Caption(kAUINames[3]).Right().BestSize(GetAnimationErrorsSize()));
    mAUIManager->AddPane(mPrintContinuityEditor, wxAuiPaneInfo().Name(kAUINames[5]).Caption(kAUINames[5]).Right().BestSize(GetPrintContinuitySize()));
    mAUIManager->AddPane(mControls, wxAuiPaneInfo().Name(kAUINames[1]).Caption(kAUINames[1]).ToolbarPane().Top());
    mAUIManager->AddPane(mSelectAndMoveToolBar, wxAuiPaneInfo().Name(kAUINames[6]).Caption(kAUINames[6]).ToolbarPane().Left());
    mAUIManager->AddPane(mMarcherToolBar, wxAuiPaneInfo().Name(kAUINames[7]).Caption(kAUINames[7]).ToolbarPane().Top());

    mAUIManager->Update();

    // restore the manager with the Current visability
    if (auto lastLayout = mConfig.Get_CalChartFrameAUILayout_3_6_1(); lastLayout != "") {
        mAUIManager->LoadPerspective(lastLayout, true);
    }

    // adjust the menu items to reflect.
    for (auto i : std::views::iota(0ul, kAUINames.size())) {
        ChangePaneVisibility(mAUIManager->GetPane(mLookupSubWindows.at(i)).IsShown(), i);
    }

    ChangeMainFieldVisibility(mMainFieldVisible);

    SetTitle(static_cast<CalChartDoc*>(doc)->GetTitle());

    // Update the command processor with the undo/redo menu items
    editMenu->FindItem(wxID_UNDO)->Enable(false);
    editMenu->FindItem(wxID_REDO)->Enable(false);
    doc->GetCommandProcessor()->SetEditMenu(editMenu.control());
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

// the default wxView print doesn't handle landscape.  rolling our own
void CalChartFrame::OnPrint()
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
void CalChartFrame::OnPrintPreview()
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

void CalChartFrame::OnLegacyPrint()
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

void CalChartFrame::OnExportViewerFile()
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

void CalChartFrame::OnInsertBefore()
{
    CalChart::Show::Sheet_container_t sht(1, *GetShow()->GetCurrentSheet());
    GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum());
    GetFieldView()->GoToPrevSheet();
}

void CalChartFrame::OnInsertAfter()
{
    CalChart::Show::Sheet_container_t sht(1, *GetShow()->GetCurrentSheet());
    GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum() + 1);
    GetFieldView()->GoToNextSheet();
}

void CalChartFrame::OnInsertFromOtherShow()
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

void CalChartFrame::OnCopySheet()
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

void CalChartFrame::OnPasteSheet()
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
void CalChartFrame::OnRelabel()
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

void CalChartFrame::OnEditPrintContinuity()
{
    mAUIManager->GetPane("Print Continuity").Show(true);
    mAUIManager->Update();
}

void CalChartFrame::OnSetSheetTitle()
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

void CalChartFrame::OnSetBeats()
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

void CalChartFrame::OnSetupMarchers()
{
    if (GetShow()) {
        SetupMarchers dialog(*GetShow(), this);
        if (dialog.ShowModal() == wxID_OK) {
            GetFieldView()->DoSetupMarchers(dialog.GetLabelsAndInstruments(), dialog.GetNumberColumns());
        }
    }
}

void CalChartFrame::OnSetupInstruments()
{
    if (GetShow()) {
        SetupInstruments dialog(*GetShow(), this);
        if (dialog.ShowModal() == wxID_OK) {
            GetFieldView()->DoSetInstruments(dialog.GetInstruments());
        }
    }
}

void CalChartFrame::OnPointPicker()
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

void CalChartFrame::OnSelectAll()
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

void CalChartFrame::OnAddBackgroundImage()
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
        mShowBackgroundImages->Check(true);
        mAdjustBackgroundImageMode->Check(true);
        mCanvas->Refresh();
    }
}

void CalChartFrame::OnAdjustBackgroundImageMode()
{
    bool toggle = !GetFieldView()->DoingPictureAdjustment();
    GetFieldView()->DoPictureAdjustment(toggle);
    mAdjustBackgroundImageMode->Check(toggle);
    if (toggle) {
        GetFieldView()->DoDrawBackground(toggle);
        mShowBackgroundImages->Check(toggle);
    }
    mCanvas->Refresh();
}

void CalChartFrame::OnShowBackgroundImages()
{
    bool toggle = !GetFieldView()->DoingDrawBackground();
    GetFieldView()->DoDrawBackground(toggle);
    mShowBackgroundImages->Check(toggle);
    if (!toggle) {
        GetFieldView()->DoPictureAdjustment(toggle);
        mAdjustBackgroundImageMode->Check(toggle);
    }
    mCanvas->Refresh();
}

void CalChartFrame::OnCmd_GhostOption(wxCommandEvent&)
{
    auto selection = static_cast<GhostSource>(static_cast<wxChoice*>(FindWindow(CALCHART__GhostControls))->GetSelection());
    return OnGhostOption(selection);
}

void CalChartFrame::OnGhostOption(GhostSource option)
{
    switch (option) {
    case GhostSource::disabled:
        GetFieldView()->SetGhostSource(GhostSource::disabled);
        break;
    case GhostSource::next:
        GetFieldView()->SetGhostSource(GhostSource::next);
        break;
    case GhostSource::previous:
        GetFieldView()->SetGhostSource(GhostSource::previous);
        break;
    case GhostSource::specific: {
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
    auto active = GetFieldView()->GetGhostModuleIsActive();
    mGhostOff->Enable(active);
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

void CalChartFrame::OnAdjustViews(size_t which)
{
    ChangePaneVisibility(!mAUIManager->GetPane(mLookupSubWindows.at(which)).IsShown(), which);
}

void CalChartFrame::OnSwapAnimation()
{
    ChangeMainFieldVisibility(!mMainFieldVisible);
}

void CalChartFrame::AUIIsClose(wxAuiManagerEvent& event)
{
    auto which = mLookupSubWindowToIndex[event.GetPane()->window];
    ChangePaneVisibility(!mAUIManager->GetPane(event.GetPane()->window).IsShown(), which);
}

void CalChartFrame::ChangePaneVisibility(bool show, size_t which)
{
    if (which >= mLookupSubWindows.size()) {
        return;
    }
    auto name = kAUINames.at(which);
    if (show) {
        mAdjustPaneIndex.at(which)->SetItemLabel(std::string("Hide ") + name);
    } else {
        mAdjustPaneIndex.at(which)->SetItemLabel(std::string("Show ") + name);
    }
    mAUIManager->GetPane(mLookupSubWindows.at(which)).Show(show);
    mAUIManager->Update();
}

void CalChartFrame::ChangeMainFieldVisibility(bool show)
{
    mMainFieldVisible = show;
    if (mMainFieldVisible) {
        mViewSwapFieldAndAnimate->SetItemLabel("View Animation\tCTRL-RETURN");
    } else {
        mViewSwapFieldAndAnimate->SetItemLabel("View Field\tCTRL-RETURN");
    }
    ShowFieldAndHideAnimation(mMainFieldVisible);
}

void CalChartFrame::OnResetReferencePoint()
{
    GetFieldView()->DoResetReferencePoint();
}

void CalChartFrame::OnCmd_ChangedColorPalette(wxCommandEvent&)
{
    GetFieldView()->OnUpdate(nullptr);
}

void CalChartFrame::OnSolveTransition()
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
    mDrawPaths->Check(event.IsChecked());
}

void CalChartFrame::zoom_callback(wxCommandEvent&)
{
    auto zoom_amount = FieldControls::GetZoomAmount(this);
    if (zoom_amount == 0) {
        zoom_amount = mCanvas->ZoomToFitFactor();
    }
    do_zoom(zoom_amount);
}

void CalChartFrame::OnZoomFit()
{
    do_zoom(mCanvas->ZoomToFitFactor());
}

void CalChartFrame::OnCmd_ZoomIn(wxCommandEvent&)
{
    OnZoomIn();
}

void CalChartFrame::OnZoomIn()
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
    OnZoomOut();
}

void CalChartFrame::OnZoomOut()
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
