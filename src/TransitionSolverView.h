#pragma once
/*
 * TransitionSolverView.h
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

#include <wx/dialog.h>
#include <wx/docview.h>

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
