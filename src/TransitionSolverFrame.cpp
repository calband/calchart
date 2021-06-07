//
//  TransitionSolver.cpp
//  CalChart
//
//  Created by Kevin Durand on 6/24/17.
//
//

#include "TransitionSolverFrame.h"
#include "CalChartApp.h"
#include "CalChartContinuity.h"
#include "CalChartDoc.h"
#include "CalChartDocCommand.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "TransitionSolverProgressFrame.h"
#include "TransitionSolverView.h"
#include "basic_ui.h"
#include "confgr.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>

using namespace std::string_literals;

#pragma mark -

enum {
    CALCHART__TRANSITION_SOLVER__CLOSE,
    CALCHART__TRANSITION_SOLVER__SELECT_GROUP,
    CALCHART__TRANSITION_SOLVER__NULL,
    CALCHART__TRANSITION_SOLVER__SET_GROUP_MEMBERS,
    CALCHART__TRANSITION_SOLVER__TRANSITION_PROGRESS,
    CALCHART__TRANSITION_SOLVER__TRANSITION_SUBTASK_PROGRESS,
    CALCHART__TRANSITION_SOLVER__TRANSITION_SOLUTION_FOUND,
    CALCHART__TRANSITION_SOLVER__CALCULATION_COMPLETE,
    CALCHART__TRANSITION_SOLVER__CANCEL_CALCULATION,
    CALCHART__TRANSITION_SOLVER__FINISH_CALCULATION_NOW_AND_APPLY,
};

BEGIN_EVENT_TABLE(TransitionSolverFrame, wxFrame)
EVT_MENU(CALCHART__TRANSITION_SOLVER__CLOSE, TransitionSolverFrame::OnCloseWindow)
EVT_LISTBOX(CALCHART__TRANSITION_SOLVER__NULL, TransitionSolverFrame::OnNullEvent)
END_EVENT_TABLE()

#pragma mark - TransitionSolverFrame Implementation

TransitionSolverFrame::TransitionSolverFrame()
{
    Init();
}

TransitionSolverFrame::TransitionSolverFrame(CalChartDoc* show, wxWindow* parent,
    wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size,
    long style)
{
    Init();

    Create(show, parent, id, caption, pos, size, style);
}

void TransitionSolverFrame::Init() { }

bool TransitionSolverFrame::Create(CalChartDoc* show, wxWindow* parent,
    wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size,
    long style)
{
    if (!wxFrame::Create(parent, id, caption, pos, size, style)) {
        return false;
    }

    mDoc = show;
    mView = new TransitionSolverView;
    mView->SetDocument(show);
    mView->SetFrame(this);
    mSelectedGroup = -1;
    mSolverParams.algorithm = CalChart::TransitionSolverParams::AlgorithmIdentifier::BEGIN;
    for (unsigned i = 0; i < mSolverParams.availableInstructionsMask.size(); i++) {
        mSolverParams.availableInstructionsMask[i] = true;
        mSolverParams.availableInstructions[i].waitBeats = (i / (unsigned)CalChart::TransitionSolverParams::MarcherInstruction::Pattern::END) * 2;
        mSolverParams.availableInstructions[i].movementPattern = (CalChart::TransitionSolverParams::MarcherInstruction::Pattern)(i % (unsigned)CalChart::TransitionSolverParams::MarcherInstruction::Pattern::END);
    }

    CreateControls();

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);

    Center();

    // now update the current screen
    Update();

    return true;
}

