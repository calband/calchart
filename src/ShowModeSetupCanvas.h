#pragma once
/*
 * ModeSetupDialog.h
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

#include "CalChartShowMode.h"
#include "basic_ui.h"
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

namespace CalChart {
class Configuration;
}

class ShowModeSetupCanvas : public ClickDragCtrlScrollCanvas {
    DECLARE_CLASS(ShowModeSetupCanvas)
    DECLARE_EVENT_TABLE()
    using super = ClickDragCtrlScrollCanvas;

public:
    ShowModeSetupCanvas(CalChart::Configuration const& config, wxWindow* parent,
        wxWindowID id = wxID_ANY);

    void OnPaint(wxPaintEvent& event);
    void SetMode(CalChart::ShowMode const& mode);
    virtual void SetZoom(float factor);

private:
    CalChart::Configuration const& mConfig;
    CalChart::ShowMode mMode;
};
