/*
 * homeview_frame.cpp
 * Frame for the viewer only version
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

#include "homeview_frame.h"

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
#include "show_ui.h"
#include "animation_frame.h"
#include "toolbar.h"
#include "ui_enums.h"
#include "field_view.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#ifdef __WXMSW__
#include <wx/helpwin.h>
#endif
#include <wx/cmdproc.h>

const wxString gridtext[] =
{
	wxT("None"),
	wxT("1"),
	wxT("2"),
	wxT("4"),
	wxT("Mil"),
	wxT("2-Mil"),
};


const int zoom_amounts[] =
{
	500, 200, 150, 100, 75, 50, 25, 10,
};


static const wxChar *file_wild = FILE_WILDCARDS;

struct GridValue
{
	Coord num, sub;
};

GridValue gridvalue[] =
{
	{1,0},
	{Int2Coord(1),0},
	{Int2Coord(2),0},
	{Int2Coord(4),0},
	{Int2Coord(4),Int2Coord(4)/3},
	{Int2Coord(8),Int2Coord(8)/3}
};

extern wxPrintDialogData *gPrintDialogData;

BEGIN_EVENT_TABLE(HomeViewFrame, wxDocMDIChildFrame)
EVT_MENU(CALCHART__wxID_PRINT, HomeViewFrame::OnCmdPrint)
EVT_MENU(CALCHART__wxID_PREVIEW, HomeViewFrame::OnCmdPrintPreview)
EVT_MENU(wxID_PAGE_SETUP, HomeViewFrame::OnCmdPageSetup)
EVT_MENU(CALCHART__LEGACY_PRINT, HomeViewFrame::OnCmdLegacyPrint)
EVT_MENU(CALCHART__LEGACY_PRINT_EPS, HomeViewFrame::OnCmdLegacyPrintEPS)
EVT_MENU(CALCHART__POINTS, HomeViewFrame::OnCmdPoints)
EVT_MENU(CALCHART__ANIMATE, HomeViewFrame::OnCmdAnimate)
EVT_MENU(CALCHART__OMNIVIEW, HomeViewFrame::OnCmdOmniView)
EVT_MENU(wxID_ABOUT, HomeViewFrame::OnCmdAbout)
EVT_MENU(wxID_HELP, HomeViewFrame::OnCmdHelp)
EVT_MENU(wxID_PREFERENCES, HomeViewFrame::OnCmdPreferences)
END_EVENT_TABLE()

class MyPrintout : public wxPrintout
{
public:
	MyPrintout(const wxString& title, const CC_show& show) : wxPrintout(title), mShow(show) {}
	virtual ~MyPrintout() {}
	virtual bool HasPage(int pageNum) { return pageNum <= mShow.GetNumSheets(); }
	virtual void GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo)
	{
		*minPage = 1;
		*maxPage = mShow.GetNumSheets();
		*pageFrom = 1;
		*pageTo = mShow.GetNumSheets();
	}
	virtual bool OnPrintPage(int pageNum)
	{
		wxDC* dc = wxPrintout::GetDC();
		CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(pageNum-1);

		int size = gPrintDialogData->GetPrintData().GetOrientation();

		DrawForPrinting(dc, *sheet, 0, 2 == size);

		return true;
	}
	const CC_show& mShow;
};


// Main frame constructor
HomeViewFrame::HomeViewFrame(wxDocument* doc, wxView* view, wxDocMDIParentFrame *frame, const wxPoint& pos, const wxSize& size):
wxDocMDIChildFrame(doc, view, frame, -1, wxT("CalChart"), pos, size)
{
// Give it an icon
	SetBandIcon(this);

// Give it a status line
	CreateStatusBar(3);
	SetStatusText(wxT("Welcome to Calchart " wxT( CC_VERSION )));

// Make a menubar
	wxMenu *file_menu = new wxMenu;
	file_menu->Append(wxID_NEW, wxT("&New Show\tCTRL-N"), wxT("Create a new show"));
	file_menu->Append(wxID_OPEN, wxT("&Open...\tCTRL-O"), wxT("Load a saved show"));
	file_menu->Append(CALCHART__APPEND_FILE, wxT("&Append..."), wxT("Append a show to the end"));
	file_menu->Append(CALCHART__IMPORT_CONT_FILE, wxT("&Import Continuity...\tCTRL-I"), wxT("Import continuity text"));
	file_menu->Append(wxID_SAVE, wxT("&Save\tCTRL-S"), wxT("Save show"));
	file_menu->Append(wxID_SAVEAS, wxT("Save &As...\tCTRL-SHIFT-S"), wxT("Save show as a new name"));
	file_menu->Append(CALCHART__wxID_PRINT, wxT("&Print...\tCTRL-P"), wxT("Print this show"));
	file_menu->Append(CALCHART__wxID_PREVIEW, wxT("Preview...\tCTRL-SHIFT-P"), wxT("Preview this show"));
	file_menu->Append(wxID_PAGE_SETUP, wxT("Page Setup...\tCTRL-SHIFT-ALT-P"), wxT("Setup Pages"));
	file_menu->Append(CALCHART__LEGACY_PRINT, wxT("Print to PS..."), wxT("Print show to PostScript"));
	file_menu->Append(CALCHART__LEGACY_PRINT_EPS, wxT("Print to EPS..."), wxT("Print show to Encapsulated PostScript"));
	file_menu->Append(wxID_PREFERENCES, wxT("&Preferences\tCTRL-,"));
	file_menu->Append(wxID_CLOSE, wxT("&Close Window\tCTRL-W"), wxT("Close this window"));
	file_menu->Append(wxID_EXIT, wxT("&Quit\tCTRL-Q"), wxT("Quit CalChart"));

	// A nice touch: a history of files visited. Use this menu.
	// causes a crash :(
	//view->GetDocumentManager()->FileHistoryUseMenu(file_menu);

	wxMenu *edit_menu = new wxMenu;
	edit_menu->Append(wxID_UNDO, wxT("&Undo\tCTRL-Z"));
	edit_menu->Append(wxID_REDO, wxT("&Redo\tCTRL-SHIFT-Z"));
	edit_menu->Append(CALCHART__INSERT_BEFORE, wxT("&Insert Sheet Before\tCTRL-["), wxT("Insert a new stuntsheet before this one"));
	edit_menu->Append(CALCHART__INSERT_AFTER, wxT("Insert Sheet &After\tCTRL-]"), wxT("Insert a new stuntsheet after this one"));
	edit_menu->Append(wxID_DELETE, wxT("&Delete Sheet\tCTRL-DEL"), wxT("Delete this stuntsheet"));
	edit_menu->Append(CALCHART__RELABEL, wxT("&Relabel Sheets\tCTRL-R"), wxT("Relabel all stuntsheets after this one"));
	edit_menu->Append(CALCHART__SETUP, wxT("Set &Up Marchers...\tCTRL-U"), wxT("Setup number of marchers"));
	edit_menu->Append(CALCHART__SETDESCRIPTION, wxT("Set Show &Description..."), wxT("Set the show description"));
	edit_menu->Append(CALCHART__SETMODE, wxT("Set Show &Mode..."), wxT("Set the show mode"));
	edit_menu->Append(CALCHART__POINTS, wxT("&Point Selections..."), wxT("Select Points"));
	edit_menu->Append(CALCHART__SET_SHEET_TITLE, wxT("Set Sheet &Title...\tCTRL-T"), wxT("Change the title of this stuntsheet"));
	edit_menu->Append(CALCHART__SET_BEATS, wxT("Set &Beats...\tCTRL-B"), wxT("Change the number of beats for this stuntsheet"));
	edit_menu->Append(CALCHART__ResetReferencePoint, wxT("Reset reference point..."), wxT("Reset the current reference point"));

	wxMenu *anim_menu = new wxMenu;
	anim_menu->Append(CALCHART__EDIT_CONTINUITY, wxT("&Edit Continuity...\tCTRL-E"), wxT("Edit continuity for this stuntsheet"));
	anim_menu->Append(CALCHART__ANIMATE, wxT("&Animate...\tCTRL-RETURN"), wxT("Open animation window"));
	anim_menu->Append(CALCHART__OMNIVIEW, wxT("&OmniView..."), wxT("Open CalChart Omniviewer"));

	wxMenu *backgroundimage_menu = new wxMenu;
	backgroundimage_menu->Append(CALCHART__AddBackgroundImage, wxT("Add Background Image..."), wxT("Add a background image"));
	backgroundimage_menu->Append(CALCHART__AdjustBackgroundImage, wxT("Adjust Background Image..."), wxT("Adjust a background image"));
	backgroundimage_menu->Append(CALCHART__RemoveBackgroundImage, wxT("Remove Background Image..."), wxT("Remove a background image"));
	backgroundimage_menu->Enable(CALCHART__AddBackgroundImage, true);
	backgroundimage_menu->Enable(CALCHART__AdjustBackgroundImage, false);
	backgroundimage_menu->Enable(CALCHART__RemoveBackgroundImage, false);
	
	wxMenu *help_menu = new wxMenu;
	help_menu->Append(wxID_ABOUT, wxT("&About CalChart...\tCTRL-A"), wxT("Information about the program"));
	help_menu->Append(wxID_HELP, wxT("&Help on CalChart...\tCTRL-H"), wxT("Help on using CalChart"));

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, wxT("&File"));
	menu_bar->Append(edit_menu, wxT("&Edit"));
	menu_bar->Append(anim_menu, wxT("&Animation"));
	menu_bar->Append(backgroundimage_menu, wxT("&Field Image"));
	menu_bar->Append(help_menu, wxT("&Help"));
	SetMenuBar(menu_bar);

// Add a toolbar
	AddCoolToolBar(GetMainToolBar(), *this);

	CC_show* show = static_cast<CC_show*>(doc);
	SetTitle(show->GetTitle());

// Add the controls
	wxBoxSizer* fullsizer = new wxBoxSizer(wxVERTICAL);

// Update the command processor with the undo/redo menu items
	edit_menu->FindItem(wxID_UNDO)->Enable(false);
	edit_menu->FindItem(wxID_REDO)->Enable(false);
	doc->GetCommandProcessor()->SetEditMenu(edit_menu);
	doc->GetCommandProcessor()->Initialize();
	
// Show the frame
	SetSizer(fullsizer);
	// re-set the size
	SetSize(size);
	Show(true);
}


HomeViewFrame::~HomeViewFrame()
{}


// the default wxView print doesn't handle landscape.  rolling our own
void HomeViewFrame::OnCmdPrint(wxCommandEvent& event)
{
	// grab our current page setup.
	wxPrinter printer(gPrintDialogData);
	MyPrintout printout(wxT("My Printout"), *static_cast<CC_show*>(GetDocument()));
	wxPrintDialogData& printDialog = printer.GetPrintDialogData();

	int minPage, maxPage, pageFrom, pageTo;
	printout.GetPageInfo(&minPage, &maxPage, &pageFrom, &pageTo);
	printDialog.SetMinPage(minPage);
	printDialog.SetMaxPage(maxPage);

	if (!printer.Print(this, &printout, true))
	{
		if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
		{
			wxMessageBox(wxT("A problem was encountered when trying to print"), wxT("Printing"), wxOK);
		}
		else
		{
			wxMessageBox(wxT("Printing cancelled"), wxT("Printing"), wxOK);
		}
	}
	else
	{
		*gPrintDialogData = printer.GetPrintDialogData();
	}
}

// the default wxView print doesn't handle landscape.  rolling our own
void HomeViewFrame::OnCmdPrintPreview(wxCommandEvent& event)
{
	// grab our current page setup.
	wxPrintPreview *preview = new wxPrintPreview(
		new MyPrintout(wxT("My Printout"), *static_cast<CC_show*>(GetDocument())),
		new MyPrintout(wxT("My Printout"), *static_cast<CC_show*>(GetDocument())),
		gPrintDialogData);
	if (!preview->Ok())
	{
		delete preview;
		wxMessageBox(wxT("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), wxT("Previewing"), wxOK);
		return;
	}
	wxPreviewFrame *frame = new wxPreviewFrame(preview, this, wxT("Show Print Preview"));
	frame->Centre(wxBOTH);
	frame->Initialize();
	frame->Show(true);
}

void HomeViewFrame::OnCmdPageSetup(wxCommandEvent& event)
{
	wxPageSetupData mPageSetupData;
	mPageSetupData.EnableOrientation(true);

	wxPageSetupDialog pageSetupDialog(this, &mPageSetupData);
	if (pageSetupDialog.ShowModal() == wxID_OK)
		mPageSetupData = pageSetupDialog.GetPageSetupData();
	// pass the print data to our global print dialog
	gPrintDialogData->SetPrintData(mPageSetupData.GetPrintData());
}

void HomeViewFrame::OnCmdLegacyPrint(wxCommandEvent& event)
{
	PrintPostScriptDialog dialog(static_cast<CC_show*>(GetDocument()), false, this);
	if (dialog.ShowModal() == wxID_OK)
	{
		dialog.PrintShow();
	}
}

void HomeViewFrame::OnCmdLegacyPrintEPS(wxCommandEvent& event)
{
	PrintPostScriptDialog dialog(static_cast<CC_show*>(GetDocument()), true, this);
	if (dialog.ShowModal() == wxID_OK)
	{
		dialog.PrintShow();
	}
}


void HomeViewFrame::OnCmdPreferences(wxCommandEvent& event)
{
	CalChartPreferences dialog1(this);
	if (dialog1.ShowModal() == wxID_OK)
	{
	}
}


void HomeViewFrame::OnCmdPoints(wxCommandEvent& event)
{
	PointPicker* pp = new PointPicker(static_cast<CC_show*>(GetDocument()), this);
	// make it modeless:
	pp->Show();
}


void HomeViewFrame::OnCmdAnimate(wxCommandEvent& event)
{
	(void)new AnimationFrame(this, static_cast<CC_show*>(GetDocument()));
}


void HomeViewFrame::OnCmdOmniView(wxCommandEvent& event)
{
	(void)new AnimationFrame(this, static_cast<CC_show*>(GetDocument()), true);
}


void HomeViewFrame::OnCmdAbout(wxCommandEvent& event)
{
	TopFrame::About();
}


void HomeViewFrame::OnCmdHelp(wxCommandEvent& event)
{
	TopFrame::Help();
}

