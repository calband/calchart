/*
 * CalChartPreferences.cpp
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

#include "PreferencesPSPrintingSetup.h"
#include "CalChartConfiguration.h"

#include <wx/listbook.h>
#include <wxUI/wxUI.hpp>

using namespace CalChart;

typedef enum {
    RESET = 1000,
    HEADFONT,
    MAINFONT,
    NUMBERFONT,
    CONTFONT,
    BOLDFONT,
    ITALFONT,
    BOLDITALFONT,
    HEADERSIZE,
    YARDSSIZE,
    TEXTSIZE,
    CONTRATIO,
} PSPrintingSetUp_IDs;

BEGIN_EVENT_TABLE(PSPrintingSetUp, PreferencePage)
END_EVENT_TABLE()

IMPLEMENT_CLASS(PSPrintingSetUp, PreferencePage)

void PSPrintingSetUp::CreateControls()
{
    wxUI::VSizer{
        LeftBasicSizerFlags(),
        wxUI::HSizer{
            VLabelWidget("Header Font:", wxUI::TextCtrl{ HEADFONT }.withSize({ 100, -1 })),
            VLabelWidget("Main Font:", wxUI::TextCtrl{ MAINFONT }.withSize({ 100, -1 })),
            VLabelWidget("Number Font:", wxUI::TextCtrl{ NUMBERFONT }.withSize({ 100, -1 })),
            VLabelWidget("Continuity Font:", wxUI::TextCtrl{ CONTFONT }.withSize({ 100, -1 })),
        },
        wxUI::HSizer{
            VLabelWidget("Bold Font:", wxUI::TextCtrl{ BOLDFONT }.withSize({ 100, -1 })),
            VLabelWidget("Italic Font:", wxUI::TextCtrl{ ITALFONT }.withSize({ 100, -1 })),
            VLabelWidget("Bold Italic Font:", wxUI::TextCtrl{ BOLDITALFONT }.withSize({ 100, -1 })),
        },
        wxUI::HSizer{
            VLabelWidget("Header Size:", wxUI::TextCtrl{ HEADERSIZE }.withSize({ 100, -1 })),
            VLabelWidget("Yards Side:", wxUI::TextCtrl{ YARDSSIZE }.withSize({ 100, -1 })),
            VLabelWidget("Text Side:", wxUI::TextCtrl{ TEXTSIZE }.withSize({ 100, -1 })),
        },
        wxUI::HSizer{
            VLabelWidget("Continuity Ratio:", wxUI::TextCtrl{ CONTRATIO }.withSize({ 100, -1 })),
        },
    }
        .fitTo(this);

    TransferDataToWindow();
}

void PSPrintingSetUp::InitFromConfig()
{
    mFontNames[0] = mConfig.Get_HeadFont();
    mFontNames[1] = mConfig.Get_MainFont();
    mFontNames[2] = mConfig.Get_NumberFont();
    mFontNames[3] = mConfig.Get_ContFont();
    mFontNames[4] = mConfig.Get_BoldFont();
    mFontNames[5] = mConfig.Get_ItalFont();
    mFontNames[6] = mConfig.Get_BoldItalFont();
    mPrintValues[0] = mConfig.Get_HeaderSize();
    mPrintValues[1] = mConfig.Get_YardsSize();
    mPrintValues[2] = mConfig.Get_TextSize();
    mPrintValues[7] = mConfig.Get_ContRatio();
}

bool PSPrintingSetUp::TransferDataToWindow()
{
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(HEADFONT);
    text->SetValue(mFontNames[0]);
    text = (wxTextCtrl*)FindWindow(MAINFONT);
    text->SetValue(mFontNames[1]);
    text = (wxTextCtrl*)FindWindow(NUMBERFONT);
    text->SetValue(mFontNames[2]);
    text = (wxTextCtrl*)FindWindow(CONTFONT);
    text->SetValue(mFontNames[3]);
    text = (wxTextCtrl*)FindWindow(BOLDFONT);
    text->SetValue(mFontNames[4]);
    text = (wxTextCtrl*)FindWindow(ITALFONT);
    text->SetValue(mFontNames[5]);
    text = (wxTextCtrl*)FindWindow(BOLDITALFONT);
    text->SetValue(mFontNames[6]);
    text = (wxTextCtrl*)FindWindow(HEADERSIZE);
    wxString buf;
    buf.Printf(wxT("%.2f"), mPrintValues[0]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(YARDSSIZE);
    buf.Printf(wxT("%.2f"), mPrintValues[1]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(TEXTSIZE);
    buf.Printf(wxT("%.2f"), mPrintValues[2]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(CONTRATIO);
    buf.Printf(wxT("%.2f"), mPrintValues[7]);
    text->SetValue(buf);

    return true;
}

bool PSPrintingSetUp::TransferDataFromWindow()
{
    // read out the values from the window
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(HEADFONT);
    mFontNames[0] = text->GetValue().ToStdString();
    text = (wxTextCtrl*)FindWindow(MAINFONT);
    mFontNames[1] = text->GetValue().ToStdString();
    text = (wxTextCtrl*)FindWindow(NUMBERFONT);
    mFontNames[2] = text->GetValue().ToStdString();
    text = (wxTextCtrl*)FindWindow(CONTFONT);
    mFontNames[3] = text->GetValue().ToStdString();
    text = (wxTextCtrl*)FindWindow(BOLDFONT);
    mFontNames[4] = text->GetValue().ToStdString();
    text = (wxTextCtrl*)FindWindow(ITALFONT);
    mFontNames[5] = text->GetValue().ToStdString();
    text = (wxTextCtrl*)FindWindow(BOLDITALFONT);
    mFontNames[6] = text->GetValue().ToStdString();
    text = (wxTextCtrl*)FindWindow(HEADERSIZE);
    text->GetValue().ToDouble(&mPrintValues[0]);
    text = (wxTextCtrl*)FindWindow(YARDSSIZE);
    text->GetValue().ToDouble(&mPrintValues[1]);
    text = (wxTextCtrl*)FindWindow(TEXTSIZE);
    text->GetValue().ToDouble(&mPrintValues[2]);
    text = (wxTextCtrl*)FindWindow(CONTRATIO);
    text->GetValue().ToDouble(&mPrintValues[7]);

    // write out the values defaults:
    mConfig.Set_HeadFont(mFontNames[0]);
    mConfig.Set_MainFont(mFontNames[1]);
    mConfig.Set_NumberFont(mFontNames[2]);
    mConfig.Set_ContFont(mFontNames[3]);
    mConfig.Set_BoldFont(mFontNames[4]);
    mConfig.Set_ItalFont(mFontNames[5]);
    mConfig.Set_BoldItalFont(mFontNames[6]);
    mConfig.Set_HeaderSize(mPrintValues[0]);
    mConfig.Set_YardsSize(mPrintValues[1]);
    mConfig.Set_TextSize(mPrintValues[2]);
    mConfig.Set_ContRatio(mPrintValues[7]);

    return true;
}

bool PSPrintingSetUp::ClearValuesToDefault()
{
    mConfig.Clear_HeadFont();
    mConfig.Clear_MainFont();
    mConfig.Clear_NumberFont();
    mConfig.Clear_ContFont();
    mConfig.Clear_BoldFont();
    mConfig.Clear_ItalFont();
    mConfig.Clear_BoldItalFont();
    mConfig.Clear_HeaderSize();
    mConfig.Clear_YardsSize();
    mConfig.Clear_TextSize();
    mConfig.Clear_ContRatio();
    InitFromConfig();
    return TransferDataToWindow();
}
