//
//  e7_transition_solver_ui.cpp
//  CalChart
//
//  Created by Kevin Durand on 6/24/17.
//
//

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/statline.h>
#include <wx/msgdlg.h>

#include "e7_transition_solver_ui.h"
#include "basic_ui.h"
#include "confgr.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_command.h"
#include "calchartapp.h"
#include "calchartdoc.h"
#include "cc_show.h"

#pragma mark -

enum {
    CALCHART__TRANSITION_SOLVER__CLOSE,
    CALCHART__TRANSITION_SOLVER__APPLY,
    CALCHART__TRANSITION_SOLVER__SELECT_ALGORITHM,
    CALCHART__TRANSITION_SOLVER__EDIT_ALLOWED_COMMANDS,
    CALCHART__TRANSITION_SOLVER__ADD_NEW_GROUP,
    CALCHART__TRANSITION_SOLVER__REMOVE_GROUP,
    CALCHART__TRANSITION_SOLVER__SELECT_GROUP,
    CALCHART__TRANSITION_SOLVER__NULL,
    CALCHART__TRANSITION_SOLVER__CLEAR_GROUP_MEMBERS,
    CALCHART__TRANSITION_SOLVER__SET_GROUP_MEMBERS,
    CALCHART__TRANSITION_SOLVER__ADD_GROUP_MEMBERS,
    CALCHART__TRANSITION_SOLVER__REMOVE_GROUP_MEMBERS,
    CALCHART__TRANSITION_SOLVER__SELECT_GROUP_MEMBERS,
    CALCHART__TRANSITION_SOLVER__CLEAR_GROUP_DESTINATIONS,
    CALCHART__TRANSITION_SOLVER__SET_GROUP_DESTINATIONS,
    CALCHART__TRANSITION_SOLVER__ADD_GROUP_DESTINATIONS,
    CALCHART__TRANSITION_SOLVER__REMOVE_GROUP_DESTINATIONS,
    CALCHART__TRANSITION_SOLVER__SELECT_GROUP_DESTINATIONS,
    CALCHART__TRANSITION_SOLVER__TRANSITION_PROGRESS,
    CALCHART__TRANSITION_SOLVER__TRANSITION_SUBTASK_PROGRESS,
    CALCHART__TRANSITION_SOLVER__TRANSITION_SOLUTION_FOUND,
    CALCHART__TRANSITION_SOLVER__CALCULATION_COMPLETE,
    CALCHART__TRANSITION_SOLVER__CANCEL_CALCULATION,
    CALCHART__TRANSITION_SOLVER__FINISH_CALCULATION_NOW_AND_APPLY,
};

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

#pragma mark - TransitionSolverProgressFrame Declaration

class TransitionSolverProgressFrame : public wxDialog {
    using super = wxDialog;
private:
    class ProgressNotification : public wxObject {
    public:
        ProgressNotification(double progress) : mProgress(progress) {};
        
        double mProgress;
    };
    
    class NewBestSolutionFoundNotification : public wxObject {
    public:
        NewBestSolutionFoundNotification(unsigned numBeatsInSolution) : mNumBeatsInSolution(numBeatsInSolution) {};
        
        unsigned mNumBeatsInSolution;
    };
    
    class FinalCalculationResultNotification : public wxObject {
    public:
        FinalCalculationResultNotification(const TransitionSolverResult &result) : mResult(result) {};
        
        TransitionSolverResult mResult;
    };
    
    class TransitionSolverThread : public wxThread, public TransitionSolverDelegate {
    public:
        TransitionSolverThread(TransitionSolverProgressFrame *progressFrame);
    private:
        void OnProgress(double progress) override;
        void OnSubtaskProgress(double progress) override;
        void OnNewPreferredSolution(unsigned numBeatsInSolution) override;
        void OnCalculationComplete(TransitionSolverResult finalSolution) override;
        bool ShouldAbortCalculation() override;
        
        void *Entry() override;
        
