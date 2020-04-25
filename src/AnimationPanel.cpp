/*
 * AnimationPanel.cpp
 * Implimentation for AnimationPanel
 */

/*
   Copyright (C) 1995-2012  Richard Michael Powell

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

#include "AnimationPanel.h"
#include "AnimationCanvas.h"
#include "AnimationView.h"
#include "CalChartView.h"
#include "basic_ui.h"
#include "cc_omniview_canvas.h"
#include "confgr.h"
#include "platconf.h"
#include "ui_enums.h"

#include "tb_stop.xbm"
#include "tb_play.xbm"

#include <wx/artprov.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>

using namespace CalChart;

BEGIN_EVENT_TABLE(AnimationPanel, AnimationPanel::super)
EVT_SPINCTRL(CALCHART__anim_tempo, AnimationPanel::OnSlider_anim_tempo)
EVT_COMMAND_SCROLL(CALCHART__anim_gotobeat, AnimationPanel::OnSlider_anim_gotobeat)
EVT_TIMER(CALCHART__anim_next_beat_timer, AnimationPanel::OnCmd_anim_next_beat_timer)
EVT_TOGGLEBUTTON(CALCHART__anim_play_button, AnimationPanel::OnCmd_PlayButton)
END_EVENT_TABLE()


AnimationPanel::AnimationPanel(wxWindow* parent,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : super(parent, winid, pos, size, style, name)
    , mCanvas(new AnimationCanvas(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 100)))
    , mTimer(new wxTimer(this, CALCHART__anim_next_beat_timer))
    , mTempo(120)
    , mTimerOn(false)
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* toprow = new wxBoxSizer(wxHORIZONTAL);
    AddToSizerBasic(topsizer, toprow);

    auto button = new wxBitmapToggleButton(this, CALCHART__anim_play_button, wxBitmap(BITMAP_NAME(tb_play)));
    button->SetBitmapPressed(wxBitmap(BITMAP_NAME(tb_stop)));
    AddToSizerBasic(toprow, button);

    // Sheet slider (will get set later with UpdatePanel())
    mBeatSlider = new wxSlider(this, CALCHART__anim_gotobeat, 1, 1, 2, wxDefaultPosition, wxSize(-1, -1), wxSL_HORIZONTAL | wxSL_LABELS);
    AddToSizerExpand(toprow, mBeatSlider);

    auto boxsizer = new wxBoxSizer(wxVERTICAL);
    AddToSizerBasic(toprow, boxsizer);
    mTempoLabel = new wxStaticText(this, wxID_ANY, wxT("Tempo"));
    AddToSizerBasic(boxsizer, mTempoLabel);
    // defect 3538572: Callings set may update tempo, cache value before call.
    unsigned tmp = GetTempo();
    mTempoCtrl = new wxSpinCtrl(this, CALCHART__anim_tempo, wxEmptyString, wxDefaultPosition, wxSize(48, -1));
    mTempoCtrl->SetRange(10, 300);
    mTempoCtrl->SetValue(tmp);
    SetTempo(tmp);
    AddToSizerBasic(boxsizer, mTempoCtrl);

    auto sizer1 = new wxBoxSizer(wxVERTICAL);
    mZoomCheckbox = new wxCheckBox(this, wxID_ANY, wxT("Zoom"));
    mZoomCheckbox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
        mCanvas->SetZoomOnMarchers(event.IsChecked());
    });
    mZoomCheckbox->SetValue(mCanvas->GetZoomOnMarchers());
    AddToSizerBasic(sizer1, mZoomCheckbox);
    mCollisionCheckbox = new wxCheckBox(this, wxID_ANY, wxT("Collision"));
    mCollisionCheckbox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
        if (mView) { mView->SetDrawCollisionWarning(event.IsChecked()); }
    });
    AddToSizerBasic(sizer1, mCollisionCheckbox);
    AddToSizerBasic(toprow, sizer1);

    AddToSizerExpand(topsizer, mCanvas);

    SetMainViewMode(false);
    SetSizer(topsizer);

    // now fit the frame to the elements
    topsizer->Fit(this);
    topsizer->SetSizeHints(this);

    // now update the current screen
    OnUpdate();
}

AnimationPanel::~AnimationPanel()
{
    mTimer->Stop();
}

void AnimationPanel::SetMainViewMode(bool mainViewMode)
{
    mMainViewMode = mainViewMode;
    if (mMainViewMode) {
        // expand the slider to fill everything!
        mBeatSlider->SetSizeHints(800, -1);
        mTempoLabel->Show();
        mTempoCtrl->Show();
        mZoomCheckbox->Show();
        mCollisionCheckbox->Show();
    }
    else {
        mBeatSlider->SetSizeHints(-1, -1);
        mTempoLabel->Hide();
        mTempoCtrl->Hide();
        mZoomCheckbox->Hide();
        mCollisionCheckbox->Hide();
    }
    Layout();
}

void AnimationPanel::OnCmd_anim_next_beat_timer(wxTimerEvent& event)
{
    // next_beat could come from the timer.  If so, stop the timer.
    if (!mView->NextBeat()) {
        StopTimer();
    }
    Refresh();
}

void AnimationPanel::OnCmd_PlayButton(wxCommandEvent& event)
{
    ToggleTimer();
}

void AnimationPanel::OnSlider_anim_tempo(wxSpinEvent& event)
{
    SetTempo(event.GetPosition());
    StartTimer();
}

void AnimationPanel::OnSlider_anim_gotobeat(wxScrollEvent& event)
{
    mView->GotoTotalBeat(event.GetPosition());
}

void AnimationPanel::ToggleTimer()
{
    if (mTimerOn)
        StopTimer();
    else
        StartTimer();
}

void AnimationPanel::UpdatePanel()
{
    if (!mView) {
        return;
    }
    auto num = mView->GetTotalNumberBeats() - 1;
    auto curr = mView->GetTotalCurrentBeat();

    if (num > 0) {
        mBeatSlider->Enable(true);
        if (mBeatSlider->GetMax() != num)
            mBeatSlider->SetValue(0); // So Motif doesn't complain about value
        mBeatSlider->SetRange(0, num);
        if (mBeatSlider->GetValue() != curr)
            mBeatSlider->SetValue(curr);
    } else {
        mBeatSlider->Enable(false);
    }
}

bool AnimationPanel::OnBeat() const { return mBeatSlider->GetValue() & 1; }

void AnimationPanel::StartTimer()
{
    if (!mTimer->Start(60000 / GetTempo())) {
        mTimerOn = false;
    } else {
        mTimerOn = true;
    }
}

void AnimationPanel::StopTimer()
{
    mTimer->Stop();
    mTimerOn = false;
}

void AnimationPanel::OnCmd_anim_prev_beat(wxCommandEvent& event)
{
    if (!mView) {
        return;
    }
    mView->PrevBeat();
    Refresh();
}

void AnimationPanel::OnCmd_anim_next_beat(wxCommandEvent& event)
{
    if (!mView) {
        return;
    }
    mView->NextBeat();
    Refresh();
}

void AnimationPanel::OnCmd_anim_prev_sheet(wxCommandEvent& event)
{
    if (!mView) {
        return;
    }
    mView->PrevSheet();
    Refresh();
}

void AnimationPanel::OnCmd_anim_next_sheet(wxCommandEvent& event)
{
    if (!mView) {
        return;
    }
    mView->NextSheet();
    Refresh();
}

void AnimationPanel::OnUpdate()
{
    if (!mView) {
        return;
    }
    UpdatePanel();
    mView->RefreshSheet();
    Refresh();
}

void AnimationPanel::SetView(CalChartView* view)
{
    mView = std::make_unique<AnimationView>(view, this);
    mView->SetDocument(view->GetDocument());
    mView->SetFrame(this);
    mCanvas->SetView(mView.get());

    mCollisionCheckbox->SetValue(mView->GetDrawCollisionWarning());
}
