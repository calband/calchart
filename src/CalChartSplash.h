#pragma once
/*
 * CalChartSplash.h
 * Header for CalChartSplash, the wxMDI parent frame
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

#include <wx/docview.h>

// CalChartSplash
// Serves as the top frame that gets displayed when CalChart starts
class CalChartSplash : public wxDocParentFrame {
    using super = wxDocParentFrame;
    DECLARE_CLASS(CalChartSplash)
    DECLARE_EVENT_TABLE()
public:
    CalChartSplash(wxDocManager* manager, wxFrame* frame, wxString const& title);
    ~CalChartSplash() = default;

    void OnCmdAbout(wxCommandEvent& event);
    void OnCmdHelp(wxCommandEvent& event);
    void OnCmdPreferences(wxCommandEvent& event);

    static void About();
    static void Help();
};
