/*
 * PrintPostScriptDialog.cpp
 * Dialox box for printing postscript
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

#include "PrintPostScriptDialog.h"

#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartPrintShowToPS.hpp"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "basic_ui.h"

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

PrintPostScriptDialog::PrintPostScriptDialog(
    const CalChartDoc* show,
    CalChart::Configuration& config,
    wxFrame* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style)
    : mShow(NULL)
    , mConfig(config)
{
    Init();

    Create(show, parent, id, caption, pos, size, style);
}

PrintPostScriptDialog::~PrintPostScriptDialog() { }

void PrintPostScriptDialog::Init() { }

void PrintPostScriptDialog::PrintShow()
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
    auto overview = mConfig.Get_PrintPSOverview();

    wxString s;
    switch (mConfig.Get_PrintPSModes()) {
    case CC_PRINT_ACTION_PREVIEW: {
#ifdef PRINT__RUN_CMD
        s = wxFileName::CreateTempFileName("cc_");
        buf.sprintf("%s %s \"%s\"", mConfig.Get_PrintViewCmd().c_str(), mConfig.Get_PrintViewCmd().c_str(), s.c_str());
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
        buf.sprintf("%s %s \"%s\"", mConfig.Get_PrintCmd().c_str(), mConfig.Get_PrintOpts().c_str(), s.c_str());
#else
#endif
    } break;
    default:
        break;
    }

    auto [result, n] = mShow->PrintToPS(overview, static_cast<int>(minyards), mIsSheetPicked, mConfig);
    // stream to file:
    wxFFileOutputStream(s).Write(result.c_str(), result.size());

#ifdef PRINT__RUN_CMD
    switch (mConfig.Get_PrintPSModes()) {
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
    for (auto&& name : mShow->GetSheetsName()) {
        choices.Add(name);
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
#ifdef PRINT__RUN_CMD
    mConfig.Clear_PrintCmd();
    mConfig.Clear_PrintOpts();
#else
    mConfig.Clear_PrintFile();
#endif
    mConfig.Clear_PrintViewCmd();
    mConfig.Clear_PrintViewOpts();

    mConfig.Clear_PageOffsetX();
    mConfig.Clear_PageOffsetY();
    mConfig.Clear_PageWidth();
    mConfig.Clear_PageHeight();

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
#ifdef PRINT__RUN_CMD
    text_cmd->SetValue(mConfig.Get_PrintCmd());
    text_opts->SetValue(mConfig.Get_PrintOpts());
    text_view_cmd->SetValue(mConfig.Get_PrintViewCmd());
    text_view_opts->SetValue(mConfig.Get_PrintViewOpts());
#else
    text_cmd->SetValue(mConfig.Get_PrintFile());
#endif
    radio_orient->SetSelection(mConfig.Get_PrintPSLandscape());
    radio_method->SetSelection(static_cast<int>(mConfig.Get_PrintPSModes()));
    check_overview->SetValue(mConfig.Get_PrintPSOverview());
    check_cont->SetValue(mConfig.Get_PrintPSDoCont());
    check_pages->SetValue(mConfig.Get_PrintPSDoContSheet());

    wxString buf;
    buf.Printf("%.2f", mConfig.Get_PageOffsetX());
    text_x->SetValue(buf);
    buf.Printf("%.2f", mConfig.Get_PageOffsetY());
    text_y->SetValue(buf);
    buf.Printf("%.2f", mConfig.Get_PageWidth());
    text_width->SetValue(buf);
    buf.Printf("%.2f", mConfig.Get_PageHeight());
    text_height->SetValue(buf);
    buf.Printf("%.2f", mConfig.Get_PaperLength());
    text_length->SetValue(buf);
    return true;
}

bool PrintPostScriptDialog::TransferDataFromWindow()
{
#ifdef PRINT__RUN_CMD
    mConfig.Set_PrintCmd(text_cmd->GetValue());
    mConfig.Set_PrintOpts(text_opts->GetValue());
    mConfig.Set_PrintViewCmd(text_view_cmd->GetValue());
    mConfig.Set_PrintViewOpts(text_view_opts->GetValue());
#else
    mConfig.Set_PrintFile(text_cmd->GetValue());
#endif
    mConfig.Set_PrintPSLandscape(radio_orient->GetSelection() == CC_PRINT_ORIENT_LANDSCAPE);
    mConfig.Set_PrintPSModes(radio_method->GetSelection());
    mConfig.Set_PrintPSOverview(check_overview->GetValue());
    mConfig.Set_PrintPSDoCont(check_cont->GetValue());
    mConfig.Set_PrintPSDoContSheet(check_pages->GetValue());

    double dval;
    text_x->GetValue().ToDouble(&dval);
    mConfig.Set_PageOffsetX(dval);
    text_y->GetValue().ToDouble(&dval);
    mConfig.Set_PageOffsetY(dval);
    text_width->GetValue().ToDouble(&dval);
    mConfig.Set_PageWidth(dval);
    text_height->GetValue().ToDouble(&dval);
    mConfig.Set_PageHeight(dval);
    text_length->GetValue().ToDouble(&dval);
    mConfig.Set_PaperLength(dval);

    return true;
}
