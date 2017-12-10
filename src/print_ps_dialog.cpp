
/*
 * print_ps_dialog.h
 * Dialox box for printing postscript
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "print_ps_dialog.h"

#include "show_ui.h"
#include "confgr.h"
#include "cc_sheet.h"
#include "print_ps.h"
#include "cc_show.h"
#include <set>

#include <wx/filename.h>
#include <wx/wfstream.h>
#include <sstream>
#include <iterator>

enum { CC_PRINT_ORIENT_PORTRAIT,
    CC_PRINT_ORIENT_LANDSCAPE };

enum { CC_PRINT_ACTION_PRINTER,
    CC_PRINT_ACTION_FILE,
    CC_PRINT_ACTION_PREVIEW };

enum {
    CC_PRINT_BUTTON_PRINT = 1000,
    CC_PRINT_BUTTON_SELECT,
    CC_PRINT_BUTTON_RESET_DEFAULTS
};

BEGIN_EVENT_TABLE(PrintPostScriptDialog, wxDialog)
EVT_BUTTON(CC_PRINT_BUTTON_SELECT, PrintPostScriptDialog::ShowPrintSelect)
EVT_BUTTON(CC_PRINT_BUTTON_RESET_DEFAULTS, PrintPostScriptDialog::ResetDefaults)
END_EVENT_TABLE()

void PrintPostScriptDialog::PrintShow(const CalChartConfiguration& config)
{
    wxString s;
#ifdef PRINT__RUN_CMD
    wxString buf;
#endif
    bool overview;
    long minyards;

    text_minyards->GetValue().ToLong(&minyards);
    if (minyards < 10 || minyards > 100) {
        wxLogError(wxT(
            "Yards entered invalid.  Please enter a number between 10 and 100."));
        return;
    }
    overview = config.Get_PrintPSOverview();

    switch (config.Get_PrintPSModes()) {
    case CC_PRINT_ACTION_PREVIEW: {
#ifdef PRINT__RUN_CMD
        s = wxFileName::CreateTempFileName(wxT("cc_"));
        buf.sprintf(wxT("%s %s \"%s\""), config.Get_PrintViewCmd().c_str(),
            config.Get_PrintViewCmd().c_str(), s.c_str());
#endif
    } break;
    case CC_PRINT_ACTION_FILE:
        s = wxFileSelector(wxT("Print to file"), wxEmptyString, wxEmptyString,
            wxEmptyString, eps ? wxT("*.eps") : wxT("*.ps"),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (s.empty())
            return;
        break;
    case CC_PRINT_ACTION_PRINTER: {
#ifdef PRINT__RUN_CMD
        s = wxFileName::CreateTempFileName(wxT("cc_"));
        buf.sprintf(wxT("%s %s \"%s\""), config.Get_PrintCmd().c_str(),
            config.Get_PrintOpts().c_str(), s.c_str());
#else
#endif
    } break;
    default:
        break;
    }

    std::ostringstream buffer;

    int n = mShow->PrintToPS(buffer, eps, overview, static_cast<int>(minyards), mIsSheetPicked, config);
    // stream to file:
    {
        wxFFileOutputStream outstream(s);
        outstream.Write(buffer.str().c_str(), buffer.str().size());
    }

#ifdef PRINT__RUN_CMD
    switch (config.Get_PrintPSModes()) {
    case CC_PRINT_ACTION_FILE:
        break;
    default:
        system(buf.utf8_str());
        wxRemoveFile(s);
        break;
    }
#endif

    wxString tempbuf;
    tempbuf.sprintf(wxT("Printed %d pages."), n);
    (void)wxMessageBox(tempbuf, mShow->GetTitle());
}

void PrintPostScriptDialog::ShowPrintSelect(wxCommandEvent&)
{
    wxArrayString choices;
    for (CalChart::Show::const_CC_sheet_iterator_t sheet = mShow->GetSheetBegin();
         sheet != mShow->GetSheetEnd(); ++sheet) {
        choices.Add(sheet->GetName());
    }
    wxMultiChoiceDialog dialog(this, wxT("Choose which pages to print"),
        wxT("Pagest to Print"), choices);
    wxArrayInt markedChoices;
    for (CalChart::Show::const_CC_sheet_iterator_t sheet = mShow->GetSheetBegin();
         sheet != mShow->GetSheetEnd(); ++sheet) {
        if (mIsSheetPicked.count(std::distance(mShow->GetSheetBegin(), sheet))) {
            markedChoices.Add(static_cast<int>(std::distance(mShow->GetSheetBegin(), sheet)));
        }
    }
    dialog.SetSelections(markedChoices);
    if (dialog.ShowModal() == wxID_OK) {
        wxArrayInt selections = dialog.GetSelections();
        mIsSheetPicked.clear();
        // build up a set of what's been selected:
        for (size_t n = 0; n < selections.GetCount(); ++n)
            mIsSheetPicked.insert(selections[n]);
    }
}

void PrintPostScriptDialog::ResetDefaults(wxCommandEvent&)
{
    auto& config = CalChartConfiguration::GetGlobalConfig();
#ifdef PRINT__RUN_CMD
    config.Clear_PrintCmd();
    config.Clear_PrintOpts();
#else
    config.Clear_PrintFile();
#endif
    config.Clear_PrintViewCmd();
    config.Clear_PrintViewOpts();

    config.Clear_PageOffsetX();
    config.Clear_PageOffsetY();
    config.Clear_PageWidth();
    config.Clear_PageHeight();

    // re-read values
    TransferDataToWindow();
}

IMPLEMENT_CLASS(PrintPostScriptDialog, wxDialog)

PrintPostScriptDialog::PrintPostScriptDialog() { Init(); }

PrintPostScriptDialog::PrintPostScriptDialog(
    const CalChartDoc* show, bool printEPS, wxFrame* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : mShow(NULL)
{
    Init();

    Create(show, printEPS, parent, id, caption, pos, size, style);
}

PrintPostScriptDialog::~PrintPostScriptDialog() {}

void PrintPostScriptDialog::Init() {}

bool PrintPostScriptDialog::Create(const CalChartDoc* show, bool printEPS,
    wxFrame* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
{
    if (!wxDialog::Create(parent, id, caption, pos, size, style))
        return false;
    mShow = show;
    for (CalChart::Show::const_CC_sheet_iterator_t sheet = mShow->GetSheetBegin();
         sheet != mShow->GetSheetEnd(); ++sheet) {
        mIsSheetPicked.insert(std::distance(mShow->GetSheetBegin(), sheet));
    }
    eps = printEPS;

    CreateControls();

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);

    Center();

    return true;
}

void PrintPostScriptDialog::CreateControls()
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    wxBoxSizer* horizontalsizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* print = new wxButton(this, wxID_OK, wxT("&Print"));
    horizontalsizer->Add(print, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* close = new wxButton(this, wxID_CANCEL, wxT("&Cancel"));
    horizontalsizer->Add(close, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    close->SetDefault();

    wxButton* resetdefaults = new wxButton(this, CC_PRINT_BUTTON_RESET_DEFAULTS, wxT("&Reset Values"));
    horizontalsizer->Add(resetdefaults, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    topsizer->Add(horizontalsizer, 0, wxALL, 5);

    wxBoxSizer* verticalsizer = NULL;
#ifdef PRINT__RUN_CMD
    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC,
                           wxT("Printer Command:"),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_cmd = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_cmd, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC,
                           wxT("Printer Options:"),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_opts = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_opts, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    topsizer->Add(horizontalsizer, 0, wxALL, 5);

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC,
                           wxT("Preview Command:"),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_view_cmd = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_view_cmd, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC,
                           wxT("Preview Options:"),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_view_opts = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_view_opts, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    topsizer->Add(horizontalsizer, 0, wxALL, 5);

#else // PRINT__RUN_CMD

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC,
                           wxT("Printer &Device:"),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_cmd = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_cmd, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    topsizer->Add(horizontalsizer, 0, wxALL, 5);
#endif

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    wxString orientation[] = { wxT("Portrait"), wxT("Landscape") };
    radio_orient = new wxRadioBox(
        this, wxID_ANY, wxT("&Orientation:"), wxDefaultPosition, wxDefaultSize,
        sizeof(orientation) / sizeof(wxString), orientation);
    horizontalsizer->Add(radio_orient, 0, wxALL, 5);

#ifdef PRINT__RUN_CMD
    wxString print_modes[] = { wxT("Send to Printer"), wxT("Print to File"),
        wxT("Preview Only") };
#else
    wxString print_modes[] = { wxT("Send to Printer"), wxT("Print to File") };
#endif
    radio_method = new wxRadioBox(
        this, -1, wxT("Post&Script:"), wxDefaultPosition, wxDefaultSize,
        sizeof(print_modes) / sizeof(wxString), print_modes);
    horizontalsizer->Add(radio_method, 0, wxALL, 5);
    topsizer->Add(horizontalsizer, 0, wxALL, 5);

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    check_overview = new wxCheckBox(this, -1, wxT("Over&view"));
    horizontalsizer->Add(check_overview, 0, wxALL, 5);

    check_cont = new wxCheckBox(this, -1, wxT("Continuit&y"));
    horizontalsizer->Add(check_cont, 0, wxALL, 5);

    if (!eps) {
        check_pages = new wxCheckBox(this, -1, wxT("Cove&r pages"));
        horizontalsizer->Add(check_pages, 0, wxALL, 5);
    }
    else {
        check_pages = NULL;
    }
    topsizer->Add(horizontalsizer, 0, wxALL, 5);

    if (!eps) {
        topsizer->Add(
            new wxButton(this, CC_PRINT_BUTTON_SELECT, wxT("S&elect sheets...")), 0,
            wxALL, 5);
    }

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Page &width: "),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_width = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_width, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Page &height: "),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_height = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_height, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Left margin: "),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_x = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_x, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Top margin: "),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_y = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_y, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Paper &length: "),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_length = new wxTextCtrl(this, wxID_ANY);
    verticalsizer->Add(text_length, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    topsizer->Add(horizontalsizer, 0, wxALL, 5);

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    verticalsizer = new wxBoxSizer(wxVERTICAL);
    verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Yards: "),
                           wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    text_minyards = new wxTextCtrl(this, wxID_ANY, wxT("50"));
    verticalsizer->Add(text_minyards, 0, wxGROW | wxALL, 5);
    horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    topsizer->Add(horizontalsizer, 0, wxALL, 5);
}

bool PrintPostScriptDialog::TransferDataToWindow()
{
    auto& config = CalChartConfiguration::GetGlobalConfig();
#ifdef PRINT__RUN_CMD
    text_cmd->SetValue(config.Get_PrintCmd());
    text_opts->SetValue(config.Get_PrintOpts());
    text_view_cmd->SetValue(config.Get_PrintViewCmd());
    text_view_opts->SetValue(config.Get_PrintViewOpts());
#else
    text_cmd->SetValue(config.Get_PrintFile());
#endif
    radio_orient->SetSelection(config.Get_PrintPSLandscape());
    radio_method->SetSelection(static_cast<int>(config.Get_PrintPSModes()));
    check_overview->SetValue(config.Get_PrintPSOverview());
    check_cont->SetValue(config.Get_PrintPSDoCont());
    if (check_pages) {
        check_pages->SetValue(config.Get_PrintPSDoContSheet());
    }

    wxString buf;
    buf.Printf(wxT("%.2f"), config.Get_PageOffsetX());
    text_x->SetValue(buf);
    buf.Printf(wxT("%.2f"), config.Get_PageOffsetY());
    text_y->SetValue(buf);
    buf.Printf(wxT("%.2f"), config.Get_PageWidth());
    text_width->SetValue(buf);
    buf.Printf(wxT("%.2f"), config.Get_PageHeight());
    text_height->SetValue(buf);
    buf.Printf(wxT("%.2f"), config.Get_PaperLength());
    text_length->SetValue(buf);
    return true;
}

bool PrintPostScriptDialog::TransferDataFromWindow()
{
    auto& config = CalChartConfiguration::GetGlobalConfig();
#ifdef PRINT__RUN_CMD
    config.Set_PrintCmd(text_cmd->GetValue());
    config.Set_PrintOpts(text_opts->GetValue());
    config.Set_PrintViewCmd(text_view_cmd->GetValue());
    config.Set_PrintViewOpts(text_view_opts->GetValue());
#else
    config.Set_PrintFile(text_cmd->GetValue());
#endif
    config.Set_PrintPSLandscape(radio_orient->GetSelection() == CC_PRINT_ORIENT_LANDSCAPE);
    config.Set_PrintPSModes(radio_method->GetSelection());
    config.Set_PrintPSOverview(check_overview->GetValue());
    config.Set_PrintPSDoCont(check_cont->GetValue());
    if (check_pages) {
        config.Set_PrintPSDoContSheet(check_pages->GetValue());
    }

    double dval;
    text_x->GetValue().ToDouble(&dval);
    config.Set_PageOffsetX(dval);
    text_y->GetValue().ToDouble(&dval);
    config.Set_PageOffsetY(dval);
    text_width->GetValue().ToDouble(&dval);
    config.Set_PageWidth(dval);
    text_height->GetValue().ToDouble(&dval);
    config.Set_PageHeight(dval);
    text_length->GetValue().ToDouble(&dval);
    config.Set_PaperLength(dval);

    return true;
}