void TransitionSolverFrame::CreateControls()
{
    // menu bar
    auto cont_menu = new wxMenu;
    cont_menu->Append(wxID_CLOSE, wxT("Close Window\tCTRL-W"), wxT("Close this window"));

    auto menu_bar = new wxMenuBar;
    menu_bar->Append(cont_menu, wxT("&File"));
    SetMenuBar(menu_bar);

    // create a scrollable window to contain all of the frame's content
    auto scrolledWindow = new wxScrolledWindow(this, wxID_ANY);

    // add all of the frame content to the scrollable window
    scrolledWindow->SetSizer(VStack([this, scrolledWindow](auto sizer) {
        HStack(sizer, [this, scrolledWindow](auto& sizer) {
            CreateText(scrolledWindow, sizer, R"T(Welcome to the CalChart Transition Solver!
The algorithms used by this solver are credited to the E7 class  of Spring 2016.
Staff: (Professor) Tina Chow, and (GSIs) Lucas Bastien and Bradley Harken
Algorithms were selected from three different student groups:
(1) Chiu, Zamora, Malani (2) Namini Asl, Ramirez, Zhang (3) Sover, Eliceiri, Hershkovitz
Additional thanks to professor Scott Moura for helping the Cal Band get support from the UC Berkeley
Civil Engineering Department for developing these algorithms.)T");
        });

        CreateHLine(scrolledWindow, sizer);

        HStack(sizer, [this, scrolledWindow](auto& sizer) {
            CreateText(scrolledWindow, sizer, "Select an algorithm: ");

            mAlgorithmChoiceControl = CreateChoiceWithHandler(scrolledWindow, sizer, {
                                                                                         "E7 Algorithm: Chiu, Zamora, Malani",
                                                                                         "E7 Algorithm: Namini Asl, Ramirez, Zhang",
                                                                                         "Ey Algorithm: Sover, Eliceiri, Hershkovitz",
                                                                                     },
                [this](wxCommandEvent& event) {
                    ChooseAlgorithm((CalChart::TransitionSolverParams::AlgorithmIdentifier)event.GetSelection());
                });
        });

        CreateHLine(scrolledWindow, sizer);

        VStack(sizer, [this, scrolledWindow](auto& sizer) {
            HStack(sizer, [this, scrolledWindow](auto& sizer) {
                VStack(sizer, [this, scrolledWindow](auto& sizer) {
                    CreateText(scrolledWindow, sizer, "Please select a set of marcher instructions to allow.\n (You may select up to 8)\n")->Wrap(180);
                    CreateText(scrolledWindow, sizer, "Selected Instructions:");
                    mNumSelectedInstructionsIndicator = CreateText(scrolledWindow, sizer, "0:");
                });

                mAvailableCommandsControl = new wxListBox(scrolledWindow, wxID_ANY, wxDefaultPosition, wxSize(400, 100), 0, NULL, wxLB_EXTENDED);
                mAvailableCommandsControl->Bind(wxEVT_LISTBOX, [this](wxCommandEvent&) {
                    EditAllowedCommands();
                });
                sizer->Add(mAvailableCommandsControl, 0, wxGROW | wxALL, 5);
            });

            CreateHLine(scrolledWindow, sizer);

            VStack(sizer, [this, scrolledWindow](auto& sizer) {
                HStack(sizer, [this, scrolledWindow](auto& sizer) {
                    CreateText(scrolledWindow, sizer, "New Group:");

                    mNewGroupNameControl = CreateTextCtrl(scrolledWindow, sizer);

                    mAddGroupButton = CreateButtonWithHandler(scrolledWindow, sizer, VerticalSizerFlags(), "Add", [this]() {
                        AddNewGroup(mNewGroupNameControl->GetValue().ToStdString());
                        SyncGroupControlsWithCurrentState();
                    });
                });

                HStack(sizer, [this, scrolledWindow](auto& sizer) {
                    CreateText(scrolledWindow, sizer, "Viewing Group:");

                    mCurrentGroupControl = new wxListBox(scrolledWindow, wxID_ANY, wxDefaultPosition, wxSize(400, 100));
                    mCurrentGroupControl->Bind(wxEVT_LISTBOX, [this](wxCommandEvent&) {
                        SelectGroup();
                    });
                    sizer->Add(mCurrentGroupControl, 0, wxGROW | wxALL, 5);

                    mRemoveGroupControl = CreateButtonWithHandler(scrolledWindow, sizer, VerticalSizerFlags(), "Remove", [this]() {
                        AddNewGroup(mNewGroupNameControl->GetValue().ToStdString());
                        SyncGroupControlsWithCurrentState();
                    });
                });

                HStack(sizer, [this, scrolledWindow](auto& sizer) {
                    VStack(sizer, [this, scrolledWindow](auto& sizer) {
                        CreateText(scrolledWindow, sizer, "Select marchers on the field to enable adding them as members or destinations of the current group.")->Wrap(150);

                        CreateText(scrolledWindow, sizer, "Number of selected marchers:");

                        mNumberOfSelectedPointsLabel = CreateText(scrolledWindow, sizer, "0");
                    });

                    VStack(sizer, [this, scrolledWindow](auto& sizer) {
                        CreateText(scrolledWindow, sizer, "Group Members:");

                        mCurrentGroupMembersList = new wxListBox(scrolledWindow, CALCHART__TRANSITION_SOLVER__NULL, wxDefaultPosition, wxSize(130, 100), 0, NULL, wxLB_EXTENDED);
                        sizer->Add(mCurrentGroupMembersList, 0, wxGROW | wxALL, 5);

                        VStack(sizer, [this, scrolledWindow](auto& sizer) {
                            HStack(sizer, HorizontalSizerFlags(), [this, scrolledWindow](auto& sizer) {
                                mClearMembersButton = CreateButtonWithHandler(scrolledWindow, sizer, "Clear", [this]() {
                                    ClearMembers();
                                    SyncGroupControlsWithCurrentState();
                                });

                                mSetMembersToSelectionButton = CreateButtonWithHandler(scrolledWindow, sizer, "Set", [this]() {
                                    SetMembers(mDoc->GetSelectionList());
                                    SyncGroupControlsWithCurrentState();
                                });
                            });

                            HStack(sizer, HorizontalSizerFlags(), [this, scrolledWindow](auto& sizer) {
                                mAddSelectionToMembersButton = CreateButtonWithHandler(scrolledWindow, sizer, "Add", [this]() {
                                    AddMembers(mDoc->GetSelectionList());
                                    SyncGroupControlsWithCurrentState();
                                });

                                mRemoveSelectionFromMembersButton = CreateButtonWithHandler(scrolledWindow, sizer, "Remove", [this]() {
                                    RemoveMembers(mDoc->GetSelectionList());
                                    SyncGroupControlsWithCurrentState();
                                });
                            });

                            HStack(sizer, HorizontalSizerFlags(), [this, scrolledWindow](auto& sizer) {
                                mSelectMembersButton = CreateButtonWithHandler(scrolledWindow, sizer, "Select", [this]() {
                                    mView->SelectMarchers(mSolverParams.groups[mSelectedGroup].marchers);
                                });
                            });
                        });
                    });

                    VStack(sizer, [this, scrolledWindow](auto& sizer) {
                        CreateText(scrolledWindow, sizer, "Group Allowed Destinations:");

                        mCurrentGroupDestinationsList = new wxListBox(scrolledWindow, CALCHART__TRANSITION_SOLVER__NULL, wxDefaultPosition, wxSize(130, 100), 0, NULL, wxLB_EXTENDED);
                        sizer->Add(mCurrentGroupDestinationsList, 0, wxGROW | wxALL, 5);

                        VStack(sizer, [this, scrolledWindow](auto& sizer) {
                            HStack(sizer, [this, scrolledWindow](auto& sizer) {
                                mClearDestinationsButton = CreateButtonWithHandler(scrolledWindow, sizer, "Clear", [this]() {
                                    ClearDestinations();
                                    SyncGroupControlsWithCurrentState();
                                });

                                mSetDestinationsToSelectionButton = CreateButtonWithHandler(scrolledWindow, sizer, "Set", [this]() {
                                    SetDestinations(mDoc->GetSelectionList());
                                    SyncGroupControlsWithCurrentState();
                                });
                            });

                            HStack(sizer, [this, scrolledWindow](auto& sizer) {
                                mAddSelectionToDestinationsButton = CreateButtonWithHandler(scrolledWindow, sizer, "Add", [this]() {
                                    AddDestinations(mDoc->GetSelectionList());
                                    SyncGroupControlsWithCurrentState();
                                });

                                mRemoveSelectionFromDestinationsButton = CreateButtonWithHandler(scrolledWindow, sizer, "Remove", [this]() {
                                    RemoveDestinations(mDoc->GetSelectionList());
                                    SyncGroupControlsWithCurrentState();
                                });
                            });

                            HStack(sizer, HorizontalSizerFlags(), [this, scrolledWindow](auto& sizer) {
                                mSelectDestinationsButton = CreateButtonWithHandler(scrolledWindow, sizer, "Select", [this]() {
                                    mView->SelectMarchers(mSolverParams.groups[mSelectedGroup].allowedDestinations);
                                });
                            });
                        });
                    });
                });
            });
        });

        CreateHLine(scrolledWindow, sizer);

        HStack(sizer, HorizontalSizerFlags(), [this, scrolledWindow](auto& sizer) {
            mCloseButton = CreateButtonWithHandler(scrolledWindow, sizer, "Close", [this]() {
                Close();
            });

            mApplyButton = CreateButtonWithHandler(scrolledWindow, sizer, "Apply (Solve Transition from This Sheet to Next)", [this]() {
                Apply();
            });
        });
    }));

    {
        // configure the minimum size of the window, and then add scroll bars
        auto minFrameSize = scrolledWindow->GetSizer()->ComputeFittingWindowSize(scrolledWindow);
        scrolledWindow->SetMinSize(minFrameSize);
        scrolledWindow->SetScrollRate(5, 5);

        // add the scrollable window to the frame
        auto frameSizer = new wxBoxSizer(wxVERTICAL);
        frameSizer->Add(scrolledWindow, 1, wxEXPAND);
        SetSizer(frameSizer);
    }

    SyncControlsWithCurrentState();
}

