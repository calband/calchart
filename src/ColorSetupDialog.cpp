/*
 * ColorSetupDialog.cpp
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

#include "ColorSetupDialog.h"
#include "CalChartConfiguration.h"
#include "CalChartDrawPrimativesHelper.h"
#include "ColorSetupCanvas.h"

#include <fstream>
#include <iomanip>
#include <wx/colordlg.h>
#include <wxUI/wxUI.hpp>

using namespace CalChart;

// how the preferences work:
// preference dialog create a copy of the CalChart config from which to read and set values
// CalChart config doesn't automatically write values to main config, it must be flushed
// out when the user presses apply.

//////// Draw setup ////////
// handling Drawing colors
////////

enum {
    BUTTON_SELECT = 1000,
    BUTTON_RESTORE,
    SPIN_WIDTH,
    NEW_COLOR_CHOICE,
    PALETTE_NAME,
    BUTTON_EDIT_PALETTE_COLOR,
    BUTTON_IMPORT,
    BUTTON_EXPORT,
};

BEGIN_EVENT_TABLE(ColorSetupDialog, wxDialog)
EVT_BUTTON(BUTTON_SELECT, ColorSetupDialog::OnCmdSelectColors)
EVT_BUTTON(BUTTON_RESTORE, ColorSetupDialog::OnCmdResetColors)
EVT_BUTTON(BUTTON_IMPORT, ColorSetupDialog::OnCmdImport)
EVT_BUTTON(BUTTON_EXPORT, ColorSetupDialog::OnCmdExport)
EVT_BUTTON(BUTTON_EDIT_PALETTE_COLOR, ColorSetupDialog::OnCmdChangePaletteColor)
EVT_BUTTON(wxID_RESET, ColorSetupDialog::OnCmdResetAll)
EVT_SPINCTRL(SPIN_WIDTH, ColorSetupDialog::OnCmdSelectWidth)
EVT_COMBOBOX(NEW_COLOR_CHOICE, ColorSetupDialog::OnCmdChooseNewColor)
EVT_TEXT_ENTER(PALETTE_NAME, ColorSetupDialog::OnCmdTextChanged)
END_EVENT_TABLE()

ColorSetupDialog::ColorSetupDialog(wxWindow* parent, int palette, CalChart::Configuration& config)
    : ColorSetupDialog::super(parent, wxID_ANY, "Color Setup")
    , mActiveColorPalette(palette)
    , mConfig(config)
{
    Init();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
}

auto CreateTempBitmap(CalChart::Color c)
{
    wxBitmap temp_bitmap(GetColorBoxSize());
    wxMemoryDC temp_dc;
    temp_dc.SelectObject(temp_bitmap);
    wxCalChart::setBackground(temp_dc, c);
    temp_dc.Clear();
    return temp_bitmap;
}

void ColorSetupDialog::CreateControls()
{
    auto colorNames = std::vector<std::tuple<wxString, wxBitmap>>{};
    for (auto i : CalChart::ColorsIterator{}) {
        colorNames.push_back({ CalChart::GetColorNames().at(toUType(i)), CreateItemBitmap(wxCalChart::toBrush(mConfig.Get_CalChartBrushAndPen(i))) });
    }
    wxUI::VSizer{
        LeftBasicSizerFlags(),
        wxUI::HSizer{
            "Palette",
            wxUI::BitmapButton{ BUTTON_EDIT_PALETTE_COLOR, CreateTempBitmap(mColorPaletteColors.at(mActiveColorPalette)) },
            wxUI::TextCtrl(
                PALETTE_NAME, mColorPaletteNames.at(mActiveColorPalette))
                .withStyle(wxTE_MULTILINE)
                .withSize({ 100, 24 }),
            wxUI::Button{ BUTTON_EXPORT, "&Export..." },
            wxUI::Button{ BUTTON_IMPORT, "&Import..." },
        },

        wxUI::VSizer{
            "Color settings",
            wxUI::HSizer{
                wxUI::BitmapComboBox(NEW_COLOR_CHOICE, colorNames)
                    .withSelection(0)
                    .withSize({ 200, -1 })
                    .withStyle(wxCB_READONLY | wxCB_DROPDOWN)
                    .withProxy(mNameBox),
                wxUI::SpinCtrl(SPIN_WIDTH, std::pair{ 1, 10 }, mCalChartPens[mActiveColorPalette][0].GetWidth())
                    .withStyle(wxSP_ARROW_KEYS)
                    .withProxy(mSpin),
            },

            wxUI::HSizer{
                wxUI::Button(BUTTON_SELECT, "&Change Color"),
                wxUI::Button(BUTTON_IMPORT, "&Import..."),
            },
        },
        wxUI::Generic{ ExpandSizerFlags(), new ColorSetupCanvas(mConfig, this) },
        wxUI::HSizer{
            BasicSizerFlags(),
            wxUI::Button{ wxID_RESET, "&Reset" },
            wxUI::Button{ wxID_OK },
            wxUI::Button{ wxID_CANCEL },

        }
    }
        .fitTo(this);
}

void ColorSetupDialog::Init()
{
    // first read out the defaults:
    mColorPaletteNames = GetColorPaletteNames(mConfig);
    mColorPaletteColors = GetColorPaletteColors(mConfig);

    for (auto palette = 0; palette < CalChart::kNumberPalettes; ++palette) {
        for (auto i = CalChart::Colors::FIELD; i != CalChart::Colors::NUM; i = static_cast<CalChart::Colors>(static_cast<int>(i) + 1)) {
            auto brushAndPen = mConfig.Get_CalChartBrushAndPen(palette, i);
            mCalChartPens[palette][toUType(i)] = wxCalChart::toPen(brushAndPen);
            mCalChartBrushes[palette][toUType(i)] = wxCalChart::toBrush(brushAndPen);
        }
    }
}

bool ColorSetupDialog::TransferDataToWindow()
{
    auto button = static_cast<wxBitmapButton*>(FindWindow(BUTTON_EDIT_PALETTE_COLOR));
    button->SetBitmap(CreateTempBitmap(mColorPaletteColors.at(mActiveColorPalette)));

    auto text = static_cast<wxTextCtrl*>(FindWindow(PALETTE_NAME));
    text->SetValue(mColorPaletteNames.at(mActiveColorPalette));

    for (auto i = CalChart::Colors::FIELD; i != CalChart::Colors::NUM; i = static_cast<CalChart::Colors>(static_cast<int>(i) + 1)) {
        CreateAndSetItemBitmap(mNameBox.control(), toUType(i), mCalChartBrushes[mActiveColorPalette][toUType(i)]);
    }
    *mSpin = mCalChartPens[mActiveColorPalette][mNameBox.selection()].GetWidth();

    return true;
}

bool ColorSetupDialog::TransferDataFromWindow()
{
    auto text = static_cast<wxTextCtrl*>(FindWindow(PALETTE_NAME));
    mConfig.SetColorPaletteName(mActiveColorPalette, text->GetValue());
    return true;
}

bool ColorSetupDialog::ClearValuesToDefault()
{
    mConfig.ClearColorPaletteColor(mActiveColorPalette);
    mConfig.ClearColorPaletteName(mActiveColorPalette);

    auto text = static_cast<wxTextCtrl*>(FindWindow(PALETTE_NAME));
    text->SetValue(mColorPaletteNames.at(mActiveColorPalette));
    for (auto i = CalChart::Colors::FIELD; i != CalChart::Colors::NUM; i = static_cast<CalChart::Colors>(static_cast<int>(i) + 1)) {
        SetColor(toUType(i), CalChart::GetDefaultPenWidth()[toUType(i)], wxColour{ CalChart::GetDefaultColors()[toUType(i)] });
        mConfig.Clear_CalChartConfigColor(mActiveColorPalette, i);
    }
    return true;
}

void ColorSetupDialog::SetColor(int selection, int width, wxColour const& color)
{
    mCalChartPens[mActiveColorPalette][selection] = *wxThePenList->FindOrCreatePen(color, width, wxPENSTYLE_SOLID);
    mCalChartBrushes[mActiveColorPalette][selection] = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

    // this is needed so we draw things out on the page correctly.
    mConfig.Set_CalChartBrushAndPen(mActiveColorPalette, static_cast<CalChart::Colors>(selection), wxCalChart::toBrushAndPen(color, width));

    CreateAndSetItemBitmap(mNameBox.control(), selection, mCalChartBrushes[mActiveColorPalette][selection]);
    Refresh();
}

void ColorSetupDialog::SetPaletteColor(const wxColour& color)
{
    mColorPaletteColors.at(mActiveColorPalette) = wxCalChart::toColor(color);

    // this is needed so we draw things out on the page correctly.
    mConfig.SetColorPaletteColor(mActiveColorPalette, mColorPaletteColors.at(mActiveColorPalette));

    auto button = static_cast<wxBitmapButton*>(FindWindow(BUTTON_EDIT_PALETTE_COLOR));
    button->SetBitmap(CreateTempBitmap(mColorPaletteColors.at(mActiveColorPalette)));
    Refresh();
}

void ColorSetupDialog::SetPaletteName(const wxString& name)
{
    mColorPaletteNames.at(mActiveColorPalette) = name;

    // this is needed so we draw things out on the page correctly.
    mConfig.SetColorPaletteName(mActiveColorPalette, name);

    auto text = static_cast<wxTextCtrl*>(FindWindow(PALETTE_NAME));
    text->SetValue(mColorPaletteNames.at(mActiveColorPalette));
    Refresh();
}

void ColorSetupDialog::OnCmdSelectColors(wxCommandEvent&)
{
    int selection = mNameBox.selection();
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(mCalChartBrushes[mActiveColorPalette][selection].GetColour());
    wxColourDialog dialog(this, &data);
    if (dialog.ShowModal() == wxID_OK) {
        wxColourData retdata = dialog.GetColourData();
        wxColour c = retdata.GetColour();
        SetColor(selection, mCalChartPens[mActiveColorPalette][selection].GetWidth(), c);
    }
}

void ColorSetupDialog::OnCmdChangePaletteColor(wxCommandEvent&)
{
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(wxCalChart::toColour(mColorPaletteColors.at(mActiveColorPalette)));
    wxColourDialog dialog(this, &data);
    if (dialog.ShowModal() == wxID_OK) {
        wxColourData retdata = dialog.GetColourData();
        wxColour c = retdata.GetColour();

        SetPaletteColor(c);
    }
    Refresh();
}

void ColorSetupDialog::OnCmdSelectWidth(wxSpinEvent& e)
{
    auto selection = static_cast<int>(mNameBox.selection());
    SetColor(selection, e.GetPosition(), mCalChartPens[mActiveColorPalette][selection].GetColour());
}

void ColorSetupDialog::OnCmdResetColors(wxCommandEvent&)
{
    auto selection = static_cast<int>(mNameBox.selection());
    SetColor(selection, CalChart::GetDefaultPenWidth()[selection], wxColour{ CalChart::GetDefaultColors()[selection] });
    mConfig.Clear_CalChartConfigColor(mActiveColorPalette, static_cast<CalChart::Colors>(selection));
}

void ColorSetupDialog::OnCmdChooseNewColor(wxCommandEvent&)
{
    *mSpin = mCalChartPens[mActiveColorPalette][mNameBox.selection()].GetWidth();
}

void ColorSetupDialog::OnCmdTextChanged(wxCommandEvent& e)
{
    auto id = e.GetId();
    if (id == PALETTE_NAME) {
        auto text = static_cast<wxTextCtrl*>(FindWindow(id));
        mColorPaletteNames.at(mActiveColorPalette) = text->GetValue();
        mConfig.SetColorPaletteName(mActiveColorPalette, text->GetValue());
    }
    Refresh();
}

using RGB_t = std::array<int, 3>;
constexpr auto kColorPaletteVersion = "Version";
constexpr auto kPaletteName = "PaletteName";
constexpr auto kPaletteColor = "PaletteColor";
constexpr auto kFieldColors = "FieldColors";
constexpr auto kFieldColorsWidth = "FieldColorsWidth";

auto ColourToRGB(wxColour const& c)
{
    return RGB_t{ c.Red(), c.Green(), c.Blue() };
}

auto RGBToColour(RGB_t const& b)
{
    return wxColour{ static_cast<wxColour::ChannelType>(b[0]), static_cast<wxColour::ChannelType>(b[1]), static_cast<wxColour::ChannelType>(b[2]) };
}

nlohmann::json ColorSetupDialog::Export() const
{
    using json = nlohmann::json;
    json j;
    j[kColorPaletteVersion] = 1;
    j[kPaletteName] = mColorPaletteNames.at(mActiveColorPalette);
    j[kPaletteColor] = ColourToRGB(wxCalChart::toColour(mColorPaletteColors.at(mActiveColorPalette)));
    std::vector<RGB_t> allColors;
    std::transform(std::begin(mCalChartBrushes[mActiveColorPalette]), std::end(mCalChartBrushes[mActiveColorPalette]), std::back_inserter(allColors), [](auto&& i) { return ColourToRGB(i.GetColour()); });
    j[kFieldColors] = allColors;
    std::vector<int> allWidths;
    std::transform(std::begin(mCalChartPens[mActiveColorPalette]), std::end(mCalChartPens[mActiveColorPalette]), std::back_inserter(allWidths), [](auto&& i) { return i.GetWidth(); });
    j[kFieldColorsWidth] = allWidths;

    return j;
}

void ColorSetupDialog::Import(nlohmann::json const& j)
{
    // Read all values and after they are read, then write them out.
    wxString newName{};
    wxColor newColor{};
    wxColor newFieldColors[toUType(CalChart::Colors::NUM)] = {};
    int newFieldColorsWidths[toUType(CalChart::Colors::NUM)] = {};
    try {
        if (j.find(kPaletteName) != j.end()) {
            newName = j.at(kPaletteName).get<std::string>();
        }
        if (j.find(kPaletteColor) != j.end()) {
            newColor = RGBToColour(j.at(kPaletteColor).get<RGB_t>());
        }
        if (j.find(kFieldColors) != j.end()) {
            int selection = 0;
            for (auto&& i : j.at(kFieldColors).get<std::vector<RGB_t>>()) {
                newFieldColors[selection] = RGBToColour(i);
                ++selection;
            }
            if (selection != toUType(CalChart::Colors::NUM)) {
                throw std::runtime_error("Did not find enough colors widths");
            }
        }
        if (j.find(kFieldColorsWidth) != j.end()) {
            int selection = 0;
            for (auto&& i : j.at(kFieldColorsWidth).get<std::vector<int>>()) {
                newFieldColorsWidths[selection] = i;
                ++selection;
            }
            if (selection != toUType(CalChart::Colors::NUM)) {
                throw std::runtime_error("Did not find enough pen widths");
            }
        }
    } catch (std::exception const& e) {
        wxMessageBox(std::string("Error importing Color Palette:\n") + e.what(), wxT("Color Palette"), wxOK);
        return;
    }
    SetPaletteName(newName);
    SetPaletteColor(newColor);
    for (auto i = CalChart::Colors::FIELD; i != CalChart::Colors::NUM; i = static_cast<CalChart::Colors>(static_cast<int>(i) + 1)) {
        SetColor(toUType(i), newFieldColorsWidths[toUType(i)], newFieldColors[toUType(i)]);
    }
}

void ColorSetupDialog::OnCmdExport(wxCommandEvent&)
{
    wxString s = wxFileSelector("Export CalChart Color Palette", wxEmptyString,
        wxEmptyString, wxEmptyString, "calchart color palette (*.ccpalette)|*.ccpalette", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (s.IsEmpty())
        return;
    auto j = Export();
    auto o = std::ofstream(s.ToStdString());
    o << std::setw(4) << j << std::endl;
}

void ColorSetupDialog::OnCmdImport(wxCommandEvent&)
{
    wxString s = wxFileSelector(wxT("Import CalChart Color Palette"), wxEmptyString,
        wxEmptyString, wxEmptyString, "calchart color palette (*.ccpalette)|*.ccpalette");
    if (s.IsEmpty())
        return;
    auto j = nlohmann::json{};
    auto input = std::ifstream(s.ToStdString());
    input >> j;
    Import(j);
    Refresh();
}

void ColorSetupDialog::OnCmdResetAll(wxCommandEvent&)
{
    // transfer everything to the config...
    ClearValuesToDefault();
    Init();
    TransferDataToWindow();
    Refresh();
}
