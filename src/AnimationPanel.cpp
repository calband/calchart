/*
 * AnimationPanel.cpp
 * Implimentation for AnimationPanel
 */

/*
   Copyright (C) 1995-2024  Richard Michael Powell

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
#include <wxUI/wxUI.h>

using namespace CalChart;

BEGIN_EVENT_TABLE(AnimationPanel, AnimationPanel::super)
EVT_SPINCTRL(CALCHART__anim_tempo, AnimationPanel::OnSlider_anim_tempo)
EVT_COMMAND_SCROLL(CALCHART__anim_gotobeat, AnimationPanel::OnSlider_anim_gotobeat)
EVT_TIMER(CALCHART__anim_next_beat_timer, AnimationPanel::OnCmd_anim_next_beat_timer)
END_EVENT_TABLE()

AnimationPanel::AnimationPanel(CalChartConfiguration& config, wxWindow* parent, wxWindowID winid, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : super(parent, winid, pos, size, style, name)
    , mCanvas(new AnimationCanvas(config, this, wxID_ANY, wxDefaultPosition, wxSize(-1, GetAnimationCanvasMinY())))
    , mOmniCanvas(new CCOmniviewCanvas(this, config))
    , mTimer(new wxTimer(this, CALCHART__anim_next_beat_timer))
    , mTempo(120)
    , mTimerOn(false)
    , mInMiniMode(true)
    , mConfig(config)
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
    auto tmp = GetTempo();

    wxUI::VSizer{
        BasicSizerFlags(),
        wxUI::HSizer{
            mAnimateOmniToggle = wxUI::Button{ "Omni" }
                                     .bind([this] {
                                         OnCmd_ToggleAnimOmni();
                                     }),
            mPlayPauseButton = wxUI::BitmapToggleButton{ ScaleButtonBitmap(wxBitmap{ BITMAP_NAME(tb_play) }), ScaleButtonBitmap(wxBitmap{ BITMAP_NAME(tb_stop) }) }
                                   .bind([this] {
                                       OnCmd_PlayButton();
                                   }),
            mBeatSlider = wxUI::Slider{ ExpandSizerFlags(), CALCHART__anim_gotobeat, std::pair{ 1, 2 }, 1 }
                              .withStyle(wxSL_HORIZONTAL | wxSL_LABELS),
            wxUI::VSizer{
                mTempoLabel = wxUI::Text{ "Tempo" },
                mTempoCtrl = wxUI::SpinCtrl{ CALCHART__anim_tempo, std::pair(10, 300), tmp },
            },
            wxUI::VSizer{
                mSpritesCheckbox = wxUI::CheckBox{ "Sprites" }
                                       .bind([this](wxCommandEvent& event) {
                                           mConfig.Set_UseSprites(event.IsChecked());
                                       }),
                mZoomCheckbox = wxUI::CheckBox{ "Zoom" }
                                    .bind([this](wxCommandEvent& event) {
                                        mCanvas->SetZoomOnMarchers(event.IsChecked());
                                    }),
            },
            wxUI::VSizer{
                mCollisionCheckbox = wxUI::CheckBox{ "Collisions" }
                                         .bind([this](wxCommandEvent& event) {
                                             if (mView) {
                                                 mView->SetDrawCollisionWarning(event.IsChecked());
                                             }
                                         }),
            },

            mOmniHelpButton = wxUI::Button{ wxID_HELP, "&Help" }
                                  .bind([this]() {
                                      mOmniCanvas->OnCmd_ShowKeyboardControls();
                                  }),
        },
        wxUI::Generic{ ExpandSizerFlags(), mCanvas },
        wxUI::Generic{ ExpandSizerFlags(), mOmniCanvas },
    }
        .attachTo(this);

    mItemsToHide.push_back(mAnimateOmniToggle.control());
    mItemsToHide.push_back(mOmniHelpButton.control());
    mItemsToHide.push_back(mSpritesCheckbox.control());
    mItemsToHide.push_back(mZoomCheckbox.control());
    mItemsToHide.push_back(mCollisionCheckbox.control());
    mItemsToHide.push_back(mTempoLabel.control());
    mItemsToHide.push_back(mTempoCtrl.control());
    SetTempo(tmp);
    mCanvas->Show();
    mOmniCanvas->Hide();

    for (auto&& i : mItemsToHide) {
        i->Show(!mInMiniMode);
    }
    SetInMiniMode(mInMiniMode);
}

void AnimationPanel::SetInMiniMode(bool miniMode)
{
    mInMiniMode = miniMode;
    for (auto&& i : mItemsToHide) {
        i->Show(!mInMiniMode);
    }
    // expand the slider to fill everything!
    mBeatSlider.control()->SetSizeHints(mInMiniMode ? -1 : GetAnimationViewBeatSliderInNonMinimode(), -1);
    Layout();
}

void AnimationPanel::OnCmd_anim_next_beat_timer(wxTimerEvent&)
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
        mSpritesCheckbox.control()->Hide();
        mZoomCheckbox.control()->Hide();
        mCollisionCheckbox.control()->Hide();
        mOmniCanvas->Show();
        mOmniHelpButton.control()->Show();
        mAnimateOmniToggle.control()->SetLabel("Animate");
    } else {
        mCanvas->Show();
        mSpritesCheckbox.control()->Show();
        mZoomCheckbox.control()->Show();
        mCollisionCheckbox.control()->Show();
        mOmniCanvas->Hide();
        mOmniHelpButton.control()->Hide();
        mAnimateOmniToggle.control()->SetLabel("Omni");
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
        mBeatSlider.control()->Enable(true);
        if (mBeatSlider.control()->GetMax() != num)
            *mBeatSlider = 0; // So Motif doesn't complain about value
        mBeatSlider.control()->SetRange(0, num);
        if (*mBeatSlider != curr)
            *mBeatSlider = curr;
    } else {
        mBeatSlider.control()->Enable(false);
    }
    *mPlayPauseButton = mTimerOn;
}

bool AnimationPanel::OnBeat() const
{
    return *mBeatSlider & 1;
}

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
    mView = new AnimationView(view, mConfig, this);
    mView->SetDocument(view->GetDocument());
    // at this point the document is manging the view.
    mView->SetFrame(this);
    mView->SetPlayCollisionWarning(!mInMiniMode);
    mCanvas->SetView(mView);
    mOmniCanvas->SetView(mView);

    *mCollisionCheckbox = mView->GetDrawCollisionWarning();
}