        TransitionSolverProgressFrame   *mProgressFrame;
    };
public:
    TransitionSolverProgressFrame();
    TransitionSolverProgressFrame(TransitionSolverParams params, TransitionSolverView *view, wxWindow* parent, wxWindowID id = wxID_ANY,
                                  const wxString& caption = wxT("Solving Transition..."),
                                  const wxPoint& pos = wxDefaultPosition,
                                  const wxSize& size = wxDefaultSize,
                                  long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    
    void OnCancel(wxCommandEvent &event);
    void OnFinishNowAndApply(wxCommandEvent &event);
    void OnProgressUpdate(wxCommandEvent &event);
    void OnSubtaskProgressUpdate(wxCommandEvent &event);
    void OnNewBestSolutionFound(wxCommandEvent &event);
    void OnCalculationComplete(wxCommandEvent &event);
    
    void Update() override;
    void SyncControlsWithCurrentState();
    
private:
    
    void Init();
    bool Create(TransitionSolverParams params, TransitionSolverView *view, wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxString& caption = wxT("Solving Transition..."),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    void CreateControls();
    
    void ApplySolution();
    
    TransitionSolverThread                          *mTaskThread;
    TransitionSolverView                            *mView;
    
    std::atomic<bool>                               mShouldAbortCalculation;
    bool                                            mShouldApplyResultOnCompletion;
    
    bool                                            mSolutionFound;
    double                                          mProgress;
    double                                          mSubtaskProgress;
    unsigned                                        mNumBeatsInBestResult;
    TransitionSolverResult                          mFinalResult;
    
    TransitionSolverParams                          mSolverParams;
    
    wxGauge                                         *mProgressBar;
    wxGauge                                         *mSubtaskProgressBar;
    wxStaticText                                    *mBestSolutionDescription;
    wxButton                                        *mAcceptButton;
    
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(TransitionSolverProgressFrame, wxDialog)
EVT_COMMAND(wxID_ANY, CALCHART__TRANSITION_SOLVER__TRANSITION_PROGRESS_EVT, TransitionSolverProgressFrame::OnProgressUpdate)
EVT_COMMAND(wxID_ANY, CALCHART__TRANSITION_SOLVER__TRANSITION_SUBTASK_PROGRESS_EVT, TransitionSolverProgressFrame::OnSubtaskProgressUpdate)
EVT_COMMAND(wxID_ANY, CALCHART__TRANSITION_SOLVER__TRANSITION_SOLUTION_FOUND_EVT, TransitionSolverProgressFrame::OnNewBestSolutionFound)
EVT_COMMAND(wxID_ANY, CALCHART__TRANSITION_SOLVER__CALCULATION_COMPLETE_EVT, TransitionSolverProgressFrame::OnCalculationComplete)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__CANCEL_CALCULATION, TransitionSolverProgressFrame::OnCancel)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__FINISH_CALCULATION_NOW_AND_APPLY, TransitionSolverProgressFrame::OnFinishNowAndApply)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(TransitionSolverFrame, wxFrame)
EVT_MENU(CALCHART__TRANSITION_SOLVER__CLOSE, TransitionSolverFrame::OnCloseWindow)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__CLOSE, TransitionSolverFrame::OnCloseWindow)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__APPLY, TransitionSolverFrame::OnApply)
EVT_CHOICE(CALCHART__TRANSITION_SOLVER__SELECT_ALGORITHM, TransitionSolverFrame::OnChooseAlgorithm)
EVT_LISTBOX(CALCHART__TRANSITION_SOLVER__EDIT_ALLOWED_COMMANDS, TransitionSolverFrame::OnEditAllowedCommands)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__ADD_NEW_GROUP, TransitionSolverFrame::OnAddNewGroup)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__REMOVE_GROUP, TransitionSolverFrame::OnRemoveGroup)
EVT_LISTBOX(CALCHART__TRANSITION_SOLVER__SELECT_GROUP, TransitionSolverFrame::OnSelectGroup)
EVT_LISTBOX(CALCHART__TRANSITION_SOLVER__NULL, TransitionSolverFrame::OnNullEvent)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__CLEAR_GROUP_MEMBERS, TransitionSolverFrame::OnClearMembers)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__SET_GROUP_MEMBERS, TransitionSolverFrame::OnSetMembers)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__ADD_GROUP_MEMBERS, TransitionSolverFrame::OnAddMembers)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__REMOVE_GROUP_MEMBERS, TransitionSolverFrame::OnRemoveMembers)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__SELECT_GROUP_MEMBERS, TransitionSolverFrame::OnSelectMembers)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__CLEAR_GROUP_DESTINATIONS, TransitionSolverFrame::OnClearDestinations)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__SET_GROUP_DESTINATIONS, TransitionSolverFrame::OnSetDestinations)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__ADD_GROUP_DESTINATIONS, TransitionSolverFrame::OnAddDestinations)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__REMOVE_GROUP_DESTINATIONS, TransitionSolverFrame::OnRemoveDestinations)
EVT_BUTTON(CALCHART__TRANSITION_SOLVER__SELECT_GROUP_DESTINATIONS, TransitionSolverFrame::OnSelectDestinations)
END_EVENT_TABLE()

#pragma mark - TransitionSolverView Implementation

TransitionSolverView::TransitionSolverView() {}
TransitionSolverView::~TransitionSolverView() {}

void TransitionSolverView::OnDraw(wxDC* dc) {}
void TransitionSolverView::OnUpdate(wxView* sender, wxObject* hint) {
    TransitionSolverFrame* frame = static_cast<TransitionSolverFrame*>(GetFrame());
    frame->Update();
}

void TransitionSolverView::ApplyTransitionSolution(TransitionSolverResult solution) {
    if (solution.successfullySolved)
    {
        GetDocument()->GetCommandProcessor()->Submit(static_cast<CalChartDoc*>(GetDocument())->Create_SetTransitionCommand(solution.finalPositions, solution.continuities, solution.marcherDotTypes).release());
    }
}

void TransitionSolverView::SelectMarchers(std::set<unsigned> marchers) {
    std::set<int> selectionList;
    for (unsigned marcher : marchers)
    {
        selectionList.insert(marcher);
    }
    GetDocument()->GetCommandProcessor()->Submit(static_cast<CalChartDoc*>(GetDocument())->Create_SetSelectionCommand(selectionList).release());
}

#pragma mark - TransitionSolverProgressFrame Implementation

TransitionSolverProgressFrame::TransitionSolverProgressFrame() { Init(); }

TransitionSolverProgressFrame::TransitionSolverProgressFrame(TransitionSolverParams params, TransitionSolverView *view, wxWindow* parent,
                                                             wxWindowID id, const wxString& caption,
                                                             const wxPoint& pos, const wxSize& size,
                                                             long style) {
    Init();
    
    Create(params, view, parent, id, caption, pos, size, style);
}

void TransitionSolverProgressFrame::Init() {}

bool TransitionSolverProgressFrame::Create(TransitionSolverParams params, TransitionSolverView *view, wxWindow* parent,
                                           wxWindowID id, const wxString& caption,
                                           const wxPoint& pos, const wxSize& size,
                                           long style) {
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

void TransitionSolverProgressFrame::CreateControls() {
    // create a sizer for laying things out top down:
    wxBoxSizer* topLevelSizer = new wxBoxSizer(wxVERTICAL);
    {
        wxStaticText    *heading;
        wxStaticText    *status;
        wxBoxSizer      *buttons;
        
        heading = new wxStaticText(this, wxID_STATIC, wxT("Working on a solution..."));
        
        mProgressBar = new wxGauge(this, wxID_ANY, 500);
        
        mSubtaskProgressBar = new wxGauge(this, wxID_ANY, 500);
        
        status = new wxStaticText(this, wxID_STATIC, wxT("Best solution so far:"));
        
        mBestSolutionDescription = new wxStaticText(this, wxID_STATIC, wxT("No solution found"));
        
        buttons = new wxBoxSizer(wxHORIZONTAL);
        {
            wxButton    *cancelButton;
            
            mAcceptButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__FINISH_CALCULATION_NOW_AND_APPLY, wxT("Finish Now and Apply"));
            
            cancelButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__CANCEL_CALCULATION, wxT("Cancel"));
            
            buttons->Add(cancelButton);
            buttons->Add(mAcceptButton);
        }
        
        topLevelSizer->Add(heading);
        topLevelSizer->Add(mProgressBar, 0, wxGROW | wxALL, 5);
        topLevelSizer->Add(mSubtaskProgressBar, 0, wxGROW | wxALL, 5);
        topLevelSizer->Add(status);
        topLevelSizer->Add(mBestSolutionDescription);
        topLevelSizer->Add(buttons);
    }
    
    SyncControlsWithCurrentState();
    SetSizer(topLevelSizer);
}

