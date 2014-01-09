/*
 * print_ps_dialog.h
 * Dialox box for printing postscript
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

#ifndef __PRINT_PS_DIALOG_H__
#define __PRINT_PS_DIALOG_H__

#include "platconf.h"

#include <wx/wx.h>
#include <wx/dialog.h>
#include <set>

class CalChartDoc;

/**
 * The dialog that appears when the user selects either the
 * "Print to PS" or "Print to EPS" options in the file menu
 * of the field frame.
 */
class PrintPostScriptDialog : public wxDialog
{
	DECLARE_CLASS( PrintPostScriptDialog )
	DECLARE_EVENT_TABLE()

public:
	/**
	 * Makes the dialog.
	 */
	PrintPostScriptDialog( );
	/**
	 * Makes the dialog.
	 * @param doc The document to print.
	 * @param printEPS True if the printing format is Encapsulated Postscript;
	 * false if the printing format is just Postscript.
	 * @param parent The parent for this dialog. If the parent is
	 * closed, this dialog will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the dialog box.
	 * @param pos The position of the dialog.
	 * @param size The size of the dialog.
	 * @param style The style in which the dialog will be drawn.
	 */
	PrintPostScriptDialog(const CalChartDoc *doc, bool printEPS,
		wxFrame *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Print Dialog"),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	/**
	 * Cleanup.
	 */
	~PrintPostScriptDialog( );

	/**
	 * Initialized the dialog. Called before Create(...) when the dialog
	 * is created.
	 */
	void Init();

	/**
	 * Called while setting up the dialog.
	 * @param parent The parent for this dialog. If the parent is
	 * closed, this dialog will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the dialog box.
	 * @param pos The position of the dialog.
	 * @param size The size of the dialog.
	 * @param style The style in which the dialog will be drawn.
	 */
	bool Create(const CalChartDoc *show, bool printEPS,
		wxFrame *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Print Dialog"),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	/**
	 * Sets up the GUI components through which the user provides
	 * input.
	 */
	void CreateControls();

	/**
	 * Called when the "Select Sheets" button is pressed in the dialog.
	 * This allows the user to choose which stuntsheets should be printed.
	 */
	void ShowPrintSelect(wxCommandEvent&);
	/**
	 * Called when the "Reset Values" button is pressed in the dialog.
	 * This makes the values reset to their defaults.
	 */
	void ResetDefaults(wxCommandEvent&);

	/**
	 * Restores the default settings to the GUI components.
	 * @return TODO
	 */
	virtual bool TransferDataToWindow();
	/**
	 * Saves the current setting stored in the GUI components as the default
	 * values for those components.
	 * @return TODO
	 */
	virtual bool TransferDataFromWindow();

	/**
	 * Prints the show, according to the user's input in the dialog.
	 */
	void PrintShow();

private:
	/**
	 * The document to print.
	 */
	const CalChartDoc* mShow;
	/**
	 * True if the printing format is encapsulated postscript,
	 * false if the printing format is just postscript.
	 */
	bool eps;
	/**
	 * The "Printer Device" box.
	 */
	wxTextCtrl *text_cmd;
#ifdef PRINT__RUN_CMD
	//TODO - comments
	wxTextCtrl *text_opts, *text_view_cmd, *text_view_opts;
#endif
	/**
	 * Page x-offset.
	 */
	wxTextCtrl *text_x;
	/**
	 * Page y-offset.
	 */
	wxTextCtrl *text_y;
	/**
	 * Page width.
	 */
	wxTextCtrl *text_width;
	/**
	 * Page height.
	 */
	wxTextCtrl *text_height;
	/**
	 * Paper length.
	 */
	wxTextCtrl *text_length;
	/**
	 * The minimum number of yards that should appear in the show printout.
	 */
	wxTextCtrl *text_minyards;
	/**
	 * Records the orientation to which to print the show.
	 * This is either landscape or portait.
	 */
	wxRadioBox *radio_orient;
	/**
	 * Records whether to print the show to a file, or
	 * to a printer.
	 */
	wxRadioBox *radio_method;
	/**
	 * The "Continuity" check box, which will record true if, when the
	 * document is printed, each stuntsheet should be printed with a
	 * description of its continuities.
	 */
	wxCheckBox *check_cont;
	/**
	 * The "Cover Pages" check box, which will be checked if, when the
	 * document is printed, a sheet should be included at the beginning
	 * which has a list of all of the show's continuities.
	 */
	wxCheckBox *check_pages;
	/**
	 * The "Overview" check box, TODO
	 */
	wxCheckBox* check_overview;
	/**
	 * A set containing the indices of all stuntsheets that
	 * should be printed.
	 */
	std::set<size_t> mIsSheetPicked;
};
#endif
