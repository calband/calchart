//
//  TransitionSolver.cpp
//  CalChart
//
//  Created by Kevin Durand on 6/24/17.
//
//

#include "TransitionSolverProgressFrame.h"
#include "CalChartApp.h"
#include "CalChartContinuity.h"
#include "CalChartDoc.h"
#include "CalChartDocCommand.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "TransitionSolverView.h"
#include "basic_ui.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>

#include <atomic>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

#pragma mark -

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(CALCHART__TRANSITION_SOLVER__TRANSITION_PROGRESS_EVT, CALCHART__TRANSITION_SOLVER__TRANSITION_PROGRESS)
DECLARE_EVENT_TYPE(CALCHART__TRANSITION_SOLVER__TRANSITION_SUBTASK_PROGRESS_EVT, CALCHART__TRANSITION_SOLVER__TRANSITION_SUBTASK_PROGRESS)
DECLARE_EVENT_TYPE(CALCHART__TRANSITION_SOLVER__TRANSITION_SOLUTION_FOUND_EVT, CALCHART__TRANSITION_SOLVER__TRANSITION_SOLUTION_FOUND)
DECLARE_EVENT_TYPE(CALCHART__TRANSITION_SOLVER__CALCULATION_COMPLETE_EVT, CALCHART__TRANSITION_SOLVER__CALCULATION_COMPLETE)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(CALCHART__TRANSITION_SOLVER__TRANSITION_PROGRESS_EVT)
DEFINE_EVENT_TYPE(CALCHART__TRANSITION_SOLVER__TRANSITION_SUBTASK_PROGRESS_EVT)
DEFINE_EVENT_TYPE(CALCHART__TRANSITION_SOLVER__TRANSITION_SOLUTION_FOUND_EVT)
DEFINE_EVENT_TYPE(CALCHART__TRANSITION_SOLVER__CALCULATION_COMPLETE_EVT)

BEGIN_EVENT_TABLE(TransitionSolverProgressFrame, wxDialog)
EVT_COMMAND(wxID_ANY, CALCHART__TRANSITION_SOLVER__TRANSITION_PROGRESS_EVT, TransitionSolverProgressFrame::OnProgressUpdate)
EVT_COMMAND(wxID_ANY, CALCHART__TRANSITION_SOLVER__TRANSITION_SUBTASK_PROGRESS_EVT, TransitionSolverProgressFrame::OnSubtaskProgressUpdate)
EVT_COMMAND(wxID_ANY, CALCHART__TRANSITION_SOLVER__TRANSITION_SOLUTION_FOUND_EVT, TransitionSolverProgressFrame::OnNewBestSolutionFound)
EVT_COMMAND(wxID_ANY, CALCHART__TRANSITION_SOLVER__CALCULATION_COMPLETE_EVT, TransitionSolverProgressFrame::OnCalculationComplete)
END_EVENT_TABLE()

#pragma mark - TransitionSolverProgressFrame Implementation

TransitionSolverProgressFrame::TransitionSolverProgressFrame()
{
    Init();
}

TransitionSolverProgressFrame::TransitionSolverProgressFrame(CalChart::TransitionSolverParams params, TransitionSolverView* view, wxWindow* parent,
    wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size,
    long style)
{
    Init();

    Create(params, view, parent, id, caption, pos, size, style);
}

void TransitionSolverProgressFrame::Init() { }

bool TransitionSolverProgressFrame::Create(CalChart::TransitionSolverParams params, TransitionSolverView* view, wxWindow* parent,
    wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size,
    long style)
{
    if (!wxDialog::Create(parent, id, caption, pos, size, style))
        return false;

    mView = view;
    mSolverParams = params;
    mProgress = 0;
    mNumBeatsInBestResult = 0;
    mSolutionFound = false;
    mShouldAbortCalculation = false;
    mShouldApplyResultOnCompletion = true;
    mTaskThread = new TransitionSolverThread(this);
    mTaskThread->Create();
    mTaskThread->Run();

    CreateControls();

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);

    Center();

    // now update the current screen
    Update();

    return true;
}

void TransitionSolverProgressFrame::CreateControls()
{
    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 5).Expand(),
        wxUI::Text{ "Working on a solution..." },
        wxUI::Gauge{}.withProxy(mProgressBar),
        wxUI::Gauge{}.withProxy(mSubtaskProgressBar),
        wxUI::Text{ "Best solution so far:" },
        wxUI::Text{ "No solution found" }.withProxy(mBestSolutionDescription),
        wxUI::HSizer{
            wxUI::Button{ "Cancel" }
                .bind([this] {
                    mShouldAbortCalculation = true;
                    mShouldApplyResultOnCompletion = false;
                }),
            wxUI::Button{ "Finish Now and Apply" }
                .bind([this] {
                    mShouldAbortCalculation = true;
                    mShouldApplyResultOnCompletion = true;
                })
                .withProxy(mAcceptButton),
        },
    }
        .fitTo(this);

    SyncControlsWithCurrentState();
}

