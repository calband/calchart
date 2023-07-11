#pragma once
/*
 * PrintContinuityPreview.h
 * Header for continuity preview
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

#include "CalChartText.h"
#include <wx/wx.h>

class CalChartConfiguration;

class PrintContinuityPreview : public wxScrolled<wxWindow> {
    using super = wxScrolled<wxWindow>;
    DECLARE_EVENT_TABLE()

public:
    PrintContinuityPreview(wxWindow* parent, CalChartConfiguration const& config);

    void SetPrintContinuity(const CalChart::Textline_list& printContinuity)
    {
        mPrintContinuity = printContinuity;
    }
    void SetOrientation(bool landscape) { m_landscape = landscape; }

private:
    CalChart::Textline_list mPrintContinuity{};
    bool m_landscape{};
    CalChartConfiguration const& mConfig;

    void OnPaint(wxPaintEvent& event);
    void OnSizeEvent(wxSizeEvent& event);
};
