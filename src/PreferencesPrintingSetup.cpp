/*
 * CalChartPrintingSetup.cpp
 * Dialox box for PrintContinuity Setup part of preferences
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

#include "PreferencesPrintingSetup.hpp"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "PrintingPreview.hpp"
#include "basic_ui.h"

#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/stattext.h>

IMPLEMENT_CLASS(PrintingSetup, PreferencePage)

enum {
    CALCHART__CONT_NEW = 100,
    PrintingSetup_KeyPress,
    PrintingSetup_DOTRATIO,
    PrintingSetup_PLINERATIO,
    PrintingSetup_SLINERATIO,
};

BEGIN_EVENT_TABLE(PrintingSetup, PrintingSetup::super)
EVT_TEXT(PrintingSetup_KeyPress, PrintingSetup::OnKeyPress)
EVT_TEXT_ENTER(PrintingSetup_DOTRATIO, PrintingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(PrintingSetup_PLINERATIO, PrintingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(PrintingSetup_SLINERATIO, PrintingSetup::OnCmdTextChanged)
END_EVENT_TABLE()

void PrintingSetup::CreateControls()
{
    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 2),
        wxUI::HSizer{
            wxUI::CheckBox{ "Landscape" }
                .bind([this] {
                    mPrintingDisplay->SetOrientation(*mLandscape);
                    Refresh();
                })
                .withProxy(mLandscape),
        },
        wxUI::HSizer{
            "Printing details",
            VLabelWidget("Symbol Ratio:", wxUI::TextCtrl{ PrintingSetup_DOTRATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mDotRatio)),
            VLabelWidget("P-Line Ratio:", wxUI::TextCtrl{ PrintingSetup_PLINERATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mPLineRatio)),
            VLabelWidget("S-Line Ratio:", wxUI::TextCtrl{ PrintingSetup_SLINERATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mSLineRatio)),
            VLabelWidget("Line Pad:", wxUI::SpinCtrl{ std::pair{ 0, 10 } }.bind([this] {
                                                                              mConfig.Set_PrintContLinePad(static_cast<int>(*mLinePad));
                                                                              Refresh();
                                                                          })
                                          .withProxy(mLinePad)),
            VLabelWidget("Max Font Size:", wxUI::SpinCtrl{ std::pair{ 6, 30 } }.bind([this] {
                                                                                   mConfig.Set_PrintContMaxFontSize(static_cast<int>(*mMaxFontSize));
                                                                                   Refresh();
                                                                               })
                                               .withProxy(mMaxFontSize)),
        },
        mPrintingDisplay = wxUI::Generic<PrintingPreview>{
            wxSizerFlags{ 1 }.Expand(),
            [this](wxWindow* parent) { return new PrintingPreview(parent, mConfig); } },
    }
        .fitTo(this);

    TransferDataToWindow();
}

void PrintingSetup::OnCmdTextChanged(wxCommandEvent& e)
{
    auto id = e.GetId();
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(id);
    double value;
    if (text->GetValue().ToDouble(&value)) {
        switch (id) {
        case PrintingSetup_DOTRATIO:
            mConfig.Set_PrintContDotRatio(value);
            break;
        case PrintingSetup_PLINERATIO:
            mConfig.Set_PrintContPLineRatio(value);
            break;
        case PrintingSetup_SLINERATIO:
            mConfig.Set_PrintContSLineRatio(value);
            break;
        }
    }
    Refresh();
}

void PrintingSetup::InitFromConfig()
{
}

bool PrintingSetup::TransferDataToWindow()
{
    wxString buf;
    wxTextCtrl* text = static_cast<wxTextCtrl*>(FindWindow(PrintingSetup_DOTRATIO));
    buf.Printf(wxT("%.2f"), mConfig.Get_PrintContDotRatio());
    text->SetValue(buf);
    text = static_cast<wxTextCtrl*>(FindWindow(PrintingSetup_PLINERATIO));
    buf.Printf(wxT("%.2f"), mConfig.Get_PrintContPLineRatio());
    text->SetValue(buf);
    text = static_cast<wxTextCtrl*>(FindWindow(PrintingSetup_SLINERATIO));
    buf.Printf(wxT("%.2f"), mConfig.Get_PrintContSLineRatio());
    text->SetValue(buf);
    *mLinePad = mConfig.Get_PrintContLinePad();
    *mMaxFontSize = mConfig.Get_PrintContMaxFontSize();

    return true;
}

bool PrintingSetup::TransferDataFromWindow()
{
    double value = 0.0;
    if (mDotRatio->GetValue().ToDouble(&value)) {
        mConfig.Set_PrintContDotRatio(value);
    }
    if (mPLineRatio->GetValue().ToDouble(&value)) {
        mConfig.Set_PrintContPLineRatio(value);
    }
    if (mSLineRatio->GetValue().ToDouble(&value)) {
        mConfig.Set_PrintContSLineRatio(value);
    }
    mConfig.Set_PrintContLinePad(*mLinePad);
    mConfig.Set_PrintContMaxFontSize(*mMaxFontSize);
    return true;
}

bool PrintingSetup::ClearValuesToDefault()
{
    mConfig.Clear_PrintContUseNewDraw();
    mConfig.Clear_PrintContDotRatio();
    mConfig.Clear_PrintContPLineRatio();
    mConfig.Clear_PrintContSLineRatio();
    mConfig.Clear_PrintContLinePad();
    mConfig.Clear_PrintContMaxFontSize();
    return TransferDataToWindow();
}

void PrintingSetup::OnKeyPress(wxCommandEvent&)
{
    // mPrintingDisplay->SetPrintContinuity(CalChart::PrintContinuity("", mUserInput->GetValue()));
    Refresh();
}