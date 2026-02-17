/*
 * AnimationPanel.cpp
 * Implimentation for AnimationPanel
 */

/*
   Copyright (C) 1995-2025  Richard Michael Powell

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
#include <wxUI/wxUI.hpp>

using namespace CalChart;

BEGIN_EVENT_TABLE(AnimationPanel, AnimationPanel::super)
EVT_COMMAND_SCROLL(CALCHART__anim_gotosheet, AnimationPanel::OnSlider_anim_gotosheet)
EVT_COMMAND_SCROLL(CALCHART__anim_gotobeat, AnimationPanel::OnSlider_anim_gotobeat)
EVT_TIMER(CALCHART__anim_next_beat_timer, AnimationPanel::OnCmd_anim_next_beat_timer)
END_EVENT_TABLE()

AnimationPanel::AnimationPanel(
    CalChart::Configuration& config,
    wxWindow* parent,
    bool miniMode)
    : super(parent)
    , mCanvas(new AnimationCanvas(config, *this, wxSize(-1, GetAnimationCanvasMinY())))
    , mOmniCanvas(new CCOmniviewCanvas(*this, config))
    , mTimer(new wxTimer(this, CALCHART__anim_next_beat_timer))
    , mTempo(120)
    , mTimerOn(false)
    , mInMiniMode(miniMode)
    , mPlayCollisionWarning{ !mInMiniMode }
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
            wxUI::Button{ "Omni" }
                .bind([this] {
                    OnCmd_ToggleAnimOmni();
                })
                .withProxy(mAnimateOmniToggle),
            wxUI::BitmapToggleButton{ ScaleButtonBitmap(wxBitmap{ BITMAP_NAME(tb_play) }), ScaleButtonBitmap(wxBitmap{ BITMAP_NAME(tb_stop) }) }
                .bind([this] {
                    OnCmd_PlayButton();
                })
                .withProxy(mPlayPauseButton),
            wxUI::LayoutIf{
                !mInMiniMode && mConfig.Get_AnimationFrameSheetSlider(),
                wxUI::Text{ "Sheet" },
                wxUI::Slider{ CALCHART__anim_gotosheet, std::pair{ 1, 2 }, 1 }
                    .withStyle(wxSL_HORIZONTAL | wxSL_LABELS)
                    .withProxy(mSheetSlider),
                wxUI::Text{ "Beat" },
            },
            wxUI::Slider{ CALCHART__anim_gotobeat, std::pair{ 1, 2 }, 1 }
                .withStyle(wxSL_HORIZONTAL | wxSL_LABELS)
                .withFlags(ExpandSizerFlags())
                .withProxy(mBeatSlider),
            wxUI::VSizer{
                wxUI::Text{ "Tempo" }.withProxy(mTempoLabel),
                wxUI::Text{ std::to_string(tmp) }.withProxy(mTempoValue),
            },
            wxUI::VSizer{
                wxUI::CheckBox{ "Sprites" }
                    .withValue(mConfig.Get_UseSprites())
                    .bind([this](wxCommandEvent& event) {
                        mConfig.Set_UseSprites(event.IsChecked());
                        Refresh();
                    })
                    .withProxy(mSpritesCheckbox),
                wxUI::CheckBox{ "Zoom" }
                    .withValue(mCanvas->GetZoomOnMarchers())
                    .bind([this](wxCommandEvent& event) {
                        mCanvas->SetZoomOnMarchers(event.IsChecked());
                    })
                    .withProxy(mZoomCheckbox),
            },
            wxUI::VSizer{
                wxUI::CheckBox{ "Collisions" }
                    .bind([this](wxCommandEvent& event) {
                        mDrawCollisionWarning = event.IsChecked();
                    })
                    .withProxy(mCollisionCheckbox),
            },

            wxUI::Button{ wxID_HELP, "&Help" }
                .bind([this]() {
                    mOmniCanvas->OnCmd_ShowKeyboardControls();
                })
                .withProxy(mOmniHelpButton),
        },
        wxUI::Generic{ ExpandSizerFlags(), mCanvas },
        wxUI::Generic{ ExpandSizerFlags(), mOmniCanvas },
    }
        .fitTo(this);

    mItemsToHide.push_back(mAnimateOmniToggle.control());
    mItemsToHide.push_back(mOmniHelpButton.control());
    mItemsToHide.push_back(mSpritesCheckbox.control());
    mItemsToHide.push_back(mZoomCheckbox.control());
    mItemsToHide.push_back(mCollisionCheckbox.control());
    mItemsToHide.push_back(mTempoLabel.control());
    mItemsToHide.push_back(mTempoValue.control());

    *mCollisionCheckbox = mDrawCollisionWarning;

    SetTempo(tmp);
    mCanvas->Show();
    mOmniCanvas->Hide();

    for (auto&& i : mItemsToHide) {
        i->Show(!mInMiniMode);
    }
    // expand the slider to fill everything!
    mBeatSlider.control()->SetSizeHints(mInMiniMode ? -1 : GetAnimationViewBeatSliderInNonMinimode(), -1);
    Layout();
}

auto AnimationPanel::GenerateDrawCommands() -> std::vector<CalChart::Draw::DrawCommand>
{
    if (!mView) {
        return {};
    }
    mSprites.RegenerateImages(mConfig);

    auto onBeat = std::optional<bool>{ std::nullopt };
    if (mTimerOn) {
        onBeat = OnBeat();
    }
    return mView->GenerateAnimationDrawCommands(
        mCurrentBeat,
        mDrawCollisionWarning,
        onBeat,
        [this](CalChart::Radian angle, CalChart::Animation::ImageBeat imageBeat, bool selected) {
            return mSprites.GetImage(angle, imageBeat, selected);
        });
}

void AnimationPanel::OnCmd_anim_next_beat_timer(wxTimerEvent&)
{
    // next_beat could come from the timer.  If so, stop the timer.
    NextBeat();
    if (AtEndOfShow()) {
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

void AnimationPanel::OnSlider_anim_gotobeat(wxScrollEvent& event)
{
    if (mSheetSlider.control()) {
        GotoSheetBeat(*mSheetSlider - 1, event.GetPosition());
    } else {
        GotoTotalBeat(event.GetPosition());
    }
    Refresh();
}

void AnimationPanel::OnSlider_anim_gotosheet(wxScrollEvent& event)
{
    if (mSheetSlider.control()) {
        // because we want to show the sheets with offset 1, we need -1.
        GotoSheetBeat(event.GetPosition() - 1, 0);
    }
    Refresh();
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

namespace {
auto disableSlider(auto& slider)
{
    if (slider.control() == nullptr) {
        return;
    }
    slider.control()->Enable(false);
}

auto updateController(auto& slider, int num, int curr, bool zeroOffset = true)
{
    if (slider.control() == nullptr) {
        return;
    }
    if (num > 0) {
        slider.control()->SetRange(zeroOffset ? 0 : 1, num);
        if (*slider != curr) {
            *slider = curr;
        }
        slider.control()->Enable(true);
    } else {
        slider.control()->Enable(false);
    }
}
}

void AnimationPanel::UpdatePanel()
{
    auto totalBeats = GetTotalNumberBeats() - 1;
    auto currentBeat = mCurrentBeat;
    auto totalSheets = GetNumSheets() - 1; // gives us the total number sheets
    auto sheetBeatStuff = BeatToSheetOffsetAndBeat(currentBeat);
    if (!sheetBeatStuff) {
        disableSlider(mBeatSlider);
        disableSlider(mSheetSlider);
        return;
    }
    auto [currentSheet, sheetBeat] = *sheetBeatStuff; // gives us which shee we are on and beat
    auto beatsForCurrentSheet = BeatForSheet(currentSheet) - 1; // gives us which shee we are on and beat

    if (mSheetSlider.control()) {
        updateController(mBeatSlider, static_cast<int>(beatsForCurrentSheet), static_cast<int>(sheetBeat));
    } else {
        updateController(mBeatSlider, static_cast<int>(totalBeats), static_cast<int>(currentBeat));
    }
    updateController(mSheetSlider, totalSheets + 1, currentSheet + 1, false);
    *mPlayPauseButton = mTimerOn;

    // Update tempo display
    if (mView && mTempoValue.control()) {
        auto currentTempo = mView->GetTempoForBeat(mCurrentBeat);
        *mTempoValue = std::to_string(currentTempo);
        SetTempo(currentTempo);
    }

    if (mPlayCollisionWarning && mView->BeatHasCollision(mCurrentBeat) && mConfig.Get_BeepOnCollisions()) {
        wxBell();
    }
}

bool AnimationPanel::OnBeat() const
{
    return mCurrentBeat & 1;
}

void AnimationPanel::UnselectAll()
{
    mView->UnselectAll();
}

void AnimationPanel::SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown)
{
    auto mouseStartTranslated = tDIP(mouseStart);
    auto mouseEndTranslated = tDIP(mouseEnd);

    auto [x_off, y_off] = mView->GetShowFieldOffset();
    auto polygon = CalChart::RawPolygon_t{
        CalChart::Coord(mouseStartTranslated.x - x_off, mouseStartTranslated.y - y_off),
        CalChart::Coord(mouseStartTranslated.x - x_off, mouseEndTranslated.y - y_off),
        CalChart::Coord(mouseEndTranslated.x - x_off, mouseEndTranslated.y - y_off),
        CalChart::Coord(mouseEndTranslated.x - x_off, mouseStartTranslated.y - y_off),
    };
    mView->SelectWithinPolygon(polygon, altDown);
}

auto AnimationPanel::GetTotalNumberBeats() const -> CalChart::Beats
{
    if (!mView) {
        return 0;
    }
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    return totalBeats.value_or(0);
}

auto AnimationPanel::GetNumSheets() const -> size_t
{
    if (!mView) {
        return 0;
    }
    return mView->GetNumSheets();
}

auto AnimationPanel::BeatToSheetOffsetAndBeat(CalChart::Beats beat) const -> std::optional<std::tuple<size_t, CalChart::Beats>>
{
    if (!mView) {
        return std::nullopt;
    }
    return mView->BeatToSheetOffsetAndBeat(beat);
}

auto AnimationPanel::BeatForSheet(int sheet) const -> CalChart::Beats
{
    if (!mView) {
        return 0;
    }
    return mView->BeatForSheet(sheet);
}

auto AnimationPanel::GetAnimationBoundingBox(bool zoomInOnMarchers) const -> std::pair<CalChart::Coord, CalChart::Coord>
{
    return mView->GetAnimationBoundingBox(zoomInOnMarchers, mCurrentBeat);
}

auto AnimationPanel::GetShowFieldSize() const -> CalChart::Coord
{
    return mView->GetShowFieldSize();
}

auto AnimationPanel::GetMarcherInfo() const -> std::vector<CalChart::Animate::Info>
{
    return mView->GetAnimationInfo(mCurrentBeat);
}

auto AnimationPanel::GetMarcherInfo(CalChart::MarcherIndex whichMarcher) const -> std::optional<CalChart::Animate::Info>
{
    return mView->GetAnimationInfo(whichMarcher, mCurrentBeat);
}

void AnimationPanel::PrevBeat()
{
    if (mCurrentBeat == 0) {
        return;
    }
    mCurrentBeat -= 1;
    UpdateTempoIfChanged();
    UpdatePanel();
}

void AnimationPanel::NextBeat()
{
    if (!mView) {
        return;
    }
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    if (totalBeats) {
        if (mCurrentBeat >= (*totalBeats - 1)) {
            return;
        }
        mCurrentBeat += 1;
        UpdateTempoIfChanged();
        UpdatePanel();
    }
}

void AnimationPanel::GotoTotalBeat(CalChart::Beats whichBeat)
{
    mCurrentBeat = whichBeat;
    UpdateTempoIfChanged();
    UpdatePanel();
}

void AnimationPanel::GotoSheetBeat(int whichSheet, CalChart::Beats whichBeat)
{
    mCurrentBeat = whichBeat + mView->GetTotalNumberBeatsUpTo(whichSheet);
    UpdateTempoIfChanged();
    UpdatePanel();
}

auto AnimationPanel::AtEndOfShow() const -> bool
{
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    if (totalBeats) {
        return (mCurrentBeat == *totalBeats);
    }
    return false;
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

void AnimationPanel::UpdateTempoIfChanged()
{
    if (!mView || !mTimerOn) {
        return;
    }

    auto newTempo = mView->GetTempoForBeat(mCurrentBeat);
    if (newTempo != GetTempo()) {
        SetTempo(newTempo);
        StopTimer();
        StartTimer();
    }
}

void AnimationPanel::OnUpdate()
{
    if (mView) {
        mCurrentBeat = mView->GetAnimationBeatForCurrentSheet();
    }
    UpdateTempoIfChanged();
    UpdatePanel();
}

void AnimationPanel::SetView(CalChartView* view)
{
    mView = view;
}
