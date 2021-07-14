#pragma once
/*
 * PreferencesPSPrintingSetup.h
 * Dialox box for PSPrinting Setup part of preferences
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

#include "PreferencesUtils.h"

//////// General setup ////////
// setup pringing values and colors
////////

class PSPrintingSetUp : public PreferencePage {
    using super = PreferencePage;
    DECLARE_CLASS(PSPrintingSetUp)
    DECLARE_EVENT_TABLE()

public:
    static PSPrintingSetUp* CreatePreference(CalChartConfiguration& config, wxWindow* parent)
    {
        auto pref = new PSPrintingSetUp(config, parent);
        pref->Initialize();
        return pref;
    }

private:
    // private, use the CreatePreference method
    PSPrintingSetUp(CalChartConfiguration& config, wxWindow* parent)
        : super(config, parent, "Printing Values")
    {
    }

public:
    ~PSPrintingSetUp() override = default;

    // use these to get and set default values
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
    bool ClearValuesToDefault() override;

private:
    void InitFromConfig() override;
    void CreateControls() override;

    wxString mFontNames[7];
    double mPrintValues[8];
};
