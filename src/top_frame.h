/*
 * top_frame.h
 * Header for TopFrame, the wxMDI parent frame
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

#ifndef _TOP_FRAME_H_
#define _TOP_FRAME_H_

#include <wx/docview.h>

/**
 * The main frame of the CalChart program.
 * When you open CalChart, without opening a show file, you are greeted with
 * an empty frame/window which you can use to open the files that you wish
 * to view/edit. That is the "top frame," and it functions as the main frame
 * for the CalChart program. If you close it, it closes all other windows,
 * and you exit the program.
 */
class TopFrame : public wxDocParentFrame
{
	DECLARE_CLASS(TopFrame)
public:

	/** Makes a top frame.
	 * @param manager The doc manager that handles the saving/opening/viewing
	 * of CalChart-related files.
	 * @param frame The parent frame. Should be null - this frame is the
	 * parent for all other CalChart frames.
	 * @param title The name of the frame.
	 */
	TopFrame(wxDocManager *manager,
			 wxFrame *frame,
			 const wxString& title);

	/** 
	 * Performs cleanup. Currently does nothing.
	 */
	~TopFrame();

	/** 
	 * Called when the 'About' item is selected in the 'Help' menu.
	 * @param event Ignored.
	 */
	void OnCmdAbout(wxCommandEvent& event);

	/**
	 * Called when the 'Help' item is selected in the 'Help' menu.
	 * @param event Ignored.
	 */
	void OnCmdHelp(wxCommandEvent& event);

	/**
	 * Called when the 'Preferences' item is selected in the 'File' menu.
	 * @param event Ignored.
	 */
	void OnCmdPreferences(wxCommandEvent& event);

	/** 
	 * Opens the 'About' dialog.
	 */
	static void About();

	/**
	 * Opens the 'Help' manual.
	 */
	static void Help();

	DECLARE_EVENT_TABLE()
};

#endif // _TOP_FRAME_H_
