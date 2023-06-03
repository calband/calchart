#pragma once
/*
 * ColorSetupCanvas.h
 * Canvas for setting up colors
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

#include "CalChartDrawCommand.h"
#include "CalChartShowMode.h"
#include "basic_ui.h"
#include <memory>
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

class CalChartConfiguration;
namespace CalChart {
class Show;
}

class ColorSetupCanvas : public ClickDragCtrlScrollCanvas {
    using super = ClickDragCtrlScrollCanvas;
    DECLARE_EVENT_TABLE()

public:
    ColorSetupCanvas(CalChartConfiguration& config, wxWindow* parent);

    void OnPaint(wxPaintEvent& event);

private:
    // Internals
    void OnEraseBackground(wxEraseEvent& event);

    std::unique_ptr<CalChart::Show> mShow;
    CalChart::ShowMode mMode;
    CalChartConfiguration& mConfig;
};
