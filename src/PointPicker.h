#pragma once
/*
 * PointPicker.h
 * Dialog for picking points
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

#include "CalChartDoc.h"
#include <vector>
#include <wx/docview.h>
#include <wx/wizard.h>
#include <wxUI/wxUI.hpp>

class PointPicker : public wxDialog {
    using super = wxDialog;

public:
    PointPicker(wxWindow* parent, CalChartDoc const& show);
    PointPicker(wxWindow* parent, CalChartDoc const& show, CalChart::SelectionList const& marchersToUse, CalChart::SelectionList const& selected);
    ~PointPicker() override = default;

    auto GetMarchersSelected() const { return mMarcherLabels; }

private:
    std::vector<std::string> mMarcherLabels;
};
