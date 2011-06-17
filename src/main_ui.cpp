/*
 * main_ui.cpp
 * Handle wxWindows interface
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include "calchartapp.h"
#include "main_ui.h"
#include "print_ui.h"
#include "show_ui.h"
#include "anim_ui.h"
#include "cont_ui.h"
#include "cc_preferences_ui.h"
#include "cc_command.h"
#include "modes.h"
#include "confgr.h"
#include "ccvers.h"
#include "cc_sheet.h"
#include "show.h"
#include "animate.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#ifdef __WXMSW__
#include <wx/helpwin.h>
#endif
#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/config.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/fs_zip.h>
#include <wx/wizard.h>
#include <wx/statline.h>

ToolBarEntry main_tb[] =
{
	{ wxITEM_NORMAL, NULL, wxT("Previous stuntsheet"), CALCHART__prev_ss },
	{ wxITEM_NORMAL, NULL, wxT("Next stuntsheet"), CALCHART__next_ss, true },
	{ wxITEM_RADIO, NULL, wxT("Select points with box"), CALCHART__box },
	{ wxITEM_RADIO, NULL, wxT("Select points with polygon"), CALCHART__poly },
	{ wxITEM_RADIO, NULL, wxT("Select points with lasso"), CALCHART__lasso, true },
	{ wxITEM_RADIO, NULL, wxT("Translate points"), CALCHART__move },
	{ wxITEM_RADIO, NULL, wxT("Move points into line"), CALCHART__line },
	{ wxITEM_RADIO, NULL, wxT("Rotate block"), CALCHART__rot },
	{ wxITEM_RADIO, NULL, wxT("Shear block"), CALCHART__shear },
	{ wxITEM_RADIO, NULL, wxT("Reflect block"), CALCHART__reflect },
	{ wxITEM_RADIO, NULL, wxT("Resize block"), CALCHART__size },
	{ wxITEM_RADIO, NULL, wxT("Genius move"), CALCHART__genius, true },
	{ wxITEM_NORMAL, NULL, wxT("Label on left"), CALCHART__label_left },
	{ wxITEM_NORMAL, NULL, wxT("Flip label"), CALCHART__label_flip },
	{ wxITEM_NORMAL, NULL, wxT("Label on right"), CALCHART__label_right, true },
	{ wxITEM_NORMAL, NULL, wxT("Change to plainmen"), CALCHART__setsym0 },
	{ wxITEM_NORMAL, NULL, wxT("Change to solidmen"), CALCHART__setsym1 },
	{ wxITEM_NORMAL, NULL, wxT("Change to backslash men"), CALCHART__setsym2 },
	{ wxITEM_NORMAL, NULL, wxT("Change to slash men"), CALCHART__setsym3 },
	{ wxITEM_NORMAL, NULL, wxT("Change to x men"), CALCHART__setsym4 },
	{ wxITEM_NORMAL, NULL, wxT("Change to solid backslash men"), CALCHART__setsym5 },
	{ wxITEM_NORMAL, NULL, wxT("Change to solid slash men"), CALCHART__setsym6 },
	{ wxITEM_NORMAL, NULL, wxT("Change to solid x men"), CALCHART__setsym7 }
};

extern ToolBarEntry anim_tb[];
extern ToolBarEntry printcont_tb[];

const wxString gridtext[] =
{
	wxT("None"),
	wxT("1"),
	wxT("2"),
	wxT("4"),
	wxT("Mil"),
	wxT("2-Mil"),
};

static const wxChar *file_wild = FILE_WILDCARDS;

struct GridValue
{
	Coord num, sub;
};

GridValue gridvalue[] =
{
	{1,0},
	{INT2COORD(1),0},
	{INT2COORD(2),0},
	{INT2COORD(4),0},
	{INT2COORD(4),INT2COORD(4)/3},
	{INT2COORD(8),INT2COORD(8)/3}
};

extern wxHtmlHelpController *gHelpController;

extern TopFrame *topframe;

extern wxPrintDialogData *gPrintDialogData;

IMPLEMENT_CLASS(TopFrame, wxDocMDIParentFrame)

BEGIN_EVENT_TABLE(TopFrame, wxDocMDIParentFrame)
EVT_MENU(wxID_ABOUT, TopFrame::OnCmdAbout)
EVT_MENU(wxID_HELP, TopFrame::OnCmdHelp)
EVT_MENU(wxID_PREFERENCES, TopFrame::OnCmdPreferences)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MainFrame, wxDocMDIChildFrame)
EVT_CHAR(MainFrame::OnChar)
EVT_MENU(CALCHART__APPEND_FILE, MainFrame::OnCmdAppend)
EVT_MENU(CALCHART__IMPORT_CONT_FILE, MainFrame::OnCmdImportCont)
EVT_MENU(CALCHART__wxID_PRINT, MainFrame::OnCmdPrint)
EVT_MENU(CALCHART__wxID_PREVIEW, MainFrame::OnCmdPrintPreview)
EVT_MENU(wxID_PAGE_SETUP, MainFrame::OnCmdPageSetup)
EVT_MENU(CALCHART__LEGACY_PRINT, MainFrame::OnCmdLegacyPrint)
EVT_MENU(CALCHART__LEGACY_PRINT_EPS, MainFrame::OnCmdLegacyPrintEPS)
EVT_MENU(CALCHART__INSERT_BEFORE, MainFrame::OnCmdInsertBefore)
EVT_MENU(CALCHART__INSERT_AFTER, MainFrame::OnCmdInsertAfter)
EVT_MENU(wxID_DELETE, MainFrame::OnCmdDelete)
EVT_MENU(CALCHART__RELABEL, MainFrame::OnCmdRelabel)
EVT_MENU(CALCHART__EDIT_CONTINUITY, MainFrame::OnCmdEditCont)
EVT_MENU(CALCHART__SET_SHEET_TITLE, MainFrame::OnCmdSetSheetTitle)
EVT_MENU(CALCHART__SET_BEATS, MainFrame::OnCmdSetBeats)
EVT_MENU(CALCHART__SETUP, MainFrame::OnCmdSetup)
EVT_MENU(CALCHART__SETDESCRIPTION, MainFrame::OnCmdSetDescription)
EVT_MENU(CALCHART__SETMODE, MainFrame::OnCmdSetMode)
EVT_MENU(CALCHART__POINTS, MainFrame::OnCmdPoints)
EVT_MENU(CALCHART__ANIMATE, MainFrame::OnCmdAnimate)
EVT_MENU(wxID_ABOUT, MainFrame::OnCmdAbout)
EVT_MENU(wxID_HELP, MainFrame::OnCmdHelp)
EVT_MENU(CALCHART__prev_ss, MainFrame::OnCmd_prev_ss)
EVT_MENU(CALCHART__next_ss, MainFrame::OnCmd_next_ss)
EVT_MENU(CALCHART__box, MainFrame::OnCmd_box)
EVT_MENU(CALCHART__poly, MainFrame::OnCmd_poly)
EVT_MENU(CALCHART__lasso, MainFrame::OnCmd_lasso)
EVT_MENU(CALCHART__move, MainFrame::OnCmd_move)
EVT_MENU(CALCHART__line, MainFrame::OnCmd_line)
EVT_MENU(CALCHART__rot, MainFrame::OnCmd_rot)
EVT_MENU(CALCHART__shear, MainFrame::OnCmd_shear)
EVT_MENU(CALCHART__reflect, MainFrame::OnCmd_reflect)
EVT_MENU(CALCHART__size, MainFrame::OnCmd_size)
EVT_MENU(CALCHART__genius, MainFrame::OnCmd_genius)
EVT_MENU(CALCHART__label_left, MainFrame::OnCmd_label_left)
EVT_MENU(CALCHART__label_right, MainFrame::OnCmd_label_right)
EVT_MENU(CALCHART__label_flip, MainFrame::OnCmd_label_flip)
EVT_MENU(CALCHART__setsym0, MainFrame::OnCmd_setsym0)
EVT_MENU(CALCHART__setsym1, MainFrame::OnCmd_setsym1)
EVT_MENU(CALCHART__setsym2, MainFrame::OnCmd_setsym2)
EVT_MENU(CALCHART__setsym3, MainFrame::OnCmd_setsym3)
EVT_MENU(CALCHART__setsym4, MainFrame::OnCmd_setsym4)
EVT_MENU(CALCHART__setsym5, MainFrame::OnCmd_setsym5)
EVT_MENU(CALCHART__setsym6, MainFrame::OnCmd_setsym6)
EVT_MENU(CALCHART__setsym7, MainFrame::OnCmd_setsym7)
EVT_MENU(wxID_PREFERENCES, MainFrame::OnCmdPreferences)
EVT_COMMAND_SCROLL(CALCHART__slider_zoom, MainFrame::slider_zoom_callback)
EVT_COMMAND_SCROLL(CALCHART__slider_sheet_callback, MainFrame::slider_sheet_callback)
EVT_CHOICE(CALCHART__refnum_callback, MainFrame::refnum_callback)
EVT_CHECKBOX(CALCHART__draw_paths, MainFrame::draw_paths)
EVT_SIZE( MainFrame::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(FieldCanvas, AutoScrollCanvas)
EVT_CHAR(FieldCanvas::OnChar)
EVT_MOUSE_EVENTS(FieldCanvas::OnMouseEvent)
EVT_PAINT(FieldCanvas::OnPaint)
EVT_ERASE_BACKGROUND(FieldCanvas::OnErase)
END_EVENT_TABLE()

class MyPrintout : public wxPrintout
{
public:
	MyPrintout(const wxString& title, CC_show* shw) : wxPrintout(title), show(shw) {}
	virtual ~MyPrintout() {}
	virtual bool HasPage(int pageNum) { return pageNum <= show->GetNumSheets(); }
	virtual void GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo)
	{
		*minPage = 1;
		*maxPage = show->GetNumSheets();
		*pageFrom = 1;
		*pageTo = show->GetNumSheets();
	}
	virtual bool OnPrintPage(int pageNum)
	{
		wxDC* dc = wxPrintout::GetDC();
		CC_show::const_CC_sheet_iterator_t sheet = show->GetNthSheet(pageNum-1);

		int size = gPrintDialogData->GetPrintData().GetOrientation();

		DrawForPrinting(dc, *sheet, 0, 2 == size);

		return true;
	}
	CC_show *show;
};

TopFrame::TopFrame(wxDocManager *manager, wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name):
wxDocMDIParentFrame(manager, frame, wxID_ANY, title, pos, size, style, name)
{
// Give it an icon
	SetBandIcon(this);

	wxMenu *file_menu = new wxMenu;
	file_menu->Append(wxID_NEW, wxT("&New Show\tCTRL-N"), wxT("Create a new show"));
	file_menu->Append(wxID_OPEN, wxT("&Open...\tCTRL-O"), wxT("Load a saved show"));
	file_menu->Append(wxID_PREFERENCES, wxT("&Preferences\tCTRL-,"));
	file_menu->Append(wxID_EXIT, wxT("&Quit\tCTRL-Q"), wxT("Quit CalChart"));

	// A nice touch: a history of files visited. Use this menu.
	manager->FileHistoryUseMenu(file_menu);

	wxMenu *help_menu = new wxMenu;
	help_menu->Append(wxID_ABOUT, wxT("&About CalChart...\tCTRL-A"), wxT("Information about the program"));
	// this comes up as a leak on Mac?
	help_menu->Append(wxID_HELP, wxT("&Help on CalChart...\tCTRL-H"), wxT("Help on using CalChart"));

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, wxT("&File"));
	menu_bar->Append(help_menu, wxT("&Help"));
	SetMenuBar(menu_bar);
	SetDropTarget(new TopFrameDropTarget(manager, this));
	Show(true);
}


TopFrame::~TopFrame()
{
}


void TopFrame::OnCmdAbout(wxCommandEvent& event)
{
	About();
}


void TopFrame::OnCmdHelp(wxCommandEvent& event)
{
	Help();
}

void TopFrame::About()
{
	(void)wxMessageBox(wxT("CalChart ") wxT(CC_VERSION) wxT("\nAuthor: Gurk Meeker\n")
		wxT("http://calchart.sourceforge.net\n")
		wxT("Copyright (c) 1994-2008 Garrick Meeker\n")
		wxT("\n")
		wxT("This program is free software: you can redistribute it and/or modify\n")
		wxT("it under the terms of the GNU General Public License as published by\n")
		wxT("the Free Software Foundation, either version 3 of the License, or\n")
		wxT("(at your option) any later version.\n")
		wxT("\n")
		wxT("This program is distributed in the hope that it will be useful,\n")
		wxT("but WITHOUT ANY WARRANTY; without even the implied warranty of\n")
		wxT("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n")
		wxT("GNU General Public License for more details.\n")
		wxT("\n")
		wxT("You should have received a copy of the GNU General Public License\n")
		wxT("along with this program.  If not, see <http://www.gnu.org/licenses/>.\n")
		wxT("\n")
		wxT("Compiled on ") __TDATE__ wxT(" at ") __TTIME__,
		wxT("About CalChart"));
}


void TopFrame::Help()
{
	gHelpController->LoadFile();
	gHelpController->DisplayContents();
}

void TopFrame::OnCmdPreferences(wxCommandEvent& event)
{
	CalChartPreferences dialog1(this);
	if (dialog1.ShowModal() == wxID_OK)
	{
	}
}

bool TopFrameDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
	for (size_t i = 0; i < filenames.Count(); i++)
	{
		mManager->CreateDocument(filenames[i], wxDOC_SILENT);
	}
	return true;
}


// Main frame constructor
MainFrame::MainFrame(wxDocument* doc, wxView* view, wxDocMDIParentFrame *frame, const wxPoint& pos, const wxSize& size):
wxDocMDIChildFrame(doc, view, frame, -1, wxT("CalChart"), pos, size),
field(NULL)
{
	unsigned ss;
	unsigned def_grid;

// Give it an icon
	SetBandIcon(this);

// Give it a status line
	CreateStatusBar(3);
	SetStatusText(wxT("Welcome to Calchart " CC_VERSION));

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

	wxMenu *anim_menu = new wxMenu;
	anim_menu->Append(CALCHART__EDIT_CONTINUITY, wxT("&Edit Continuity...\tCTRL-E"), wxT("Edit continuity for this stuntsheet"));
	anim_menu->Append(CALCHART__ANIMATE, wxT("&Animate...\tCTRL-RETURN"), wxT("Open animation window"));

	wxMenu *help_menu = new wxMenu;
	help_menu->Append(wxID_ABOUT, wxT("&About CalChart...\tCTRL-A"), wxT("Information about the program"));
	help_menu->Append(wxID_HELP, wxT("&Help on CalChart...\tCTRL-H"), wxT("Help on using CalChart"));

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, wxT("&File"));
	menu_bar->Append(edit_menu, wxT("&Edit"));
	menu_bar->Append(anim_menu, wxT("&Animation"));
	menu_bar->Append(help_menu, wxT("&Help"));
	SetMenuBar(menu_bar);

// Add a toolbar
	CreateCoolToolBar(main_tb, sizeof(main_tb)/sizeof(ToolBarEntry), this);

// Add the field canvas
	ss = 0;
	def_grid = 2;
	field = new FieldCanvas(view, ss, this, GetConfiguration_MainFrameZoom());

	CC_show* show = static_cast<CC_show*>(doc);
	SetTitle(show->GetTitle());

// Add the controls
	wxBoxSizer* fullsizer = new wxBoxSizer(wxVERTICAL);

// Grid choice
	grid_choice = new wxChoice(this, -1, wxPoint(-1, -1), wxSize(-1, -1),
		sizeof(gridtext)/sizeof(const wxChar*),
		gridtext);
	grid_choice->SetSelection(def_grid);

// Zoom slider
	// on Mac using wxSL_LABELS will cause a crash?
	zoom_slider = new wxSlider(this, CALCHART__slider_zoom, GetConfiguration_MainFrameZoom(), 1, FIELD_MAXZOOM, wxDefaultPosition,
                    wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS);

// set up a sizer for the field panel
	wxBoxSizer* rowsizer = new wxBoxSizer(wxHORIZONTAL);
	rowsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Grid")), 0, wxALL, 5);
	rowsizer->Add(grid_choice, 0, wxALL, 5);
	rowsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Zoom")), 0, wxALL, 5);
	rowsizer->Add(zoom_slider, 1, wxEXPAND, 5);
	fullsizer->Add(rowsizer);
// Reference choice
	{
		wxString buf;
		unsigned i;

		ref_choice = new wxChoice(this, CALCHART__refnum_callback);
		ref_choice->Append(wxT("Off"));
		for (i = 1; i <= CC_point::kNumRefPoints; i++)
		{
			buf.sprintf(wxT("%u"), i);
			ref_choice->Append(buf);
		}
	}

	wxCheckBox* checkbox = new wxCheckBox(this, CALCHART__draw_paths, wxT("Draw &Paths"));
	checkbox->SetValue(false);
	rowsizer->Add(checkbox, 1, wxEXPAND, 5);
// Sheet slider (will get set later with UpdatePanel())
	// on Mac using wxSL_LABELS will cause a crash?
	sheet_slider = new wxSlider(this, CALCHART__slider_sheet_callback, 1, 1, 2, wxDefaultPosition,
                    wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS);

	rowsizer = new wxBoxSizer(wxHORIZONTAL);
	rowsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Ref Group")), 0, wxALL, 5);
	rowsizer->Add(ref_choice, 0, wxALL, 5);
	rowsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Sheet")), 0, wxALL, 5);
	rowsizer->Add(sheet_slider, 1, wxEXPAND, 5);
	fullsizer->Add(rowsizer);

// Update the tool bar
	SetCurrentLasso(field->curr_lasso);
	SetCurrentMove(field->curr_move);

// Show the frame
	UpdatePanel();
	field->RefreshShow();

	fullsizer->Add(field, 1, wxEXPAND);
	SetSizer(fullsizer);
	// re-set the size
	SetSize(size);
	Show(true);
}


MainFrame::~MainFrame()
{}


// Intercept menu commands

void MainFrame::OnCmdAppend(wxCommandEvent& event)
{
	AppendShow();
}


void MainFrame::OnCmdImportCont(wxCommandEvent& event)
{
	ImportContFile();
}


// the default wxView print doesn't handle landscape.  rolling our own
void MainFrame::OnCmdPrint(wxCommandEvent& event)
{
	// grab our current page setup.
	wxPrinter printer(gPrintDialogData);
	MyPrintout printout(wxT("My Printout"), field->mShow);
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
void MainFrame::OnCmdPrintPreview(wxCommandEvent& event)
{
	// grab our current page setup.
	wxPrintPreview *preview = new wxPrintPreview(
		new MyPrintout(wxT("My Printout"), field->mShow),
		new MyPrintout(wxT("My Printout"), field->mShow),
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

void MainFrame::OnCmdPageSetup(wxCommandEvent& event)
{
	wxPageSetupData mPageSetupData;
	mPageSetupData.EnableOrientation(true);

	wxPageSetupDialog pageSetupDialog(this, &mPageSetupData);
	if (pageSetupDialog.ShowModal() == wxID_OK)
		mPageSetupData = pageSetupDialog.GetPageSetupData();
	// pass the print data to our global print dialog
	gPrintDialogData->SetPrintData(mPageSetupData.GetPrintData());
}

void MainFrame::OnCmdLegacyPrint(wxCommandEvent& event)
{
	if (field->mShow)
	{
		ShowPrintDialog dialog(field->mShow, false, this);
		if (dialog.ShowModal() == wxID_OK)
		{
			dialog.PrintShow();
		}
	}
}

void MainFrame::OnCmdLegacyPrintEPS(wxCommandEvent& event)
{
	if (field->mShow)
	{
		ShowPrintDialog dialog(field->mShow, true, this);
		if (dialog.ShowModal() == wxID_OK)
		{
			dialog.PrintShow();
		}
	}
}


void MainFrame::OnCmdPreferences(wxCommandEvent& event)
{
	CalChartPreferences dialog1(this);
	if (dialog1.ShowModal() == wxID_OK)
	{
	}
}


void MainFrame::OnCmdInsertBefore(wxCommandEvent& event)
{
	CC_show::CC_sheet_container_t sht(1, *field->mShow->GetCurrentSheet());
	static_cast<MainFrameView*>(GetView())->DoInsertSheets(sht, field->mShow->GetCurrentSheetNum());
	field->PrevSS();
}


void MainFrame::OnCmdInsertAfter(wxCommandEvent& event)
{
	CC_show::CC_sheet_container_t sht(1, *field->mShow->GetCurrentSheet());
	static_cast<MainFrameView*>(GetView())->DoInsertSheets(sht, field->mShow->GetCurrentSheetNum()+1);
	field->NextSS();
}


void MainFrame::OnCmdDelete(wxCommandEvent& event)
{
	if (field->mShow->GetNumSheets() > 1)
	{
		static_cast<MainFrameView*>(GetView())->DoDeleteSheet(field->mShow->GetCurrentSheetNum());
	}
}


void MainFrame::OnCmdRelabel(wxCommandEvent& event)
{
	if (field->mShow->GetCurrentSheetNum()+1 < field->mShow->GetNumSheets())
	{
		if(wxMessageBox(wxT("Relabeling sheets is not undoable.\nProceed?"),
			wxT("Relabel sheets"), wxYES_NO) == wxYES)
		{
			if (!field->mShow->RelabelSheets(field->mShow->GetCurrentSheetNum()))
				(void)wxMessageBox(wxT("Stuntsheets don't match"),
					wxT("Relabel sheets"));
			else
			{
				field->mShow->Modify(true);
			}
		}
	}
	else
	{
		(void)wxMessageBox(wxT("This can't used on the last stuntsheet"),
			wxT("Relabel sheets"));
	}
}


void MainFrame::OnCmdEditCont(wxCommandEvent& event)
{
	if (field->mShow)
	{
		ContinuityEditor* ce = new ContinuityEditor(field->mShow, this, wxID_ANY,
			wxT("Animation Continuity"));
		// make it modeless:
		ce->Show();
	}
}


void MainFrame::OnCmdSetSheetTitle(wxCommandEvent& event)
{
	wxString s;
	if (field->mShow)
	{
		s = wxGetTextFromUser(wxT("Enter the sheet title"),
			field->mShow->GetCurrentSheet()->GetName(),
			field->mShow->GetCurrentSheet()->GetName(),
			this);
		if (s)
		{
			static_cast<MainFrameView*>(GetView())->DoSetSheetTitle(s);
		}
	}
}


void MainFrame::OnCmdSetBeats(wxCommandEvent& event)
{
	wxString s;
	if (field->mShow)
	{
		wxString buf;
		buf.sprintf(wxT("%u"), field->mShow->GetCurrentSheet()->GetBeats());
		s = wxGetTextFromUser(wxT("Enter the number of beats"),
			field->mShow->GetCurrentSheet()->GetName(),
			buf, this);
		if (!s.empty())
		{
			long val;
			if (s.ToLong(&val))
			{
				static_cast<MainFrameView*>(GetView())->DoSetSheetBeats(val);
			}
		}
	}
}


void MainFrame::OnCmdSetup(wxCommandEvent& event)
{
	Setup();
}


void MainFrame::OnCmdSetDescription(wxCommandEvent& event)
{
	SetDescription();
}


void MainFrame::OnCmdSetMode(wxCommandEvent& event)
{
	SetMode();
}


void MainFrame::OnCmdPoints(wxCommandEvent& event)
{
	if (field->mShow)
	{
		PointPicker* pp = new PointPicker(field->mShow, this);
		// make it modeless:
		pp->Show();
	}
}


void MainFrame::OnCmdAnimate(wxCommandEvent& event)
{
	if (field->mShow)
	{
		AnimationFrame *anim =
			new AnimationFrame(this, field->mShow);
		anim->canvas->Generate();
	}
}


void MainFrame::OnCmdAbout(wxCommandEvent& event)
{
	topframe->About();
}


void MainFrame::OnCmdHelp(wxCommandEvent& event)
{
	topframe->Help();
}


void MainFrame::OnCmd_prev_ss(wxCommandEvent& event)
{
	field->PrevSS();
}


void MainFrame::OnCmd_next_ss(wxCommandEvent& event)
{
	field->NextSS();
}


void MainFrame::OnCmd_box(wxCommandEvent& event)
{
	SetCurrentLasso(CC_DRAG_BOX);
}


void MainFrame::OnCmd_poly(wxCommandEvent& event)
{
	SetCurrentLasso(CC_DRAG_POLY);
}


void MainFrame::OnCmd_lasso(wxCommandEvent& event)
{
	SetCurrentLasso(CC_DRAG_LASSO);
}


void MainFrame::OnCmd_move(wxCommandEvent& event)
{
	SetCurrentMove(CC_MOVE_NORMAL);
}


void MainFrame::OnCmd_line(wxCommandEvent& event)
{
	SetCurrentMove(CC_MOVE_LINE);
}


void MainFrame::OnCmd_rot(wxCommandEvent& event)
{
	SetCurrentMove(CC_MOVE_ROTATE);
}


void MainFrame::OnCmd_shear(wxCommandEvent& event)
{
	SetCurrentMove(CC_MOVE_SHEAR);
}


void MainFrame::OnCmd_reflect(wxCommandEvent& event)
{
	SetCurrentMove(CC_MOVE_REFL);
}


void MainFrame::OnCmd_size(wxCommandEvent& event)
{
	SetCurrentMove(CC_MOVE_SIZE);
}


void MainFrame::OnCmd_genius(wxCommandEvent& event)
{
	SetCurrentMove(CC_MOVE_GENIUS);
}


void MainFrame::OnCmd_label_left(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsLabel(false);
}


void MainFrame::OnCmd_label_right(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsLabel(true);
}


void MainFrame::OnCmd_label_flip(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsLabelFlip();
}


void MainFrame::OnCmd_setsym0(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsSymbol(SYMBOL_PLAIN);
}


void MainFrame::OnCmd_setsym1(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsSymbol(SYMBOL_SOL);
}


void MainFrame::OnCmd_setsym2(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsSymbol(SYMBOL_BKSL);
}


void MainFrame::OnCmd_setsym3(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsSymbol(SYMBOL_SL);
}


void MainFrame::OnCmd_setsym4(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsSymbol(SYMBOL_X);
}


void MainFrame::OnCmd_setsym5(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsSymbol(SYMBOL_SOLBKSL);
}


void MainFrame::OnCmd_setsym6(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsSymbol(SYMBOL_SOLSL);
}


void MainFrame::OnCmd_setsym7(wxCommandEvent& event)
{
	static_cast<MainFrameView*>(GetView())->DoSetPointsSymbol(SYMBOL_SOLX);
}


void MainFrame::OnChar(wxKeyEvent& event)
{
	field->OnChar(event);
}

void MainFrame::OnSize(wxSizeEvent& event)
{
	// HACK: Prevent width and height from growing out of control
	int w = event.GetSize().GetWidth();
	int h = event.GetSize().GetHeight();
	SetConfiguration_MainFrameWidth((w > 1200) ? 1200 : w);
	SetConfiguration_MainFrameHeight((h > 700) ? 700 : h);
	wxDocMDIChildFrame::OnSize(event);
}

// Append a show with file selector
void MainFrame::AppendShow()
{
	wxString s;
	CC_show *shw;
	unsigned currend;

	s = wxFileSelector(wxT("Append show"), NULL, NULL, NULL, file_wild);
	if (!s.empty())
	{
		shw = new CC_show();
		if (shw->OnOpenDocument(s))
		{
			if (shw->GetNumPoints() == field->mShow->GetNumPoints())
			{
				currend = field->mShow->GetNumSheets();
				static_cast<MainFrameView*>(GetView())->DoInsertSheets(CC_show::CC_sheet_container_t(shw->GetSheetBegin(), shw->GetSheetEnd()), currend);
				// This is bad, we are relabeling outside of adding sheets...
				if (!field->mShow->RelabelSheets(currend-1))
					(void)wxMessageBox(wxT("Stuntsheets don't match"),
						wxT("Append Error"));
			}
			else
			{
				(void)wxMessageBox(wxT("The blocksize doesn't match"), wxT("Append Error"));
			}
		}
		else
		{
			(void)wxMessageBox(wxT("Error Opening show"), wxT("Load Error"));
		}
		delete shw;
	}
}


// Append a show with file selector
void MainFrame::ImportContFile()
{
	wxString s;
	wxString err;

	s = wxFileSelector(wxT("Import Continuity"), NULL, NULL, NULL, wxT("*.txt"));
	if (!s.empty())
	{
		err = field->mShow->ImportContinuity(s);
		if (!err.empty())
		{
			(void)wxMessageBox(err, wxT("Load Error"));
			delete err;
		}
	}
}


static inline Coord SNAPGRID(Coord a, Coord n, Coord s)
{
	Coord a2 = (a+(n>>1)) & (~(n-1));
	Coord h = s>>1;
	if ((a-a2) >= h) return a2+s;
	else if ((a-a2) < -h) return a2-s;
	else return a2;
}


void MainFrame::SnapToGrid(CC_coord& c)
{
	Coord gridn, grids;
	int n = grid_choice->GetSelection();

	gridn = gridvalue[n].num;
	grids = gridvalue[n].sub;

	c.x = SNAPGRID(c.x, gridn, grids);
// Adjust so 4 step grid will be on visible grid
	c.y = SNAPGRID(c.y - INT2COORD(2), gridn, grids) + INT2COORD(2);
}


void MainFrame::SetCurrentLasso(CC_DRAG_TYPES type)
{
	field->curr_lasso = type;
}


void MainFrame::SetCurrentMove(CC_MOVE_MODES type)
{
	field->curr_move = type;
	field->EndDrag();
}


void MainFrame::Setup()
{
	if (field->mShow)
	{
		ShowInfoReq dialog(field->mShow, this);
		if (dialog.ShowModal() == wxID_OK)
		{
			static_cast<MainFrameView*>(GetView())->DoSetShowInfo(dialog.GetNumberPoints(), dialog.GetNumberColumns(), dialog.GetLabels());
		}
	}
}


void MainFrame::SetDescription()
{
	if (field->mShow)
	{
		wxTextEntryDialog dialog(this,
			wxT("Please modify the show description\n"),
			wxT("Edit show description\n"),
			field->mShow->GetDescr(),
			wxOK | wxCANCEL);
		if (dialog.ShowModal() == wxID_OK)
		{
			static_cast<MainFrameView*>(GetView())->DoSetDescription(dialog.GetValue());
		}
	}
}


void MainFrame::SetMode()
{
	if (field->mShow)
	{
		wxArrayString modeStrings;
		unsigned whichMode = 0, tmode = 0;
		for (ShowModeList::Iter mode = wxGetApp().GetModeList().Begin(); mode != wxGetApp().GetModeList().End(); ++mode, ++tmode)
		{
			modeStrings.Add((*mode)->GetName());
			if ((*mode)->GetName() == field->mShow->GetMode().GetName())
				whichMode = tmode;
		}
		wxSingleChoiceDialog dialog(this,
			wxT("Please select the show mode\n"),
			wxT("Set show mode\n"),
			modeStrings);
		dialog.SetSelection(whichMode);
		if (dialog.ShowModal() == wxID_OK)
		{
			static_cast<MainFrameView*>(GetView())->DoSetMode(dialog.GetStringSelection());
		}
	}
}


// Define a constructor for field canvas
FieldCanvas::FieldCanvas(wxView *view, unsigned ss, MainFrame *frame,
int def_zoom, const wxPoint& pos, const wxSize& size):
AutoScrollCanvas(frame, -1, pos, size), ourframe(frame), mShow(static_cast<CC_show*>(view->GetDocument())), mView(static_cast<MainFrameView*>(view)), curr_lasso(CC_DRAG_BOX),
curr_move(CC_MOVE_NORMAL),
mCurrentReferencePoint(0), mDrawPaths(false), drag(CC_DRAG_NONE), curr_shape(NULL), dragon(false)
{
	SetPalette(CalChartPalette);

	mShow->SetCurrentSheet(ss);

	SetZoomQuick(def_zoom);

	UpdateBars();
}


FieldCanvas::~FieldCanvas(void)
{
	ClearShapes();
}


void FieldCanvas::ClearShapes()
{
	for (ShapeList::iterator i=shape_list.begin();
		i != shape_list.end();
		++i)
	{
		delete *i;
	}
	shape_list.clear();
	curr_shape = NULL;
}


// Draw the current drag feedback
void FieldCanvas::DrawDrag(bool on)
{
	wxDC *dc = GetMemDC();
	CC_coord origin;

	if ((on != dragon) && curr_shape)
	{
		dragon = on;
		if (on)
		{
			dc->SetBrush(*wxTRANSPARENT_BRUSH);
			dc->SetPen(*CalChartPens[COLOR_SHAPES]);
			origin = mShow->GetMode().Offset();
			for (ShapeList::const_iterator i=shape_list.begin();
				i != shape_list.end();
				++i)
			{
				(*i)->Draw(dc, origin.x,
					origin.y);
			}
		}
		else
		{
			Blit(*dc);
		}
	}
}


// Define the repainting behaviour
void FieldCanvas::OnPaint(wxPaintEvent& event)
{
	wxBufferedPaintDC dc(this);
	dc.SetBackground(*CalChartBrushes[COLOR_FIELD]);
	dc.Clear();
	Blit(dc);
	dragon = false;								  // since the canvas gets cleared
	DrawDrag(true);
}

void FieldCanvas::OnErase(wxEraseEvent& event)
{
}


// Allow clicking within pixels to close polygons
#define CLOSE_ENOUGH_TO_CLOSE 10
void FieldCanvas::OnMouseEvent(wxMouseEvent& event)
{
	long x,y;
	int i;
	CC_coord pos;

	if (mShow)
	{
		CC_show::const_CC_sheet_iterator_t sheet = mShow->GetCurrentSheet();
		if (sheet != mShow->GetSheetEnd())
		{
			event.GetPosition(&x, &y);
			if (event.ControlDown())
			{
				Move(x, y);
				dragon = false;					  // since the canvas gets cleared
			}
			else
			{
				Move(x, y, 1);
			}

			pos = mShow->GetMode().Offset();
			pos.x = Coord(((x-x_off)/GetScaleX()) - pos.x);
			pos.y = Coord(((y-y_off)/GetScaleY()) - pos.y);

			if (event.LeftDown())
			{
				switch (curr_move)
				{
					case CC_MOVE_LINE:
						ourframe->SnapToGrid(pos);
						BeginDrag(CC_DRAG_LINE, pos);
						break;
					case CC_MOVE_ROTATE:
						ourframe->SnapToGrid(pos);
						if (curr_shape &&
							(((CC_shape_1point*)curr_shape)->GetOrigin() != pos))
						{
							AddDrag(CC_DRAG_LINE,
								new CC_shape_arc(((CC_shape_1point*)
								curr_shape)->GetOrigin(), pos));
						}
						else
						{
							BeginDrag(CC_DRAG_CROSS, pos);
						}
						break;
					case CC_MOVE_SHEAR:
						ourframe->SnapToGrid(pos);
						if (curr_shape &&
							(((CC_shape_1point*)curr_shape)->GetOrigin() != pos))
						{
							CC_coord vect(pos - ((CC_shape_1point*)curr_shape)->GetOrigin());
// rotate vect 90 degrees
							AddDrag(CC_DRAG_LINE,
								new CC_shape_angline(pos,CC_coord(-vect.y, vect.x)));
						}
						else
						{
							BeginDrag(CC_DRAG_CROSS, pos);
						}
						break;
					case CC_MOVE_REFL:
						ourframe->SnapToGrid(pos);
						BeginDrag(CC_DRAG_LINE, pos);
						break;
					case CC_MOVE_SIZE:
						ourframe->SnapToGrid(pos);
						if (curr_shape &&
							(((CC_shape_1point*)curr_shape)->GetOrigin() != pos))
						{
							AddDrag(CC_DRAG_LINE, new CC_shape_line(pos));
						}
						else
						{
							BeginDrag(CC_DRAG_CROSS, pos);
						}
						break;
					case CC_MOVE_GENIUS:
						ourframe->SnapToGrid(pos);
						AddDrag(CC_DRAG_LINE, new CC_shape_line(pos));
						break;
					default:
						switch (drag)
						{
							case CC_DRAG_POLY:
							{
								const wxPoint *p = ((CC_lasso*)curr_shape)->FirstPoint();
								float d;
								if (p != NULL)
								{
									Coord polydist =
										(Coord)GetMemDC()->DeviceToLogicalXRel(CLOSE_ENOUGH_TO_CLOSE);
									d = p->x - pos.x;
									if (ABS(d) < polydist)
									{
										d = p->y - pos.y;
										if (ABS(d) < polydist)
										{
											SelectWithLasso((CC_lasso*)curr_shape, event.AltDown());
											EndDrag();
											break;
										}
									}
								}
								((CC_lasso*)curr_shape)->Append(pos);
							}
							break;
							default:
								bool changed = false;
								if (!(event.ShiftDown() || event.AltDown())) changed = mShow->UnselectAll();
								i = sheet->FindPoint(pos.x, pos.y, mCurrentReferencePoint);
								if (i < 0)
								{
									// if no point selected, we grab using the current lasso
									BeginDrag(curr_lasso, pos);
								}
								else
								{
									CC_show::SelectionList select;
									select.insert(i);
									if (event.AltDown())
									{
										mShow->ToggleSelection(select);
									}
									else
									{
										mShow->AddToSelection(select);
									}

									changed = true;
									BeginDrag(CC_DRAG_LINE, sheet->GetPosition(i, mCurrentReferencePoint));
								}
						}
						break;
				}
			}
			else if (event.LeftUp() && curr_shape)
			{
				const CC_shape_2point *shape = (CC_shape_2point*)curr_shape;
				const CC_shape_1point *origin;
				switch (curr_move)
				{
					case CC_MOVE_LINE:
						mView->DoMovePointsInLine(shape->GetOrigin(), shape->GetPoint(), mCurrentReferencePoint);
						EndDrag();
						ourframe->SetCurrentMove(CC_MOVE_NORMAL);
						break;
					case CC_MOVE_ROTATE:
						if (shape_list.size() > 1)
						{
							origin = (CC_shape_1point*)shape_list[0];
							if (shape->GetOrigin() == shape->GetPoint())
							{
								BeginDrag(CC_DRAG_CROSS, pos);
							}
							else
							{
								Matrix m;
								CC_coord c1 = origin->GetOrigin();
								float r = -((CC_shape_arc*)curr_shape)->GetAngle();

								m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) *
									ZRotationMatrix(r) *
									TranslationMatrix(Vector(c1.x, c1.y, 0));
								mView->DoTransformPoints(m, mCurrentReferencePoint);
								EndDrag();
								ourframe->SetCurrentMove(CC_MOVE_NORMAL);
							}
						}
						break;
					case CC_MOVE_SHEAR:
						if (shape_list.size() > 1)
						{
							origin = (CC_shape_1point*)shape_list[0];
							if (shape->GetOrigin() == shape->GetPoint())
							{
								BeginDrag(CC_DRAG_CROSS, pos);
							}
							else
							{
								Matrix m;
								CC_coord o = origin->GetOrigin();
								CC_coord c1 = shape->GetOrigin();
								CC_coord c2 = shape->GetPoint();
								CC_coord v1, v2;
								float ang, amount;

								v1 = c1 - o;
								v2 = c2 - c1;
								amount = v2.Magnitude() / v1.Magnitude();
								if (BoundDirectionSigned(v1.Direction() -
									(c2-o).Direction()) < 0)
									amount = -amount;
								ang = -v1.Direction()*PI/180.0;
								m = TranslationMatrix(Vector(-o.x, -o.y, 0)) *
									ZRotationMatrix(-ang) *
									YXShearMatrix(amount) *
									ZRotationMatrix(ang) *
									TranslationMatrix(Vector(o.x, o.y, 0));
								mView->DoTransformPoints(m, mCurrentReferencePoint);
								EndDrag();
								ourframe->SetCurrentMove(CC_MOVE_NORMAL);
							}
						}
						break;
					case CC_MOVE_REFL:
						if (shape->GetOrigin() != shape->GetPoint())
						{
							Matrix m;
							CC_coord c1 = shape->GetOrigin();
							CC_coord c2;
							float ang;

							c2 = shape->GetPoint() - c1;
							ang = -c2.Direction()*PI/180.0;
							m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) *
								ZRotationMatrix(-ang) *
								YReflectionMatrix() *
								ZRotationMatrix(ang) *
								TranslationMatrix(Vector(c1.x, c1.y, 0));
							mView->DoTransformPoints(m, mCurrentReferencePoint);
						}
						EndDrag();
						ourframe->SetCurrentMove(CC_MOVE_NORMAL);
						break;
					case CC_MOVE_SIZE:
						if (shape_list.size() > 1)
						{
							origin = (CC_shape_1point*)shape_list[0];
							if (shape->GetOrigin() == shape->GetPoint())
							{
								BeginDrag(CC_DRAG_CROSS, pos);
							}
							else
							{
								Matrix m;
								CC_coord c1 = origin->GetOrigin();
								CC_coord c2;
								float sx, sy;

								c2 = shape->GetPoint() - c1;
								sx = c2.x;
								sy = c2.y;
								c2 = shape->GetOrigin() - c1;
								if ((c2.x != 0) || (c2.y != 0))
								{
									if (c2.x != 0)
									{
										sx /= c2.x;
									}
									else
									{
										sx = 1;
									}
									if (c2.y != 0)
									{
										sy /= c2.y;
									}
									else
									{
										sy = 1;
									}
									m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) *
										ScaleMatrix(Vector(sx, sy, 0)) *
										TranslationMatrix(Vector(c1.x, c1.y, 0));
									mView->DoTransformPoints(m, mCurrentReferencePoint);
								}
								EndDrag();
								ourframe->SetCurrentMove(CC_MOVE_NORMAL);
							}
						}
						break;
					case CC_MOVE_GENIUS:
						if (shape_list.size() >= 3)
						{
							CC_shape_2point* v1 = (CC_shape_2point*)shape_list[0];
							CC_shape_2point* v2 = (CC_shape_2point*)shape_list[1];
							CC_shape_2point* v3 = (CC_shape_2point*)shape_list[2];
							CC_coord s1, s2, s3;
							CC_coord e1, e2, e3;
							float d;
							Matrix m;

							s1 = v1->GetOrigin();
							e1 = v1->GetPoint();
							s2 = v2->GetOrigin();
							e2 = v2->GetPoint();
							s3 = v3->GetOrigin();
							e3 = v3->GetPoint();
							d = (float)s1.x*(float)s2.y - (float)s2.x*(float)s1.y +
								(float)s3.x*(float)s1.y - (float)s1.x*(float)s3.y +
								(float)s2.x*(float)s3.y - (float)s3.x*(float)s2.y;
							if (IS_ZERO(d))
							{
								(void)wxMessageBox(wxT("Invalid genius move definition"),
									wxT("Genius Move"));
							}
							else
							{
								Matrix A = Matrix(Vector(e1.x,e2.x,0,e3.x),
									Vector(e1.y,e2.y,0,e3.y),
									Vector(0,0,0,0),
									Vector(1,1,0,1));
								Matrix Binv = Matrix(Vector((float)s2.y-(float)s3.y,
									(float)s3.x-(float)s2.x, 0,
									(float)s2.x*(float)s3.y -
									(float)s3.x*(float)s2.y),
									Vector((float)s3.y-(float)s1.y,
									(float)s1.x-(float)s3.x, 0,
									(float)s3.x*(float)s1.y -
									(float)s1.x*(float)s3.y),
									Vector(0, 0, 0, 0),
									Vector((float)s1.y-(float)s2.y,
									(float)s2.x-(float)s1.x, 0,
									(float)s1.x*(float)s2.y -
									(float)s2.x*(float)s1.y));
								Binv /= d;
								m = Binv*A;
								mView->DoTransformPoints(m, mCurrentReferencePoint);
							}
							EndDrag();
							ourframe->SetCurrentMove(CC_MOVE_NORMAL);
						}
						break;
					default:
						switch (drag)
						{
							case CC_DRAG_BOX:
								SelectPointsInRect(shape->GetOrigin(), shape->GetPoint(),
									mCurrentReferencePoint, event.AltDown());
								EndDrag();
								break;
							case CC_DRAG_LINE:
								pos = shape->GetPoint() - shape->GetOrigin();
								mView->DoTranslatePoints(pos, mCurrentReferencePoint);
								EndDrag();
								break;
							case CC_DRAG_LASSO:
								((CC_lasso*)curr_shape)->End();
								SelectWithLasso((CC_lasso*)curr_shape, event.AltDown());
								EndDrag();
								break;
							default:
								break;
						}
						break;
				}
			}
			else if (event.RightDown() && curr_shape)
			{
				switch (drag)
				{
					case CC_DRAG_POLY:
						SelectWithLasso((CC_lasso*)curr_shape, event.AltDown());
						EndDrag();
						break;
					default:
						break;
				}
			}
			else if (event.Dragging() && event.LeftIsDown() && curr_shape)
			{
				MoveDrag(pos);
			}
			else if (event.Moving() && curr_shape)
			{
				switch (drag)
				{
					case CC_DRAG_POLY:
						MoveDrag(pos);
						break;
					default:
						break;
				}
			}
			RefreshShow();
		}
	}
	Refresh();
}


void FieldCanvas::OnScroll(wxScrollEvent& event)
{
	event.Skip();
}


// Intercept character input
void FieldCanvas::OnChar(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_LEFT)
		PrevSS();
	else if (event.GetKeyCode() == WXK_RIGHT)
		NextSS();
	else
		event.Skip();
}

void FieldCanvas::GeneratePaths()
{
	mAnimation.reset(new Animation(mShow, NotifyStatus(), NotifyErrorList()));
}

void FieldCanvas::DrawPaths(wxDC& dc, const CC_sheet& sheet)
{
	if (mAnimation && mAnimation->GetNumberSheets() && (static_cast<unsigned>(mAnimation->GetNumberSheets()) > mShow->GetCurrentSheetNum()))
	{
		CC_coord origin = mShow->GetMode().Offset();
		mAnimation->GotoSheet(mShow->GetCurrentSheetNum());
		for (CC_show::SelectionList::const_iterator point = mShow->GetSelectionList().begin(); point != mShow->GetSelectionList().end(); ++point)
		{
			mAnimation->DrawPath(dc, *point, origin);
		}
	}
}

void FieldCanvas::RefreshShow(bool drawall, int point)
{
	if (mShow)
	{
		CC_show::const_CC_sheet_iterator_t sheet = mShow->GetCurrentSheet();
		if (sheet != mShow->GetSheetEnd())
		{
			if (mCurrentReferencePoint > 0)
			{
				Draw(GetMemDC(), *sheet, 0, false, drawall, point);
				Draw(GetMemDC(), *sheet, mCurrentReferencePoint, true, false, point);
			}
			else
			{
				Draw(GetMemDC(), *sheet, mCurrentReferencePoint, true, drawall, point);
			}
			if (mDrawPaths)
			{
				DrawPaths(*GetMemDC(), *sheet);
			}
			dragon = false;						  // since the canvas gets cleared
			DrawDrag(true);
		}
	}
	Refresh();
}


void MainFrame::UpdatePanel()
{
	wxString tempbuf;
	CC_show::const_CC_sheet_iterator_t sht = field->mShow->GetCurrentSheet();
	unsigned num = field->mShow->GetNumSheets();
	unsigned curr = field->mShow->GetCurrentSheetNum()+1;

	tempbuf.sprintf(wxT("%s%d of %d \"%.32s\" %d beats"),
		field->mShow->IsModified() ? wxT("* "):wxT(""), curr,
		num, (sht != field->mShow->GetSheetEnd())?sht->GetName().c_str():wxT(""), (sht != field->mShow->GetSheetEnd())?sht->GetBeats():0);
	SetStatusText(tempbuf, 1);
	tempbuf.sprintf(wxT("%d of %d selected"),
		field->mShow->GetSelectionList().size(), field->mShow->GetNumPoints());
	SetStatusText(tempbuf, 2);

	if (num > 1)
	{
		sheet_slider->Enable(true);
		if ((unsigned)sheet_slider->GetMax() != num)
			sheet_slider->SetValue(1);			  // So Motif doesn't complain about value
		sheet_slider->SetRange(1, num);
		if ((unsigned)sheet_slider->GetValue() != curr)
			sheet_slider->SetValue(curr);
	}
	else
	{
		sheet_slider->Enable(false);
	}
}


void FieldCanvas::UpdateBars()
{
	if (mShow)
	{
		SetSize(wxSize(COORD2INT(mShow->GetMode().Size().x) * zoomf,
			COORD2INT(mShow->GetMode().Size().y) * zoomf));
	}
}


void FieldCanvas::BeginDrag(CC_DRAG_TYPES type, CC_coord start)
{
	DrawDrag(false);
	drag = type;
	ClearShapes();
	curr_shape = NULL;
	switch (type)
	{
		case CC_DRAG_BOX:
			AddDrag(type, new CC_shape_rect(start));
			break;
		case CC_DRAG_POLY:
			AddDrag(type, new CC_poly(start));
			break;
		case CC_DRAG_LASSO:
			AddDrag(type, new CC_lasso(start));
			break;
		case CC_DRAG_LINE:
			AddDrag(type, new CC_shape_line(start));
			break;
		case CC_DRAG_CROSS:
			AddDrag(type, new CC_shape_cross(start, INT2COORD(2)));
		default:
			break;
	}
}


void FieldCanvas::BeginDrag(CC_DRAG_TYPES type, CC_shape *shape)
{
	DrawDrag(false);
	drag = type;
	ClearShapes();
	curr_shape = NULL;
	AddDrag(type, shape);
}


void FieldCanvas::AddDrag(CC_DRAG_TYPES type, CC_shape *shape)
{
	DrawDrag(false);
	drag = type;
	shape_list.push_back(shape);
	curr_shape = shape;
	DrawDrag(true);
}


void FieldCanvas::MoveDrag(CC_coord end)
{
	if (curr_shape)
	{
		DrawDrag(false);
		curr_shape->OnMove(end, ourframe);
		DrawDrag(true);
	}
}


void FieldCanvas::EndDrag()
{
	DrawDrag(false);
	ClearShapes();
	drag = CC_DRAG_NONE;
}


// toggle selection means toggle it as selected to unselected
// otherwise, always select it

void FieldCanvas::SelectOrdered(PointList& pointlist,
const CC_coord& start, bool toggleSelected)
{
	if (toggleSelected)
	{
		mShow->ToggleSelection(CC_show::SelectionList(pointlist.begin(), pointlist.end()));
	}
	else
	{
		mShow->AddToSelection(CC_show::SelectionList(pointlist.begin(), pointlist.end()));
	}
}


bool FieldCanvas::SelectWithLasso(const CC_lasso* lasso, bool toggleSelected)
{
	bool changed = false;
	CC_show::const_CC_sheet_iterator_t sheet = mShow->GetCurrentSheet();
	PointList pointlist;
	const wxPoint *pnt;

	for (unsigned i = 0; i < mShow->GetNumPoints(); i++)
	{
		if (lasso->Inside(sheet->GetPosition(i, mCurrentReferencePoint)))
		{
			changed = true;
			pointlist.push_back(i);
		}
	}
	pnt = lasso->FirstPoint();
	if (changed && pnt)
	{
		SelectOrdered(pointlist, CC_coord((Coord)pnt->x, (Coord)pnt->y), toggleSelected);
	}

	return changed;
}


// Select points within rectangle
bool FieldCanvas::SelectPointsInRect(const CC_coord& c1, const CC_coord& c2,
unsigned ref, bool toggleSelected)
{
	unsigned i;
	bool changed = false;
	CC_show::const_CC_sheet_iterator_t sheet = mShow->GetCurrentSheet();
	CC_coord top_left, bottom_right;
	const CC_coord *pos;
	PointList pointlist;

	if (c1.x > c2.x)
	{
		top_left.x = c2.x;
		bottom_right.x = c1.x;
	}
	else
	{
		top_left.x = c1.x;
		bottom_right.x = c2.x;
	}
	if (c1.y > c2.y)
	{
		top_left.y = c2.y;
		bottom_right.y = c1.y;
	}
	else
	{
		top_left.y = c1.y;
		bottom_right.y = c2.y;
	}
	for (i = 0; i < mShow->GetNumPoints(); i++)
	{
		pos = &sheet->GetPosition(i, ref);
		if ((pos->x >= top_left.x) && (pos->x <= bottom_right.x) &&
			(pos->y >= top_left.y) && (pos->y <= bottom_right.y))
		{
			pointlist.push_back(i);
			changed = true;
		}
	}
	if (changed)
	{
		SelectOrdered(pointlist, c1, toggleSelected);
	}

	return changed;
}


void MainFrame::refnum_callback(wxCommandEvent &)
{
	field->mCurrentReferencePoint = ref_choice->GetSelection();
	field->RefreshShow();
}

void MainFrame::draw_paths(wxCommandEvent &event)
{
	field->mDrawPaths = event.IsChecked();
	field->RefreshShow();
}


void MainFrame::slider_sheet_callback(wxScrollEvent &)
{
	field->GotoSS(sheet_slider->GetValue()-1);
}


void MainFrame::slider_zoom_callback(wxScrollEvent &)
{
	SetConfiguration_MainFrameZoom(zoom_slider->GetValue());
	field->SetZoom(zoom_slider->GetValue());
}

IMPLEMENT_DYNAMIC_CLASS(MainFrameView, wxView)

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool MainFrameView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
	mShow = static_cast<CC_show*>(doc);
	mFrame = new MainFrame(doc, this, GetMainFrame(), wxPoint(50, 50),
		wxSize(GetConfiguration_MainFrameWidth(), GetConfiguration_MainFrameHeight()));

	mFrame->Show(true);
	Activate(true);
	return true;
}

// Sneakily gets used for default print/preview
// as well as drawing on the screen.
void MainFrameView::OnDraw(wxDC *dc)
{
}


// page for deciding the field type
class ChooseShowModeWizard : public wxWizardPageSimple
{
public:
	ChooseShowModeWizard(wxWizard *parent) : wxWizardPageSimple(parent)
	{
		for (ShowModeList::Iter mode = wxGetApp().GetModeList().Begin(); mode != wxGetApp().GetModeList().End(); ++mode)
		{
			modeStrings.Add((*mode)->GetName());
		}

		wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
		SetSizer( topsizer );
		wxStaticText* label = new wxStaticText(this, wxID_STATIC, 
			wxT("Choose a field to set your show:"), wxDefaultPosition, wxDefaultSize, 0);
		topsizer->Add(label, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		mChoice = new wxChoice(this, wxID_ANY, wxPoint(5,5), wxDefaultSize, modeStrings);
		topsizer->Add(mChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		topsizer->Fit(this);
	}
	wxString GetValue()
	{
		return modeStrings[mChoice->GetSelection()];
	}

private:
	wxArrayString modeStrings;
	wxChoice *mChoice;
};

// page for giving a description
class SetDescriptionWizard : public wxWizardPageSimple
{
public:
	SetDescriptionWizard(wxWizard *parent) : wxWizardPageSimple(parent)
	{
		wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
		SetSizer( topsizer );
		wxStaticText* label = new wxStaticText(this, wxID_STATIC, 
			wxT("Enter a show description for your show:"), wxDefaultPosition, wxDefaultSize, 0);
		topsizer->Add(label, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		mText = new FancyTextWin(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(240, 100));
		topsizer->Add(mText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		topsizer->Fit(this);
	}
	wxString GetValue()
	{
		return mText->GetValue();
	}

private:
	wxArrayString modeStrings;
	FancyTextWin *mText;
};

void MainFrameView::OnWizardSetup(CC_show& show)
{
	wxWizard *wizard = new wxWizard(mFrame, wxID_ANY, wxT("New Show Setup Wizard"));
	// page 1:
	// set the number of points and the labels
	ShowInfoReqWizard *page1 = new ShowInfoReqWizard(wizard);

	// page 2:
	// choose the show mode
	ChooseShowModeWizard *page2 = new ChooseShowModeWizard(wizard);

	// page 3:
	// and maybe a description
	SetDescriptionWizard *page3 = new SetDescriptionWizard(wizard);
	// page 4:

	wxWizardPageSimple::Chain(page1, page2);
	wxWizardPageSimple::Chain(page2, page3);

	wizard->GetPageAreaSizer()->Add(page1);
	if (wizard->RunWizard(page1))
	{
		show.SetNumPoints(page1->GetNumberPoints(), page1->GetNumberColumns());
		show.SetPointLabel(page1->GetLabels());
		ShowMode *newmode = wxGetApp().GetModeList().Find(page2->GetValue());
		if (newmode)
		{
			show.SetMode(newmode);
		}
		show.SetDescr(page3->GetValue());
	}
	else
	{
		wxMessageBox(
			wxT("Show setup not completed.\n")
			wxT("You can change the number of marchers\n")
			wxT("and show mode via the menu options"), wxT("Show not setup"), wxICON_INFORMATION|wxOK);
	}
	wizard->Destroy();
}

void MainFrameView::OnUpdate(wxView *WXUNUSED(sender), wxObject *hint)
{
	if (hint && hint->IsKindOf(CLASSINFO(CC_show_setup)))
	{
		// give our show a first page
		CC_show* show = static_cast<CC_show*>(GetDocument());
		show->InsertSheetInternal(CC_sheet(show, wxT("1")), 0);
		show->SetCurrentSheet(0);

		// Set up everything else
		OnWizardSetup(*show);

		// make the show modified so it gets repainted
		show->Modify(true);
	}
	else
	{
		if (mFrame)
		{
			mFrame->UpdatePanel();
			wxString buf;
			GetDocument()->GetPrintableName(buf);
			mFrame->SetTitle(buf);
		}
		if (mFrame && mFrame->GetCanvas())
		{
			if (hint && hint->IsKindOf(CLASSINFO(CC_show_modified)))
			{
				mFrame->GetCanvas()->GeneratePaths();
			}
			mFrame->GetCanvas()->RefreshShow();
		}
	}
}

// Clean up windows used for displaying the view.
bool MainFrameView::OnClose(bool deleteWindow)
{
	if (!GetDocument()->Close())
		return false;

	wxString s(wxTheApp->GetAppName());
	if (mFrame)
		mFrame->SetTitle(s);

	SetFrame((wxFrame*)NULL);

	Activate(false);

	if (deleteWindow)
	{
		delete mFrame;
	}
	return true;
}

bool MainFrameView::DoTranslatePoints(const CC_coord& delta, unsigned ref)
{
	if (((delta.x == 0) && (delta.y == 0)) ||
		(mShow->GetSelectionList().size() == 0))
		return false;
	GetDocument()->GetCommandProcessor()->Submit(new TranslatePointsByDeltaCommand(*mShow, delta, ref), true);
	return true;
}

bool MainFrameView::DoTransformPoints(const Matrix& transmat, unsigned ref)
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new TransformPointsCommand(*mShow, transmat, ref), true);
	return true;
}

bool MainFrameView::DoMovePointsInLine(const CC_coord& start, const CC_coord& second, unsigned ref)
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new TransformPointsInALineCommand(*mShow, start, second, ref), true);
	return true;
}

bool MainFrameView::DoSetPointsSymbol(SYMBOL_TYPE sym)
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new SetSymbolAndContCommand(*mShow, sym), true);
	return true;
}

bool MainFrameView::DoSetDescription(const wxString& descr)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetDescriptionCommand(*mShow, descr), true);
	return true;
}

void MainFrameView::DoSetMode(const wxString& mode)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetModeCommand(*mShow, mode), true);
}

void MainFrameView::DoSetShowInfo(unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetShowInfoCommand(*mShow, numPoints, numColumns, labels), true);
}

bool MainFrameView::DoSetSheetTitle(const wxString& descr)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetSheetTitleCommand(*mShow, descr), true);
	return true;
}

bool MainFrameView::DoSetSheetBeats(unsigned short beats)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetSheetBeatsCommand(*mShow, beats), true);
	return true;
}

bool MainFrameView::DoSetPointsLabel(bool right)
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new SetLabelRightCommand(*mShow, right), true);
	return true;
}

bool MainFrameView::DoSetPointsLabelFlip()
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new SetLabelFlipCommand(*mShow), true);
	return true;
}

bool MainFrameView::DoInsertSheets(const CC_show::CC_sheet_container_t& sht, unsigned where)
{
	GetDocument()->GetCommandProcessor()->Submit(new AddSheetsCommand(*mShow, sht, where), true);
	return true;
}

bool MainFrameView::DoDeleteSheet(unsigned where)
{
	GetDocument()->GetCommandProcessor()->Submit(new RemoveSheetsCommand(*mShow, where), true);
	return true;
}
