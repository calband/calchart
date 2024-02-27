/*
 * mode_dialog.cpp
 * Dialox box for preferences
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

#include "ModeSetupDialog.h"
#include "CalChartConfiguration.h"
#include "CalChartRanges.h"
#include "CalChartShowMode.h"
#include "ShowModeSetupCanvas.h"
#include "basic_ui.h"
#include <algorithm>
#include <ranges>
#include <wxUI/wxUI.h>

//////// Show Mode setup ////////
// setup drawing characteristics
////////

constexpr int zoom_amounts[] = { 500, 200, 150, 100, 75, 50, 25, 10 };
constexpr auto defaultZoom = 3;

class ShowModeDialogSetup : public wxPanel {
    DECLARE_CLASS(ShowModeDialogSetup)
    DECLARE_EVENT_TABLE()

public:
    static auto CreateDialog(wxWindow* parent, CalChart::ShowMode const& current_mode, CalChart::Configuration& config)
    {
        auto dialog = new ShowModeDialogSetup{ parent, current_mode, config };
        dialog->TransferDataToWindow();
        return dialog;
    }

private:
    // private, use the CreateDialog method
    ShowModeDialogSetup(wxWindow* parent, CalChart::ShowMode const& current_mode, CalChart::Configuration& config);

public:
    ~ShowModeDialogSetup() = default;

    void CreateControls();

    // use these to get and set default values
    auto TransferDataToWindow() -> bool override;
    auto TransferDataFromWindow() -> bool override;

    auto GetShowMode() const -> CalChart::ShowMode;

private:
    void OnCmdLineText(wxCommandEvent&);
    void OnCmdChoice();

    CalChart::ShowModeData_t mShowModeValues;
    CalChart::YardLinesInfo_t mYardText;
    int mWhichYardLine{};
    CalChart::Configuration const& mConfig;
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

ShowModeDialogSetup::ShowModeDialogSetup(wxWindow* parent, CalChart::ShowMode const& current_mode, CalChart::Configuration& config)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU, "Setup Modes")
    , mShowModeValues{ current_mode.GetShowModeData() }
    , mYardText{ current_mode.Get_yard_text() }
    , mWhichYardLine{ 0 }
    , mConfig(config)
{
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Center();
}

template <std::ranges::input_range Range>
    requires(std::is_convertible_v<std::ranges::range_value_t<Range>, wxString>)
auto convert(Range&& input)
{
    return CalChart::Ranges::ToVector<wxString>(input);
}

void ShowModeDialogSetup::CreateControls()
{
    auto choices = convert(CalChart::GetShowModeNames());
    choices.insert(choices.begin(), "Reload from...");

    auto refresh_action = [this]() {
        this->TransferDataFromWindow();
        Refresh();
    };
    std::vector<wxString> zoomtext;
    std::ranges::transform(zoom_amounts, std::back_inserter(zoomtext), [](auto amount) { return std::to_string(amount) + "%"; });

    wxUI::VSizer{
        LeftBasicSizerFlags(),
        wxUI::Choice{ MODE_CHOICE, choices }
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
                wxUI::Choice{ SHOW_LINE_MARKING, convert(CalChart::kDefaultYardLines) }
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
                modeSetupCanvas->SetMode(CalChart::ShowMode::CreateShowMode(mShowModeValues, mYardText));
                modeSetupCanvas->SetZoom(zoom_amounts[defaultZoom] / 100.0);
                return modeSetupCanvas;
            }() },
    }
        .attachTo(this);
}

bool ShowModeDialogSetup::TransferDataToWindow()
{
    // standard show
    for (auto i = mShowModeValues.begin(); i != mShowModeValues.end(); ++i) {
        static_cast<wxTextCtrl*>(FindWindow(WESTHASH + std::distance(mShowModeValues.begin(), i)))->ChangeValue(std::to_string(*i));
    }

    static_cast<wxTextCtrl*>(FindWindow(SHOW_LINE_VALUE))->SetValue(std::string{ mYardText[mWhichYardLine] });
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
        mShowModeValues = mConfig.Get_ShowModeData(static_cast<CalChart::ShowModes>(whichMode - 1));
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

ModeSetupDialog::ModeSetupDialog(wxWindow* parent, CalChart::ShowMode const& current_mode, CalChart::Configuration& config)
    : super(parent, wxID_ANY, "CalChart Preferences", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
{
    wxUI::VSizer{
        BasicSizerFlags(),
        m_setup = [current_mode, &config](wxWindow* parent) {
            return ShowModeDialogSetup::CreateDialog(parent, current_mode, config);
        },
        wxUI::HSizer{
            wxUI::Button{ wxID_CANCEL },
            wxUI::Button{ wxID_OK },
        },
    }
        .attachTo(this);
    Center();
}

CalChart::ShowMode ModeSetupDialog::GetShowMode() const
{
    return m_setup->GetShowMode();
}
