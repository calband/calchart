#pragma once
//
//  TransitionSolverView.h
//  CalChart
//
//  Created by Kevin Durand on 6/24/17.
//
//

#include <wx/dialog.h>
#include <wx/docview.h>

#include "CalChartDoc.h"
#include "e7_transition_solver.h"

// View for linking CalChartDoc with the Transition Solver
class TransitionSolverView : public wxView {
    using super = wxView;

public:
    TransitionSolverView();
    ~TransitionSolverView();

    virtual void OnDraw(wxDC* dc) override;
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL) override;

    void ApplyTransitionSolution(CalChart::TransitionSolverResult solution);
    void SelectMarchers(std::set<unsigned> marchers);
};