TransitionSolverFrame::~TransitionSolverFrame()
{
    if (mView) {
        delete mView;
    }
}

void TransitionSolverFrame::OnCloseWindow(wxCommandEvent& event)
{
    Close();
}

void TransitionSolverFrame::Update()
{
    super::Update();
    SyncControlsWithCurrentState();
}

void TransitionSolverFrame::SyncInstructionOptionsControlWithCurrentState()
{
    static const auto marchInstructions = std::map<CalChart::TransitionSolverParams::MarcherInstruction::Pattern, std::string>{
        { CalChart::TransitionSolverParams::MarcherInstruction::EWNS, "EWNS" },
        { CalChart::TransitionSolverParams::MarcherInstruction::NSEW, "NSEW" },
        { CalChart::TransitionSolverParams::MarcherInstruction::DMHS, "DMHS" },
        { CalChart::TransitionSolverParams::MarcherInstruction::HSDM, "HSDM" },
    };

    mInstructionOptions.clear();
    for (auto waitBeats = 0; waitBeats < (*mDoc->GetCurrentSheet()).GetBeats(); waitBeats += 2) {
        for (auto pattern : marchInstructions) {
            mInstructionOptions.emplace_back(pattern.first, waitBeats);
        }
    }

    unsigned numSelectedCommands = 0;

    // Populate the list of available commands depending on the duration of the sheet
    std::vector<wxString> commandLabels;

    std::transform(mInstructionOptions.begin(), mInstructionOptions.end(), std::back_inserter(commandLabels), [](auto&& instruction) {
        std::string label;
        if (auto i = marchInstructions.find(instruction.movementPattern); i != marchInstructions.end()) {
            label = i->second;
        } else {
            label = "ERROR";
        }
        return "Wait "s + std::to_string(instruction.waitBeats) + ", then " + label;
    });

    mAvailableCommandsControl->Set(commandLabels);

    // Update the commands
    for (auto i = 0; i < mSolverParams.availableInstructions.size(); i++) {
        auto instruction = mSolverParams.availableInstructions[i];
        auto commandIndex = (unsigned)CalChart::TransitionSolverParams::MarcherInstruction::Pattern::END * (instruction.waitBeats / 2) + ((unsigned)instruction.movementPattern);

        if (mSolverParams.availableInstructionsMask[i] && commandIndex < commandLabels.size()) {
            mAvailableCommandsControl->SetSelection(commandIndex);
            numSelectedCommands++;
        }
    }

    mNumSelectedInstructionsIndicator->SetLabel(std::to_string(numSelectedCommands));
}

