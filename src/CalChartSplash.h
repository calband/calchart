#pragma once
/*
 * CalChartSplash.h
 * Header for CalChartSplash, the wxMDI parent frame
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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

namespace CalChart {
class Configuration;
}

// CalChartSplash
// Serves as the top frame that gets displayed when CalChart starts
class CalChartSplash : public wxDocParentFrame {
    using super = wxDocParentFrame;
    DECLARE_CLASS(CalChartSplash)
public:
    CalChartSplash(wxDocManager* manager, wxFrame* frame, wxString const& title, CalChart::Configuration& config);
    ~CalChartSplash() = default;

    static void About();
    static void Help();

private:
    void Preferences();
    CalChart::Configuration& mConfig;
};
