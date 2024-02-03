/*
 * TransitionSolverView.cpp
 * CalChart
 */

/*
   Copyright (C) 2017-2024  Kevin Durand , Richard Michael Powell

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

#include "TransitionSolverView.h"
#include "CalChartApp.h"
#include "CalChartContinuity.h"
#include "CalChartDocCommand.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "TransitionSolverFrame.h"
#include "basic_ui.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>

TransitionSolverView::TransitionSolverView()
{
}
TransitionSolverView::~TransitionSolverView() { }

void TransitionSolverView::OnDraw(wxDC*) { }
void TransitionSolverView::OnUpdate(wxView*, wxObject*)
{
    TransitionSolverFrame* frame = static_cast<TransitionSolverFrame*>(GetFrame());
    if (frame) {
        frame->Update();
    }
}

void TransitionSolverView::ApplyTransitionSolution(CalChart::TransitionSolverResult solution)
{
    if (solution.successfullySolved) {
        GetDocument()->GetCommandProcessor()->Submit(static_cast<CalChartDoc*>(GetDocument())->Create_SetTransitionCommand(solution.finalPositions, solution.continuities, solution.marcherDotTypes).release());
    }
}

void TransitionSolverView::SelectMarchers(std::set<unsigned> marchers)
{
    GetDocument()->GetCommandProcessor()->Submit(static_cast<CalChartDoc*>(GetDocument())->Create_SetSelectionListCommand({ marchers.begin(), marchers.end() }).release());
}
