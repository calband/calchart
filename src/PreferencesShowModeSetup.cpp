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

#include "PreferencesShowModeSetup.h"
#include "PreferencesUtils.h"
#include "CalChartDoc.h"
#include "CalChartSizes.h"
#include "ColorSetupCanvas.h"
#include "ContinuityBrowserPanel.h"
#include "ContinuityComposerDialog.h"
#include "cc_drawcommand.h"
#include "cc_shapes.h"
#include "cc_sheet.h"
#include "confgr.h"
#include "cont.h"
#include "draw.h"
#include "mode_dialog.h"
#include "mode_dialog_canvas.h"
#include "modes.h"

#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/stattext.h>

using namespace CalChart;


enum {
    MODE_CHOICE = 1000,
    WESTHASH,
    EASTHASH,
    BORDER_LEFT,
    BORDER_TOP,
    BORDER_RIGHT,
    BORDER_BOTTOM,
    OFFSET_X,
    OFFSET_Y,
    SIZE_X,
    SIZE_Y,
    SHOW_LINE_MARKING,
    SHOW_LINE_VALUE,
    CANVAS,
};

BEGIN_EVENT_TABLE(ShowModeSetup, PreferencePage)
EVT_CHOICE(MODE_CHOICE, ShowModeSetup::OnCmdChoice)
EVT_CHOICE(SHOW_LINE_MARKING, ShowModeSetup::OnCmdChoice)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ShowModeSetup, PreferencePage)

void ShowModeSetup::CreateControls()
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    wxChoice* modes = new wxChoice(this, MODE_CHOICE, wxDefaultPosition,
        wxDefaultSize, SHOWMODE_NUM, kShowModeStrings);
    modes->SetSelection(0);
    topsizer->Add(modes, LeftBasicSizerFlags());

    wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, LeftBasicSizerFlags());

    auto refresh_action = [this](wxCommandEvent&) {
        this->TransferDataFromWindow();
        Refresh();
    };

    AddTextboxWithCaptionAndAction(this, sizer1, WESTHASH, wxT("West Hash"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, EASTHASH, wxT("East Hash"),
        refresh_action, wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, LeftBasicSizerFlags());
    AddTextboxWithCaptionAndAction(this, sizer1, BORDER_LEFT, wxT("Left Border"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, BORDER_TOP, wxT("Top Border"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, BORDER_RIGHT,
        wxT("Right Border"), refresh_action,
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, BORDER_BOTTOM,
        wxT("Bottom Border"), refresh_action,
        wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, LeftBasicSizerFlags());
    AddTextboxWithCaptionAndAction(this, sizer1, OFFSET_X, wxT("Offset X"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, OFFSET_Y, wxT("Offset Y"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, SIZE_X, wxT("Size X"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, SIZE_Y, wxT("Size Y"),
        refresh_action, wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, LeftBasicSizerFlags());
    wxBoxSizer* textsizer = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(textsizer, BasicSizerFlags());
    textsizer->Add(new wxStaticText(this, wxID_STATIC,
                       wxT("Adjust yardline marker"),
                       wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    wxChoice* textchoice = new wxChoice(this, SHOW_LINE_MARKING, wxDefaultPosition, wxDefaultSize,
        wxArrayString{ mConfig.Get_yard_text_index().size(), mConfig.Get_yard_text_index().data() });
    textchoice->SetSelection(0);
    textsizer->Add(textchoice);
    auto show_line_value = new wxTextCtrl(this, SHOW_LINE_VALUE, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, wxTE_PROCESS_ENTER);
    show_line_value->Bind(wxEVT_TEXT_ENTER, refresh_action);
    textsizer->Add(show_line_value, BasicSizerFlags());

    textsizer = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(textsizer, BasicSizerFlags());
    textsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Zoom"),
                       wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    wxArrayString zoomtext;
    for (auto& i : zoom_amounts) {
        wxString buf;
        buf.sprintf(wxT("%d%%"), i);
        zoomtext.Add(buf);
    }
    auto zoomBox = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, zoomtext);
    zoomBox->Bind(wxEVT_CHOICE, [=](wxCommandEvent& event) {
        size_t sel = event.GetInt();
        float zoom_amount = zoom_amounts[sel] / 100.0;
        static_cast<ShowModeSetupCanvas*>(FindWindow(CANVAS))
            ->SetZoom(zoom_amount);
    });

    // set the text to the default zoom level
    textsizer->Add(zoomBox, LeftBasicSizerFlags());

    auto modeSetupCanvas = new ShowModeSetupCanvas(mConfig, this, CANVAS);
    modeSetupCanvas->SetScrollRate(1, 1);
    topsizer->Add(modeSetupCanvas, 1, wxEXPAND);

    modeSetupCanvas->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(mWhichMode)), mYardText));
    modeSetupCanvas->SetZoom(zoom_amounts[5] / 100.0);
    zoomBox->SetSelection(5);

    TransferDataToWindow();
}

void ShowModeSetup::Init()
{
    mWhichMode = 0;
    mWhichYardLine = 0;
    for (size_t i = 0; i < SHOWMODE_NUM; ++i) {
        mShowModeValues[i] = mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(i));
    }
    for (size_t i = 0; i < kYardTextValues; ++i) {
        mYardText[i] = mConfig.Get_yard_text(i);
    }
}

bool ShowModeSetup::TransferDataToWindow()
{
    // standard show
    for (auto i = mShowModeValues[mWhichMode].begin();
         i != mShowModeValues[mWhichMode].end(); ++i) {
        wxString buf;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            WESTHASH + std::distance(mShowModeValues[mWhichMode].begin(), i));
        buf.Printf(wxT("%d"), static_cast<int>(*i));
        text->ChangeValue(buf);
    }

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    text->SetValue(mYardText[mWhichYardLine]);
    return true;
}

bool ShowModeSetup::TransferDataFromWindow()
{
    // read out the values from the window
    // standard show
    for (auto i = mShowModeValues[mWhichMode].begin();
         i != mShowModeValues[mWhichMode].end(); ++i) {
        long val;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            WESTHASH + std::distance(mShowModeValues[mWhichMode].begin(), i));
        text->GetValue().ToLong(&val);
        *i = val;
    }
    // write out the values defaults:
    for (size_t i = 0; i < SHOWMODE_NUM; ++i) {
        mConfig.Set_ShowModeInfo(static_cast<CalChartShowModes>(i),
            mShowModeValues[i]);
    }

    // grab whatever's in the box
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();
    for (size_t i = 0; i < kYardTextValues; ++i) {
        mConfig.Set_yard_text(i, mYardText[i]);
    }
    // now set the canvas
    ((ShowModeSetupCanvas*)FindWindow(CANVAS))->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(mWhichMode)), mYardText));

    return true;
}

