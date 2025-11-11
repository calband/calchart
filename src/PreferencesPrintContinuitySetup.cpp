/*
 * CalChartPrintContinuitySetup.cpp
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

#include "PreferencesPrintContinuitySetup.h"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "PrintContinuityPreview.h"
#include "basic_ui.h"

#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/stattext.h>

IMPLEMENT_CLASS(PrintContinuitySetup, PreferencePage)

enum {
    CALCHART__CONT_NEW = 100,
    PrintContinuitySetup_KeyPress,
    PrintContinuitySetup_DOTRATIO,
    PrintContinuitySetup_PLINERATIO,
    PrintContinuitySetup_SLINERATIO,
};

BEGIN_EVENT_TABLE(PrintContinuitySetup, PrintContinuitySetup::super)
EVT_TEXT(PrintContinuitySetup_KeyPress, PrintContinuitySetup::OnKeyPress)
EVT_TEXT_ENTER(PrintContinuitySetup_DOTRATIO, PrintContinuitySetup::OnCmdTextChanged)
EVT_TEXT_ENTER(PrintContinuitySetup_PLINERATIO, PrintContinuitySetup::OnCmdTextChanged)
EVT_TEXT_ENTER(PrintContinuitySetup_SLINERATIO, PrintContinuitySetup::OnCmdTextChanged)
END_EVENT_TABLE()

static constexpr auto DefaultText
    = "~This is a centered line of text\n"
      "Normal \\bsBold \\isBold+Italics \\beItalics \\ieNormal\n"
      "Next line is all tabs with numbers\n"
      "1\t2\t3\t4\t5\t6\t7\n"
      "All the symbols with two tabs\n"
      "\t\t\\po:\tplainman\n"
      "\t\t\\pb:\tbackslashman\n"
      "\t\t\\ps:\tslashman\n"
      "\t\t\\px:\txman\n"
      "\t\t\\so:\tsolidman\n"
      "\t\t\\sb:\tsolidbackslashman\n"
      "\t\t\\ss:\tsolidslashman\n"
      "\t\t\\sx:\tsolidxman\n"
      "";

void PrintContinuitySetup::CreateControls()
{
    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 2),
        wxUI::HSizer{
            wxUI::CheckBox{ "Landscape" }
                .bind([this] {
                    mPrintContDisplay->SetOrientation(*mLandscape);
                    Refresh();
                })
                .withProxy(mLandscape),
        },
        wxUI::HSizer{
            "Draw details",
            wxUI::CheckBox{ "Use New Draw" }
                .bind([this] {
                    mConfig.Set_PrintContUseNewDraw(*mUseNewDraw);
                    Refresh();
                })
                .withProxy(mUseNewDraw),
            VLabelWidget("Symbol Ratio:", wxUI::TextCtrl{ PrintContinuitySetup_DOTRATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mDotRatio)),
            VLabelWidget("P-Line Ratio:", wxUI::TextCtrl{ PrintContinuitySetup_PLINERATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mPLineRatio)),
            VLabelWidget("S-Line Ratio:", wxUI::TextCtrl{ PrintContinuitySetup_SLINERATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mSLineRatio)),
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
        wxUI::HSplitter{
            mPrintContDisplay = [this](wxWindow* parent) { return new PrintContinuityPreview(parent, mConfig); },
            mUserInput = [](wxWindow* parent) { return new FancyTextWin(parent, PrintContinuitySetup_KeyPress); } }
            .withStashGravity(0.5)
            .withFlags(wxSizerFlags{ 1 }.Expand()),
    }
        .fitTo(this);

    mUserInput->SetValue(DefaultText);

    mPrintContDisplay->SetPrintContinuity(CalChart::PrintContinuity("", mUserInput->GetValue().ToStdString()));
    TransferDataToWindow();
}

void PrintContinuitySetup::OnCmdTextChanged(wxCommandEvent& e)
{
    auto id = e.GetId();
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(id);
    double value;
    if (text->GetValue().ToDouble(&value)) {
        switch (id) {
        case PrintContinuitySetup_DOTRATIO:
            mConfig.Set_PrintContDotRatio(value);
            break;
        case PrintContinuitySetup_PLINERATIO:
            mConfig.Set_PrintContPLineRatio(value);
            break;
        case PrintContinuitySetup_SLINERATIO:
            mConfig.Set_PrintContSLineRatio(value);
            break;
        }
    }
    Refresh();
}

void PrintContinuitySetup::InitFromConfig()
{
}

bool PrintContinuitySetup::TransferDataToWindow()
{
    *mUseNewDraw = mConfig.Get_PrintContUseNewDraw();
    wxString buf;
    wxTextCtrl* text = static_cast<wxTextCtrl*>(FindWindow(PrintContinuitySetup_DOTRATIO));
    buf.Printf(wxT("%.2f"), mConfig.Get_PrintContDotRatio());
    text->SetValue(buf);
    text = static_cast<wxTextCtrl*>(FindWindow(PrintContinuitySetup_PLINERATIO));
    buf.Printf(wxT("%.2f"), mConfig.Get_PrintContPLineRatio());
    text->SetValue(buf);
    text = static_cast<wxTextCtrl*>(FindWindow(PrintContinuitySetup_SLINERATIO));
    buf.Printf(wxT("%.2f"), mConfig.Get_PrintContSLineRatio());
    text->SetValue(buf);
    *mLinePad = mConfig.Get_PrintContLinePad();
    *mMaxFontSize = mConfig.Get_PrintContMaxFontSize();

    return true;
}

bool PrintContinuitySetup::TransferDataFromWindow()
{
    mConfig.Set_PrintContUseNewDraw(*mUseNewDraw);
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

bool PrintContinuitySetup::ClearValuesToDefault()
{
    mConfig.Clear_PrintContUseNewDraw();
    mConfig.Clear_PrintContDotRatio();
    mConfig.Clear_PrintContPLineRatio();
    mConfig.Clear_PrintContSLineRatio();
    mConfig.Clear_PrintContLinePad();
    mConfig.Clear_PrintContMaxFontSize();
    return TransferDataToWindow();
}

void PrintContinuitySetup::OnKeyPress(wxCommandEvent&)
{
    mPrintContDisplay->SetPrintContinuity(CalChart::PrintContinuity("", mUserInput->GetValue().ToStdString()));
    Refresh();
}