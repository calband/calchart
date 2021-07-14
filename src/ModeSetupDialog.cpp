/*
 * mode_dialog.cpp
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

#include "ModeSetupDialog.h"
#include "CalChartConfiguration.h"
#include "CalChartShowMode.h"
#include "ShowModeSetupCanvas.h"
#include "basic_ui.h"
#include "draw.h"

//////// Show Mode setup ////////
// setup drawing characteristics
////////

constexpr int zoom_amounts[] = { 500, 200, 150, 100, 75, 50, 25, 10 };
constexpr auto defaultZoom = 3;

class ShowModeDialogSetup : public wxPanel {
    DECLARE_CLASS(ShowModeDialogSetup)
    DECLARE_EVENT_TABLE()

public:
    static auto CreateDialog(CalChart::ShowMode const& current_mode, wxWindow* parent)
    {
        auto dialog = new ShowModeDialogSetup{ current_mode, parent };
        dialog->TransferDataToWindow();
        return dialog;
    }

private:
    // private, use the CreateDialog method
    ShowModeDialogSetup(CalChart::ShowMode const& current_mode, wxWindow* parent);

public:
    ~ShowModeDialogSetup() = default;

    void Init(CalChart::ShowMode const& current_mode);
    void CreateControls();

    // use these to get and set default values
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    CalChart::ShowMode GetShowMode() const;

private:
    void OnCmdLineText(wxCommandEvent&);
    void OnCmdChoice();

    CalChartConfiguration::ShowModeInfo_t mShowModeValues;
    CalChart::ShowMode::YardLinesInfo_t mYardText;
    int mWhichYardLine{};
    CalChartConfiguration mConfig;
};

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

BEGIN_EVENT_TABLE(ShowModeDialogSetup, wxPanel)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ShowModeDialogSetup, wxPanel)

ShowModeDialogSetup::ShowModeDialogSetup(CalChart::ShowMode const& current_mode, wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU, "Setup Modes")
    , mConfig(CalChartConfiguration::GetGlobalConfig())
{
    Init(current_mode);
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Center();
}

void ShowModeDialogSetup::CreateControls()
{
    SetSizer(VStack([this](auto sizer) {
        std::vector<wxString> choices;
        choices.emplace_back("Reload from...");
        choices.insert(choices.end(), std::begin(kShowModeStrings), std::end(kShowModeStrings));
        CreateChoiceWithHandler(this, sizer, LeftBasicSizerFlags(), MODE_CHOICE, choices, [this](wxCommandEvent&) {
            OnCmdChoice();
        });

        auto refresh_action = [this](wxCommandEvent&) {
            this->TransferDataFromWindow();
            Refresh();
        };
        HStack(sizer, LeftBasicSizerFlags(), [this, refresh_action](auto sizer) {
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
                CreateChoiceWithHandler(this, sizer, SHOW_LINE_MARKING, mConfig.Get_yard_text_index(), [this](wxCommandEvent&) {
                    OnCmdChoice();
                });
                CreateTextboxWithAction(this, sizer, SHOW_LINE_VALUE, refresh_action, wxTE_PROCESS_ENTER);
            });

            HStack(sizer, BasicSizerFlags(), [this](auto& sizer) {
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

        modeSetupCanvas->SetMode(CalChart::ShowMode::CreateShowMode(mShowModeValues, mYardText));
        modeSetupCanvas->SetZoom(zoom_amounts[defaultZoom] / 100.0);
    }));
}

void ShowModeDialogSetup::Init(CalChart::ShowMode const& current_mode)
{
    mWhichYardLine = 0;
    mShowModeValues = current_mode.GetShowModeInfo();
    mYardText = current_mode.Get_yard_text();
}

bool ShowModeDialogSetup::TransferDataToWindow()
{
    // standard show
    for (auto i = mShowModeValues.begin(); i != mShowModeValues.end(); ++i) {
        static_cast<wxTextCtrl*>(FindWindow(WESTHASH + std::distance(mShowModeValues.begin(), i)))->ChangeValue(std::to_string(*i));
    }

    static_cast<wxTextCtrl*>(FindWindow(SHOW_LINE_VALUE))->SetValue(mYardText[mWhichYardLine]);
    return true;
}

bool ShowModeDialogSetup::TransferDataFromWindow()
{
    // read out the values from the window
    // standard show
    for (auto i = mShowModeValues.begin(); i != mShowModeValues.end(); ++i) {
        auto text = static_cast<wxTextCtrl*>(FindWindow(WESTHASH + std::distance(mShowModeValues.begin(), i)));
        long val;
        text->GetValue().ToLong(&val);
        *i = val;
    }

    // grab whatever's in the box
    mYardText[mWhichYardLine] = static_cast<wxTextCtrl*>(FindWindow(SHOW_LINE_VALUE))->GetValue();
    static_cast<ShowModeSetupCanvas*>(FindWindow(CANVAS))->SetMode(CalChart::ShowMode::CreateShowMode(mShowModeValues, mYardText));

    return true;
}

void ShowModeDialogSetup::OnCmdChoice()
{
    // save off all the old values:
    auto modes = static_cast<wxChoice*>(FindWindow(MODE_CHOICE));
    auto whichMode = modes->GetSelection();
    if (whichMode > 0) {
        modes->SetSelection(0);
        mShowModeValues = mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(whichMode - 1));
        static_cast<ShowModeSetupCanvas*>(FindWindow(CANVAS))->SetMode(CalChart::ShowMode::CreateShowMode(mShowModeValues, mYardText));
    }

    mYardText[mWhichYardLine] = static_cast<wxTextCtrl*>(FindWindow(SHOW_LINE_VALUE))->GetValue();
    mWhichYardLine = static_cast<wxChoice*>(FindWindow(SHOW_LINE_MARKING))->GetSelection();

    TransferDataToWindow();
}

CalChart::ShowMode ShowModeDialogSetup::GetShowMode() const
{
    return CalChart::ShowMode::CreateShowMode(mShowModeValues, mYardText);
}

////////////////

BEGIN_EVENT_TABLE(ModeSetupDialog, wxDialog)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ModeSetupDialog, wxDialog)

ModeSetupDialog::ModeSetupDialog(CalChart::ShowMode const& current_mode, wxWindow* parent)
    : super(parent, wxID_ANY, "CalChart Preferences", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
{
    m_setup = ShowModeDialogSetup::CreateDialog(current_mode, this);
    SetSizer(VStack([this](auto sizer) {
        sizer->Add(m_setup, BasicSizerFlags());
        // the buttons on the bottom
        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            CreateButton(this, sizer, BasicSizerFlags(), wxID_CANCEL);
            CreateButton(this, sizer, BasicSizerFlags(), wxID_OK);
        });
    }));

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);
    Center();
}

CalChart::ShowMode ModeSetupDialog::GetShowMode() const
{
    return m_setup->GetShowMode();
}
