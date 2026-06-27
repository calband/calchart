/*
 * DebugFieldThumbnailBrowser.cpp
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

#include "DebugFieldThumbnailBrowser.hpp"
#include "CalChartConfiguration.h"
#include "CalChartContinuity.h"
#include "CalChartRanges.h"
#include "CalChartShow.h"
#include "FieldThumbnailBrowser.h"
#include "ViewHandlers.hpp"

namespace {
auto GetDebugShow()
{
    auto show = CalChart::Show::Create(CalChart::ShowMode::GetDefaultShowMode());
    show->Create_SetupMarchersCommand({ { "A", "A" }, { "B", "B" }, { "C", "C" }, { "D", "D" } }, 1, 0).first(*show);
    {
        auto sheet = CalChart::Sheet(4);
        sheet.SetPosition({ 0, 0 }, 0);
        sheet.SetPosition({ 0x10, 0x10 }, 1);
        sheet.SetPosition({ 0x20, 0x20 }, 2);
        sheet.SetPosition({ 0x30, 0x30 }, 3);
        show->Create_AddSheetsCommand({ sheet }, 1).first(*show);
    }
    {
        auto sheet = CalChart::Sheet(4);
        sheet.SetPosition({ 0x110, 0x110 }, 0);
        sheet.SetPosition({ 0x120, 0x120 }, 1);
        sheet.SetPosition({ 0x130, 0x130 }, 2);
        sheet.SetPosition({ 0x140, 0x140 }, 3);
        show->Create_AddSheetsCommand({ sheet }, 2).first(*show);
    }
    return show;
}

auto GetDebugFieldThumbnailBrowserHandlers(
    CalChart::Show& show,
    CalChart::Configuration& config,
    wxWindow* window) -> FieldThumbnailBrowser::Handlers
{
    return {
        [&show]() {
            return show.GetShowMode().Size();
        },
        [&show]() {
            return show.GetNumSheets();
        },
        [&show]() {
            return show.GetCurrentSheetNum();
        },
        [&show]() {
            return show.GetSheetsName();
        },
        [&show, &config]() {
            return show.GenerateFieldWithMarchersDrawCommands(config);
        },
        [&show, window](size_t sheet_num) {
            auto numSheets = show.GetNumSheets();
            if (sheet_num >= numSheets) {
                return;
            }
            show.Create_SetCurrentSheetCommand(sheet_num).first(show);
            window->Refresh();
        },
    };
}
}

void DebugFieldThumbnailBrowser(wxWindow* parent, CalChart::Configuration& config)
{
    auto show = GetDebugShow();
    auto* dialog = new wxDialog(parent, wxID_ANY, "Field Thumbnail Browser Test",
        wxDefaultPosition, wxSize(800, 600),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    auto* browser = new FieldThumbnailBrowser(config, dialog);

    browser->SetHandlers(GetDebugFieldThumbnailBrowserHandlers(*show, config, browser));

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(browser, 1, wxEXPAND | wxALL);
    dialog->SetSizer(sizer);
    dialog->ShowModal();
    dialog->Destroy();
}
