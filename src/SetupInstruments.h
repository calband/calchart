#pragma once
/*
 * SetupInstruments.h
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

class SetupInstruments : public wxDialog {
    DECLARE_CLASS(SetupInstruments)

public:
    SetupInstruments(CalChartDoc const& shw, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Set Instruments"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    ~SetupInstruments() = default;

    // returns label, symbol, instrument
    std::map<int, std::string> GetInstruments() const;

private:
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Set Instruments"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

    void CreateControls();

private:
    CalChartDoc const& mShow;
    std::vector<int> mDotIndices;
    std::vector<std::string> mLabels;
    std::vector<std::string> mInstruments;
    std::vector<std::string> mInstrumentChoices;
    std::vector<CalChart::SYMBOL_TYPE> mSymbols;

    void SelectAll();
    void SelectAllPoints();
    void Select();
    void SelectNone();
    void SelectSymbol(CalChart::SYMBOL_TYPE);
    void SelectInstrument();
    void OnCmdChoice();

    void SelectionListChanged();

    wxUI::ListBox::Proxy mSetupInstrumentList;
    wxUI::Choice::Proxy mSelectInstrument;
    wxUI::Choice::Proxy mInstrumentChoice;
};
