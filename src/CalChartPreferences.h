#pragma once
/*
 * cc_preferences.h
 * Dialox box for preferences
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

#include <wx/wx.h>

class wxNotebook;

namespace CalChart {
class Configuration;
}
class CalChartPreferences : public wxDialog {
    using super = wxDialog;
    DECLARE_CLASS(CalChartPreferences)
    DECLARE_EVENT_TABLE()

public:
    CalChartPreferences(wxWindow* parent, CalChart::Configuration& config);
    ~CalChartPreferences() = default;

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

private:
    void OnCmdResetAll(wxCommandEvent&);
    wxNotebook* mNotebook;
    CalChart::Configuration& mConfig;
};
