/*
 * PrintPostScriptDialog.cpp
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

#include "PrintPostScriptDialog.h"

#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "basic_ui.h"
#include "print_ps.h"

#include <set>

#include <iterator>
#include <sstream>
#include <wx/filename.h>
#include <wx/wfstream.h>

enum {
    CC_PRINT_ORIENT_PORTRAIT,
    CC_PRINT_ORIENT_LANDSCAPE
};

enum {
    CC_PRINT_ACTION_PRINTER,
    CC_PRINT_ACTION_FILE,
    CC_PRINT_ACTION_PREVIEW
};

enum {
    CC_PRINT_BUTTON_PRINT = 1000,
    CC_PRINT_BUTTON_SELECT,
    CC_PRINT_BUTTON_RESET_DEFAULTS
};

BEGIN_EVENT_TABLE(PrintPostScriptDialog, wxDialog)
EVT_BUTTON(CC_PRINT_BUTTON_SELECT, PrintPostScriptDialog::ShowPrintSelect)
EVT_BUTTON(CC_PRINT_BUTTON_RESET_DEFAULTS, PrintPostScriptDialog::ResetDefaults)
END_EVENT_TABLE()

IMPLEMENT_CLASS(PrintPostScriptDialog, wxDialog)

PrintPostScriptDialog::PrintPostScriptDialog() { Init(); }

PrintPostScriptDialog::PrintPostScriptDialog(
    const CalChartDoc* show, wxFrame* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : mShow(NULL)
{
    Init();

    Create(show, parent, id, caption, pos, size, style);
}

PrintPostScriptDialog::~PrintPostScriptDialog() { }

void PrintPostScriptDialog::Init() { }

void PrintPostScriptDialog::PrintShow(const CalChartConfiguration& config)
{
#ifdef PRINT__RUN_CMD
    wxString buf;
#endif

    long minyards;
    text_minyards->GetValue().ToLong(&minyards);
    if (minyards < 10 || minyards > 100) {
        wxLogError("Yards entered invalid.  Please enter a number between 10 and 100.");
        return;
    }
    auto overview = config.Get_PrintPSOverview();

    wxString s;
    switch (config.Get_PrintPSModes()) {
    case CC_PRINT_ACTION_PREVIEW: {
#ifdef PRINT__RUN_CMD
        s = wxFileName::CreateTempFileName("cc_");
        buf.sprintf("%s %s \"%s\"", config.Get_PrintViewCmd().c_str(), config.Get_PrintViewCmd().c_str(), s.c_str());
#endif
    } break;
    case CC_PRINT_ACTION_FILE:
        s = wxFileSelector("Print to file", wxEmptyString, wxEmptyString, wxEmptyString, "*.ps", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (s.empty())
            return;
        break;
    case CC_PRINT_ACTION_PRINTER: {
#ifdef PRINT__RUN_CMD
        s = wxFileName::CreateTempFileName("cc_");
        buf.sprintf("%s %s \"%s\"", config.Get_PrintCmd().c_str(), config.Get_PrintOpts().c_str(), s.c_str());
#else
#endif
    } break;
    default:
        break;
    }

    std::ostringstream buffer;
    auto n = mShow->PrintToPS(buffer, overview, static_cast<int>(minyards), mIsSheetPicked, config);
    // stream to file:
    wxFFileOutputStream(s).Write(buffer.str().c_str(), buffer.str().size());

#ifdef PRINT__RUN_CMD
    switch (config.Get_PrintPSModes()) {
    case CC_PRINT_ACTION_FILE:
        break;
    default:
        // intentionally ignoring the result
        (void)system(buf.utf8_str());
        wxRemoveFile(s);
        break;
    }
#endif

    wxString tempbuf;
    tempbuf.sprintf("Printed %d pages.", n);
    (void)wxMessageBox(tempbuf, mShow->GetTitle());
}

void PrintPostScriptDialog::ShowPrintSelect(wxCommandEvent&)
{
    wxArrayString choices;
    for (auto&& sheet : mShow->GetSheets()) {
        choices.Add(sheet.GetName());
    }
    wxMultiChoiceDialog dialog(this, "Choose which pages to print", "Pages to Print", choices);
    wxArrayInt markedChoices(mIsSheetPicked.begin(), mIsSheetPicked.end());
    dialog.SetSelections(markedChoices);
    if (dialog.ShowModal() == wxID_OK) {
        mIsSheetPicked.clear();
        // build up a set of what's been selected:
        for (auto&& selection : dialog.GetSelections())
            mIsSheetPicked.insert(selection);
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

bool PrintPostScriptDialog::Create(const CalChartDoc* show,
    wxFrame* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
{
    if (!wxDialog::Create(parent, id, caption, pos, size, style)) {
        return false;
    }
    mShow = show;
    for (auto i = 0; i < mShow->GetNumSheets(); ++i) {
        mIsSheetPicked.insert(i);
    }

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
    constexpr auto minWidth = 100;
    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 5).Left(),
        wxUI::HSizer{
            wxUI::Button{ wxID_OK, "&Print" },
            wxUI::Button{ wxID_CANCEL, "&Cancel" }
                .setDefault(),
            wxUI::Button{ CC_PRINT_BUTTON_RESET_DEFAULTS, "&Reset Values" },
        },
#ifdef PRINT__RUN_CMD
        wxUI::HSizer{
            wxUI::VSizer{
                wxUI::Text{ "Printer Command:" },
                text_cmd = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },
            wxUI::VSizer{
                wxUI::Text{ "Printer Options:" },
                text_opts = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },
        },
        wxUI::HSizer{
            wxUI::VSizer{
                wxUI::Text{ "Preview Command:" },
                text_view_cmd = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },
            wxUI::VSizer{
                wxUI::Text{ "Preview Options:" },
                text_view_opts = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },
        },
#else
        wxUI::HSizer{
            wxUI::VSizer{
                wxUI::Text{ "Printer &Device:" },
                text_cmd = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },
        },
#endif
        wxUI::HSizer{
            radio_orient = wxUI::RadioBox{ "&Orientation:", wxUI::RadioBox::withChoices{}, { "Portrait", "Landscape" } },
#ifdef PRINT__RUN_CMD
            radio_method = wxUI::RadioBox{ "Post&Script:", wxUI::RadioBox::withChoices{}, { "Send to Printer", "Print to File", "Preview Only" } },
#else
            radio_method = wxUI::RadioBox{ "Post&Script:", wxUI::RadioBox::withChoices{}, { "Send to Printer", "Print to File" } },
#endif
        },
        wxUI::HSizer{
            check_overview = wxUI::CheckBox{ "Over&view" },
            check_cont = wxUI::CheckBox{ "Continuit&y" },
            check_pages = wxUI::CheckBox{ "Cove&r pages" },
        },
        wxUI::Button{ CC_PRINT_BUTTON_SELECT, "S&elect sheets..." },
        wxUI::HSizer{
            wxUI::VSizer{
                wxUI::Text{ "Page &width:" },
                text_width = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },
            wxUI::VSizer{
                wxUI::Text{ "Page &height:" },
                text_height = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },
            wxUI::VSizer{
                wxUI::Text{ "&Left margin:" },
                text_x = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },
            wxUI::VSizer{
                wxUI::Text{ "&Top margin:" },
                text_y = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },
            wxUI::VSizer{
                wxUI::Text{ "Paper &length:" },
                text_length = wxUI::TextCtrl{ wxSizerFlags{}.Expand() }.withWidthSize(minWidth),
            },

        },
        wxUI::HSizer{
            wxUI::VSizer{
                wxUI::Text{ "Yards:" },
                text_minyards = wxUI::TextCtrl{ wxSizerFlags{}.Expand(), "50" }.withWidthSize(minWidth),

            },
        },
    }
        .attachTo(this);
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
    check_pages->SetValue(config.Get_PrintPSDoContSheet());

    wxString buf;
    buf.Printf("%.2f", config.Get_PageOffsetX());
    text_x->SetValue(buf);
    buf.Printf("%.2f", config.Get_PageOffsetY());
    text_y->SetValue(buf);
    buf.Printf("%.2f", config.Get_PageWidth());
    text_width->SetValue(buf);
    buf.Printf("%.2f", config.Get_PageHeight());
    text_height->SetValue(buf);
    buf.Printf("%.2f", config.Get_PaperLength());
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
    config.Set_PrintPSDoContSheet(check_pages->GetValue());

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
