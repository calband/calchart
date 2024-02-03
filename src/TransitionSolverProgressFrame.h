#pragma once
/*
 * TransitionSolverProgressFrame.h
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

#include <atomic>
#include <wx/dialog.h>
#include <wx/docview.h>
#include <wxUI/wxUI.h>

#include "e7_transition_solver.h"

class TransitionSolverView;

class TransitionSolverProgressFrame : public wxDialog {
    using super = wxDialog;

private:
    struct ProgressNotification : public wxObject {
        ProgressNotification(double progress)
            : mProgress(progress){};
        double mProgress;
    };

    struct NewBestSolutionFoundNotification : public wxObject {
        NewBestSolutionFoundNotification(unsigned numBeatsInSolution)
            : mNumBeatsInSolution(numBeatsInSolution){};
        unsigned mNumBeatsInSolution;
    };

    struct FinalCalculationResultNotification : public wxObject {
        FinalCalculationResultNotification(const CalChart::TransitionSolverResult& result)
            : mResult(result){};
        CalChart::TransitionSolverResult mResult;
    };

    class TransitionSolverThread : public wxThread, public CalChart::TransitionSolverDelegate {
    public:
        TransitionSolverThread(TransitionSolverProgressFrame* progressFrame);

    private:
        void OnProgress(double progress) override;
        void OnSubtaskProgress(double progress) override;
        void OnNewPreferredSolution(unsigned numBeatsInSolution) override;
        void OnCalculationComplete(CalChart::TransitionSolverResult finalSolution) override;
        bool ShouldAbortCalculation() override;

        void* Entry() override;

        TransitionSolverProgressFrame* mProgressFrame;
    };

public:
    TransitionSolverProgressFrame();
    TransitionSolverProgressFrame(CalChart::TransitionSolverParams params, TransitionSolverView* view, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Solving Transition..."),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

    void OnProgressUpdate(wxCommandEvent& event);
    void OnSubtaskProgressUpdate(wxCommandEvent& event);
    void OnNewBestSolutionFound(wxCommandEvent& event);
    void OnCalculationComplete(wxCommandEvent& event);

    void Update() override;
    void SyncControlsWithCurrentState();

private:
    void Init();
    bool Create(CalChart::TransitionSolverParams params, TransitionSolverView* view, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Solving Transition..."),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    void CreateControls();

    void ApplySolution();

    TransitionSolverThread* mTaskThread;
    TransitionSolverView* mView;

    std::atomic<bool> mShouldAbortCalculation;
    bool mShouldApplyResultOnCompletion;

    bool mSolutionFound;
    double mProgress;
    double mSubtaskProgress;
    unsigned mNumBeatsInBestResult;
    CalChart::TransitionSolverResult mFinalResult;

    CalChart::TransitionSolverParams mSolverParams;

    wxUI::Gauge::Proxy mProgressBar;
    wxUI::Gauge::Proxy mSubtaskProgressBar;
    wxUI::Text::Proxy mBestSolutionDescription;
    wxUI::Button::Proxy mAcceptButton;

    DECLARE_EVENT_TABLE()
};
