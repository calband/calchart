#pragma once
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

#include "basic_ui.h"
#include "confgr.h"
#include <wx/wx.h>

// the basic class panel we use for all the pages.
// Each page gets a references to the CalChartConfig which will be used for
// getting and setting
class PreferencePage : public wxPanel {
    DECLARE_ABSTRACT_CLASS(GeneralSetup)
public:
    PreferencePage(CalChartConfiguration& config)
        : mConfig(config)
    {
        Init();
    }
    virtual ~PreferencePage() { }
    virtual void Init() { }
    virtual bool Create(wxWindow* parent, wxWindowID id, const wxString& caption,
        const wxPoint& pos, const wxSize& size, long style)
    {
        if (!wxPanel::Create(parent, id, pos, size, style, caption))
            return false;
        CreateControls();
        GetSizer()->Fit(this);
        GetSizer()->SetSizeHints(this);
        Center();
        return true;
    }

    // use these to get and set default values
    virtual bool TransferDataToWindow() = 0;
    virtual bool TransferDataFromWindow() = 0;
    virtual bool ClearValuesToDefault() = 0;

private:
    virtual void CreateControls() = 0;

protected:
    CalChartConfiguration& mConfig;
};
