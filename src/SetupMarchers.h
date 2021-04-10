/*
 * SetupMarchers.h
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

#include "CalChartDoc.h"
#include <wx/docview.h>
#include <wx/wizard.h>

#include <vector>

class SetupMarchers : public wxDialog {
    DECLARE_CLASS(SetupMarchers)
    DECLARE_EVENT_TABLE()

public:
    SetupMarchers(CalChartDoc& shw, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Setup Marchers"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    ~SetupMarchers();

private:
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Setup Marchers"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

    void CreateControls();

    virtual bool Validate();
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    // The data this dialog sets for the user
private:
    unsigned mNumberColumns;
    std::vector<std::pair<std::string, std::string>> mLabelsAndInstruments;
    void OnCmd_label_type(wxCommandEvent& event);

public:
    auto GetNumberColumns() const { return mNumberColumns; }
    auto GetLabelsAndInstruments() const { return mLabelsAndInstruments; }

private:
    CalChartDoc& mShow;
    void OnReset(wxCommandEvent&);
};

class SetupMarchersWizard : public wxWizardPageSimple {
    DECLARE_CLASS(SetupMarchersWizard)
    DECLARE_EVENT_TABLE()
public:
    SetupMarchersWizard(wxWizard* parent);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    virtual bool Validate();

    // The data this dialog sets for the user
private:
    bool mTransferDataToWindowFirstTime;
    unsigned mNumberColumns;
    std::vector<std::pair<std::string, std::string>> mLabelsAndInstruments;
    void OnCmd_label_type(wxCommandEvent& event);

public:
    auto GetNumberColumns() const { return mNumberColumns; }
    auto GetLabelsAndInstruments() const { return mLabelsAndInstruments; }
};
