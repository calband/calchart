/*
 * cc_preferences_ui.cpp
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

#include "ColorSetupDialog.h"
#include "CalChartDoc.h"
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

#include <wx/bmpbuttn.h>
#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include <fstream>
#include <iomanip>

using namespace CalChart;

// how the preferences work:
// preference dialog create a copy of the CalChart config from which to read and
// set values
// CalChart config doesn't automatically write values to main config, it must be
// flushed
// out when the user presses apply.

// convience sizers to change the view behavior in all at once.
static auto sBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Center().Proportion(0);
static auto sLeftBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Left().Proportion(0);
static auto sRightBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Right().Proportion(0);
static auto sExpandSizerFlags = wxSizerFlags().Border(wxALL, 2).Center().Proportion(0);

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

ColorSetupDialog::ColorSetupDialog(wxWindow* parent,
    wxWindowID id,
    int palette,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style)
    : ColorSetupDialog::super(parent, id, caption)
    , mActiveColorPalette(palette)
    , mConfig(CalChartConfiguration::GetGlobalConfig())
{
    Init();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
}

template <typename Color>
auto CreateTempBitmap(Color const& c)
{
    wxBitmap temp_bitmap(GetColorBoxSize());
    wxMemoryDC temp_dc;
    temp_dc.SelectObject(temp_bitmap);
    temp_dc.SetBackground(c);
    temp_dc.Clear();
    return temp_bitmap;
}

void ColorSetupDialog::CreateControls()
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    wxStaticBoxSizer* boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Palette")), wxHORIZONTAL);
    topsizer->Add(boxsizer);

    boxsizer->Add(new wxBitmapButton(this, BUTTON_EDIT_PALETTE_COLOR, CreateTempBitmap(mColorPaletteColors.at(mActiveColorPalette))), sBasicSizerFlags);

    boxsizer->Add(new wxTextCtrl(this, PALETTE_NAME, mColorPaletteNames.at(mActiveColorPalette), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), sBasicSizerFlags);

    boxsizer->Add(new wxButton(this, BUTTON_EXPORT, wxT("&Export...")), sBasicSizerFlags);
    boxsizer->Add(new wxButton(this, BUTTON_IMPORT, wxT("&Import...")), sBasicSizerFlags);

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Color settings")), wxVERTICAL);
    topsizer->Add(boxsizer);

    auto horizontalsizer = new wxBoxSizer(wxHORIZONTAL);

    nameBox = new wxBitmapComboBox(this, NEW_COLOR_CHOICE, mConfig.GetColorNames().at(0), wxDefaultPosition, wxDefaultSize, COLOR_NUM, mConfig.GetColorNames().data(), wxCB_READONLY | wxCB_DROPDOWN);
    horizontalsizer->Add(nameBox, sBasicSizerFlags);

    for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
        CreateAndSetItemBitmap(nameBox, i, mConfig.Get_CalChartBrushAndPen(i).first);
    }
    nameBox->SetSelection(0);

    spin = new wxSpinCtrl(this, SPIN_WIDTH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mCalChartPens[mActiveColorPalette][nameBox->GetSelection()].GetWidth());
    spin->SetValue(mCalChartPens[mActiveColorPalette][nameBox->GetSelection()].GetWidth());
    horizontalsizer->Add(spin, sBasicSizerFlags);
    boxsizer->Add(horizontalsizer, sLeftBasicSizerFlags);

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);

    horizontalsizer->Add(new wxButton(this, BUTTON_SELECT, wxT("&Change Color")), sBasicSizerFlags);
    horizontalsizer->Add(new wxButton(this, BUTTON_RESTORE, wxT("&Reset Color")), sBasicSizerFlags);
    boxsizer->Add(horizontalsizer, sBasicSizerFlags);

    auto prefCanvas = new ColorSetupCanvas(mConfig, this);
    // set scroll rate 1 to 1, so we can have even scrolling of whole field
    topsizer->Add(prefCanvas, 1, wxEXPAND);
    //	mCanvas->SetScrollRate(1, 1);

    // the buttons on the bottom
    wxBoxSizer* okCancelBox = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(okCancelBox, sBasicSizerFlags);

    okCancelBox->Add(new wxButton(this, wxID_RESET, wxT("&Reset")), sBasicSizerFlags);
    okCancelBox->Add(new wxButton(this, wxID_OK), sBasicSizerFlags);
    okCancelBox->Add(new wxButton(this, wxID_CANCEL), sBasicSizerFlags);

    TransferDataToWindow();
}

void ColorSetupDialog::Init()
{
    // first read out the defaults:
    mColorPaletteNames = GetColorPaletteNames(mConfig);
    mColorPaletteColors = GetColorPaletteColors(mConfig);

    for (auto palette = 0; palette < kNumberPalettes; ++palette) {
        for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
            auto brushAndPen = mConfig.Get_CalChartBrushAndPen(palette, i);
            mCalChartPens[palette][i] = brushAndPen.second;
            mCalChartBrushes[palette][i] = brushAndPen.first;
        }
    }
}

bool ColorSetupDialog::TransferDataToWindow()
{
    auto button = static_cast<wxBitmapButton*>(FindWindow(BUTTON_EDIT_PALETTE_COLOR));
    button->SetBitmap(CreateTempBitmap(mColorPaletteColors.at(mActiveColorPalette)));

    auto text = static_cast<wxTextCtrl*>(FindWindow(PALETTE_NAME));
    text->SetValue(mColorPaletteNames.at(mActiveColorPalette));

    for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
        CreateAndSetItemBitmap(nameBox, i, mCalChartBrushes[mActiveColorPalette][i]);
    }
    spin->SetValue(mCalChartPens[mActiveColorPalette][nameBox->GetSelection()].GetWidth());

    return true;
}

bool ColorSetupDialog::TransferDataFromWindow()
{
    auto text = static_cast<wxTextCtrl*>(FindWindow(PALETTE_NAME));
    mConfig.SetColorPaletteName(mActiveColorPalette, text->GetValue());
    CalChartConfiguration::AssignConfig(mConfig);
    return true;
}

bool ColorSetupDialog::ClearValuesToDefault()
{
    mConfig.ClearColorPaletteColor(mActiveColorPalette);
    mConfig.ClearColorPaletteName(mActiveColorPalette);

    auto text = static_cast<wxTextCtrl*>(FindWindow(PALETTE_NAME));
    text->SetValue(mColorPaletteNames.at(mActiveColorPalette));
    for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
        SetColor(i, mConfig.GetDefaultPenWidth()[i], mConfig.GetDefaultColors()[i]);
        mConfig.Clear_CalChartConfigColor(i);
    }
    return true;
}

void ColorSetupDialog::SetColor(int selection, int width, const wxColour& color)
{
    mCalChartPens[mActiveColorPalette][selection] = *wxThePenList->FindOrCreatePen(color, width, wxPENSTYLE_SOLID);
    mCalChartBrushes[mActiveColorPalette][selection] = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

    // this is needed so we draw things out on the page correctly.
    mConfig.Set_CalChartBrushAndPen(static_cast<CalChartColors>(selection), mCalChartBrushes[mActiveColorPalette][selection], mCalChartPens[mActiveColorPalette][selection]);

    CreateAndSetItemBitmap(nameBox, selection, mCalChartBrushes[mActiveColorPalette][selection]);
    Refresh();
}

void ColorSetupDialog::SetPaletteColor(const wxColour& color)
{
    mColorPaletteColors.at(mActiveColorPalette) = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

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
    int selection = nameBox->GetSelection();
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
    data.SetColour(mColorPaletteColors.at(mActiveColorPalette).GetColour());
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
    int selection = nameBox->GetSelection();
    SetColor(selection, e.GetPosition(), mCalChartPens[mActiveColorPalette][selection].GetColour());
}

void ColorSetupDialog::OnCmdResetColors(wxCommandEvent&)
{
    int selection = nameBox->GetSelection();
    SetColor(selection, mConfig.GetDefaultPenWidth()[selection], mConfig.GetDefaultColors()[selection]);
    mConfig.Clear_CalChartConfigColor(static_cast<CalChartColors>(selection));
}

void ColorSetupDialog::OnCmdChooseNewColor(wxCommandEvent&)
{
    spin->SetValue(mCalChartPens[mActiveColorPalette][nameBox->GetSelection()].GetWidth());
}

void ColorSetupDialog::OnCmdTextChanged(wxCommandEvent& e)
{
    auto id = e.GetId();
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(id);
    if (id == PALETTE_NAME) {
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
    j[kPaletteColor] = ColourToRGB(mColorPaletteColors.at(mActiveColorPalette).GetColour());
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
    wxColor newFieldColors[COLOR_NUM] = {};
    int newFieldColorsWidths[COLOR_NUM] = {};
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
            if (selection != COLOR_NUM) {
                throw std::runtime_error("Did not find enough colors widths");
            }
        }
        if (j.find(kFieldColorsWidth) != j.end()) {
            int selection = 0;
            for (auto&& i : j.at(kFieldColorsWidth).get<std::vector<int>>()) {
                newFieldColorsWidths[selection] = i;
                ++selection;
            }
            if (selection != COLOR_NUM) {
                throw std::runtime_error("Did not find enough pen widths");
            }
        }
    } catch (std::exception const& e) {
        wxMessageBox(std::string("Error importing Color Palette:\n") + e.what(), wxT("Color Palette"), wxOK);
        return;
    }
    SetPaletteName(newName);
    SetPaletteColor(newColor);
    for (auto i = 0; i < COLOR_NUM; ++i) {
        SetColor(i, newFieldColorsWidths[i], newFieldColors[i]);
    }
}

void ColorSetupDialog::OnCmdExport(wxCommandEvent& e)
{
    wxString s = wxFileSelector("Export CalChart Color Palette", wxEmptyString,
        wxEmptyString, wxEmptyString, "calchart color palette (*.ccpalette)|*.ccpalette", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (s.IsEmpty())
        return;
    auto j = Export();
    auto o = std::ofstream(s.ToStdString());
    o << std::setw(4) << j << std::endl;
}

void ColorSetupDialog::OnCmdImport(wxCommandEvent& e)
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
    CalChartConfiguration::AssignConfig(mConfig);
    Init();
    TransferDataToWindow();
    Refresh();
}
