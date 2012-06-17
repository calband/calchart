/*
 * homeview_frame.h
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

#ifndef __HOMEVIEW_FRAME_H__
#define __HOMEVIEW_FRAME_H__

#include <wx/docview.h>

// Define the main editing frame
class HomeViewFrame : public wxDocChildFrame
{
public:
	// HomeViewFrame will own the show that is passed in
	HomeViewFrame(wxDocument* doc, wxView* view, wxDocParentFrame *frame, const wxPoint& pos, const wxSize& size);
	~HomeViewFrame();

	void OnCmdPrint(wxCommandEvent& event);
	void OnCmdPrintPreview(wxCommandEvent& event);
	void OnCmdLegacyPrint(wxCommandEvent& event);
	void OnCmdLegacyPrintEPS(wxCommandEvent& event);
	void OnCmdPageSetup(wxCommandEvent& event);
	void OnCmdPreferences(wxCommandEvent& event);
	void OnCmdClose(wxCommandEvent& event);
	void OnCmdPoints(wxCommandEvent& event);
	void OnCmdAnimate(wxCommandEvent& event);
	void OnCmdOmniView(wxCommandEvent& event);
	void OnCmdAbout(wxCommandEvent& event);
	void OnCmdHelp(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif
