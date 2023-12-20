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
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartDrawCommand.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "CalChartShowMode.h"
#include "CalChartSizes.h"
#include "ColorSetupCanvas.h"
#include "ModeSetupDialog.h"
#include "PreferencesUtils.h"
#include "ShowModeSetupCanvas.h"

#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wxUI/wxUI.h>

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

IMPLEMENT_CLASS(ShowModeSetup, PreferencePage)

auto convert(std::vector<std::string> const& input)
{
    std::vector<wxString> output;
    std::copy(input.begin(), input.end(), std::back_inserter(output));
    return output;
}

void ShowModeSetup::CreateControls()
{
    wxArrayString zoomtext;
    for (auto& i : zoom_amounts) {
        wxString buf;
        buf.sprintf(wxT("%d%%"), i);
        zoomtext.Add(buf);
    }
    auto refresh_action = [this] {
        TransferDataFromWindow();
        Refresh();
    };
    wxUI::VSizer{
        LeftBasicSizerFlags(),
        wxUI::Choice{ MODE_CHOICE, convert(CalChart::GetShowModeNames()) }
            .bind([this] {
                OnCmdChoice();
            }),
        wxUI::HSizer{
            VLabelWidget("West Hash", wxUI::TextCtrl{ WESTHASH }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
            VLabelWidget("East Hash", wxUI::TextCtrl{ EASTHASH }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
        },
        wxUI::HSizer{
            VLabelWidget("Left Border", wxUI::TextCtrl{ BORDER_LEFT }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
            VLabelWidget("Top Border", wxUI::TextCtrl{ BORDER_TOP }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
            VLabelWidget("Right Border", wxUI::TextCtrl{ BORDER_RIGHT }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
            VLabelWidget("Bottom Border", wxUI::TextCtrl{ BORDER_BOTTOM }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
        },
        wxUI::HSizer{
            VLabelWidget("Offset X", wxUI::TextCtrl{ OFFSET_X }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
            VLabelWidget("Offset Y", wxUI::TextCtrl{ OFFSET_Y }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
            VLabelWidget("Size X", wxUI::TextCtrl{ SIZE_X }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
            VLabelWidget("Size Y", wxUI::TextCtrl{ SIZE_Y }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER).bind(refresh_action)),
        },
        wxUI::HSizer{
            LeftBasicSizerFlags(),
            wxUI::HSizer{
                BasicSizerFlags(),
                wxUI::Text{ "Adjust yardline marker" },
                wxUI::Choice{ SHOW_LINE_MARKING, convert(mConfig.Get_yard_text_index()) }
                    .bind([this] {
                        OnCmdChoice();
                    }),
                wxUI::TextCtrl{ SHOW_LINE_VALUE }
                    .withSize({ 100, -1 })
                    .withStyle(wxTE_PROCESS_ENTER)
                    .bind(refresh_action),
            },
            wxUI::HSizer{
                BasicSizerFlags(),
                wxUI::Text{ "Zoom" },
                wxUI::Choice{ zoomtext }
                    .withSelection(defaultZoom)
                    .bind([this](wxCommandEvent& e) {
                        auto sel = e.GetInt();
                        auto zoom_amount = zoom_amounts[sel] / 100.0;
                        static_cast<ShowModeSetupCanvas*>(FindWindow(CANVAS))->SetZoom(zoom_amount);
                    }),
            },

        },
        wxUI::Generic{
            ExpandSizerFlags(), [this] {
                auto modeSetupCanvas = new ShowModeSetupCanvas(mConfig, this, CANVAS);
                modeSetupCanvas->SetScrollRate(1, 1);

                modeSetupCanvas->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeData(static_cast<CalChart::ShowModes>(mWhichMode)), mYardText));
                modeSetupCanvas->SetZoom(zoom_amounts[defaultZoom] / 100.0);
                return modeSetupCanvas;
            }() },
    }
        .attachTo(this);

    TransferDataToWindow();
}

void ShowModeSetup::InitFromConfig()
{
    mWhichMode = 0;
    mWhichYardLine = 0;
    for (size_t i = 0; i < toUType(CalChart::ShowModes::NUM); ++i) {
        mShowModeValues[i] = mConfig.Get_ShowModeData(static_cast<CalChart::ShowModes>(i));
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
    text->ChangeValue(mYardText[mWhichYardLine]);
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
    for (size_t i = 0; i < toUType(CalChart::ShowModes::NUM); ++i) {
        mConfig.Set_ShowModeData(static_cast<CalChart::ShowModes>(i),
            mShowModeValues[i]);
    }

    // grab whatever's in the box
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();
    for (size_t i = 0; i < kYardTextValues; ++i) {
        mConfig.Set_yard_text(i, mYardText[i]);
    }
    // now set the canvas
    ((ShowModeSetupCanvas*)FindWindow(CANVAS))->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeData(static_cast<CalChart::ShowModes>(mWhichMode)), mYardText));

    return true;
}

bool ShowModeSetup::ClearValuesToDefault()
{
    for (size_t i = 0; i < toUType(CalChart::ShowModes::NUM); ++i) {
        mConfig.Clear_ShowModeData(static_cast<CalChart::ShowModes>(i));
    }
    for (auto i = 0; i < kYardTextValues; ++i) {
        mConfig.Clear_yard_text(i);
    }
    InitFromConfig();
    ((ShowModeSetupCanvas*)FindWindow(CANVAS))->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeData(static_cast<CalChart::ShowModes>(mWhichMode)), mYardText));
    return TransferDataToWindow();
}

void ShowModeSetup::OnCmdChoice()
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
    canvas->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeData(static_cast<CalChart::ShowModes>(mWhichMode)), mYardText));

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
