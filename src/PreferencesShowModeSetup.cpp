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
#include "CalChartDoc.h"
#include "CalChartSizes.h"
#include "ColorSetupCanvas.h"
#include "ContinuityBrowserPanel.h"
#include "ContinuityComposerDialog.h"
#include "PreferencesUtils.h"
#include "cc_drawcommand.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "confgr.h"
#include "CalChartContinuityToken.h"
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

IMPLEMENT_CLASS(ShowModeSetup, PreferencePage)

void ShowModeSetup::CreateControls()
{
    SetSizer(VStack([this](auto& sizer) {
        CreateChoiceWithHandler(this, sizer, MODE_CHOICE, { std::begin(kShowModeStrings), std::end(kShowModeStrings) }, [this](wxCommandEvent& e) {
            OnCmdChoice();
        });

        auto refresh_action = [this](wxCommandEvent&) {
            this->TransferDataFromWindow();
            Refresh();
        };

        HStack(sizer, LeftBasicSizerFlags(), [this, refresh_action](auto& sizer) {
            CreateTextboxWithCaptionAndAction(this, sizer, WESTHASH, "West Hash", refresh_action, wxTE_PROCESS_ENTER);
            CreateTextboxWithCaptionAndAction(this, sizer, EASTHASH, "East Hash", refresh_action, wxTE_PROCESS_ENTER);
        });

        HStack(sizer, LeftBasicSizerFlags(), [this, refresh_action](auto& sizer) {
            CreateTextboxWithCaptionAndAction(this, sizer, BORDER_LEFT, "Left Border", refresh_action, wxTE_PROCESS_ENTER);
            CreateTextboxWithCaptionAndAction(this, sizer, BORDER_TOP, "Top Border", refresh_action, wxTE_PROCESS_ENTER);
            CreateTextboxWithCaptionAndAction(this, sizer, BORDER_RIGHT, "Right Border", refresh_action, wxTE_PROCESS_ENTER);
            CreateTextboxWithCaptionAndAction(this, sizer, BORDER_BOTTOM, "Bottom Border", refresh_action, wxTE_PROCESS_ENTER);
        });

        HStack(sizer, LeftBasicSizerFlags(), [this, refresh_action](auto& sizer) {
            CreateTextboxWithCaptionAndAction(this, sizer, OFFSET_X, "Offset X", refresh_action, wxTE_PROCESS_ENTER);
            CreateTextboxWithCaptionAndAction(this, sizer, OFFSET_Y, "Offset Y", refresh_action, wxTE_PROCESS_ENTER);
            CreateTextboxWithCaptionAndAction(this, sizer, SIZE_X, "Size X", refresh_action, wxTE_PROCESS_ENTER);
            CreateTextboxWithCaptionAndAction(this, sizer, SIZE_Y, "Size Y", refresh_action, wxTE_PROCESS_ENTER);
        });

        HStack(sizer, LeftBasicSizerFlags(), [this, refresh_action](auto& sizer) {
            HStack(sizer, BasicSizerFlags(), [this, refresh_action](auto& sizer) {
                CreateText(this, sizer, "Adjust yardline marker");
                CreateChoiceWithHandler(this, sizer, SHOW_LINE_MARKING, mConfig.Get_yard_text_index(), [this](wxCommandEvent& e) {
                    OnCmdChoice();
                });
                CreateTextboxWithAction(this, sizer, SHOW_LINE_VALUE, refresh_action, wxTE_PROCESS_ENTER);
            });

            HStack(sizer, BasicSizerFlags(), [this, refresh_action](auto& sizer) {
                CreateText(this, sizer, "Zoom");

                wxArrayString zoomtext;
                for (auto& i : zoom_amounts) {
                    wxString buf;
                    buf.sprintf(wxT("%d%%"), i);
                    zoomtext.Add(buf);
                }
                auto zoomBox = CreateChoiceWithHandler(this, sizer, wxID_ANY, zoomtext, [this](wxCommandEvent& e) {
                    auto sel = e.GetInt();
                    auto zoom_amount = zoom_amounts[sel] / 100.0;
                    static_cast<ShowModeSetupCanvas*>(FindWindow(CANVAS))->SetZoom(zoom_amount);
                });
                zoomBox->SetSelection(defaultZoom);
            });
        });

        auto modeSetupCanvas = new ShowModeSetupCanvas(mConfig, this, CANVAS);
        modeSetupCanvas->SetScrollRate(1, 1);
        sizer->Add(modeSetupCanvas, 1, wxEXPAND);

        modeSetupCanvas->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(mWhichMode)), mYardText));
        modeSetupCanvas->SetZoom(zoom_amounts[defaultZoom] / 100.0);
    }));

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
