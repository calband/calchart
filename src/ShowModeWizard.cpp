/*
 * setup_wizards.cpp
 * Classes for setting up shows
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

#include "ShowModeWizard.h"
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartShowMode.h"
#include "basic_ui.h"

#include <wx/wx.h>

// page for deciding the field type
ShowModeWizard::ShowModeWizard(wxWizard* parent)
    : wxWizardPageSimple(parent)
{
    wxUI::VSizer{
        BasicSizerFlags(),
        wxUI::Text{ "Choose a field to set your show:" },
        wxUI::Choice{ CalChart::kShowModeDefaultValues | std::views::transform([](auto item) { return std::get<0>(item); }) }.withProxy(mChoice)
    }
        .fitTo(this);
}

std::string ShowModeWizard::GetValue()
{
    return mChoice->GetString(*mChoice).ToStdString();
}
