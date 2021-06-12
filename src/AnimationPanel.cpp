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
#include "CCOmniviewCanvas.h"
#include "CalChartConfiguration.h"
#include "CalChartSizes.h"
#include "CalChartView.h"
#include "basic_ui.h"
#include "platconf.h"
#include "ui_enums.h"

#include "tb_play.xbm"
#include "tb_stop.xbm"

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
END_EVENT_TABLE()

AnimationPanel::AnimationPanel(wxWindow* parent, wxWindowID winid, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : super(parent, winid, pos, size, style, name)
    , mCanvas(new AnimationCanvas(this, wxID_ANY, wxDefaultPosition, wxSize(-1, GetAnimationCanvasMinY())))
    , mTimer(new wxTimer(this, CALCHART__anim_next_beat_timer))
    , mTempo(120)
    , mTimerOn(false)
    , mInMiniMode(true)
{
    Init();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    OnUpdate();
}

AnimationPanel::~AnimationPanel()
{
    mTimer->Stop();
}

void AnimationPanel::Init()
{
}

void AnimationPanel::CreateControls()
{
    // create a sizer and populate
    SetSizer(VStack([this](auto sizer) {
        HStack(sizer, [this](auto sizer) {
            mAnimateOmniToggle = CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "Omni", [this]() {
                OnCmd_ToggleAnimOmni();
            });
            mItemsToHide.push_back(mAnimateOmniToggle);

            mPlayPauseButton = CreateBitmapToggleWithHandler(this, sizer, BasicSizerFlags(), ScaleButtonBitmap(wxBitmap{ BITMAP_NAME(tb_play) }), ScaleButtonBitmap(wxBitmap{ BITMAP_NAME(tb_stop) }), [this]() {
                OnCmd_PlayButton();
            });

            // Sheet slider (will get set later with UpdatePanel())
            mBeatSlider = new wxSlider(this, CALCHART__anim_gotobeat, 1, 1, 2, wxDefaultPosition, wxSize(-1, -1), wxSL_HORIZONTAL | wxSL_LABELS);
            AddToSizerExpand(sizer, mBeatSlider);

            VStack(sizer, [this](auto sizer) {
                mTempoLabel = CreateText(this, sizer, BasicSizerFlags(), "Tempo");
                mItemsToHide.push_back(mTempoLabel);
                // defect 3538572: Callings set may update tempo, cache value before call.
                auto tmp = GetTempo();
                mTempoCtrl = new wxSpinCtrl(this, CALCHART__anim_tempo, wxEmptyString, wxDefaultPosition, wxSize(GetAnimationViewTempoSpinnerMinX(), -1));
                mTempoCtrl->SetRange(10, 300);
                mTempoCtrl->SetValue(tmp);
                SetTempo(tmp);
                AddToSizerBasic(sizer, mTempoCtrl);
                mItemsToHide.push_back(mTempoCtrl);
            });

            VStack(sizer, [this](auto sizer) {
                mSpritesCheckbox = CreateCheckbox(this, sizer, "Sprites", [this](wxCommandEvent& event) {
                    mCanvas->SetUseSprites(event.IsChecked());
                });
                mSpritesCheckbox->SetValue(mCanvas->GetUseSprites());
                mItemsToHide.push_back(mSpritesCheckbox);

                mZoomCheckbox = CreateCheckbox(this, sizer, "Zoom", [this](wxCommandEvent& event) {
                    mCanvas->SetZoomOnMarchers(event.IsChecked());
                });
                mZoomCheckbox->SetValue(mCanvas->GetZoomOnMarchers());
                mItemsToHide.push_back(mZoomCheckbox);

                mCollisionCheckbox = CreateCheckbox(this, sizer, "Collisions", [this](wxCommandEvent& event) {
                    if (mView) {
                        mView->SetDrawCollisionWarning(event.IsChecked());
                    }
                });
                mItemsToHide.push_back(mCollisionCheckbox);
            });

            mOmniHelpButton = new wxButton(this, wxID_HELP, "&Help");
            mOmniHelpButton->Bind(wxEVT_BUTTON, [this](auto const&) {
                mOmniCanvas->OnCmd_ShowKeyboardControls();
            });
            AddToSizerBasic(sizer, mOmniHelpButton);
            mItemsToHide.push_back(mOmniHelpButton);
        });

        mOmniCanvas = new CCOmniviewCanvas(this, CalChartConfiguration::GetGlobalConfig());

        AddToSizerExpand(sizer, mCanvas);
        AddToSizerExpand(sizer, mOmniCanvas);

        // we default to animate view
        mCanvas->Show();
        mOmniCanvas->Hide();

        for (auto&& i : mItemsToHide) {
            i->Show(!mInMiniMode);
        }
        SetInMiniMode(mInMiniMode);
    }));
}

