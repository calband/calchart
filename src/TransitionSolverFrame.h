#pragma once
//
//  TransitionSolverFrame.h
//  CalChart
//
//  Created by Kevin Durand on 6/24/17.
//
//

#include <wx/dialog.h>
#include <wx/docview.h>

#include "CalChartDoc.h"
#include "e7_transition_solver.h"

class TransitionSolverView;

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

    void OnCloseWindow(wxCommandEvent& event);
    void OnSolutionCompleted(wxEvent& event);
    void OnNullEvent(wxCommandEvent& event);

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

    void OnApply();
    void Apply();
    void EditAllowedCommands();
    std::pair<std::vector<std::string>, std::vector<std::string>> ValidateForTransitionSolver();
    void ChooseAlgorithm(CalChart::TransitionSolverParams::AlgorithmIdentifier algorithm);
    void SetAllowedCommands(std::vector<unsigned> commandIndices);
    void AddNewGroup(std::string groupName);
    void RemoveGroup(unsigned groupIndex);
    void SelectGroup();
    void SelectGroup(unsigned groupIndex);
    void UnselectGroup();
    void ClearMembers();
    void SetMembers(std::set<int> marchers);
    void AddMembers(std::set<int> marchers);
    void RemoveMembers(std::set<int> marchers);
    void ClearDestinations();
    void SetDestinations(std::set<int> marchers);
    void AddDestinations(std::set<int> marchers);
    void RemoveDestinations(std::set<int> marchers);

    std::vector<CalChart::TransitionSolverParams::MarcherInstruction> mInstructionOptions;
    int mSelectedGroup;
    std::vector<std::string> mGroupNames;

    CalChart::TransitionSolverParams mSolverParams;

    CalChartDoc* mDoc;
    TransitionSolverView* mView;

    wxChoice* mAlgorithmChoiceControl;
    wxButton* mCloseButton;
    wxButton* mApplyButton;
    wxListBox* mAvailableCommandsControl;
    wxTextCtrl* mNewGroupNameControl;
    wxButton* mAddGroupButton;
    wxListBox* mCurrentGroupControl;
    wxButton* mRemoveGroupControl;
    wxStaticText* mNumberOfSelectedPointsLabel;
    wxListBox* mCurrentGroupMembersList;
    wxListBox* mCurrentGroupDestinationsList;
    wxButton* mClearMembersButton;
    wxButton* mSetMembersToSelectionButton;
    wxButton* mAddSelectionToMembersButton;
    wxButton* mRemoveSelectionFromMembersButton;
    wxButton* mSelectMembersButton;
    wxButton* mClearDestinationsButton;
    wxButton* mSetDestinationsToSelectionButton;
    wxButton* mAddSelectionToDestinationsButton;
    wxButton* mRemoveSelectionFromDestinationsButton;
    wxButton* mSelectDestinationsButton;
    wxStaticText* mNumSelectedInstructionsIndicator;

    DECLARE_EVENT_TABLE()
};
