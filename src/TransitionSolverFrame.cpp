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

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>
#include <wxUI/wxUI.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

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

constexpr auto Description = R"T(Welcome to the CalChart Transition Solver!
The algorithms used by this solver are credited to the E7 class  of Spring 2016.
Staff: (Professor) Tina Chow, and (GSIs) Lucas Bastien and Bradley Harken
Algorithms were selected from three different student groups:
(1) Chiu, Zamora, Malani (2) Namini Asl, Ramirez, Zhang (3) Sover, Eliceiri, Hershkovitz
Additional thanks to professor Scott Moura for helping the Cal Band get support from the UC Berkeley
Civil Engineering Department for developing these algorithms.)T";
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
    // create a scrollable window to contain all of the frame's content
    auto scrolledWindow = new wxScrolledWindow(this, wxID_ANY);

    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 2).Center(),
        wxUI::Text{ Description },
        wxUI::HLine(),
        wxUI::HSizer{
            wxUI::Text{ "Select an algorithm: " },
            wxUI::Choice{ { "E7 Algorithm: Chiu, Zamora, Malani",
                              "E7 Algorithm: Namini Asl, Ramirez, Zhang",
                              "Ey Algorithm: Sover, Eliceiri, Hershkovitz" } }
                .bind([this](wxCommandEvent& event) {
                    ChooseAlgorithm((CalChart::TransitionSolverParams::AlgorithmIdentifier)event.GetSelection());
                })
                .withProxy(mAlgorithmChoiceControl),
        },
        wxUI::HLine(),
        wxUI::VSizer{
            wxUI::HSizer{
                wxUI::VSizer{
                    wxUI::Text{ "Please select a set of marcher instructions to allow.\n (You may select up to 8)\n" }
                        .withWrap(180),
                    wxUI::Text{ "Selected Instructions:" },
                    wxUI::Text{ "0:" }.withProxy(mNumSelectedInstructionsIndicator),
                },
                wxUI::ListBox{}.withSize({ 400, 100 }).setStyle(wxLB_EXTENDED).bind([this] {
                                                                                  EditAllowedCommands();
                                                                              })
                    .withFlags(wxSizerFlags{}.Expand())
                    .withProxy(mAvailableCommandsControl),
            },
            wxUI::HLine(),
            wxUI::HSizer{
                wxUI::Text{ "New Group:" },
                wxUI::TextCtrl{}.withWidth(100).withProxy(mNewGroupNameControl),
                wxUI::Button{ "Add" }.bind([this] {
                                         AddNewGroup(mNewGroupNameControl->GetValue().ToStdString());
                                         SyncGroupControlsWithCurrentState();
                                     })
                    .withProxy(mAddGroupButton),
            },
            wxUI::HSizer{
                wxUI::Text{ "Viewing Group:" },
                wxUI::ListBox{}.withSize({ 400, 100 }).bind([this] {
                                                          SelectGroup();
                                                      })
                    .withFlags(wxSizerFlags{}.Expand())
                    .withProxy(mCurrentGroupControl),
                wxUI::Button{ "Remove" }.bind([this] {
                                            AddNewGroup(mNewGroupNameControl->GetValue().ToStdString());
                                            SyncGroupControlsWithCurrentState();
                                        })
                    .withProxy(mRemoveGroupControl),
            },
            wxUI::HSizer{
                wxUI::VSizer{
                    wxUI::Text{ "Select marchers on the field to enable adding them as members or destinations of the current group." }
                        .withWrap(150),
                    wxUI::Text{ "Number of selected marchers:" },
                    wxUI::Text{ "0" }.withProxy(mNumberOfSelectedPointsLabel),
                },
                wxUI::VSizer{
                    wxUI::Text{ "Group Members:" },
                    wxUI::ListBox{ CALCHART__TRANSITION_SOLVER__NULL }
                        .setStyle(wxLB_EXTENDED)
                        .withSize({ 130, 100 })
                        .withFlags(wxSizerFlags{}.Expand())
                        .withProxy(mCurrentGroupMembersList),
                    wxUI::HSizer{
                        wxUI::Button{ "Clear" }.bind([this] {
                                                   ClearMembers();
                                                   SyncGroupControlsWithCurrentState();
                                               })
                            .withProxy(mClearMembersButton),
                        wxUI::Button{ "Set" }.bind([this] {
                                                 SetMembers(mDoc->GetSelectionList());
                                                 SyncGroupControlsWithCurrentState();
                                             })
                            .withProxy(mSetMembersToSelectionButton),
                    },
                    wxUI::HSizer{
                        wxUI::Button{ "Add" }.bind([this] {
                                                 AddMembers(mDoc->GetSelectionList());
                                                 SyncGroupControlsWithCurrentState();
                                             })
                            .withProxy(mAddSelectionToMembersButton),
                        wxUI::Button{ "Remove" }.bind([this] {
                                                    RemoveMembers(mDoc->GetSelectionList());
                                                    SyncGroupControlsWithCurrentState();
                                                })
                            .withProxy(mRemoveSelectionFromMembersButton),
                    },
                    wxUI::HSizer{
                        wxUI::Button{ "Select" }.bind([this] {
                                                    mView->SelectMarchers(mSolverParams.groups[mSelectedGroup].marchers);
                                                })
                            .withProxy(mSelectMembersButton),
                    },
                },
                wxUI::VSizer{
                    wxUI::Text{ "Group Allowed Destinations:" },
                    wxUI::ListBox{ CALCHART__TRANSITION_SOLVER__NULL }
                        .setStyle(wxLB_EXTENDED)
                        .withSize({ 130, 100 })
                        .withFlags(wxSizerFlags{}.Expand())
                        .withProxy(mCurrentGroupDestinationsList),
                    wxUI::HSizer{
                        wxUI::Button{ "Clear" }.bind([this] {
                                                   ClearDestinations();
                                                   SyncGroupControlsWithCurrentState();
                                               })
                            .withProxy(mClearDestinationsButton),
                        wxUI::Button{ "Set" }.bind([this] {
                                                 SetDestinations(mDoc->GetSelectionList());
                                                 SyncGroupControlsWithCurrentState();
                                             })
                            .withProxy(mSetDestinationsToSelectionButton),
                    },
                    wxUI::HSizer{
                        wxUI::Button{ "Add" }.bind([this] {
                                                 AddDestinations(mDoc->GetSelectionList());
                                                 SyncGroupControlsWithCurrentState();
                                             })
                            .withProxy(mAddSelectionToDestinationsButton),
                        wxUI::Button{ "Remove" }.bind([this] {
                                                    RemoveDestinations(mDoc->GetSelectionList());
                                                    SyncGroupControlsWithCurrentState();
                                                })
                            .withProxy(mRemoveSelectionFromDestinationsButton),
                    },
                    wxUI::HSizer{
                        wxUI::Button{ "Select" }.bind([this] {
                                                    mView->SelectMarchers(mSolverParams.groups[mSelectedGroup].allowedDestinations);
                                                })
                            .withProxy(mSelectDestinationsButton),
                    },
                },
            },
        },
        wxUI::HLine(),
        wxUI::HSizer{
            wxUI::Button{ "Close" }.bind([this] {
                                       Close();
                                   })
                .withProxy(mCloseButton),
            wxUI::Button{ "Apply (Solve Transition from This Sheet to Next)" }.bind([this] {
                                                                                  Apply();
                                                                              })
                .withProxy(mApplyButton),
        },

    }
        .fitTo(scrolledWindow);

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

