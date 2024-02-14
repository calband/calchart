#pragma once
/*
 * PrintContinuityPreview.h
 * Header for continuity preview
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

#include "CalChartText.h"
#include <wx/wx.h>

namespace CalChart {
class Configuration;
}

class PrintContinuityPreview : public wxScrolled<wxWindow> {
    using super = wxScrolled<wxWindow>;
    DECLARE_EVENT_TABLE()

public:
    PrintContinuityPreview(wxWindow* parent, CalChart::Configuration const& config);

    void SetPrintContinuity(const CalChart::PrintContinuity& printContinuity)
    {
        mPrintContinuity = printContinuity;
    }
    void SetOrientation(bool landscape)
    {
        m_landscape = landscape;
    }

private:
    CalChart::PrintContinuity mPrintContinuity{};
    bool m_landscape{};
    CalChart::Configuration const& mConfig;

    void OnPaint(wxPaintEvent& event);
    void OnSizeEvent(wxSizeEvent& event);
};