void TransitionSolverFrame::SyncGroupControlsWithCurrentState()
{
    auto numPointsInSelection = mDoc->GetSelectionList().size();

    // Populate the list of groups
    std::vector<wxString> groupLabels;
    std::transform(mGroupNames.begin(), mGroupNames.end(), std::back_inserter(groupLabels), [](auto&& i) { return i; });

    mCurrentGroupControl->Set(groupLabels);

    if (mSelectedGroup != -1) {
        mCurrentGroupControl->SetSelection(mSelectedGroup);
    }

    // Display the current groups
    if (mSelectedGroup != -1) {
        auto& group = mSolverParams.groups[mSelectedGroup];

        std::vector<wxString> memberLabels;
        std::transform(group.marchers.begin(), group.marchers.end(), std::back_inserter(memberLabels), [this](auto&& marcher) { return mDoc->GetPointLabel(marcher); });
        mCurrentGroupMembersList->Set(memberLabels);

        std::vector<wxString> destinationLabels;
        std::transform(group.allowedDestinations.begin(), group.allowedDestinations.end(), std::back_inserter(destinationLabels), [this](auto&& marcher) { return mDoc->GetPointLabel(marcher); });
        mCurrentGroupDestinationsList->Set(destinationLabels);
    }

    // Enable/disable buttons that depend on the selection in the field frame
    for (auto button : {
             mSetMembersToSelectionButton,
             mAddSelectionToMembersButton,
             mRemoveSelectionFromMembersButton,
             mSetDestinationsToSelectionButton,
             mAddSelectionToDestinationsButton,
             mRemoveSelectionFromDestinationsButton,
         }) {
        if (mSelectedGroup != -1 && numPointsInSelection) {
            button->Enable();
        } else {
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

void TransitionSolverFrame::SyncControlsWithCurrentState()
{
    auto numPointsInSelection = mDoc->GetSelectionList().size();

    mAlgorithmChoiceControl->SetSelection((int)mSolverParams.algorithm);

    SyncInstructionOptionsControlWithCurrentState();
    SyncGroupControlsWithCurrentState();

    // Refresh display of how many points are selected
    mNumberOfSelectedPointsLabel->SetLabelText(std::to_string(numPointsInSelection));
}

#pragma mark - UI ENDPOINTS

void TransitionSolverFrame::OnApply()
{
    auto [firstSheetErrors, secondSheetErrors] = ValidateForTransitionSolver();

    if (firstSheetErrors.size() || secondSheetErrors.size()) {
        auto finalErrorMessage = "Cannot Attempt to Solve Transition"s;
        auto finalErrorDetails = "Failed to validate the start and finish stuntsheets.\n"s;

        finalErrorDetails += "\n-- Errors on start stuntsheet: --\n";
        if (firstSheetErrors.size() == 0) {
            finalErrorDetails += "No errors!\n";
        } else {
            for (auto err : firstSheetErrors) {
                finalErrorDetails += err + "\n";
            }
        }

        finalErrorDetails += "\n-- Errors on finish stuntsheet: --\n";
        if (secondSheetErrors.size() == 0) {
            finalErrorDetails += "No errors!\n";
        } else {
            for (auto err : secondSheetErrors) {
                finalErrorDetails += err + "\n";
            }
        }

        wxMessageDialog* errorDialog = new wxMessageDialog(this, finalErrorDetails, finalErrorMessage);
        errorDialog->ShowModal();
    } else {
        Apply();
    }
}

void TransitionSolverFrame::EditAllowedCommands()
{
    auto rawListSelections = wxArrayInt(mAvailableCommandsControl->GetCount());
    auto numListSelections = mAvailableCommandsControl->GetSelections(rawListSelections);

    auto listSelections = std::set<unsigned>{ rawListSelections.begin(), rawListSelections.end() };

    // Get a list of everything that is selected in the list now
    std::set<unsigned> previouslySelectedCommands;
    for (unsigned i = 0; i < mSolverParams.availableInstructions.size(); i++) {
        if (mSolverParams.availableInstructionsMask[i]) {
            auto instruction = mSolverParams.availableInstructions[i];
            auto commandIndex = (unsigned)CalChart::TransitionSolverParams::MarcherInstruction::Pattern::END * (instruction.waitBeats / 2) + ((unsigned)instruction.movementPattern);

            previouslySelectedCommands.insert(commandIndex);
        }
    }

    // First add the commands that we already have selected, if they're still selected
    // that would be the intersection of the previous and new commands
    std::vector<unsigned> commandsToSelect;
    std::set_intersection(previouslySelectedCommands.begin(), previouslySelectedCommands.end(), listSelections.begin(), listSelections.end(), std::back_inserter(commandsToSelect));

    // now remove all the previously selected commands
    for (auto i : previouslySelectedCommands) {
        listSelections.erase(i);
    }

    // Then, add the commands that we haven't selected yet
    std::copy(listSelections.begin(), listSelections.end(), std::back_inserter(commandsToSelect));

    if (commandsToSelect.size() > 8) {
        for (auto i = commandsToSelect.begin() + 8; i < commandsToSelect.end(); ++i) {
            mAvailableCommandsControl->Deselect(*i);
        }
        commandsToSelect.erase(commandsToSelect.begin() + 8, commandsToSelect.end());
    }

    SetAllowedCommands(commandsToSelect);
    mNumSelectedInstructionsIndicator->SetLabel(std::to_string(commandsToSelect.size()));
}

void TransitionSolverFrame::SelectGroup()
{
    if (mCurrentGroupControl->GetSelection() == wxNOT_FOUND) {
        UnselectGroup();
    } else {
        SelectGroup(mCurrentGroupControl->GetSelection());
    }

    SyncGroupControlsWithCurrentState();
}

void TransitionSolverFrame::OnNullEvent(wxCommandEvent& event)
{
    SyncGroupControlsWithCurrentState();
}

#pragma mark - UNDER-THE-UI

void TransitionSolverFrame::Apply()
{
    TransitionSolverProgressFrame* progressFrame;

    // Create a modal window that will run the transition solver for us, and will apply the result when it finishes
    progressFrame = new TransitionSolverProgressFrame(mSolverParams, mView, this);

    progressFrame->ShowModal();
}

std::pair<std::vector<std::string>, std::vector<std::string>> TransitionSolverFrame::ValidateForTransitionSolver()
{
    std::vector<std::string> firstSheetErrors;
    std::vector<std::string> secondSheetErrors;
    const auto sheetIterOnFirstSheet = mDoc->GetCurrentSheet();
    const auto endSheetIter = mDoc->GetSheetEnd();
    unsigned numInstructions = 0;

    for (unsigned i = 0; i < mSolverParams.availableInstructions.size(); i++) {
        if (mSolverParams.availableInstructionsMask[i]) {
            numInstructions++;
        }
    }
    if (numInstructions == 0) {
        firstSheetErrors.push_back("No command options have been provided.");
    }

    if (sheetIterOnFirstSheet != endSheetIter) {
        const CalChart::Sheet& firstSheet = *sheetIterOnFirstSheet;

        firstSheetErrors = validateSheetForTransitionSolver(firstSheet);
    } else {
        firstSheetErrors.push_back("No first sheet exists.\n");
    }

    if ((sheetIterOnFirstSheet + 1) != endSheetIter) {
        const CalChart::Sheet& secondSheet = *(sheetIterOnFirstSheet + 1);

        secondSheetErrors = validateSheetForTransitionSolver(secondSheet);
    } else {
        secondSheetErrors.push_back("No next sheet exists.\n");
    }

    return std::make_pair(firstSheetErrors, secondSheetErrors);
}

void TransitionSolverFrame::ChooseAlgorithm(CalChart::TransitionSolverParams::AlgorithmIdentifier algorithm)
{
    mSolverParams.algorithm = algorithm;
}

void TransitionSolverFrame::SetAllowedCommands(std::vector<unsigned> commandIndices)
{
    for (unsigned i = 0; i < mSolverParams.availableInstructions.size(); i++) {
        mSolverParams.availableInstructionsMask[i] = false;
    }

    for (unsigned i = 0; i < std::min(commandIndices.size(), mSolverParams.availableInstructions.size()); i++) {
        mSolverParams.availableInstructions[i] = mInstructionOptions[commandIndices[i]];
        mSolverParams.availableInstructionsMask[i] = true;
    }
}

void TransitionSolverFrame::AddNewGroup(std::string groupName)
{
    mSolverParams.groups.push_back(CalChart::TransitionSolverParams::GroupConstraint());
    mGroupNames.push_back(groupName);
}

void TransitionSolverFrame::RemoveGroup(unsigned groupIndex)
{
    mSolverParams.groups.erase(mSolverParams.groups.begin() + groupIndex);
    mGroupNames.erase(mGroupNames.begin() + groupIndex);
}

void TransitionSolverFrame::SelectGroup(unsigned groupIndex)
{
    mSelectedGroup = groupIndex;
}

void TransitionSolverFrame::UnselectGroup()
{
    mSelectedGroup = -1;
}

void TransitionSolverFrame::ClearMembers()
{
    if (mSelectedGroup == -1) {
        return;
    }
    mSolverParams.groups[mSelectedGroup].marchers.clear();
}

void TransitionSolverFrame::SetMembers(std::set<int> marchers)
{
    ClearMembers();
    AddMembers(marchers);
}

void TransitionSolverFrame::AddMembers(std::set<int> marchers)
{
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].marchers.insert(*iter);
    }
}

void TransitionSolverFrame::RemoveMembers(std::set<int> marchers)
{
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].marchers.erase(*iter);
    }
}

void TransitionSolverFrame::ClearDestinations()
{
    mSolverParams.groups[mSelectedGroup].allowedDestinations.clear();
}

void TransitionSolverFrame::SetDestinations(std::set<int> marchers)
{
    ClearDestinations();
    AddDestinations(marchers);
}

void TransitionSolverFrame::AddDestinations(std::set<int> marchers)
{
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].allowedDestinations.insert(*iter);
    }
}

void TransitionSolverFrame::RemoveDestinations(std::set<int> marchers)
{
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].allowedDestinations.erase(*iter);
    }
}
