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

#include "mode_dialog.h"
#include "basic_ui.h"
#include "draw.h"
#include "mode_dialog_canvas.h"
#include "modes.h"

// convience sizers to change the view behavior in all at once.
static auto sBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Center().Proportion(0);
static auto sLeftBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Left().Proportion(0);
static auto sRightBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Right().Proportion(0);
static auto sExpandSizerFlags = wxSizerFlags().Border(wxALL, 2).Center().Proportion(0);

template <typename Function>
void AddTextboxWithCaptionAndAction(wxWindow* parent, wxBoxSizer* verticalsizer,
    int id, const wxString& caption,
    Function&& f, long style = 0)
{
    wxBoxSizer* textsizer = new wxBoxSizer(wxVERTICAL);
    textsizer->Add(new wxStaticText(parent, wxID_STATIC, caption,
                       wxDefaultPosition, wxDefaultSize, 0),
        sLeftBasicSizerFlags);
    auto textCtrl = new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, style);
    textCtrl->Bind((style & wxTE_PROCESS_ENTER) ? wxEVT_TEXT_ENTER : wxEVT_TEXT,
        f);
    textsizer->Add(textCtrl, sBasicSizerFlags);
    verticalsizer->Add(textsizer, sBasicSizerFlags);
}

//////// Show Mode setup ////////
// setup drawing characteristics
////////

const int zoom_amounts[] = { 500, 200, 150, 100, 75, 50, 25, 10 };

class ShowModeDialogSetup : public wxPanel {
    DECLARE_CLASS(ShowModeDialogSetup)
    DECLARE_EVENT_TABLE()

public:
    ShowModeDialogSetup(CalChart::ShowMode const& current_mode,
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Setup Modes"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    ~ShowModeDialogSetup() = default;

    virtual void Init(CalChart::ShowMode const& current_mode);
    virtual void CreateControls();

    // use these to get and set default values
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    CalChart::ShowMode GetShowMode() const;

private:
    void OnCmdLineText(wxCommandEvent&);
    void OnCmdChoice(wxCommandEvent&);

    CalChartConfiguration::ShowModeInfo_t mShowModeValues;
    CalChart::ShowMode::YardLinesInfo_t mYardText;
    int mWhichYardLine;
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
EVT_CHOICE(MODE_CHOICE, ShowModeDialogSetup::OnCmdChoice)
EVT_CHOICE(SHOW_LINE_MARKING, ShowModeDialogSetup::OnCmdChoice)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ShowModeDialogSetup, wxPanel)

ShowModeDialogSetup::ShowModeDialogSetup(CalChart::ShowMode const& current_mode, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style, caption)
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
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    std::vector<wxString> choices;
    choices.emplace_back("Reload from...");
    choices.insert(choices.end(), std::begin(kShowModeStrings), std::end(kShowModeStrings));

    wxChoice* modes = new wxChoice(this, MODE_CHOICE, wxDefaultPosition,
        wxDefaultSize, choices.size(), choices.data());
    modes->SetSelection(0);
    topsizer->Add(modes, sLeftBasicSizerFlags);

    wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);

    auto refresh_action = [this](wxCommandEvent&) {
        this->TransferDataFromWindow();
        Refresh();
    };

    AddTextboxWithCaptionAndAction(this, sizer1, WESTHASH, wxT("West Hash"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, EASTHASH, wxT("East Hash"),
        refresh_action, wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
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
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
    AddTextboxWithCaptionAndAction(this, sizer1, OFFSET_X, wxT("Offset X"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, OFFSET_Y, wxT("Offset Y"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, SIZE_X, wxT("Size X"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, SIZE_Y, wxT("Size Y"),
        refresh_action, wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
    wxBoxSizer* textsizer = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(textsizer, sBasicSizerFlags);
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
    textsizer->Add(show_line_value, sBasicSizerFlags);

    textsizer = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(textsizer, sBasicSizerFlags);
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
    textsizer->Add(zoomBox, sLeftBasicSizerFlags);

    auto modeSetupCanvas = new ShowModeSetupCanvas(mConfig, this, CANVAS);
    modeSetupCanvas->SetScrollRate(1, 1);
    topsizer->Add(modeSetupCanvas, 1, wxEXPAND);

    modeSetupCanvas->SetMode(CalChart::ShowMode::CreateShowMode(mShowModeValues, mYardText));
    modeSetupCanvas->SetZoom(zoom_amounts[5] / 100.0);
    zoomBox->SetSelection(5);

    TransferDataToWindow();
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
        wxString buf;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(WESTHASH + std::distance(mShowModeValues.begin(), i));
        buf.Printf(wxT("%d"), static_cast<int>(*i));
        text->ChangeValue(buf);
    }

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    text->SetValue(mYardText[mWhichYardLine]);
    return true;
}

bool ShowModeDialogSetup::TransferDataFromWindow()
{
    // read out the values from the window
    // standard show
    for (auto i = mShowModeValues.begin(); i != mShowModeValues.end(); ++i) {
        long val;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            WESTHASH + std::distance(mShowModeValues.begin(), i));
        text->GetValue().ToLong(&val);
        *i = val;
    }

    // grab whatever's in the box
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();

    ShowModeSetupCanvas* canvas = (ShowModeSetupCanvas*)FindWindow(CANVAS);
    canvas->SetMode(CalChart::ShowMode::CreateShowMode(mShowModeValues, mYardText));

    return true;
}

void ShowModeDialogSetup::OnCmdChoice(wxCommandEvent&)
{
    // save off all the old values:
    wxChoice* modes = (wxChoice*)FindWindow(MODE_CHOICE);
    auto whichMode = modes->GetSelection();
    if (whichMode > 0) {
        modes->SetSelection(0);
        mShowModeValues = mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(whichMode - 1));
        ShowModeSetupCanvas* canvas = (ShowModeSetupCanvas*)FindWindow(CANVAS);
        canvas->SetMode(CalChart::ShowMode::CreateShowMode(mShowModeValues, mYardText));
    }

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();

    modes = (wxChoice*)FindWindow(SHOW_LINE_MARKING);
    mWhichYardLine = modes->GetSelection();
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

ModeSetupDialog::ModeSetupDialog(CalChart::ShowMode const& current_mode, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, caption, pos, size, style)
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    m_setup = new ShowModeDialogSetup(current_mode, this, wxID_ANY);

    topsizer->Add(m_setup, sBasicSizerFlags);

    // the buttons on the bottom
    wxBoxSizer* okCancelBox = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(okCancelBox, sBasicSizerFlags);

    okCancelBox->Add(new wxButton(this, wxID_CANCEL), sBasicSizerFlags);
    okCancelBox->Add(new wxButton(this, wxID_OK), sBasicSizerFlags);

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
