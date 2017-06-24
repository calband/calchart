//
//  e7_transition_solver_ui.h
//  CalChart
//
//  Created by Kevin Durand on 6/24/17.
//
//

#pragma once

#include "calchartdoc.h"

#include <array>

#include <wx/docview.h>
#include <wx/dialog.h>

struct TransitionSolverParams {
    
    struct GroupParams {
        std::vector<unsigned> marchers;
        std::vector<unsigned> allowedDestinations;
    };
    
    struct InstructionOption {
        enum Pattern {
            EWNS,
            NSEW,
            DMHS,
            HSDM
        };
        
        Pattern movementPattern;
        unsigned waitBeats;
    };
    
    std::vector<GroupParams> groups;
    std::array<InstructionOption, 8> availableInstructions;
    std::array<bool, 8> availableInstructionsMask[8];
};


// View for linking CalChartDoc with the Transition Solver
class TransitionSolverView : public wxView {
public:
    TransitionSolverView();
    ~TransitionSolverView();

    virtual void OnDraw(wxDC* dc) override;
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL) override;
    
    void SolveTransition(TransitionSolverParams params);
};

// TransitionSolverFrame
// This offers a UI for editing the options that will affect transition calculation
class TransitionSolverFrame : public wxFrame {
    friend class TransitionSolverView;
    
public:
    TransitionSolverFrame();
    TransitionSolverFrame(CalChartDoc* dcr, wxWindow* parent, wxWindowID id = wxID_ANY,
                          const wxString& caption = wxT("Solve Transition"),
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxDefaultSize,
                          long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    ~TransitionSolverFrame();
    
    void OnCloseWindow(wxCommandEvent& event);
    void OnApply(wxCommandEvent&);
    
    void SyncControlsWithCurrentState();
    void SyncCurrentStateWithControls();
    
private:
    void Init();
    bool Create(CalChartDoc* shw, wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxString& caption = wxT("Select Points"),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    void CreateControls();
    
    
    void Apply();
    
    
    TransitionSolverParams mSolverParams;
    
    CalChartDoc* mDoc;
    TransitionSolverView* mView;
    
    DECLARE_EVENT_TABLE()
};