void TransitionSolverProgressFrame::SyncControlsWithCurrentState()
{
    // Display the current estimation of progress
    mProgressBar->SetValue(mProgressBar->GetRange() * mProgress);
    mSubtaskProgressBar->SetValue(mSubtaskProgressBar->GetRange() * mSubtaskProgress);

    // Check the current best solution; if one exists, display information about it
    if (mSolutionFound) {
        mBestSolutionDescription->SetLabel("Solution found that lasts " + std::to_string(mNumBeatsInBestResult) + " beats");
    } else {
        mBestSolutionDescription->SetLabel("No solution found");
    }

    // If a solution exists, enable the 'Finish Now' button
    if (mSolutionFound) {
        mAcceptButton->Enable();
    } else {
        mAcceptButton->Disable();
    }
}

void TransitionSolverProgressFrame::Update()
{
    super::Update();
    SyncControlsWithCurrentState();
}

void TransitionSolverProgressFrame::OnProgressUpdate(wxCommandEvent& event)
{
    // Extract the level of progress we've now achieved
    mProgress = static_cast<ProgressNotification*>(event.GetClientData())->mProgress;

    // Update the window to reflect the new changes
    SyncControlsWithCurrentState();
}

void TransitionSolverProgressFrame::OnSubtaskProgressUpdate(wxCommandEvent& event)
{
    // Extract the level of progress we've now achieved
    mSubtaskProgress = static_cast<ProgressNotification*>(event.GetClientData())->mProgress;

    // Update the window to reflect the new changes
    SyncControlsWithCurrentState();
}

void TransitionSolverProgressFrame::OnNewBestSolutionFound(wxCommandEvent& event)
{
    // Get some information about the new best solution
    mSolutionFound = true;
    mNumBeatsInBestResult = static_cast<NewBestSolutionFoundNotification*>(event.GetClientData())->mNumBeatsInSolution;

    // Update the window to reflect the new changes
    SyncControlsWithCurrentState();
}

void TransitionSolverProgressFrame::OnCalculationComplete(wxCommandEvent& event)
{
    // Get information about the solution
    mFinalResult = static_cast<FinalCalculationResultNotification*>(event.GetClientData())->mResult;
    if (mShouldApplyResultOnCompletion) {
        ApplySolution();

        std::string resultSummary = "Could not find a solution. Please adjust the transition parameters and try again.";
        if (mFinalResult.successfullySolved) {
            resultSummary = "Successfully found a solution with a duration of " + std::to_string(mFinalResult.numBeatsOfMovement) + " beats.";
        }

        auto completionDialog = new wxMessageDialog(this, "Calculation Complete", resultSummary);
        completionDialog->ShowModal();
    }

    // We're finished; go ahead and close
    Close();
}

void TransitionSolverProgressFrame::ApplySolution()
{
    mView->ApplyTransitionSolution(mFinalResult);
}

TransitionSolverProgressFrame::TransitionSolverThread::TransitionSolverThread(TransitionSolverProgressFrame* progressFrame)
    : mProgressFrame(progressFrame)
{
}

void TransitionSolverProgressFrame::TransitionSolverThread::OnProgress(double progress)
{
    wxCommandEvent* event = new wxCommandEvent(CALCHART__TRANSITION_SOLVER__TRANSITION_PROGRESS_EVT, wxID_ANY);
    event->SetClientData(new ProgressNotification(progress));
    wxQueueEvent(mProgressFrame, event);
}

void TransitionSolverProgressFrame::TransitionSolverThread::OnSubtaskProgress(double progress)
{
    wxCommandEvent* event = new wxCommandEvent(CALCHART__TRANSITION_SOLVER__TRANSITION_SUBTASK_PROGRESS_EVT, wxID_ANY);
    event->SetClientData(new ProgressNotification(progress));
    wxQueueEvent(mProgressFrame, event);
}

void TransitionSolverProgressFrame::TransitionSolverThread::OnNewPreferredSolution(unsigned numBeatsInSolution)
{
    wxCommandEvent* event = new wxCommandEvent(CALCHART__TRANSITION_SOLVER__TRANSITION_SOLUTION_FOUND_EVT, wxID_ANY);
    event->SetClientData(new NewBestSolutionFoundNotification(numBeatsInSolution));
    wxQueueEvent(mProgressFrame, event);
}

void TransitionSolverProgressFrame::TransitionSolverThread::OnCalculationComplete(CalChart::TransitionSolverResult finalSolution)
{
    wxCommandEvent* event = new wxCommandEvent(CALCHART__TRANSITION_SOLVER__CALCULATION_COMPLETE_EVT, wxID_ANY);
    event->SetClientData(new FinalCalculationResultNotification(finalSolution));
    wxQueueEvent(mProgressFrame, event);
}

bool TransitionSolverProgressFrame::TransitionSolverThread::ShouldAbortCalculation()
{
    return mProgressFrame->mShouldAbortCalculation;
}

void* TransitionSolverProgressFrame::TransitionSolverThread::Entry()
{
    CalChartDoc* doc = static_cast<CalChartDoc*>(mProgressFrame->mView->GetDocument());

    runTransitionSolver(*doc->GetCurrentSheet(), *(doc->GetCurrentSheet() + 1), mProgressFrame->mSolverParams, this);

    return nullptr;
}

#pragma GCC diagnostic pop
