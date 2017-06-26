//
//  e7_transition_solver_ui.h
//  CalChart
//
//  Created by Kevin Durand on 6/24/17.
//
//

#pragma once

#include "calchartdoc.h"
#include "e7_transition_solver.h"

#include <array>
#include <wx/docview.h>
#include <wx/dialog.h>


// View for linking CalChartDoc with the Transition Solver
class TransitionSolverView : public wxView {
    using super = wxView;
public:
    TransitionSolverView();
    ~TransitionSolverView();

    virtual void OnDraw(wxDC* dc) override;
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL) override;
    
    void ApplyTransitionSolution(TransitionSolverResult solution);
    void SelectMarchers(std::set<unsigned> marchers);
};

// TransitionSolverFrame
// This offers a UI for editing the options that will affect transition calculation
class TransitionSolverFrame : public wxFrame {
    friend class TransitionSolverView;
    using super = wxFrame;
public:
    TransitionSolverFrame();
    TransitionSolverFrame(CalChartDoc* dcr, wxWindow* parent, wxWindowID id = wxID_ANY,
                          const wxString& caption = wxT("Solve Transition"),
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxDefaultSize,
                          long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    ~TransitionSolverFrame();
    
    void OnCloseWindow(wxCommandEvent &event);
    void OnApply(wxCommandEvent &event);
    void OnSolutionCompleted(wxEvent &event);
    void OnChooseAlgorithm(wxCommandEvent &event);
    void OnEditAllowedCommands(wxCommandEvent &event);
    void OnAddNewGroup(wxCommandEvent &event);
    void OnRemoveGroup(wxCommandEvent &event);
    void OnSelectGroup(wxCommandEvent &event);
    void OnNullEvent(wxCommandEvent &event);
    void OnClearMembers(wxCommandEvent &event);
    void OnSetMembers(wxCommandEvent &event);
    void OnAddMembers(wxCommandEvent &event);
    void OnRemoveMembers(wxCommandEvent &event);
    void OnSelectMembers(wxCommandEvent &event);
    void OnClearDestinations(wxCommandEvent &event);
    void OnSetDestinations(wxCommandEvent &event);
    void OnAddDestinations(wxCommandEvent &event);
    void OnRemoveDestinations(wxCommandEvent &event);
    void OnSelectDestinations(wxCommandEvent &event);
    
    void Update() override;
    void SyncControlsWithCurrentState();
    void SyncGroupControlsWithCurrentState();
    void SyncInstructionOptionsControlWithCurrentState();
    
private:
    void Init();
    bool Create(CalChartDoc* shw, wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxString& caption = wxT("Solve Transition"),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    void CreateControls();
    
    void Apply();
    std::pair<std::vector<std::string>, std::vector<std::string>> ValidateForTransitionSolver();
    void ChooseAlgorithm(TransitionSolverParams::AlgorithmIdentifier algorithm);
    void SetAllowedCommands(std::vector<unsigned> commandIndices);
    void AddNewGroup(std::string groupName);
    void RemoveGroup(unsigned groupIndex);
    void SelectGroup(unsigned groupIndex);
    void UnselectGroup();
    void ClearMembers();
    void SetMembers(std::set<unsigned> marchers);
    void AddMembers(std::set<unsigned> marchers);
    void RemoveMembers(std::set<unsigned> marchers);
    void ClearDestinations();
    void SetDestinations(std::set<unsigned> marchers);
    void AddDestinations(std::set<unsigned> marchers);
    void RemoveDestinations(std::set<unsigned> marchers);
    
    std::vector<TransitionSolverParams::InstructionOption> mInstructionOptions;
    int mSelectedGroup;
    std::vector<std::string> mGroupNames;
    
    TransitionSolverParams mSolverParams;
    
    CalChartDoc* mDoc;
    TransitionSolverView* mView;
    
    wxChoice *mAlgorithmChoiceControl;
    wxButton *mCloseButton;
    wxButton *mApplyButton;
    wxListBox *mAvailableCommandsControl;
    wxTextCtrl *mNewGroupNameControl;
    wxButton *mAddGroupButton;
    wxListBox *mCurrentGroupControl;
    wxButton *mRemoveGroupControl;
    wxStaticText *mNumberOfSelectedPointsLabel;
    wxListBox *mCurrentGroupMembersList;
    wxListBox *mCurrentGroupDestinationsList;
    wxButton *mClearMembersButton;
    wxButton *mSetMembersToSelectionButton;
    wxButton *mAddSelectionToMembersButton;
    wxButton *mRemoveSelectionFromMembersButton;
    wxButton *mSelectMembersButton;
    wxButton *mClearDestinationsButton;
    wxButton *mSetDestinationsToSelectionButton;
    wxButton *mAddSelectionToDestinationsButton;
    wxButton *mRemoveSelectionFromDestinationsButton;
    wxButton *mSelectDestinationsButton;
    wxStaticText *mNumSelectedInstructionsIndicator;
    
    DECLARE_EVENT_TABLE()
};