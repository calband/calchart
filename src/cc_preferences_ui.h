/*
 * cc_preferences.h
 * Dialox box for preferences
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

#ifndef _COLOR_SELECT_UI_H_
#define _COLOR_SELECT_UI_H_

#include <wx/wx.h>
#include <wx/bmpcbox.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>

/**
 * The Preferences dialog.
 * This appears when the user selects the "Preferences" option from
 * the File menu of the Top Frame or the Field Frame.
 */
class CalChartPreferences : public wxDialog
{
	DECLARE_CLASS( CalChartPreferences )
	DECLARE_EVENT_TABLE()

public:
	/**
	 * Makes the preferences dialog.
	 */
	CalChartPreferences( );
	/**
	 * Makes the preferences dialog.
	 * @param parent The parent for this dialog. If the parent is
	 * closed, this dialog will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the dialog box.
	 * @param pos The position of the dialog.
	 * @param size The size of the dialog.
	 * @param style The style in which the dialog will be drawn.
	 */
	CalChartPreferences( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("CalChart Preferences"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	/**
	 * Cleanup.
	 */
	~CalChartPreferences( );

	/**
	 * Called when the preferences dialog is created, before
	 * the Create(...) method is called.
	 */
	void Init();

	/**
	 * Sets up the dialog.
	 * @param parent The parent for this dialog. If the parent is
	 * closed, this dialog will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the dialog box.
	 * @param pos The position of the dialog.
	 * @param size The size of the dialog.
	 * @param style The style in which the dialog will be drawn.
	 */
	bool Create(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("CalChart Preferences"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	/**
	 * Sets up the components of the dialog.
	 */
	void CreateControls();

	/**
	 * Currently does nothing.
	 * @return TODO
	 */
	bool TransferDataToWindow();
	/**
	 * Loads the data collected through the pages of the dialog
	 * into the CalChart config file.
	 * @return TODO
	 */
	bool TransferDataFromWindow();
private:
	/**
	 * Called when the "Reset All" button is clicked.
	 * All settings, in all pages of the preferences window,
	 * are reset to their default values.
	 */
	void OnCmdResetAll(wxCommandEvent&);
	/**
	 * A notebook filled with the pages of the
	 * preferences dialog.
	 */
	wxNotebook* mNotebook;
};

#endif
