//
//  e7_transition_solver_ui.cpp
//  CalChart
//
//  Created by Kevin Durand on 6/24/17.
//
//

#include "e7_transition_solver_ui.h"
#include "basic_ui.h"
#include "confgr.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_command.h"
#include "calchartapp.h"
#include "calchartdoc.h"
#include "cc_show.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/statline.h>
#include <wx/msgdlg.h>

BEGIN_EVENT_TABLE(TransitionSolverFrame, wxFrame)
EVT_MENU(wxID_CLOSE, TransitionSolverFrame::OnCloseWindow)
EVT_BUTTON(wxID_CLOSE, TransitionSolverFrame::OnCloseWindow)
EVT_BUTTON(wxID_APPLY, TransitionSolverFrame::OnApply)
END_EVENT_TABLE()

TransitionSolverView::TransitionSolverView() {}
TransitionSolverView::~TransitionSolverView() {}

void TransitionSolverView::OnDraw(wxDC* dc) {}
void TransitionSolverView::OnUpdate(wxView* sender, wxObject* hint)
{
    TransitionSolverFrame* frame = static_cast<TransitionSolverFrame*>(GetFrame());
    frame->Update();
}

void TransitionSolverView::SolveTransition(TransitionSolverParams params)
{
//    GetDocument()->GetCommandProcessor()->Submit( // TODO
//                                                 new SetContinuityTextCommand(*static_cast<CalChartDoc*>(GetDocument()),
//                                                                              which, text),
//                                                 true);
}

TransitionSolverFrame::TransitionSolverFrame() { Init(); }

TransitionSolverFrame::TransitionSolverFrame(CalChartDoc* show, wxWindow* parent,
                                             wxWindowID id, const wxString& caption,
                                             const wxPoint& pos, const wxSize& size,
                                             long style)
{
    Init();
    
    Create(show, parent, id, caption, pos, size, style);
}

void TransitionSolverFrame::Init() {}

bool TransitionSolverFrame::Create(CalChartDoc* show, wxWindow* parent,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size,
                                   long style)
{
    if (!wxFrame::Create(parent, id, caption, pos, size, style))
        return false;
    
    mDoc = show;
    mView = new TransitionSolverView;
    mView->SetDocument(show);
    mView->SetFrame(this);
    
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

void TransitionSolverFrame::CreateControls()
{
    // menu bar
    wxMenu* cont_menu = new wxMenu;
    cont_menu->Append(wxID_CLOSE, wxT("Close Window\tCTRL-W"),
                      wxT("Close this window"));
    
    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(cont_menu, wxT("&File"));
    SetMenuBar(menu_bar);
    
    // create a sizer for laying things out top down:
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);
    
    wxButton *button = new wxButton(this, wxID_CLOSE, wxT("Close"));
    
    topsizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    
//    // add buttons to the top row
//    // New, delete, choices
//    wxBoxSizer* top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
//    mContinuityChoices = new wxChoice(this, ContinuityEditor_ContEditCurrent);
//    top_button_sizer->Add(mContinuityChoices, 0, wxALIGN_CENTER_VERTICAL | wxALL,
//                          5);
//    
//    // select
//    wxButton* button = new wxButton(this, ContinuityEditor_ContEditSelect,
//                                    wxT("Select &Points"));
//    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
//    topsizer->Add(top_button_sizer);
//    
//    mUserInput = new FancyTextWin(this, ContinuityEditor_KeyPress, wxEmptyString,
//                                  wxDefaultPosition, wxSize(50, 300));
//    
//    topsizer->Add(mUserInput, 0, wxGROW | wxALL, 5);
//    
//    // add a horizontal bar to make things clear:
//    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition,
//                                          wxDefaultSize, wxLI_HORIZONTAL);
//    topsizer->Add(line, 0, wxGROW | wxALL, 5);
//    
//    // add a save, discard, close, and help
//    top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
//    button = new wxButton(this, ContinuityEditor_Save, wxT("&Save"));
//    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
//    button = new wxButton(this, ContinuityEditor_Discard, wxT("&Discard"));
//    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
//    button = new wxButton(this, wxID_CLOSE, wxT("Close"));
//    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
//    button = new wxButton(this, wxID_HELP, wxT("&Help"));
//    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
//    topsizer->Add(top_button_sizer);
}

TransitionSolverFrame::~TransitionSolverFrame()
{
    if (mView)
    {
        delete mView;
    }
}

void TransitionSolverFrame::OnCloseWindow(wxCommandEvent& event)
{
    Close();
}

void TransitionSolverFrame::SyncControlsWithCurrentState()
{
}

void TransitionSolverFrame::SyncCurrentStateWithControls()
{
}

void TransitionSolverFrame::OnApply(wxCommandEvent&) { Apply(); }

void TransitionSolverFrame::Apply()
{
    SyncCurrentStateWithControls();
    mView->SolveTransition(mSolverParams);
}