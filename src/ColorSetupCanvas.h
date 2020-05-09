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

#pragma once

#include "basic_ui.h"
#include "cc_drawcommand.h"
#include "modes.h"
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

class CalChartConfiguration;
namespace CalChart {
class Show;
}

class ColorSetupCanvas : public ClickDragCtrlScrollCanvas {
    DECLARE_EVENT_TABLE()
    using super = ClickDragCtrlScrollCanvas;

public:
    ColorSetupCanvas(CalChartConfiguration& config, wxWindow* parent);

    void OnPaint(wxPaintEvent& event);

private:
    void OnEraseBackground(wxEraseEvent& event);

    std::unique_ptr<CalChart::Show> mShow;
    CalChart::ShowMode mMode;
    CalChartConfiguration& mConfig;
    std::vector<CalChart::DrawCommand> mPath;
    CalChart::Coord mPathEnd;
    std::vector<CalChart::DrawCommand> mShape;
};