void TransitionSolverFrame::OnCloseWindow(wxCommandEvent&)
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
    for (auto waitBeats = CalChart::Beats{}; waitBeats < (*mDoc->GetCurrentSheet()).GetBeats(); waitBeats += 2) {
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
    for (auto i = 0lu; i < mSolverParams.availableInstructions.size(); i++) {
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

#if 0
    if (mSelectedGroup != -1) {
        mSelectMembersButton->Enable();
        mSelectDestinationsButton->Enable();
    } else {
        mSelectMembersButton->Disable();
        mSelectDestinationsButton->Disable();
    }
#endif
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

void TransitionSolverFrame::OnNullEvent(wxCommandEvent&)
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
    const auto currentSheetNum = mDoc->GetCurrentSheetNum();
    const auto totalSheets = mDoc->GetNumSheets();
    unsigned numInstructions = 0;

    for (unsigned i = 0; i < mSolverParams.availableInstructions.size(); i++) {
        if (mSolverParams.availableInstructionsMask[i]) {
            numInstructions++;
        }
    }
    if (numInstructions == 0) {
        firstSheetErrors.push_back("No command options have been provided.");
    }

    if (currentSheetNum < totalSheets) {
        const CalChart::Sheet& firstSheet = *sheetIterOnFirstSheet;

        firstSheetErrors = validateSheetForTransitionSolver(firstSheet);
    } else {
        firstSheetErrors.push_back("No first sheet exists.\n");
    }

    if ((currentSheetNum + 1) < totalSheets) {
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

void TransitionSolverFrame::SetMembers(CalChart::SelectionList marchers)
{
    ClearMembers();
    AddMembers(marchers);
}

void TransitionSolverFrame::AddMembers(CalChart::SelectionList marchers)
{
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].marchers.insert(*iter);
    }
}

void TransitionSolverFrame::RemoveMembers(CalChart::SelectionList marchers)
{
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].marchers.erase(*iter);
    }
}

void TransitionSolverFrame::ClearDestinations()
{
    mSolverParams.groups[mSelectedGroup].allowedDestinations.clear();
}

void TransitionSolverFrame::SetDestinations(CalChart::SelectionList marchers)
{
    ClearDestinations();
    AddDestinations(marchers);
}

void TransitionSolverFrame::AddDestinations(CalChart::SelectionList marchers)
{
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].allowedDestinations.insert(*iter);
    }
}

void TransitionSolverFrame::RemoveDestinations(CalChart::SelectionList marchers)
{
    for (auto iter = marchers.begin(); iter != marchers.end(); iter++) {
        mSolverParams.groups[mSelectedGroup].allowedDestinations.erase(*iter);
    }
}

#pragma GCC diagnostic pop
