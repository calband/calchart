#pragma once
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

#include <wx/wx.h>

namespace CalChart {
class ContProcedure;
}

class ContinuityComposerPanel;

class ContinuityComposerDialog : public wxDialog {
    using super = wxDialog;
    DECLARE_CLASS(ContinuityComposerDialog)

public:
    ContinuityComposerDialog(std::unique_ptr<CalChart::ContProcedure> starting_continuity, wxWindow* parent, wxWindowID id = wxID_ANY, wxString const& caption = wxT("Compose Continuity"), wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, wxString const& name = wxDialogNameStr);
    ~ContinuityComposerDialog() override = default;

    std::unique_ptr<CalChart::ContProcedure> GetContinuity();
    bool Validate() override;

private:
    ContinuityComposerPanel* mPanel{};
};
