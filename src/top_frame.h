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

#pragma once

#include <wx/docview.h>

// TopFrame
// Serves as the top frame that gets displayed when CalChart starts
class TopFrame : public wxDocParentFrame {
    DECLARE_CLASS(TopFrame)
public:
    TopFrame(wxDocManager* manager, wxFrame* frame, const wxString& title);
    ~TopFrame();

    void OnCmdAbout(wxCommandEvent& event);
    void OnCmdHelp(wxCommandEvent& event);
    void OnCmdPreferences(wxCommandEvent& event);

    static void About();
    static void Help();

    DECLARE_EVENT_TABLE()
};