void AnimationPanel::SetInMiniMode(bool miniMode)
{
    mInMiniMode = miniMode;
    for (auto&& i : mItemsToHide) {
        i->Show(!mInMiniMode);
    }
    // expand the slider to fill everything!
    mBeatSlider->SetSizeHints(mInMiniMode ? -1 : GetAnimationViewBeatSliderInNonMinimode(), -1);
    Layout();
}

void AnimationPanel::OnCmd_anim_next_beat_timer(wxTimerEvent& event)
{
    if (!mView) {
        return;
    }
    // next_beat could come from the timer.  If so, stop the timer.
    mView->NextBeat();
    if (mView->AtEndOfShow()) {
        StopTimer();
    }
    Refresh();
}

void AnimationPanel::OnCmd_PlayButton()
{
    ToggleTimer();
}

void AnimationPanel::OnCmd_ToggleAnimOmni()
{
    mShowOmni = !mShowOmni;
    if (mShowOmni) {
        mCanvas->Hide();
        mSpritesCheckbox->Hide();
        mZoomCheckbox->Hide();
        mCollisionCheckbox->Hide();
        mOmniCanvas->Show();
        mOmniHelpButton->Show();
        mAnimateOmniToggle->SetLabel("Animate");
    } else {
        mCanvas->Show();
        mSpritesCheckbox->Show();
        mZoomCheckbox->Show();
        mCollisionCheckbox->Show();
        mOmniCanvas->Hide();
        mOmniHelpButton->Hide();
        mAnimateOmniToggle->SetLabel("Omni");
    }
    Layout();
    Refresh();
}

void AnimationPanel::OnSlider_anim_tempo(wxSpinEvent& event)
{
    SetTempo(event.GetPosition());
    if (mTimerOn) {
        StopTimer();
        StartTimer();
    }
    UpdatePanel();
}

void AnimationPanel::OnSlider_anim_gotobeat(wxScrollEvent& event)
{
    mView->GotoTotalBeat(event.GetPosition());
}

void AnimationPanel::ToggleTimer()
{
    if (mTimerOn) {
        StopTimer();
    } else {
        StartTimer();
    }
    UpdatePanel();
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
    mPlayPauseButton->SetValue(mTimerOn);
}

bool AnimationPanel::OnBeat() const { return mBeatSlider->GetValue() & 1; }

void AnimationPanel::SetPlayState(bool playState)
{
    if (playState) {
        StartTimer();
    } else {
        StopTimer();
    }
}

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

void AnimationPanel::OnUpdate()
{
    if (!mView) {
        return;
    }
    UpdatePanel();
    mView->RefreshAnimationSheet();
    Refresh();
}

void AnimationPanel::SetView(CalChartView* view)
{
    if (!view) {
        mView = nullptr;
        return;
    }
    mView = new AnimationView(view, this);
    mView->SetDocument(view->GetDocument());
    // at this point the document is manging the view.
    mView->SetFrame(this);
    mView->SetPlayCollisionWarning(!mInMiniMode);
    mCanvas->SetView(mView);
    mOmniCanvas->SetView(mView);

    mCollisionCheckbox->SetValue(mView->GetDrawCollisionWarning());
}