bool ShowModeSetup::ClearValuesToDefault()
{
    for (size_t i = 0; i < SHOWMODE_NUM; ++i) {
        mConfig.Clear_ShowModeInfo(static_cast<CalChartShowModes>(i));
    }
    for (auto i = 0; i < kYardTextValues; ++i) {
        mConfig.Clear_yard_text(i);
    }
    Init();
    ((ShowModeSetupCanvas*)FindWindow(CANVAS))->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(mWhichMode)), mYardText));
    return TransferDataToWindow();
}

void ShowModeSetup::OnCmdChoice(wxCommandEvent&)
{
    // save off all the old values:
    for (auto i = mShowModeValues[mWhichMode].begin();
         i != mShowModeValues[mWhichMode].end(); ++i) {
        long val;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            WESTHASH + std::distance(mShowModeValues[mWhichMode].begin(), i));
        text->GetValue().ToLong(&val);
        *i = val;
    }
    wxChoice* modes = (wxChoice*)FindWindow(MODE_CHOICE);
    mWhichMode = modes->GetSelection();
    ShowModeSetupCanvas* canvas = (ShowModeSetupCanvas*)FindWindow(CANVAS);
    canvas->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(mWhichMode)), mYardText));

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();
    // update mode
    for (size_t i = 0; i < kYardTextValues; ++i) {
        mConfig.Set_yard_text(i, mYardText[i]);
    }

    modes = (wxChoice*)FindWindow(SHOW_LINE_MARKING);
    mWhichYardLine = modes->GetSelection();
    TransferDataToWindow();
}

