/*
 * calchartapp.cpp
 * Central App for CalChart
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
#include "top_frame.h"
#include "modes.h"
#include "confgr.h"
#include "basic_ui.h"
#include "main_ui.h"
#include "show.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/fs_zip.h>

#include <wx/mdi.h>
#include <wx/docview.h>
#include <wx/docmdi.h>

#ifdef __CC_INCLUDE_BITMAPS__
#include "tb_left.xbm"
#include "tb_right.xbm"
#include "tb_box.xbm"
#include "tb_poly.xbm"
#include "tb_lasso.xbm"
#include "tb_mv.xbm"
#include "tb_line.xbm"
#include "tb_rot.xbm"
#include "tb_shr.xbm"
#include "tb_ref.xbm"
#include "tb_siz.xbm"
#include "tb_gen.xbm"
#include "tb_lbl_l.xbm"
#include "tb_lbl_r.xbm"
#include "tb_lbl_f.xbm"
#include "tb_sym0.xbm"
#include "tb_sym1.xbm"
#include "tb_sym2.xbm"
#include "tb_sym3.xbm"
#include "tb_sym4.xbm"
#include "tb_sym5.xbm"
#include "tb_sym6.xbm"
#include "tb_sym7.xbm"
#include "tb_stop.xbm"
#include "tb_play.xbm"
#include "tb_pbeat.xbm"
#include "tb_nbeat.xbm"
#include "tb_pshet.xbm"
#include "tb_nshet.xbm"
#endif

wxPrintDialogData *gPrintDialogData;

extern ToolBarEntry main_tb[];
extern ToolBarEntry anim_tb[];

wxFont *contPlainFont;
wxFont *contBoldFont;
wxFont *contItalFont;
wxFont *contBoldItalFont;
wxFont *pointLabelFont;
wxFont *yardLabelFont;

wxHtmlHelpController *gHelpController = NULL;

TopFrame *topframe = NULL;

TopFrame *GetMainFrame(void)
{
  return topframe;
}

void CC_continuity_UnitTests();
void CC_point_UnitTests();
void CC_coord_UnitTests();

// This statement initializes the whole application and calls OnInit
IMPLEMENT_APP(CalChartApp)


// Create windows and initialize app
bool CalChartApp::OnInit()
{
    //// Create a document manager
    mDocManager = new wxDocManager;

	//// Create a template relating drawing documents to their views
	(void) new wxDocTemplate(mDocManager, _T("CalChart Show"), _T("*.shw"), _T(""), _T("shw"), _T("CalChart"), _T("Field View"),
			CLASSINFO(CC_show), CLASSINFO(MainFrameView));

	gPrintDialogData = new wxPrintDialogData();

	// setup the configuration.
	ReadConfig();
	
//Create toolbar bitmaps
	int i = 0;

	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_left));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_right));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_box));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_poly));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_lasso));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_mv));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_line));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_rot));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_shr));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_ref));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_siz));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_gen));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_lbl_l));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_lbl_f));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_lbl_r));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym0));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym1));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym2));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym3));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym4));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym5));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym6));
	main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym7));

	i = 0;

	anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_stop));
	anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_play));
	anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_pbeat));
	anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_nbeat));
	anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_pshet));
	anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_nshet));

	contPlainFont = new wxFont(11, wxMODERN, wxNORMAL, wxNORMAL);
	contBoldFont = new wxFont(11, wxMODERN, wxNORMAL, wxBOLD);
	contItalFont = new wxFont(11, wxMODERN, wxITALIC, wxNORMAL);
	contBoldItalFont = new wxFont(11, wxMODERN, wxITALIC, wxBOLD);
	pointLabelFont = new wxFont((int)Float2Coord(GetConfiguration_DotRatio() * GetConfiguration_NumRatio()),
		wxSWISS, wxNORMAL, wxNORMAL);
	yardLabelFont = new wxFont((int)Float2Coord(GetConfiguration_YardsSize()),
		wxSWISS, wxNORMAL, wxNORMAL);


	//// Create the main frame window
	topframe = new TopFrame(mDocManager, (wxFrame *) NULL, _T("CalChart"));

	{
		// Required for images in the online documentation
		wxImage::AddHandler(new wxGIFHandler);

		// Required for advanced HTML help
		wxFileSystem::AddHandler(new wxZipFSHandler);
		wxFileSystem::AddHandler(new wxArchiveFSHandler);

#if defined(__APPLE__) && (__APPLE__)
		wxString helpfile(wxT("CalChart.app/docs"));
#else
		wxString helpfile(wxT("docs"));
#endif
		helpfile.Append(PATH_SEPARATOR wxT("charthlp.hhp"));
		gHelpController = new wxHtmlHelpController;
		if ( !gHelpController->AddBook(wxFileName(helpfile) ))
		{
			wxLogError(wxT("Cannot find the help system."));
		}
	}

#ifndef __WXMAC__
	topframe->Show(true);
#endif //ndef __WXMAC__
	SetTopWindow(topframe);
	
	// Get the file history
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/FileHistory"));
	mDocManager->FileHistoryLoad(*config);

	CC_continuity_UnitTests();
	CC_point_UnitTests();
	CC_coord_UnitTests();
	
	return true;
}

void CalChartApp::MacOpenFile(const wxString &fileName)
{
	mDocManager->CreateDocument(fileName, wxDOC_SILENT);
}

int CalChartApp::OnExit()
{
	// Get the file history
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/FileHistory"));
	mDocManager->FileHistorySave(*config);
	config->Flush();

	if (gPrintDialogData) delete gPrintDialogData;
	if (gHelpController) delete gHelpController;

    // delete the doc manager
    delete wxDocManager::GetDocumentManager();
    
	return wxApp::OnExit();
}

