#pragma once
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

#include <vector>
#include <wx/docview.h>
#include <wx/wizard.h>

class CalChartDoc;

class SetupMarchers : public wxDialog {
    using super = wxDialog;
    DECLARE_CLASS(SetupMarchers)
    DECLARE_EVENT_TABLE()

public:
    SetupMarchers(CalChartDoc& shw, wxWindow* parent);
    ~SetupMarchers();

    auto GetNumberColumns() const { return mNumberColumns; }
    auto GetLabelsAndInstruments() const { return mLabelsAndInstruments; }

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
    bool Validate() override;

private:
    void CreateControls();
    void OnCmd_label_type(wxCommandEvent& event);
    void OnReset(wxCommandEvent&);

    int mNumberColumns = 8;
    std::vector<std::pair<std::string, std::string>> mLabelsAndInstruments;
    CalChartDoc& mShow;
};

class SetupMarchersWizard : public wxWizardPageSimple {
    using super = wxWizardPageSimple;
    DECLARE_CLASS(SetupMarchersWizard)
    DECLARE_EVENT_TABLE()
public:
    SetupMarchersWizard(wxWizard* parent);

    auto GetNumberColumns() const { return mNumberColumns; }
    auto GetLabelsAndInstruments() const { return mLabelsAndInstruments; }

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
    bool Validate() override;

    // The data this dialog sets for the user
private:
    bool mTransferDataToWindowFirstTime;
    int mNumberColumns;
    std::vector<std::pair<std::string, std::string>> mLabelsAndInstruments;
    void OnCmd_label_type(wxCommandEvent& event);
};