void TransitionSolverProgressFrame::SyncControlsWithCurrentState() {
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

void TransitionSolverProgressFrame::Update() {
    super::Update();
    SyncControlsWithCurrentState();
}

void TransitionSolverProgressFrame::OnCancel(wxCommandEvent &event) {
    mShouldAbortCalculation = true;
    mShouldApplyResultOnCompletion = false;
}

void TransitionSolverProgressFrame::OnFinishNowAndApply(wxCommandEvent &event) {
    mShouldAbortCalculation = true;
    mShouldApplyResultOnCompletion = true;
}

void TransitionSolverProgressFrame::OnProgressUpdate(wxCommandEvent &event) {
    // Extract the level of progress we've now achieved
    ProgressNotification        *progressNotification;
    
    progressNotification = static_cast<ProgressNotification *>(event.GetClientData());
    mProgress = progressNotification->mProgress;
    
    // Update the window to reflect the new changes
    SyncControlsWithCurrentState();
}

void TransitionSolverProgressFrame::OnSubtaskProgressUpdate(wxCommandEvent &event) {
    // Extract the level of progress we've now achieved
    ProgressNotification        *progressNotification;
    
    progressNotification = static_cast<ProgressNotification *>(event.GetClientData());
    mSubtaskProgress = progressNotification->mProgress;
    
    // Update the window to reflect the new changes
    SyncControlsWithCurrentState();
}

void TransitionSolverProgressFrame::OnNewBestSolutionFound(wxCommandEvent &event) {
    // Get some information about the new best solution
    NewBestSolutionFoundNotification    *solutionNotification;
    
    solutionNotification = static_cast<NewBestSolutionFoundNotification *>(event.GetClientData());
    mSolutionFound = true;
    mNumBeatsInBestResult = solutionNotification->mNumBeatsInSolution;
    
    // Update the window to reflect the new changes
    SyncControlsWithCurrentState();
}

void TransitionSolverProgressFrame::OnCalculationComplete(wxCommandEvent &event) {
    // Get information about the solution
    FinalCalculationResultNotification    *solutionNotification;
    
    solutionNotification = static_cast<FinalCalculationResultNotification *>(event.GetClientData());
    mFinalResult = solutionNotification->mResult;
    if (mShouldApplyResultOnCompletion) {
        wxMessageDialog                 *completionDialog;
        std::string                     resultSummary;
        
        ApplySolution();
        
        if (mFinalResult.successfullySolved) {
            resultSummary = "Successfully found a solution with a duration of " + std::to_string(mFinalResult.numBeatsOfMovement) + " beats.";
        } else {
            resultSummary = "Could not find a solution. Please adjust the transition parameters and try again.";
        }
        
        completionDialog = new wxMessageDialog(this, "Calculation Complete", resultSummary);
        
        completionDialog->ShowModal();
    }
    
    // We're finished; go ahead and close
    Close();
}

void TransitionSolverProgressFrame::ApplySolution() {
    mView->ApplyTransitionSolution(mFinalResult);
}

TransitionSolverProgressFrame::TransitionSolverThread::TransitionSolverThread(TransitionSolverProgressFrame *progressFrame)
: mProgressFrame(progressFrame)
{}

void TransitionSolverProgressFrame::TransitionSolverThread::OnProgress(double progress) {
    wxCommandEvent *event = new wxCommandEvent(CALCHART__TRANSITION_SOLVER__TRANSITION_PROGRESS_EVT, wxID_ANY);
    event->SetClientData(new ProgressNotification(progress));
    wxQueueEvent(mProgressFrame, event);
}

void TransitionSolverProgressFrame::TransitionSolverThread::OnSubtaskProgress(double progress) {
    wxCommandEvent *event = new wxCommandEvent(CALCHART__TRANSITION_SOLVER__TRANSITION_SUBTASK_PROGRESS_EVT, wxID_ANY);
    event->SetClientData(new ProgressNotification(progress));
    wxQueueEvent(mProgressFrame, event);
}

void TransitionSolverProgressFrame::TransitionSolverThread::OnNewPreferredSolution(unsigned numBeatsInSolution) {
    wxCommandEvent *event = new wxCommandEvent(CALCHART__TRANSITION_SOLVER__TRANSITION_SOLUTION_FOUND_EVT, wxID_ANY);
    event->SetClientData(new NewBestSolutionFoundNotification(numBeatsInSolution));
    wxQueueEvent(mProgressFrame, event);
}

void TransitionSolverProgressFrame::TransitionSolverThread::OnCalculationComplete(TransitionSolverResult finalSolution) {
    wxCommandEvent *event = new wxCommandEvent(CALCHART__TRANSITION_SOLVER__CALCULATION_COMPLETE_EVT, wxID_ANY);
    event->SetClientData(new FinalCalculationResultNotification(finalSolution));
    wxQueueEvent(mProgressFrame, event);
}

bool TransitionSolverProgressFrame::TransitionSolverThread::ShouldAbortCalculation() {
    return mProgressFrame->mShouldAbortCalculation;
}

void *TransitionSolverProgressFrame::TransitionSolverThread::Entry() {
    CalChartDoc         *doc;
    
    doc = static_cast<CalChartDoc *>(mProgressFrame->mView->GetDocument());
    
    runTransitionSolver(*doc->GetCurrentSheet(), *(doc->GetCurrentSheet() + 1), mProgressFrame->mSolverParams, this);
    
    return nullptr;
}

#pragma mark - TransitionSolverFrame Implementation

TransitionSolverFrame::TransitionSolverFrame() { Init(); }

TransitionSolverFrame::TransitionSolverFrame(CalChartDoc* show, wxWindow* parent,
                                             wxWindowID id, const wxString& caption,
                                             const wxPoint& pos, const wxSize& size,
                                             long style) {
    Init();
    
    Create(show, parent, id, caption, pos, size, style);
}

void TransitionSolverFrame::Init() {}

bool TransitionSolverFrame::Create(CalChartDoc* show, wxWindow* parent,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size,
                                   long style) {
    if (!wxFrame::Create(parent, id, caption, pos, size, style)) {
        return false;
    }
    
    mDoc = show;
    mView = new TransitionSolverView;
    mView->SetDocument(show);
    mView->SetFrame(this);
    mSelectedGroup = -1;
    mSolverParams.algorithm = TransitionSolverParams::AlgorithmIdentifier::BEGIN;
    for (unsigned i = 0; i < mSolverParams.availableInstructionsMask.size(); i++) {
        mSolverParams.availableInstructionsMask[i] = true;
        mSolverParams.availableInstructions[i].waitBeats = (i / (unsigned)TransitionSolverParams::MarcherInstruction::Pattern::END) * 2;
        mSolverParams.availableInstructions[i].movementPattern = (TransitionSolverParams::MarcherInstruction::Pattern)(i % (unsigned)TransitionSolverParams::MarcherInstruction::Pattern::END);
    }
    
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

void TransitionSolverFrame::CreateControls() {
    // menu bar
    wxMenu* cont_menu = new wxMenu;
    cont_menu->Append(wxID_CLOSE, wxT("Close Window\tCTRL-W"),
                      wxT("Close this window"));
    
    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(cont_menu, wxT("&File"));
    SetMenuBar(menu_bar);
    
    // create a sizer for laying things out top down:
    wxBoxSizer* topLevelSizer = new wxBoxSizer(wxVERTICAL);
    {
        wxBoxSizer *headerSizer;
        wxStaticLine *headerUnderLine;
        wxBoxSizer *algorithmSelectionRegion;
        wxStaticLine *algorithmUnderLine;
        wxBoxSizer *parameterEditor;
        wxStaticLine *parameterEditorUnderLine;
        wxBoxSizer *bottomButtonRowSizer;
        
        headerSizer = new wxBoxSizer(wxHORIZONTAL);
        {
            wxStaticText *introText;
            
            introText = new wxStaticText(this, wxID_STATIC, wxT("Welcome to the CalChart Transition Solver!\n"
                                                                "The algorithms used by this solver are credited to the E7 class  of Spring 2016.\n"
                                                                "Staff: (Professor) Tina Chow, and (GSIs) Lucas Bastien and Bradley Harken\n"
                                                                "Algorithms were selected from three different student groups:\n"
                                                                "(1) Chiu, Zamora, Malani (2) Namini Asl, Ramirez, Zhang (3) Sover, Eliceiri, Hershkovitz\n"
                                                                "Additional thanks to professor Scott Moura for helping the Cal Band get support from the UC Berkeley\n"
                                                                "Civil Engineering Department for developing these algorithms."));
            
            headerSizer->Add(introText);
        }
        
        headerUnderLine = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
        
        algorithmSelectionRegion = new wxBoxSizer(wxHORIZONTAL);
        {
            const wxString algorithmChoices[] = {
                "E7 Algorithm: Chiu, Zamora, Malani",
                "E7 Algorithm: Namini Asl, Ramirez, Zhang",
                "Ey Algorithm: Sover, Eliceiri, Hershkovitz",
            };
            
            wxStaticText *label;
            
            label = new wxStaticText(this, wxID_STATIC, wxT("Select an algorithm: "));
            
            mAlgorithmChoiceControl = new wxChoice(this, CALCHART__TRANSITION_SOLVER__SELECT_ALGORITHM, wxDefaultPosition,
                                                   wxDefaultSize, TransitionSolverParams::AlgorithmIdentifier::END, algorithmChoices);
            
            algorithmSelectionRegion->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            algorithmSelectionRegion->Add(mAlgorithmChoiceControl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                                     
        }
        
        algorithmUnderLine = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
        
        parameterEditor = new wxBoxSizer(wxVERTICAL);
        {
            wxBoxSizer *availableCommandsEditorRegion;
            wxStaticLine *editorSeparator;
            wxBoxSizer *groupsEditor;
            
            availableCommandsEditorRegion = new wxBoxSizer(wxHORIZONTAL);
            {
                wxBoxSizer *helpArea;
                
                helpArea = new wxBoxSizer(wxVERTICAL);
                {
                    wxStaticText *controlLabel;
                    wxStaticText *promptLabel;
                    
                    controlLabel = new wxStaticText(this, wxID_STATIC, wxT("Please select a set of marcher instructions to allow.\n (You may select up to 8)\n"));
                    {
                        controlLabel->Wrap(180);
                    }
                    
                    promptLabel = new wxStaticText(this, wxID_STATIC, wxT("Selected Instructions:"));
                    
                    mNumSelectedInstructionsIndicator = new wxStaticText(this, wxID_ANY, wxT("0"));
                    
                    helpArea->Add(controlLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                    helpArea->Add(promptLabel,  0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                    helpArea->Add(mNumSelectedInstructionsIndicator, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                }
                
                mAvailableCommandsControl = new wxListBox(this, CALCHART__TRANSITION_SOLVER__EDIT_ALLOWED_COMMANDS, wxDefaultPosition, wxSize(400, 100), 0, NULL, wxLB_EXTENDED);
                
                availableCommandsEditorRegion->Add(helpArea, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                availableCommandsEditorRegion->Add(mAvailableCommandsControl, 0, wxGROW | wxALL, 5);
            }
            
            editorSeparator = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
            
            groupsEditor = new wxBoxSizer(wxVERTICAL);
            {
                wxBoxSizer *addGroupRow;
                wxBoxSizer *selectGroupRow;
                wxBoxSizer *groupDetailsRow;
                
                addGroupRow = new wxBoxSizer(wxHORIZONTAL);
                {
                    wxStaticText *controlLabel;
                    
                    controlLabel = new wxStaticText(this, wxID_STATIC, wxT("New Group:"));
                    
                    mNewGroupNameControl = new wxTextCtrl(this, wxID_ANY);
                    
                    mAddGroupButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__ADD_NEW_GROUP, wxT("Add"));
                    
                    addGroupRow->Add(controlLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                    addGroupRow->Add(mNewGroupNameControl, 0, wxGROW | wxALL, 5);
                    addGroupRow->Add(mAddGroupButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                }
                
                selectGroupRow = new wxBoxSizer(wxHORIZONTAL);
                {
                    wxStaticText *controlLabel;
                    
                    controlLabel = new wxStaticText(this, wxID_STATIC, wxT("Viewing Group:"));
                    
                    mCurrentGroupControl = new wxListBox(this, CALCHART__TRANSITION_SOLVER__SELECT_GROUP, wxDefaultPosition, wxSize(400, 100), 0, NULL, wxLB_EXTENDED);
                    
                    mRemoveGroupControl = new wxButton(this, CALCHART__TRANSITION_SOLVER__REMOVE_GROUP, wxT("Remove"));
                    
                    selectGroupRow->Add(controlLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                    selectGroupRow->Add(mCurrentGroupControl, 0, wxGROW | wxALL, 5);
                    selectGroupRow->Add(mRemoveGroupControl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                }
                
                groupDetailsRow = new wxBoxSizer(wxHORIZONTAL);
                {
                    wxBoxSizer *hintRegion;
                    wxBoxSizer *currentMembersDisplayRegion;
                    wxBoxSizer *currentDestinationsDisplayRegion;
                    
                    hintRegion = new wxBoxSizer(wxVERTICAL);
                    {
                        wxStaticText *hint;
                        wxStaticText *numSelectedPointsPrompt;
                        
                        hint = new wxStaticText(this, wxID_STATIC, wxT("Select marchers on the field to enable adding them as members or destinations of the current group."));
                        {
                            hint->Wrap(150);
                        }
                        
                        numSelectedPointsPrompt = new wxStaticText(this, wxID_STATIC, wxT("Number of selected marchers:"));
                        
                        mNumberOfSelectedPointsLabel = new wxStaticText(this, wxID_STATIC, wxT("0"));
                        
                        hintRegion->Add(hint, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                        hintRegion->Add(numSelectedPointsPrompt, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                        hintRegion->Add(mNumberOfSelectedPointsLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                    }
                    
                    currentMembersDisplayRegion = new wxBoxSizer(wxVERTICAL);
                    {
                        wxStaticText *controlLabel;
                        wxBoxSizer *memberEditControls;
                        
                        controlLabel = new wxStaticText(this, wxID_STATIC, wxT("Group Members:"));
                        
                        mCurrentGroupMembersList = new wxListBox(this, CALCHART__TRANSITION_SOLVER__NULL, wxDefaultPosition, wxSize(130, 100), 0, NULL, wxLB_EXTENDED);
                        
                        memberEditControls = new wxBoxSizer(wxVERTICAL);
                        {
                            wxBoxSizer *firstRow;
                            wxBoxSizer *secondRow;
                            wxBoxSizer *thirdRow;
                            
                            firstRow = new wxBoxSizer(wxHORIZONTAL);
                            {
                                mClearMembersButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__CLEAR_GROUP_MEMBERS, wxT("Clear"));
                                mSetMembersToSelectionButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__SET_GROUP_MEMBERS, wxT("Set"));
                                
                                firstRow->Add(mClearMembersButton);
                                firstRow->Add(mSetMembersToSelectionButton);
                            }
                            
                            secondRow = new wxBoxSizer(wxHORIZONTAL);
                            {
                                mAddSelectionToMembersButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__ADD_GROUP_MEMBERS, wxT("Add"));
                                mRemoveSelectionFromMembersButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__REMOVE_GROUP_MEMBERS, wxT("Remove"));
                                
                                
                                secondRow->Add(mAddSelectionToMembersButton);
                                secondRow->Add(mRemoveSelectionFromMembersButton);
                            }
                            
                            thirdRow = new wxBoxSizer(wxHORIZONTAL);
                            {
                                mSelectMembersButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__SELECT_GROUP_MEMBERS, wxT("Select"));
                                
                                thirdRow->Add(mSelectMembersButton);
                            }
                            
                            memberEditControls->Add(firstRow);
                            memberEditControls->Add(secondRow);
                            memberEditControls->Add(thirdRow, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                        }
                        
                        currentMembersDisplayRegion->Add(controlLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                        currentMembersDisplayRegion->Add(mCurrentGroupMembersList, 0, wxGROW | wxALL, 5);
                        currentMembersDisplayRegion->Add(memberEditControls, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                        
                    }
                    
                    currentDestinationsDisplayRegion = new wxBoxSizer(wxVERTICAL);
                    {
                        wxStaticText *controlLabel;
                        wxBoxSizer *destinationEditControls;
                        
                        controlLabel = new wxStaticText(this, wxID_STATIC, wxT("Group Allowed Destinations:"));
                        
                        mCurrentGroupDestinationsList = new wxListBox(this, CALCHART__TRANSITION_SOLVER__NULL, wxDefaultPosition, wxSize(130, 100), 0, NULL, wxLB_EXTENDED);
                        
                        destinationEditControls = new wxBoxSizer(wxVERTICAL);
                        {
                            wxBoxSizer *firstRow;
                            wxBoxSizer *secondRow;
                            wxBoxSizer *thirdRow;
                            
                            firstRow = new wxBoxSizer(wxHORIZONTAL);
                            {
                                mClearDestinationsButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__CLEAR_GROUP_DESTINATIONS, wxT("Clear"));
                                mSetDestinationsToSelectionButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__SET_GROUP_DESTINATIONS, wxT("Set"));
                                
                                firstRow->Add(mClearDestinationsButton);
                                firstRow->Add(mSetDestinationsToSelectionButton);
                            }
                            
                            secondRow = new wxBoxSizer(wxHORIZONTAL);
                            {
                                mAddSelectionToDestinationsButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__ADD_GROUP_DESTINATIONS, wxT("Add"));
                                mRemoveSelectionFromDestinationsButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__REMOVE_GROUP_DESTINATIONS, wxT("Remove"));
                                
                                secondRow->Add(mAddSelectionToDestinationsButton);
                                secondRow->Add(mRemoveSelectionFromDestinationsButton);
                            }
                            
                            thirdRow = new wxBoxSizer(wxHORIZONTAL);
                            {
                                mSelectDestinationsButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__SELECT_GROUP_DESTINATIONS, wxT("Select"));
                                
                                thirdRow->Add(mSelectDestinationsButton);
                            }
                            
                            destinationEditControls->Add(firstRow);
                            destinationEditControls->Add(secondRow);
                            destinationEditControls->Add(thirdRow, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                        }
                        
                        currentDestinationsDisplayRegion->Add(controlLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                        currentDestinationsDisplayRegion->Add(mCurrentGroupDestinationsList, 0, wxGROW | wxALL, 5);
                        currentDestinationsDisplayRegion->Add(destinationEditControls, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
                    }
                    
                    groupDetailsRow->Add(hintRegion, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                    groupDetailsRow->Add(currentMembersDisplayRegion);
                    groupDetailsRow->Add(currentDestinationsDisplayRegion);
                }
                
                groupsEditor->Add(addGroupRow);
                groupsEditor->Add(selectGroupRow);
                groupsEditor->Add(groupDetailsRow);
            }
            
            parameterEditor->Add(availableCommandsEditorRegion, 0, wxGROW | wxALL, 5);
            parameterEditor->Add(editorSeparator, 0, wxGROW | wxALL, 5);
            parameterEditor->Add(groupsEditor, wxGROW | wxALL, 5);
        }
        
        parameterEditorUnderLine = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
        
        bottomButtonRowSizer = new wxBoxSizer(wxHORIZONTAL);
        {
            mCloseButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__CLOSE, wxT("Close"));
            mApplyButton = new wxButton(this, CALCHART__TRANSITION_SOLVER__APPLY, wxT("Apply (Solve Transition from This Sheet to Next)"));
            
            bottomButtonRowSizer->Add(mCloseButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            bottomButtonRowSizer->Add(mApplyButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        }
        
        topLevelSizer->Add(headerSizer);
        topLevelSizer->Add(headerUnderLine, 0, wxGROW | wxALL, 5);
        topLevelSizer->Add(algorithmSelectionRegion, 0, wxGROW | wxALL, 5);
        topLevelSizer->Add(algorithmUnderLine, 0, wxGROW | wxALL, 5);
        topLevelSizer->Add(parameterEditor, 0, wxGROW | wxALL, 5);
        topLevelSizer->Add(parameterEditorUnderLine, 0, wxGROW | wxALL, 5);
        topLevelSizer->Add(bottomButtonRowSizer);
    }
    
    SyncControlsWithCurrentState();
    SetSizer(topLevelSizer);
}

TransitionSolverFrame::~TransitionSolverFrame() {
    if (mView) {
        delete mView;
    }
}

void TransitionSolverFrame::OnCloseWindow(wxCommandEvent& event) {
    Close();
}

void TransitionSolverFrame::Update() {
    super::Update();
    SyncControlsWithCurrentState();
}


void TransitionSolverFrame::SyncInstructionOptionsControlWithCurrentState() {
    unsigned        numSelectedCommands = 0;
    
    mInstructionOptions.clear();
    for (unsigned waitBeats = 0; waitBeats < (*mDoc->GetCurrentSheet()).GetBeats(); waitBeats+=2) {
        for (TransitionSolverParams::MarcherInstruction::Pattern pattern = TransitionSolverParams::MarcherInstruction::Pattern::BEGIN; pattern != TransitionSolverParams::MarcherInstruction::Pattern::END; pattern = (TransitionSolverParams::MarcherInstruction::Pattern)(((unsigned)pattern) + 1)) {
            TransitionSolverParams::MarcherInstruction      instruction;
            
            instruction.movementPattern = pattern;
            instruction.waitBeats = waitBeats;
            
            mInstructionOptions.push_back(instruction);
        }
    }
    
    // Populate the list of available commands depending on the duration of the sheet
    {
        std::vector<wxString> commandLabels;
        
        for (TransitionSolverParams::MarcherInstruction instruction : mInstructionOptions) {
            std::string         label;
            
            switch (instruction.movementPattern) {
                case TransitionSolverParams::MarcherInstruction::Pattern::EWNS:
                    label = "EWNS";
                    break;
                case TransitionSolverParams::MarcherInstruction::Pattern::NSEW:
                    label = "NSEW";
                    break;
                case TransitionSolverParams::MarcherInstruction::Pattern::DMHS:
                    label = "DMHS";
                    break;
                case TransitionSolverParams::MarcherInstruction::Pattern::HSDM:
                    label = "HSDM";
                    break;
                default:
                    label = "ERROR";
                    break;
            }
            
            label = "Wait " + std::to_string(instruction.waitBeats) + ", then " + label;
            commandLabels.push_back(label);
        }
        
        mAvailableCommandsControl->Clear();
        mAvailableCommandsControl->Set(wxArrayString{ commandLabels.size(), &commandLabels[0] });
        
        // Update the commands
        for (unsigned i = 0; i < mSolverParams.availableInstructions.size(); i++) {
            TransitionSolverParams::MarcherInstruction  &instruction = mSolverParams.availableInstructions[i];
            unsigned                                    commandIndex;
            
            commandIndex = (unsigned)TransitionSolverParams::MarcherInstruction::Pattern::END * (instruction.waitBeats/2) + ((unsigned)instruction.movementPattern);
            
            if (mSolverParams.availableInstructionsMask[i] && commandIndex < commandLabels.size())
            {
                mAvailableCommandsControl->SetSelection(commandIndex);
                numSelectedCommands++;
            }
        }
    }
    
    mNumSelectedInstructionsIndicator->SetLabel(std::to_string(numSelectedCommands));
}

void TransitionSolverFrame::SyncGroupControlsWithCurrentState() {
    size_t          numPointsInSelection;
    
    numPointsInSelection = mDoc->GetSelectionList().size();
    
    // Populate the list of groups
    {
        std::vector<wxString>       groupLabels;
        
        for (unsigned i = 0; i < mGroupNames.size(); i++) {
            groupLabels.push_back(mGroupNames[i]);
        }
        
        mCurrentGroupControl->Clear();
        mCurrentGroupControl->Set(wxArrayString{ groupLabels.size(), &groupLabels[0] });
        
        if (mSelectedGroup != -1) {
            mCurrentGroupControl->SetSelection(mSelectedGroup);
        }
    }
    
    // Display the current groups
    if (mSelectedGroup != -1) {
        TransitionSolverParams::GroupConstraint     &group = mSolverParams.groups[mSelectedGroup];
        
        std::vector<wxString>                       memberLabels;
        std::vector<wxString>                       destinationLabels;
        
        for (auto marcher : group.marchers) {
            memberLabels.push_back(mDoc->GetPointLabel(marcher));
        }
        
        for (auto destination : group.allowedDestinations) {
            destinationLabels.push_back(mDoc->GetPointLabel(destination));
        }
        
        mCurrentGroupMembersList->Clear();
        mCurrentGroupMembersList->Set(wxArrayString{ memberLabels.size(), &memberLabels[0] });
        
        mCurrentGroupDestinationsList->Clear();
        mCurrentGroupDestinationsList->Set(wxArrayString{ destinationLabels.size(), &destinationLabels[0] });
    }
    
    // Enable/disable buttons that depend on the selection in the field frame
    {
        std::vector<wxButton *>         buttonsDependentOnSelectionSize = {
            mSetMembersToSelectionButton,
            mAddSelectionToMembersButton,
            mRemoveSelectionFromMembersButton,
            mSetDestinationsToSelectionButton,
            mAddSelectionToDestinationsButton,
            mRemoveSelectionFromDestinationsButton,
        };
        
        for (wxButton *button : buttonsDependentOnSelectionSize) {
            if (mSelectedGroup != -1 && numPointsInSelection) {
                button->Enable();
            }
            else {
                button->Disable();
            }
        }
        
        if (mSelectedGroup != -1) {
            mSelectMembersButton->Enable();
            mSelectDestinationsButton->Enable();
        } else {
            mSelectMembersButton->Disable();
            mSelectDestinationsButton->Disable();
        }
    }
}

void TransitionSolverFrame::SyncControlsWithCurrentState() {
    size_t          numPointsInSelection;
    
    numPointsInSelection = mDoc->GetSelectionList().size();
    
    mAlgorithmChoiceControl->SetSelection((int)mSolverParams.algorithm);
    
    SyncInstructionOptionsControlWithCurrentState();
    SyncGroupControlsWithCurrentState();
    
    // Refresh display of how many points are selected
    mNumberOfSelectedPointsLabel->SetLabelText(std::to_string(numPointsInSelection));
}


#pragma mark - UI ENDPOINTS

void TransitionSolverFrame::OnApply(wxCommandEvent&) {
    std::vector<std::string>        firstSheetErrors;
    std::vector<std::string>        secondSheetErrors;
    
    std::tie(firstSheetErrors, secondSheetErrors) = ValidateForTransitionSolver();
    
    if (firstSheetErrors.size() || secondSheetErrors.size()) {
        std::string                     finalErrorMessage;
        std::string                     finalErrorDetails;
        wxMessageDialog                 *errorDialog;
        
        finalErrorMessage = "Cannot Attempt to Solve Transition";
        finalErrorDetails = "Failed to validate the start and finish stuntsheets.\n";
        
        finalErrorDetails += "\n-- Errors on start stuntsheet: --\n";
        if (firstSheetErrors.size() == 0) {
            finalErrorDetails += "No errors!\n";
        } else {
            for (std::string err : firstSheetErrors) {
                finalErrorDetails += err + "\n";
            }
        }
        
        finalErrorDetails += "\n-- Errors on finish stuntsheet: --\n";
        if (secondSheetErrors.size() == 0) {
            finalErrorDetails += "No errors!\n";
        } else {
            for (std::string err : secondSheetErrors) {
                finalErrorDetails += err + "\n";
            }
        }
        
        errorDialog = new wxMessageDialog(this, finalErrorDetails, finalErrorMessage);
        errorDialog->ShowModal();
    } else {
        Apply();
    }
}

void TransitionSolverFrame::OnChooseAlgorithm(wxCommandEvent &event) {
    ChooseAlgorithm((TransitionSolverParams::AlgorithmIdentifier)event.GetSelection());
}

void TransitionSolverFrame::OnEditAllowedCommands(wxCommandEvent &event) {
    std::set<unsigned>              listSelections;
    std::set<unsigned>              previouslySelectedCommands;
    std::vector<unsigned>           commandsToSelect;
    wxArrayInt                      rawListSelections(mAvailableCommandsControl->GetCount());
    unsigned                        numListSelections;
    
    numListSelections = mAvailableCommandsControl->GetSelections(rawListSelections);
    for (unsigned i = 0; i < rawListSelections.size(); i++) {
        listSelections.insert(rawListSelections[i]);
    }
    
    // Get a list of everything that is selected in the list now
    for (unsigned i = 0; i < mSolverParams.availableInstructions.size(); i++) {
        if (mSolverParams.availableInstructionsMask[i]) {
            TransitionSolverParams::MarcherInstruction      &instruction = mSolverParams.availableInstructions[i];
            unsigned commandIndex;
            
            commandIndex = (unsigned)TransitionSolverParams::MarcherInstruction::Pattern::END * (instruction.waitBeats/2) + ((unsigned)instruction.movementPattern);
            
            previouslySelectedCommands.insert(commandIndex);
        }
    }
    
    // First add the commands that we already have selected, if they're still selected
    for (unsigned commandIndex : previouslySelectedCommands) {
        if (listSelections.find(commandIndex) != listSelections.end()) {
            commandsToSelect.push_back(commandIndex);
            listSelections.erase(commandIndex);
        }
    }
    
    // Then, add the commands that we haven't selected yet
    for (unsigned commandIndex : listSelections) {
        commandsToSelect.push_back(commandIndex);
    }
    
    if (commandsToSelect.size() > 8) {
        commandsToSelect.erase(commandsToSelect.begin() + 8, commandsToSelect.end());
    }
    
    SetAllowedCommands(commandsToSelect);
    
    // Deselect anything that shouldn't be selected
    for (unsigned commandIndex : commandsToSelect) {
        listSelections.erase(commandIndex);
    }
    for (unsigned commandIndex : listSelections) {
        mAvailableCommandsControl->Deselect(commandIndex);
        numListSelections--;
    }
    
    mNumSelectedInstructionsIndicator->SetLabel(std::to_string(numListSelections));
}

void TransitionSolverFrame::OnAddNewGroup(wxCommandEvent &event) {
    AddNewGroup(mNewGroupNameControl->GetValue().ToStdString());
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnRemoveGroup(wxCommandEvent &event) {
    RemoveGroup(mSelectedGroup);
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnSelectGroup(wxCommandEvent &event) {
    if (mCurrentGroupControl->GetSelection() == wxNOT_FOUND) {
        UnselectGroup();
    } else {
        SelectGroup(mCurrentGroupControl->GetSelection());
    }
    
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnNullEvent(wxCommandEvent &event) {
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnClearMembers(wxCommandEvent &event) {
    ClearMembers();
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnSetMembers(wxCommandEvent &event) {
    std::set<unsigned> selection;
    for (int marcher : mDoc->GetSelectionList()) {
        selection.insert((unsigned)marcher);
    }
    SetMembers(selection);
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnAddMembers(wxCommandEvent &event) {
    std::set<unsigned> selection;
    for (int marcher : mDoc->GetSelectionList()) {
        selection.insert((unsigned)marcher);
    }
    AddMembers(selection);
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnRemoveMembers(wxCommandEvent &event) {
    std::set<unsigned> selection;
    for (int marcher : mDoc->GetSelectionList()) {
        selection.insert((unsigned)marcher);
    }
    RemoveMembers(selection);
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnSelectMembers(wxCommandEvent &event) {
    mView->SelectMarchers(mSolverParams.groups[mSelectedGroup].marchers);
}

void TransitionSolverFrame::OnClearDestinations(wxCommandEvent &event) {
    ClearDestinations();
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnSetDestinations(wxCommandEvent &event) {
    std::set<unsigned> selection;
    for (int marcher : mDoc->GetSelectionList()) {
        selection.insert((unsigned)marcher);
    }
    SetDestinations(selection);
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnAddDestinations(wxCommandEvent &event) {
    std::set<unsigned> selection;
    for (int marcher : mDoc->GetSelectionList()) {
        selection.insert((unsigned)marcher);
    }
    AddDestinations(selection);
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnRemoveDestinations(wxCommandEvent &event) {
    std::set<unsigned> selection;
    for (int marcher : mDoc->GetSelectionList()) {
        selection.insert((unsigned)marcher);
    }
    RemoveDestinations(selection);
    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnSelectDestinations(wxCommandEvent &event) {
    mView->SelectMarchers(mSolverParams.groups[mSelectedGroup].allowedDestinations);
}

#pragma mark - UNDER-THE-UI

void TransitionSolverFrame::Apply() {
    TransitionSolverProgressFrame   *progressFrame;
    
    // Create a modal window that will run the transition solver for us, and will apply the result when it finishes
    progressFrame = new TransitionSolverProgressFrame(mSolverParams, mView, this);
    
    progressFrame->ShowModal();
}

std::pair<std::vector<std::string>, std::vector<std::string>> TransitionSolverFrame::ValidateForTransitionSolver() {
    std::vector<std::string>        firstSheetErrors;
    std::vector<std::string>        secondSheetErrors;
    const auto                      sheetIterOnFirstSheet = mDoc->GetCurrentSheet();
    const auto                      endSheetIter = mDoc->GetSheetEnd();
    unsigned                        numInstructions = 0;
    
    for (unsigned i = 0; i < mSolverParams.availableInstructions.size(); i++) {
        if (mSolverParams.availableInstructionsMask[i]) {
            numInstructions++;
        }
    }
    if (numInstructions == 0) {
        firstSheetErrors.push_back("No command options have been provided.");
    }
    
    if (sheetIterOnFirstSheet != endSheetIter) {
        const CC_sheet                      &firstSheet = *sheetIterOnFirstSheet;
        
        firstSheetErrors = validateSheetForTransitionSolver(firstSheet);
    } else {
        firstSheetErrors.push_back("No first sheet exists.\n");
    }
    
    if ((sheetIterOnFirstSheet + 1) != endSheetIter) {
        const CC_sheet                      &secondSheet = *(sheetIterOnFirstSheet + 1);
        
        secondSheetErrors = validateSheetForTransitionSolver(secondSheet);
    } else {
        secondSheetErrors.push_back("No next sheet exists.\n");
    }
    
    return std::make_pair(firstSheetErrors, secondSheetErrors);
}

void TransitionSolverFrame::ChooseAlgorithm(TransitionSolverParams::AlgorithmIdentifier algorithm) {
    mSolverParams.algorithm = algorithm;
}

void TransitionSolverFrame::SetAllowedCommands(std::vector<unsigned> commandIndices) {
    for (unsigned i = 0; i < mSolverParams.availableInstructions.size(); i++) {
        mSolverParams.availableInstructionsMask[i] = false;
    }
    
    for (unsigned i = 0; i < std::min(commandIndices.size(), mSolverParams.availableInstructions.size()); i++) {
        mSolverParams.availableInstructions[i] = mInstructionOptions[commandIndices[i]];
        mSolverParams.availableInstructionsMask[i] = true;
    }
}

void TransitionSolverFrame::AddNewGroup(std::string groupName) {
    mSolverParams.groups.push_back(TransitionSolverParams::GroupConstraint());
    mGroupNames.push_back(groupName);
}

void TransitionSolverFrame::RemoveGroup(unsigned groupIndex) {
    mSolverParams.groups.erase(mSolverParams.groups.begin() + groupIndex);
    mGroupNames.erase(mGroupNames.begin() + groupIndex);
}

void TransitionSolverFrame::SelectGroup(unsigned groupIndex) {
    mSelectedGroup = groupIndex;
}

void TransitionSolverFrame::UnselectGroup() {
    mSelectedGroup = -1;
}

void TransitionSolverFrame::ClearMembers() {
    mSolverParams.groups[mSelectedGroup].marchers.clear();
}

void TransitionSolverFrame::SetMembers(std::set<unsigned> marchers) {
    ClearMembers();
    AddMembers(marchers);
}

void TransitionSolverFrame::AddMembers(std::set<unsigned> marchers) {
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].marchers.insert(*iter);
    }
}

void TransitionSolverFrame::RemoveMembers(std::set<unsigned> marchers) {
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].marchers.erase(*iter);
    }
}

void TransitionSolverFrame::ClearDestinations() {
    mSolverParams.groups[mSelectedGroup].allowedDestinations.clear();
}

void TransitionSolverFrame::SetDestinations(std::set<unsigned> marchers) {
    ClearDestinations();
    AddDestinations(marchers);
}

void TransitionSolverFrame::AddDestinations(std::set<unsigned> marchers) {
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].allowedDestinations.insert(*iter);
    }
}

void TransitionSolverFrame::RemoveDestinations(std::set<unsigned> marchers) {
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].allowedDestinations.erase(*iter);
    }
}
