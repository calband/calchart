/*
 * field_frame.h
 * Frame for the field window
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
#include <sstream>
#include <fstream>

#include "field_frame.h"

#include "platconf.h"
#include "cc_coord.h"
#include "field_canvas.h"
#include "cc_show.h"
#include "top_frame.h"
#include "calchartapp.h"
#include "print_ps_dialog.h"
#include "cc_preferences_ui.h"
#include "modes.h"
#include "confgr.h"
#include "ccvers.h"
#include "cont_ui.h"
#include "print_cont_ui.h"
#include "show_ui.h"
#include "animation_frame.h"
#include "toolbar.h"
#include "ui_enums.h"
#include "field_view.h"
#include "draw.h"
#include "cc_sheet.h"
#include "cc_point.h"
#include "cc_fileformat.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#ifdef __WXMSW__
#include <wx/helpwin.h>
#endif
#include <wx/cmdproc.h>
#include <wx/tglbtn.h>
#include <wx/clipbrd.h>

const wxString kSheetDataClipboardFormat = wxT("CC_sheet_clipboard_v1");

const wxString gridtext[] = {
    wxT("None"), wxT("1"), wxT("2"), wxT("4"), wxT("Mil"), wxT("2-Mil"),
};

const int zoom_amounts[] = {
    500, 200, 150, 100, 75, 50, 25, 10,
};

static const wxChar* file_wild = FILE_WILDCARDS;

struct GridValue {
    Coord num, sub;
};

GridValue gridvalue[] = { { 1, 0 },
    { Int2Coord(1), 0 },
    { Int2Coord(2), 0 },
    { Int2Coord(4), 0 },
    { Int2Coord(4), static_cast<Coord>(Int2Coord(4) / 3) },
    { Int2Coord(8), static_cast<Coord>(Int2Coord(8) / 3) } };

extern wxPrintDialogData* gPrintDialogData;

BEGIN_EVENT_TABLE(FieldFrame, wxDocChildFrame)
EVT_CHAR(FieldFrame::OnChar)
EVT_MENU(CALCHART__APPEND_FILE, FieldFrame::OnCmdAppend)
EVT_MENU(CALCHART__IMPORT_CONT_FILE, FieldFrame::OnCmdImportCont)
EVT_MENU(CALCHART__wxID_PRINT, FieldFrame::OnCmdPrint)
EVT_MENU(CALCHART__wxID_PREVIEW, FieldFrame::OnCmdPrintPreview)
EVT_MENU(wxID_PAGE_SETUP, FieldFrame::OnCmdPageSetup)
EVT_MENU(CALCHART__LEGACY_PRINT, FieldFrame::OnCmdLegacyPrint)
EVT_MENU(CALCHART__LEGACY_PRINT_EPS, FieldFrame::OnCmdLegacyPrintEPS)
EVT_MENU(CALCHART__COPY_SHEET, FieldFrame::OnCmdCopySheet)
EVT_MENU(CALCHART__PASTE_SHEET, FieldFrame::OnCmdPasteSheet)
EVT_MENU(CALCHART__INSERT_BEFORE, FieldFrame::OnCmdInsertBefore)
EVT_MENU(CALCHART__INSERT_AFTER, FieldFrame::OnCmdInsertAfter)
EVT_MENU(CALCHART__INSERT_OTHER_SHOW, FieldFrame::OnCmdInsertFromOtherShow)
EVT_MENU(wxID_DELETE, FieldFrame::OnCmdDelete)
EVT_MENU(CALCHART__RELABEL, FieldFrame::OnCmdRelabel)
EVT_MENU(CALCHART__EDIT_CONTINUITY, FieldFrame::OnCmdEditCont)
EVT_MENU(CALCHART__PRINT_EDIT_CONTINUITY, FieldFrame::OnCmdEditPrintCont)
EVT_MENU(CALCHART__SET_SHEET_TITLE, FieldFrame::OnCmdSetSheetTitle)
EVT_MENU(CALCHART__SET_BEATS, FieldFrame::OnCmdSetBeats)
EVT_MENU(CALCHART__SETUP, FieldFrame::OnCmdSetup)
EVT_MENU(CALCHART__SETDESCRIPTION, FieldFrame::OnCmdSetDescription)
EVT_MENU(CALCHART__SETMODE, FieldFrame::OnCmdSetMode)
EVT_MENU(CALCHART__POINTS, FieldFrame::OnCmdPoints)
EVT_MENU(CALCHART__ANIMATE, FieldFrame::OnCmdAnimate)
EVT_MENU(wxID_ABOUT, FieldFrame::OnCmdAbout)
EVT_MENU(wxID_HELP, FieldFrame::OnCmdHelp)
EVT_MENU(CALCHART__ShowBackgroundImages, FieldFrame::OnCmd_ShowBackgroundImages)
EVT_MENU(CALCHART__AddBackgroundImage, FieldFrame::OnCmd_AddBackgroundImage)
EVT_MENU(CALCHART__AdjustBackgroundImageMode, FieldFrame::OnCmd_AdjustBackgroundImageMode)
EVT_MENU(CALCHART__GhostOff, FieldFrame::OnCmd_GhostOption)
EVT_MENU(CALCHART__GhostNextSheet, FieldFrame::OnCmd_GhostOption)
EVT_MENU(CALCHART__GhostPreviousSheet, FieldFrame::OnCmd_GhostOption)
EVT_MENU(CALCHART__GhostNthSheet, FieldFrame::OnCmd_GhostOption)
EVT_MENU(CALCHART__prev_ss, FieldFrame::OnCmd_prev_ss)
EVT_MENU(CALCHART__next_ss, FieldFrame::OnCmd_next_ss)
EVT_MENU(CALCHART__box, FieldFrame::OnCmd_box)
EVT_MENU(CALCHART__poly, FieldFrame::OnCmd_poly)
EVT_MENU(CALCHART__lasso, FieldFrame::OnCmd_lasso)
EVT_MENU(CALCHART__move, FieldFrame::OnCmd_move)
EVT_MENU(CALCHART__swap, FieldFrame::OnCmd_swap)
EVT_MENU(CALCHART__line, FieldFrame::OnCmd_line)
EVT_MENU(CALCHART__rot, FieldFrame::OnCmd_rot)
EVT_MENU(CALCHART__shear, FieldFrame::OnCmd_shear)
EVT_MENU(CALCHART__reflect, FieldFrame::OnCmd_reflect)
EVT_MENU(CALCHART__size, FieldFrame::OnCmd_size)
EVT_MENU(CALCHART__genius, FieldFrame::OnCmd_genius)
EVT_MENU(CALCHART__label_left, FieldFrame::OnCmd_label_left)
EVT_MENU(CALCHART__label_right, FieldFrame::OnCmd_label_right)
EVT_MENU(CALCHART__label_flip, FieldFrame::OnCmd_label_flip)
EVT_MENU(CALCHART__label_hide, FieldFrame::OnCmd_label_hide)
EVT_MENU(CALCHART__label_show, FieldFrame::OnCmd_label_show)
EVT_MENU(CALCHART__label_visibility_toggle,
    FieldFrame::OnCmd_label_visibility_toggle)
EVT_MENU(CALCHART__setsym0, FieldFrame::OnCmd_setsym0)
EVT_MENU(CALCHART__setsym1, FieldFrame::OnCmd_setsym1)
EVT_MENU(CALCHART__setsym2, FieldFrame::OnCmd_setsym2)
EVT_MENU(CALCHART__setsym3, FieldFrame::OnCmd_setsym3)
EVT_MENU(CALCHART__setsym4, FieldFrame::OnCmd_setsym4)
EVT_MENU(CALCHART__setsym5, FieldFrame::OnCmd_setsym5)
EVT_MENU(CALCHART__setsym6, FieldFrame::OnCmd_setsym6)
EVT_MENU(CALCHART__setsym7, FieldFrame::OnCmd_setsym7)
EVT_MENU(CALCHART__EXPORT_VIEWER_FILE, FieldFrame::OnCmdExportViewerFile)
EVT_MENU(wxID_PREFERENCES, FieldFrame::OnCmdPreferences)
EVT_COMMAND_SCROLL(CALCHART__slider_sheet_callback,
    FieldFrame::slider_sheet_callback)
EVT_COMBOBOX(CALCHART__slider_zoom, FieldFrame::zoom_callback)
EVT_TEXT_ENTER(CALCHART__slider_zoom, FieldFrame::zoom_callback_textenter)
EVT_CHOICE(CALCHART__refnum_callback, FieldFrame::refnum_callback)
EVT_TOGGLEBUTTON(CALCHART__draw_paths, FieldFrame::OnEnableDrawPaths)
EVT_MENU(CALCHART__ResetReferencePoint, FieldFrame::OnCmd_ResetReferencePoint)
EVT_BUTTON(CALCHART__ResetReferencePoint, FieldFrame::OnCmd_ResetReferencePoint)
EVT_SIZE(FieldFrame::OnSize)
END_EVENT_TABLE()

class MyPrintout : public wxPrintout {
public:
    MyPrintout(const wxString& title, const CalChartDoc& show,
        const CalChartConfiguration& config_)
        : wxPrintout(title)
        , mShow(show)
        , config(config_)
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
        CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(pageNum - 1);

        int size = gPrintDialogData->GetPrintData().GetOrientation();

        DrawForPrinting(dc, config, mShow, *sheet, 0, 2 == size);

        return true;
    }
    const CalChartDoc& mShow;
    const CalChartConfiguration& config;
};

// Main frame constructor
FieldFrame::FieldFrame(wxDocument* doc, wxView* view,
    CalChartConfiguration& config_, wxDocParentFrame* frame,
    const wxPoint& pos, const wxSize& size)
    : wxDocChildFrame(doc, view, frame, -1, wxT("CalChart"), pos, size)
    , mCanvas(NULL)
    , mAnimationFrame(NULL)
    , config(config_)
{
    this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
    // Give it an icon
    SetBandIcon(this);

    // Give it a status line
    CreateStatusBar(3);
    SetStatusText(wxT("Welcome to Calchart v" wxT(STRINGIZE(CC_MAJOR_VERSION))
            wxT(".") wxT(STRINGIZE(CC_MINOR_VERSION)) wxT(".")
                wxT(STRINGIZE(CC_SUB_MINOR_VERSION))));

    // Make a menubar
    wxMenu* file_menu = new wxMenu;
    file_menu->Append(wxID_NEW, wxT("&New Show\tCTRL-N"),
        wxT("Create a new show"));
    file_menu->Append(wxID_OPEN, wxT("&Open...\tCTRL-O"),
        wxT("Load a saved show"));
    file_menu->Append(CALCHART__APPEND_FILE, wxT("&Append..."),
        wxT("Append a show to the end"));
    file_menu->Append(CALCHART__IMPORT_CONT_FILE,
        wxT("&Import Continuity...\tCTRL-I"),
        wxT("Import continuity text"));
    file_menu->Append(wxID_SAVE, wxT("&Save\tCTRL-S"), wxT("Save show"));
    file_menu->Append(wxID_SAVEAS, wxT("Save &As...\tCTRL-SHIFT-S"),
        wxT("Save show as a new name"));
    file_menu->Append(CALCHART__wxID_PRINT, wxT("&Print...\tCTRL-P"),
        wxT("Print this show"));
    file_menu->Append(CALCHART__wxID_PREVIEW, wxT("Preview...\tCTRL-SHIFT-P"),
        wxT("Preview this show"));
    file_menu->Append(wxID_PAGE_SETUP, wxT("Page Setup...\tCTRL-SHIFT-ALT-P"),
        wxT("Setup Pages"));
    file_menu->Append(CALCHART__LEGACY_PRINT, wxT("Print to PS..."),
        wxT("Print show to PostScript"));
    file_menu->Append(CALCHART__LEGACY_PRINT_EPS, wxT("Print to EPS..."),
        wxT("Print show to Encapsulated PostScript"));
    file_menu->Append(CALCHART__EXPORT_VIEWER_FILE, wxT("Export for Online Viewer..."), wxT("Export show to be viewed using the CalChart Online Viewer"));
    file_menu->Append(wxID_PREFERENCES, wxT("&Preferences\tCTRL-,"));
    file_menu->Append(wxID_CLOSE, wxT("Close Window\tCTRL-W"),
        wxT("Close this window"));
    file_menu->Append(wxID_EXIT, wxT("&Quit\tCTRL-Q"), wxT("Quit CalChart"));

    // A nice touch: a history of files visited. Use this menu.
    // causes a crash :(
    // view->GetDocumentManager()->FileHistoryUseMenu(file_menu);

    wxMenu* edit_menu = new wxMenu;
    edit_menu->Append(wxID_UNDO, wxT("&Undo\tCTRL-Z"));
    edit_menu->Append(wxID_REDO, wxT("&Redo\tCTRL-SHIFT-Z"));
    edit_menu->Append(CALCHART__COPY_SHEET, wxT("&Copy Sheet\tCTRL-C"),
        wxT("Copy the current stuntsheet"));
    edit_menu->Append(CALCHART__PASTE_SHEET, wxT("&Paste Sheet\tCTRL-V"),
        wxT("Paste the current stuntsheet"));
    edit_menu->Append(CALCHART__INSERT_BEFORE,
        wxT("&Insert Sheet Before\tCTRL-["),
        wxT("Insert a new stuntsheet before this one"));
    edit_menu->Append(CALCHART__INSERT_AFTER, wxT("Insert Sheet &After\tCTRL-]"),
        wxT("Insert a new stuntsheet after this one"));
    edit_menu->Append(CALCHART__INSERT_OTHER_SHOW,
        wxT("Insert Sheets From Other Show..."),
        wxT("Insert a saved stuntsheet after this one"));
    edit_menu->Append(wxID_DELETE, wxT("&Delete Sheet\tCTRL-DEL"),
        wxT("Delete this stuntsheet"));
    edit_menu->Append(CALCHART__RELABEL, wxT("&Relabel Sheets\tCTRL-R"),
        wxT("Relabel all stuntsheets after this one"));
    edit_menu->Append(CALCHART__SETUP, wxT("Set &Up Marchers...\tCTRL-U"),
        wxT("Setup number of marchers"));
    edit_menu->Append(CALCHART__SETDESCRIPTION, wxT("Set Show &Description..."),
        wxT("Set the show description"));
    edit_menu->Append(CALCHART__SETMODE, wxT("Set Show &Mode..."),
        wxT("Set the show mode"));
    edit_menu->Append(CALCHART__POINTS, wxT("&Point Selections..."),
        wxT("Select Points"));
    edit_menu->Append(CALCHART__SET_SHEET_TITLE,
        wxT("Set Sheet &Title...\tCTRL-T"),
        wxT("Change the title of this stuntsheet"));
    edit_menu->Append(CALCHART__SET_BEATS, wxT("Set &Beats...\tCTRL-B"),
        wxT("Change the number of beats for this stuntsheet"));
    edit_menu->Append(CALCHART__EDIT_CONTINUITY,
        wxT("&Edit Continuity...\tCTRL-E"),
        wxT("Edit continuity for this stuntsheet"));
    edit_menu->Append(CALCHART__PRINT_EDIT_CONTINUITY,
        wxT("Edit Print Continuity..."),
        wxT("Edit Print continuity for this stuntsheet"));
    edit_menu->Append(CALCHART__ResetReferencePoint,
        wxT("Reset reference point..."),
        wxT("Reset the current reference point"));

    wxMenu* anim_menu = new wxMenu;
    anim_menu->Append(CALCHART__ANIMATE, wxT("Open in &Viewer...\tCTRL-RETURN"),
        wxT("Open show in CalChart Viewer"));

    wxMenu* backgroundimage_menu = new wxMenu;
    backgroundimage_menu->AppendCheckItem(CALCHART__ShowBackgroundImages,
        wxT("Show Background Images"),
        wxT("Toggle showing background images"));
    backgroundimage_menu->Append(CALCHART__AddBackgroundImage,
        wxT("Add Background Image..."),
        wxT("Add a background image"));
    backgroundimage_menu->AppendCheckItem(CALCHART__AdjustBackgroundImageMode,
        wxT("Image Adjust Mode..."),
        wxT("Mode to adjust background images"));
    backgroundimage_menu->Enable(CALCHART__ShowBackgroundImages, true);
    backgroundimage_menu->Enable(CALCHART__AddBackgroundImage, true);
    backgroundimage_menu->Enable(CALCHART__AdjustBackgroundImageMode, true);

    wxMenu* ghost_menu = new wxMenu;
    ghost_menu->Append(CALCHART__GhostOff, wxT("Disable Ghost View"),
        wxT("Turn off ghost view"));
    ghost_menu->Append(CALCHART__GhostNextSheet, wxT("Ghost Next Sheet"),
        wxT("Draw a ghost of the next stuntsheet"));
    ghost_menu->Append(CALCHART__GhostPreviousSheet, wxT("Ghost Previous Sheet"),
        wxT("Draw a ghost of the previous stuntsheet"));
    ghost_menu->Append(CALCHART__GhostNthSheet, wxT("Ghost Particular Sheet..."),
        wxT("Draw a ghost of a particular stuntsheet"));

    wxMenu* help_menu = new wxMenu;
    help_menu->Append(wxID_ABOUT, wxT("&About CalChart...\tCTRL-A"),
        wxT("Information about the program"));
    help_menu->Append(wxID_HELP, wxT("&Help on CalChart...\tCTRL-H"),
        wxT("Help on using CalChart"));

    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, wxT("&File"));
    menu_bar->Append(edit_menu, wxT("&Edit"));
    menu_bar->Append(backgroundimage_menu, wxT("&Field Image"));
    menu_bar->Append(ghost_menu, wxT("&Ghost View"));
    menu_bar->Append(anim_menu, wxT("&CalChart Viewer"));
    menu_bar->Append(help_menu, wxT("&Help"));
    SetMenuBar(menu_bar);

    refreshGhostOptionStates();

    // Add a toolbar
    AddCoolToolBar(GetMainToolBar(), *this);

    // Add the field canvas
    this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
    mCanvas = new FieldCanvas(view, this, config.Get_FieldFrameZoom());
    // set scroll rate 1 to 1, so we can have even scrolling of whole field
    mCanvas->SetScrollRate(1, 1);

    CalChartDoc* show = static_cast<CalChartDoc*>(doc);
    SetTitle(show->GetTitle());
    show->SetCurrentSheet(0);

    // Add the controls
    wxSizerFlags topRowSizerFlags = wxSizerFlags(1).Expand().Border(0, 5);
    wxSizerFlags centerText = wxSizerFlags(0).Border(wxALL, 5).Align(wxALIGN_CENTER_HORIZONTAL);
    wxSizerFlags centerWidget = wxSizerFlags(0).Expand().Border(wxALL, 5);
    wxBoxSizer* fullsizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* toprow = new wxBoxSizer(wxHORIZONTAL);

    // Grid choice
    wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Grid Spacing")),
        centerText);
    mGridChoice = new wxChoice(this, -1, wxPoint(-1, -1), wxSize(-1, -1),
        sizeof(gridtext) / sizeof(wxString), gridtext);
    mGridChoice->SetSelection(2);
    sizer1->Add(mGridChoice, centerWidget);
    toprow->Add(sizer1, topRowSizerFlags);

    // Zoom slider
    sizer1 = new wxBoxSizer(wxVERTICAL);
    sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Zoom")), centerText);
    wxArrayString zoomtext;
    for (size_t i = 0; i < sizeof(zoom_amounts) / (sizeof(zoom_amounts[0]));
         ++i) {
        wxString buf;
        buf.sprintf(wxT("%d%%"), zoom_amounts[i]);
        zoomtext.Add(buf);
    }
    zoomtext.Add(wxT("Fit"));
    mZoomBox = new wxComboBox(this, CALCHART__slider_zoom, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, zoomtext,
        wxTE_PROCESS_ENTER);
    // set the text to the default zoom level
    if (mZoomBox) {
        wxString zoomtxt;
        zoomtxt.sprintf("%d%%", (int)(config.Get_FieldFrameZoom() * 100));
        mZoomBox->SetValue(zoomtxt);
    }
    sizer1->Add(mZoomBox, centerWidget);
    toprow->Add(sizer1, topRowSizerFlags);

    // Reference choice
    sizer1 = new wxBoxSizer(wxVERTICAL);
    sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Ref Group")), centerText);
    {
        wxString buf;
        unsigned i;

        mRefChoice = new wxChoice(this, CALCHART__refnum_callback);
        mRefChoice->Append(wxT("Off"));
        for (i = 1; i <= CC_point::kNumRefPoints; i++) {
            buf.sprintf(wxT("%u"), i);
            mRefChoice->Append(buf);
        }
    }
    sizer1->Add(mRefChoice, centerWidget);
    toprow->Add(sizer1, topRowSizerFlags);

    sizer1 = new wxBoxSizer(wxVERTICAL);
    sizer1->Add(new wxButton(this, CALCHART__ResetReferencePoint,
                    wxT("Reset Ref Points")),
        centerWidget);
    toprow->Add(sizer1, topRowSizerFlags);

    sizer1 = new wxBoxSizer(wxVERTICAL);
    wxToggleButton* checkbox = new wxToggleButton(this, CALCHART__draw_paths, wxT("Draw Paths"));
    checkbox->SetValue(false);
    sizer1->Add(checkbox, centerWidget);
    toprow->Add(sizer1, topRowSizerFlags);

    // Sheet slider (will get set later with UpdatePanel())
    sizer1 = new wxBoxSizer(wxVERTICAL);
    sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Sheet")), centerText);
    // on Mac using wxSL_LABELS will cause a crash?
    mSheetSlider = new wxSlider(this, CALCHART__slider_sheet_callback, 1, 1, 2,
        wxDefaultPosition, wxDefaultSize,
        wxSL_HORIZONTAL | wxSL_LABELS);
    sizer1->Add(mSheetSlider, centerWidget);
    toprow->Add(sizer1, topRowSizerFlags);

    fullsizer->Add(toprow, wxSizerFlags(0).Border(0, 5));

    // Update the command processor with the undo/redo menu items
    edit_menu->FindItem(wxID_UNDO)->Enable(false);
    edit_menu->FindItem(wxID_REDO)->Enable(false);
    doc->GetCommandProcessor()->SetEditMenu(edit_menu);
    doc->GetCommandProcessor()->Initialize();

    // Update the tool bar
    SetCurrentLasso(mCanvas->GetCurrentLasso());
    SetCurrentMove(mCanvas->GetCurrentMove());

    // Show the frame
    UpdatePanel();
    mCanvas->Refresh();

    fullsizer->Add(mCanvas, 1, wxEXPAND);
    SetSizer(fullsizer);
    // re-set the size
    SetSize(size);
    Show(true);
}

FieldFrame::~FieldFrame() {}

// Intercept menu commands

void FieldFrame::OnCmdAppend(wxCommandEvent& event) { AppendShow(); }

void FieldFrame::OnCmdImportCont(wxCommandEvent& event) { ImportContFile(); }

// the default wxView print doesn't handle landscape.  rolling our own
void FieldFrame::OnCmdPrint(wxCommandEvent& event)
{
    // grab our current page setup.
    wxPrinter printer(gPrintDialogData);
    MyPrintout printout(wxT("My Printout"), *GetShow(), config);
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
        else {
            wxMessageBox(wxT("Printing cancelled"), wxT("Printing"), wxOK);
        }
    }
    else {
        *gPrintDialogData = printer.GetPrintDialogData();
    }
}

// the default wxView print doesn't handle landscape.  rolling our own
void FieldFrame::OnCmdPrintPreview(wxCommandEvent& event)
{
    // grab our current page setup.
    wxPrintPreview* preview = new wxPrintPreview(
        new MyPrintout(wxT("My Printout"), *GetShow(), config),
        new MyPrintout(wxT("My Printout"), *GetShow(), config), gPrintDialogData);
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

void FieldFrame::OnCmdPageSetup(wxCommandEvent& event)
{
    wxPageSetupData mPageSetupData;
    mPageSetupData.EnableOrientation(true);

    wxPageSetupDialog pageSetupDialog(this, &mPageSetupData);
    if (pageSetupDialog.ShowModal() == wxID_OK)
        mPageSetupData = pageSetupDialog.GetPageSetupData();
    // pass the print data to our global print dialog
    gPrintDialogData->SetPrintData(mPageSetupData.GetPrintData());
}

void FieldFrame::OnCmdLegacyPrint(wxCommandEvent& event)
{
    if (GetShow()) {
        PrintPostScriptDialog dialog(static_cast<CalChartDoc*>(GetDocument()),
            false, this);
        if (dialog.ShowModal() == wxID_OK) {
            dialog.PrintShow(config);
        }
    }
}

void FieldFrame::OnCmdLegacyPrintEPS(wxCommandEvent& event)
{
    if (GetShow()) {
        PrintPostScriptDialog dialog(static_cast<CalChartDoc*>(GetDocument()),
            true, this);
        if (dialog.ShowModal() == wxID_OK) {
            dialog.PrintShow(config);
        }
    }
}

void FieldFrame::OnCmdExportViewerFile(wxCommandEvent& event) {
    if (GetShow())
    {
        wxFileDialog saveFileDialog(this, _("Save viewer file"), "", "", "viewer files (*.viewer)|*.viewer", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

        if (saveFileDialog.ShowModal() == wxID_CANCEL)
            return;
        
        if (!GetShow()->exportViewerFile(saveFileDialog.GetPath())) {
            wxMessageBox(wxT("There was a problem exporting the viewer file.\n") + saveFileDialog.GetPath(), wxT("Exporting Viewer File"));
            return;
        }
    }
}

void FieldFrame::OnCmdPreferences(wxCommandEvent& event)
{
    CalChartPreferences dialog1(this);
    if (dialog1.ShowModal() == wxID_OK) {
    }
}

void FieldFrame::OnCmdInsertBefore(wxCommandEvent& event)
{
    CC_show::CC_sheet_container_t sht(1, *GetShow()->GetCurrentSheet());
    GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum());
    GetFieldView()->GoToPrevSheet();
}

void FieldFrame::OnCmdInsertAfter(wxCommandEvent& event)
{
    CC_show::CC_sheet_container_t sht(1, *GetShow()->GetCurrentSheet());
    GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum() + 1);
    GetFieldView()->GoToNextSheet();
}

void FieldFrame::OnCmdInsertFromOtherShow(wxCommandEvent& event)
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
    }
    else {
        // don't ask user if the first sheet they want to copy over is the last
        // sheet in the show being copied from
        endValue = beginValue;
    }

    int currend = GetShow()->GetNumSheets();
    CC_show::CC_sheet_container_t sheets((&show)->GetNthSheet(beginValue - 1),
        (&show)->GetNthSheet(endValue));
    GetFieldView()->DoInsertSheetsOtherShow(
        sheets, GetFieldView()->GetCurrentSheetNum() + 1, currend - 1);
}

void FieldFrame::OnCmdCopySheet(wxCommandEvent& event)
{
    if (wxTheClipboard->Open()) {
        std::unique_ptr<wxCustomDataObject> clipboardObject(
            new wxCustomDataObject(kSheetDataClipboardFormat));
        std::vector<uint8_t> serializedSheet = GetShow()->GetCurrentSheet()->SerializeSheet();

        uint16_t numPoints = GetShow()->GetNumPoints();

        int bytesForNumPoints = sizeof(numPoints);
        int bytesForSheetData = serializedSheet.size() * sizeof(uint8_t);
        int totalBytes = bytesForNumPoints + bytesForSheetData;
        std::vector<char> clipboardData(totalBytes);
        memcpy(clipboardData.data(), &numPoints, bytesForNumPoints);
        memcpy(clipboardData.data() + bytesForNumPoints, serializedSheet.data(),
            bytesForSheetData);

        clipboardObject->SetData(totalBytes, clipboardData.data());

        wxTheClipboard->SetData(clipboardObject.release());
        wxTheClipboard->Close();
    }
}

void FieldFrame::OnCmdPasteSheet(wxCommandEvent& event)
{
    if (wxTheClipboard->Open()) {
        if (wxTheClipboard->IsSupported(kSheetDataClipboardFormat)) {
            wxCustomDataObject clipboardObject(kSheetDataClipboardFormat);
            wxTheClipboard->GetData(clipboardObject);

            uint16_t numPoints;
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

            ReadLong(sheetStream);
            ReadLong(sheetStream);

            sheetStream.unsetf(std::ios_base::skipws);
            std::istream_iterator<uint8_t> theBegin(sheetStream);
            std::istream_iterator<uint8_t> theEnd{};
            std::vector<uint8_t> data(theBegin, theEnd);

            CC_show::CC_sheet_container_t sht(
                1, CC_sheet(numPoints, data.data(), data.size(),
                       Current_version_and_later()));
            GetFieldView()->DoInsertSheets(sht, GetFieldView()->GetCurrentSheetNum());
        }
        wxTheClipboard->Close();
    }
}

void FieldFrame::OnCmdDelete(wxCommandEvent& event)
{
    if (GetFieldView()->GetNumSheets() > 1) {
        GetFieldView()->DoDeleteSheet(GetFieldView()->GetCurrentSheetNum());
    }
}

void FieldFrame::OnCmdRelabel(wxCommandEvent& event)
{
    if (GetFieldView()->GetCurrentSheetNum() + 1 < GetFieldView()->GetNumSheets()) {
        if (wxMessageBox(wxT("Relabeling sheets is not undoable.\nProceed?"),
                wxT("Relabel sheets"), wxYES_NO)
            == wxYES) {
            if (!GetShow()->RelabelSheets(GetFieldView()->GetCurrentSheetNum()))
                (void)wxMessageBox(wxT("Stuntsheets don't match"),
                    wxT("Relabel sheets"));
            else {
                GetDocument()->Modify(true);
            }
        }
    }
    else {
        (void)wxMessageBox(wxT("This can't used on the last stuntsheet"),
            wxT("Relabel sheets"));
    }
}

void FieldFrame::OnCmdEditCont(wxCommandEvent& event)
{
    if (GetShow()) {
        ContinuityEditor* ce = new ContinuityEditor(static_cast<CalChartDoc*>(GetDocument()), this,
            wxID_ANY, wxT("Animation Continuity"));
        // make it modeless:
        ce->Show();
    }
}

void FieldFrame::OnCmdEditPrintCont(wxCommandEvent& event)
{
    if (GetShow()) {
        PrintContinuityEditor* ce = new PrintContinuityEditor(
            static_cast<CalChartDoc*>(GetDocument()), this);
        // make it modeless:
        ce->Show();
    }
}

void FieldFrame::OnCmdSetSheetTitle(wxCommandEvent& event)
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

void FieldFrame::OnCmdSetBeats(wxCommandEvent& event)
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
                GetFieldView()->DoSetSheetBeats(val);
            }
        }
    }
}

void FieldFrame::OnCmdSetup(wxCommandEvent& event) { Setup(); }

void FieldFrame::OnCmdSetDescription(wxCommandEvent& event)
{
    SetDescription();
}

void FieldFrame::OnCmdSetMode(wxCommandEvent& event) { SetMode(); }

void FieldFrame::OnCmdPoints(wxCommandEvent& event)
{
    if (GetShow()) {
        PointPicker* pp = new PointPicker(*GetShow(), this);
        // make it modeless:
        pp->Show();
    }
}

void FieldFrame::OnCmdAnimate(wxCommandEvent& event)
{
    // we want to have only animation frame at a time
    if (mAnimationFrame) {
        mAnimationFrame->Raise();
    }
    else if (GetShow()) {
        mAnimationFrame = new AnimationFrame(
            [this]() { this->ClearAnimationFrame(); }, GetShow(), config, GetView(),
            this, wxSize(config.Get_AnimationFrameWidth(),
                      config.Get_AnimationFrameHeight()));
    }
}

void FieldFrame::ClearAnimationFrame() { mAnimationFrame = NULL; }

void FieldFrame::OnCmdAbout(wxCommandEvent& event) { TopFrame::About(); }

void FieldFrame::OnCmdHelp(wxCommandEvent& event) { TopFrame::Help(); }

void FieldFrame::OnCmd_prev_ss(wxCommandEvent& event)
{
    GetFieldView()->GoToPrevSheet();
}

void FieldFrame::OnCmd_next_ss(wxCommandEvent& event)
{
    GetFieldView()->GoToNextSheet();
}

void FieldFrame::OnCmd_box(wxCommandEvent& event)
{
    SetCurrentLasso(CC_DRAG_BOX);
}

void FieldFrame::OnCmd_poly(wxCommandEvent& event)
{
    SetCurrentLasso(CC_DRAG_POLY);
}

void FieldFrame::OnCmd_lasso(wxCommandEvent& event)
{
    SetCurrentLasso(CC_DRAG_LASSO);
}

void FieldFrame::OnCmd_move(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_NORMAL);
}

void FieldFrame::OnCmd_swap(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SWAP);
}

void FieldFrame::OnCmd_line(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_LINE);
}

void FieldFrame::OnCmd_rot(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_ROTATE);
}

void FieldFrame::OnCmd_shear(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SHEAR);
}

void FieldFrame::OnCmd_reflect(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_REFL);
}

void FieldFrame::OnCmd_size(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_SIZE);
}

void FieldFrame::OnCmd_genius(wxCommandEvent& event)
{
    SetCurrentMove(CC_MOVE_GENIUS);
}

void FieldFrame::OnCmd_label_left(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabel(false);
}

void FieldFrame::OnCmd_label_right(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabel(true);
}

void FieldFrame::OnCmd_label_flip(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabelFlip();
}

void FieldFrame::OnCmd_label_hide(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabelVisibility(false);
}

void FieldFrame::OnCmd_label_show(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsLabelVisibility(true);
}

void FieldFrame::OnCmd_label_visibility_toggle(wxCommandEvent& event)
{
    GetFieldView()->DoTogglePointsLabelVisibility();
}

void FieldFrame::OnCmd_setsym0(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_PLAIN);
}

void FieldFrame::OnCmd_setsym1(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SOL);
}

void FieldFrame::OnCmd_setsym2(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_BKSL);
}

void FieldFrame::OnCmd_setsym3(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SL);
}

void FieldFrame::OnCmd_setsym4(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_X);
}

void FieldFrame::OnCmd_setsym5(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SOLBKSL);
}

void FieldFrame::OnCmd_setsym6(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SOLSL);
}

void FieldFrame::OnCmd_setsym7(wxCommandEvent& event)
{
    GetFieldView()->DoSetPointsSymbol(SYMBOL_SOLX);
}

void FieldFrame::OnChar(wxKeyEvent& event) { mCanvas->OnChar(event); }

void FieldFrame::OnCmd_AddBackgroundImage(wxCommandEvent& event)
{
    wxString filename;
    wxInitAllImageHandlers();
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

void FieldFrame::OnCmd_AdjustBackgroundImageMode(wxCommandEvent& event)
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

void FieldFrame::OnCmd_ShowBackgroundImages(wxCommandEvent& event)
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

void FieldFrame::OnCmd_GhostOption(wxCommandEvent& event)
{
    switch (event.GetId()) {
    case CALCHART__GhostOff:
        GetFieldView()->getGhostModule().setGhostSource(GhostModule::disabled);
        break;
    case CALCHART__GhostNextSheet:
        GetFieldView()->getGhostModule().setGhostSource(GhostModule::next);
        break;
    case CALCHART__GhostPreviousSheet:
        GetFieldView()->getGhostModule().setGhostSource(GhostModule::previous);
        break;
    case CALCHART__GhostNthSheet: {
        wxString targetSheet = wxGetTextFromUser("Enter the sheet number to ghost:",
            "Ghost Sheet", "1", this);
        long targetSheetNum = 0;
        if (targetSheet.ToLong(&targetSheetNum)) {
            GetFieldView()->getGhostModule().setGhostSource(GhostModule::specific,
                targetSheetNum - 1);
        }
        else {
            wxMessageBox(wxT("The input must be a number."),
                wxT("Operation failed."));
        }
    } break;
    }
    refreshGhostOptionStates();
    GetCanvas()->Refresh();
}

void FieldFrame::refreshGhostOptionStates()
{
    bool active = GetFieldView()->getGhostModule().isActive();
    GetMenuBar()->FindItem(CALCHART__GhostOff)->Enable(active);
}

void FieldFrame::OnCmd_ResetReferencePoint(wxCommandEvent& event)
{
    GetFieldView()->DoResetReferencePoint();
}

void FieldFrame::OnSize(wxSizeEvent& event)
{
    // HACK: Prevent width and height from growing out of control
    int w = event.GetSize().GetWidth();
    int h = event.GetSize().GetHeight();
    config.Set_FieldFrameWidth((w > 1200) ? 1200 : w);
    config.Set_FieldFrameHeight((h > 700) ? 700 : h);
    super::OnSize(event);
}

// Append a show with file selector
void FieldFrame::AppendShow()
{
    wxString s;
    unsigned currend;

    s = wxFileSelector(wxT("Append show"), wxEmptyString, wxEmptyString,
        wxEmptyString, file_wild);
    if (!s.IsEmpty()) {
        CalChartDoc* shw = new CalChartDoc();
        if (shw->OnOpenDocument(s)) {
            if (shw->GetNumPoints() == GetShow()->GetNumPoints()) {
                currend = GetShow()->GetNumSheets();
                GetFieldView()->DoInsertSheets(
                    CC_show::CC_sheet_container_t(shw->GetSheetBegin(),
                        shw->GetSheetEnd()),
                    currend);
                // This is bad, we are relabeling outside of adding sheets...
                if (!GetShow()->RelabelSheets(currend - 1))
                    (void)wxMessageBox(wxT("Stuntsheets don't match"),
                        wxT("Append Error"));
            }
            else {
                (void)wxMessageBox(wxT("The blocksize doesn't match"),
                    wxT("Append Error"));
            }
        }
        else {
            (void)wxMessageBox(wxT("Error Opening show"), wxT("Load Error"));
        }
        delete shw;
    }
}

// Append a show with file selector
void FieldFrame::ImportContFile()
{
    wxString s;

    s = wxFileSelector(wxT("Import Continuity"), wxEmptyString, wxEmptyString,
        wxEmptyString, wxT("*.txt"));
    if (!s.empty()) {
        GetFieldView()->DoImportPrintableContinuity(s);
    }
}

static inline Coord SNAPGRID(Coord a, Coord n, Coord s)
{
    Coord a2 = (a + (n >> 1)) & (~(n - 1));
    Coord h = s >> 1;
    if ((a - a2) >= h)
        return a2 + s;
    else if ((a - a2) < -h)
        return a2 - s;
    else
        return a2;
}

void FieldFrame::SnapToGrid(CC_coord& c)
{
    Coord gridn, grids;
    int n = mGridChoice->GetSelection();

    gridn = gridvalue[n].num;
    grids = gridvalue[n].sub;

    c.x = SNAPGRID(c.x, gridn, grids);
    // Adjust so 4 step grid will be on visible grid
    c.y = SNAPGRID(c.y - Int2Coord(2), gridn, grids) + Int2Coord(2);
}

void FieldFrame::SetCurrentLasso(CC_DRAG_TYPES type)
{
    // retoggle the tool because we want it to draw as selected
    int toggleID = (type == CC_DRAG_POLY) ? CALCHART__poly : (type == CC_DRAG_LASSO)
            ? CALCHART__lasso
            : CALCHART__box;
    wxToolBar* tb = GetToolBar();
    tb->ToggleTool(toggleID, true);

    mCanvas->SetCurrentLasso(type);
}

void FieldFrame::SetCurrentMove(CC_MOVE_MODES type)
{
    // retoggle the tool because we want it to draw as selected
    wxToolBar* tb = GetToolBar();
    tb->ToggleTool(CALCHART__move + type, true);

    mCanvas->SetCurrentMove(type);
}

void FieldFrame::Setup()
{
    if (GetShow()) {
        ShowInfoReq dialog(*GetShow(), this);
        if (dialog.ShowModal() == wxID_OK) {
            GetFieldView()->DoSetShowInfo(dialog.GetNumberPoints(),
                dialog.GetNumberColumns(),
                dialog.GetLabels());
        }
    }
}

void FieldFrame::SetDescription()
{
    if (GetShow()) {
        wxTextEntryDialog dialog(this, wxT("Please modify the show description\n"),
            wxT("Edit show description\n"),
            GetShow()->GetDescr(), wxOK | wxCANCEL);
        if (dialog.ShowModal() == wxID_OK) {
            GetFieldView()->DoSetDescription(dialog.GetValue());
        }
    }
}

void FieldFrame::SetMode()
{
    if (GetShow()) {
        wxArrayString modeStrings;
        unsigned whichMode = 0, tmode = 0;
        for (auto mode : kShowModeStrings) {
            modeStrings.Add(mode);
            if (mode == GetShow()->GetMode().GetName())
                whichMode = tmode;
        }
        for (auto mode : kSpringShowModeStrings) {
            modeStrings.Add(mode);
            if (mode == GetShow()->GetMode().GetName())
                whichMode = tmode;
        }
        wxSingleChoiceDialog dialog(this, wxT("Please select the show mode\n"),
            wxT("Set show mode\n"), modeStrings);
        dialog.SetSelection(whichMode);
        if (dialog.ShowModal() == wxID_OK) {
            GetFieldView()->DoSetMode(dialog.GetStringSelection());
        }
    }
}

void FieldFrame::refnum_callback(wxCommandEvent&)
{
    GetFieldView()->SetReferencePoint(mRefChoice->GetSelection());
}

void FieldFrame::OnEnableDrawPaths(wxCommandEvent& event)
{
    GetFieldView()->OnEnableDrawPaths(event.IsChecked());
}

void FieldFrame::slider_sheet_callback(wxScrollEvent&)
{
    GetFieldView()->GoToSheet(mSheetSlider->GetValue() - 1);
}

void FieldFrame::zoom_callback(wxCommandEvent& event)
{
    size_t sel = event.GetInt();
    float zoom_amount = 1.0;
    if (sel < sizeof(zoom_amounts) / sizeof(zoom_amounts[0])) {
        zoom_amount = zoom_amounts[sel] / 100.0;
    }
    else if (sel == sizeof(zoom_amounts) / sizeof(zoom_amounts[0])) {
        zoom_amount = mCanvas->ZoomToFitFactor();
    }
    config.Set_FieldFrameZoom(zoom_amount);
    mCanvas->SetZoom(zoom_amount);
}

void FieldFrame::zoom_callback_textenter(wxCommandEvent& event)
{
    wxString zoomtxt = mZoomBox->GetValue();
    // strip the trailing '%' if it exists
    if (zoomtxt.Length() && (zoomtxt.Last() == wxT('%'))) {
        zoomtxt.RemoveLast();
    }
    long zoomnum = 100;
    float zoom_amount = 1.0;
    int zoommax = zoom_amounts[0];
    int zoommin = zoom_amounts[sizeof(zoom_amounts) / sizeof(zoom_amounts[0]) - 1];
    if (zoomtxt.ToLong(&zoomnum) && (zoomnum >= zoommin && zoomnum <= zoommax)) {
        zoom_amount = zoomnum / 100.0;
    }
    else {
        wxString msg;
        msg.sprintf(wxT("Please enter a valid number between %d and %d\n"), zoommin,
            zoommax);
        wxMessageBox(msg, wxT("Invalid number"), wxICON_INFORMATION | wxOK);
        mZoomBox->SetValue(wxT(""));
        // return if not valid
        return;
    }
    config.Set_FieldFrameZoom(zoom_amount);
    // set the text to have '%' appended
    zoomtxt += wxT("%");
    mZoomBox->SetValue(zoomtxt);
    mCanvas->SetZoom(zoom_amount);
}

void FieldFrame::UpdatePanel()
{
    wxString tempbuf;
    CC_show::const_CC_sheet_iterator_t sht = GetShow()->GetCurrentSheet();
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

    if (num > 1) {
        mSheetSlider->Enable(true);
        if ((unsigned)mSheetSlider->GetMax() != num)
            mSheetSlider->SetValue(1); // So Motif doesn't complain about value
        mSheetSlider->SetRange(1, num);
        if ((unsigned)mSheetSlider->GetValue() != curr)
            mSheetSlider->SetValue(curr);
    }
    else {
        mSheetSlider->Enable(false);
    }

    SetTitle(GetDocument()->GetUserReadableName());
}

const FieldView* FieldFrame::GetFieldView() const
{
    return static_cast<const FieldView*>(GetView());
}

FieldView* FieldFrame::GetFieldView()
{
    return static_cast<FieldView*>(GetView());
}

const CalChartDoc* FieldFrame::GetShow() const
{
    return static_cast<const CalChartDoc*>(GetDocument());
}

CalChartDoc* FieldFrame::GetShow()
{
    return static_cast<CalChartDoc*>(GetDocument());
}
