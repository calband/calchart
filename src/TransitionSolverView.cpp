//
//  TransitionSolver.cpp
//  CalChart
//
//  Created by Kevin Durand on 6/24/17.
//
//

#include "TransitionSolverView.h"
#include "CalChartApp.h"
#include "CalChartDoc.h"
#include "CalChartDocCommand.h"
#include "TransitionSolverFrame.h"
#include "basic_ui.h"
#include "cc_continuity.h"
#include "cc_sheet.h"
#include "cc_show.h"
#include "confgr.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>

TransitionSolverView::TransitionSolverView()
{
}
TransitionSolverView::~TransitionSolverView() { }

void TransitionSolverView::OnDraw(wxDC* dc) { }
void TransitionSolverView::OnUpdate(wxView* sender, wxObject* hint)
{
    TransitionSolverFrame* frame = static_cast<TransitionSolverFrame*>(GetFrame());
    frame->Update();
}

void TransitionSolverView::ApplyTransitionSolution(CalChart::TransitionSolverResult solution)
{
    if (solution.successfullySolved) {
        GetDocument()->GetCommandProcessor()->Submit(static_cast<CalChartDoc*>(GetDocument())->Create_SetTransitionCommand(solution.finalPositions, solution.continuities, solution.marcherDotTypes).release());
    }
}

void TransitionSolverView::SelectMarchers(std::set<unsigned> marchers)
{
    std::set<int> selectionList;
    for (unsigned marcher : marchers) {
        selectionList.insert(marcher);
    }
    GetDocument()->GetCommandProcessor()->Submit(static_cast<CalChartDoc*>(GetDocument())->Create_SetSelectionCommand(selectionList).release());
}
