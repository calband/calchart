/*
 * ContinuityBrowserPanel
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

#include "ContinuityBrowserPanel.h"
#include "CalChartView.h"
#include "ContinuityComposerDialog.h"
#include "cont.h"
#include "cont_box_draw.h"

using namespace CalChart;

BEGIN_EVENT_TABLE(ContinuityBrowserPanel, ContinuityBrowserPanel::super)
EVT_SET_FOCUS(ContinuityBrowserPanel::DoSetFocus)
EVT_KILL_FOCUS(ContinuityBrowserPanel::DoKillFocus)
END_EVENT_TABLE()

// Define a constructor for field canvas
ContinuityBrowserPanel::ContinuityBrowserPanel(SYMBOL_TYPE sym, CalChartConfiguration& config, wxWindow* parent, wxWindowID winid, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : super(parent, winid, pos, size, style, name)
    , mSym(sym)
    , mConfig(config)
{
    Init();
}

void ContinuityBrowserPanel::Init()
{
    // make room for about 3.5
    auto current_size = GetMinSize();
    current_size.y = ContinuityBoxDrawer::GetHeight(mConfig);
    current_size.y *= 3.5;
    SetMinSize(current_size);
}

void ContinuityBrowserPanel::DoSetContinuity(CalChart::Continuity const& new_cont)
{
    std::vector<std::unique_ptr<DrawableCell>> contCells;
    mCont = new_cont;
    for (auto&& i : mCont.GetParsedContinuity()) {
        contCells.emplace_back(std::make_unique<ContinuityBoxDrawer>(i->GetDrawableCont(), mConfig));
    }

    // resize based on the new number of continuities.
    auto current_size = GetMinSize();
    current_size.y = ContinuityBoxDrawer::GetHeight(mConfig);
    current_size.y *= (static_cast<double>(contCells.size()) + 0.5);
    SetMinSize(current_size);

    SetCells(std::move(contCells));
}

template <typename T>
static auto do_cloning(T const& cont)
{
    std::vector<std::unique_ptr<ContProcedure>> copied_cont;
    for (auto&& i : cont.GetParsedContinuity()) {
        copied_cont.emplace_back(i->clone());
    }
    return copied_cont;
}

void ContinuityBrowserPanel::AddNewEntry()
{
    OnNewEntry(mCont.GetParsedContinuity().size());
}

void ContinuityBrowserPanel::OnNewEntry(int cell)
{
    if (!mView) {
        return;
    }

    ContinuityComposerDialog dialog(nullptr, this);

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }
    try {
        // if we have -1, that means push_back.
        auto copied_cont = do_cloning(mCont);
        copied_cont.insert(copied_cont.begin() + cell, dialog.GetContinuity());
        UpdateCont(CalChart::Continuity{ std::move(copied_cont) });
    } catch (std::runtime_error const& e) {
        wxMessageBox(wxT("Error: ") + wxString{ e.what() }, wxT("Parsing Error"), wxOK | wxICON_INFORMATION, this);
        return;
    }
}

void ContinuityBrowserPanel::OnEditEntry(int cell)
{
    if (!mView) {
        return;
    }

    ContinuityComposerDialog dialog(mCont.GetParsedContinuity().at(cell)->clone(), this);

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }
    try {
        // if we have -1, that means push_back.
        auto copied_cont = do_cloning(mCont);
        copied_cont.at(cell) = dialog.GetContinuity();
        UpdateCont(CalChart::Continuity{ std::move(copied_cont) });
    } catch (std::runtime_error const& e) {
        wxMessageBox(wxT("Error: ") + wxString{ e.what() }, wxT("Parsing Error"), wxOK | wxICON_INFORMATION, this);
        return;
    }
}

void ContinuityBrowserPanel::OnDeleteEntry(int cell)
{
    if (!mView) {
        return;
    }

    // make a copy, then delete it, then set as new continuity:
    if (cell < static_cast<int>(mCont.GetParsedContinuity().size())) {
        auto copied_cont = do_cloning(mCont);
        copied_cont.erase(copied_cont.begin() + cell);
        UpdateCont(CalChart::Continuity{ std::move(copied_cont) });
    }
}

void ContinuityBrowserPanel::OnMoveEntry(int start_cell, int end_cell)
{
    auto copied_cont = do_cloning(mCont);
    if (start_cell < end_cell) {
        auto end = end_cell >= static_cast<int>(copied_cont.size()) ? copied_cont.end() : copied_cont.begin() + end_cell + 1;
        std::rotate(copied_cont.begin() + start_cell, copied_cont.begin() + start_cell + 1, end);
    } else {
        auto end = copied_cont.begin() + start_cell + 1;
        std::rotate(copied_cont.begin() + end_cell, copied_cont.begin() + start_cell, end);
    }
    UpdateCont(CalChart::Continuity{ std::move(copied_cont) });
}

void ContinuityBrowserPanel::UpdateCont(Continuity const& new_cont)
{
    if (!mView) {
        return;
    }

    mView->DoSetContinuityCommand(mSym, new_cont);
}

void ContinuityBrowserPanel::DoSetFocus(wxFocusEvent& event)
{
    if (!mView) {
        return;
    }

    auto&& sht = mView->GetCurrentSheet();
    mView->SetSelection(sht->MakeSelectPointsBySymbol(mSym));
}

void ContinuityBrowserPanel::DoKillFocus(wxFocusEvent& event)
{
    SetSelection(-1);
}
