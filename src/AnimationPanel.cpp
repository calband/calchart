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
#include <wxUI/wxUI.hpp>

using namespace CalChart;

BEGIN_EVENT_TABLE(AnimationPanel, AnimationPanel::super)
EVT_SPINCTRL(CALCHART__anim_tempo, AnimationPanel::OnSlider_anim_tempo)
EVT_COMMAND_SCROLL(CALCHART__anim_gotosheet, AnimationPanel::OnSlider_anim_gotosheet)
EVT_COMMAND_SCROLL(CALCHART__anim_gotobeat, AnimationPanel::OnSlider_anim_gotobeat)
EVT_TIMER(CALCHART__anim_next_beat_timer, AnimationPanel::OnCmd_anim_next_beat_timer)
END_EVENT_TABLE()

AnimationPanel::AnimationPanel(
    CalChart::Configuration& config,
    wxWindow* parent,
    bool miniMode,
    wxWindowID winid,
    wxPoint const& pos,
    wxSize const& size,
    long style,
    wxString const& name)
    : super(parent, winid, pos, size, style, name)
    , mCanvas(new AnimationCanvas(config, *this, wxSize(-1, GetAnimationCanvasMinY())))
    , mOmniCanvas(new CCOmniviewCanvas(this, config))
    , mTimer(new wxTimer(this, CALCHART__anim_next_beat_timer))
    , mTempo(120)
    , mTimerOn(false)
    , mInMiniMode(miniMode)
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
                wxUI::SpinCtrl{ CALCHART__anim_tempo, std::pair(10, 300), tmp }.withProxy(mTempoCtrl),
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
                        if (mView) {
                            mView->SetDrawCollisionWarning(event.IsChecked());
                        }
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
    mItemsToHide.push_back(mTempoCtrl.control());

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
    if (!mCalChartView) {
        return {};
    }
    mSprites.RegenerateImages(mConfig);

    auto onBeat = std::optional<bool>{ std::nullopt };
    if (mTimerOn) {
        onBeat = OnBeat();
    }
    return mCalChartView->GenerateAnimationDrawCommands(
        mCurrentBeat,
        mDrawCollisionWarning,
        onBeat,
        [this](CalChart::Radian angle, CalChart::Animation::ImageBeat imageBeat, bool selected) {
            return mSprites.GetImage(angle, imageBeat, selected);
        });
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
    if (mSheetSlider.control()) {
        mView->GotoSheetBeat(*mSheetSlider - 1, event.GetPosition());
    } else {
        mView->GotoTotalBeat(event.GetPosition());
    }
}

void AnimationPanel::OnSlider_anim_gotosheet(wxScrollEvent& event)
{
    if (mSheetSlider.control()) {
        // because we want to show the sheets with offset 1, we need -1.
        mView->GotoSheetBeat(event.GetPosition() - 1, 0);
    }
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
    if (!mView) {
        return;
    }
    auto totalBeats = mView->GetTotalNumberBeats() - 1;
    auto currentBeat = mView->GetTotalCurrentBeat();
    auto totalSheets = mView->GetNumSheets() - 1; // gives us the total number sheets
    auto sheetBeatStuff = mView->BeatToSheetOffsetAndBeat(currentBeat);
    if (!sheetBeatStuff) {
        disableSlider(mBeatSlider);
        disableSlider(mSheetSlider);
        return;
    }
    auto [currentSheet, sheetBeat] = *sheetBeatStuff; // gives us which shee we are on and beat
    auto beatsForCurrentSheet = mView->BeatForSheet(currentSheet) - 1; // gives us which shee we are on and beat

    if (mSheetSlider.control()) {
        updateController(mBeatSlider, static_cast<int>(beatsForCurrentSheet), static_cast<int>(sheetBeat));
    } else {
        updateController(mBeatSlider, static_cast<int>(totalBeats), static_cast<int>(currentBeat));
    }
    updateController(mSheetSlider, totalSheets + 1, currentSheet + 1, false);
    *mPlayPauseButton = mTimerOn;
    if (mPlayCollisionWarning && mCalChartView->BeatHasCollision(mCurrentBeat) && mConfig.Get_BeepOnCollisions()) {
        wxBell();
    }
}

bool AnimationPanel::OnBeat() const
{
    return mView->GetTotalCurrentBeat() & 1;
}

void AnimationPanel::UnselectAll()
{
    mCalChartView->UnselectAll();
}

void AnimationPanel::SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown)
{
    auto mouseStartTranslated = tDIP(mouseStart);
    auto mouseEndTranslated = tDIP(mouseEnd);

    auto [x_off, y_off] = mCalChartView->GetShowFieldOffset();
    auto polygon = CalChart::RawPolygon_t{
        CalChart::Coord(mouseStartTranslated.x - x_off, mouseStartTranslated.y - y_off),
        CalChart::Coord(mouseStartTranslated.x - x_off, mouseEndTranslated.y - y_off),
        CalChart::Coord(mouseEndTranslated.x - x_off, mouseEndTranslated.y - y_off),
        CalChart::Coord(mouseEndTranslated.x - x_off, mouseStartTranslated.y - y_off),
    };
    mCalChartView->SelectWithinPolygon(polygon, altDown);
}

auto AnimationPanel::GetAnimationBoundingBox(bool zoomInOnMarchers) const -> std::pair<CalChart::Coord, CalChart::Coord>
{
    return mCalChartView->GetAnimationBoundingBox(zoomInOnMarchers, mCurrentBeat);
}

void AnimationPanel::PrevBeat()
{
    if (mCurrentBeat == 0) {
        return;
    }
    mCurrentBeat -= 1;
    UpdatePanel();
}

void AnimationPanel::NextBeat()
{
    auto totalBeats = mCalChartView->GetTotalNumberAnimationBeats();
    if (totalBeats) {
        if (mCurrentBeat >= (*totalBeats - 1)) {
            return;
        }
        mCurrentBeat += 1;
        UpdatePanel();
    }
}

void AnimationPanel::GotoTotalBeat(CalChart::Beats whichBeat)
{
    mCurrentBeat = whichBeat;
    UpdatePanel();
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
    mCurrentBeat = mCalChartView->GetAnimationBeatForCurrentSheet();
    Refresh();
}

void AnimationPanel::SetView(CalChartView* view)
{
    mCalChartView = view;
    if (!view) {
        mView = nullptr;
        return;
    }
    mView = new AnimationView(view, mConfig, this);
    mView->SetDocument(view->GetDocument());
    // at this point the document is manging the view.
    mView->SetFrame(this);
    mView->SetPlayCollisionWarning(!mInMiniMode);
    mOmniCanvas->SetView(mView);

    *mCollisionCheckbox = mView->GetDrawCollisionWarning();
}

#if 0
void AnimationPanel::RefreshAnimationSheet()
{
    mCurrentBeat = mCalChartView->GetAnimationBeatForCurrentSheet();
    UpdatePanel();
}

int AnimationView::GetTotalNumberBeats() const
{
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    return totalBeats.value_or(0);
}

int AnimationView::GetTotalCurrentBeat() const
{
    return mCurrentBeat;
}

// #define GENERATE_SHOW_DUMP 1

void AnimationView::Generate()
{
    mCurrentBeat = 0;
}

void AnimationView::PrevBeat()
{
    if (mCurrentBeat == 0) {
        return;
    }
    mCurrentBeat -= 1;
    RefreshFrame();
}

void AnimationView::NextBeat()
{
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    if (totalBeats) {
        if (mCurrentBeat >= (*totalBeats - 1)) {
            return;
        }
        mCurrentBeat += 1;
        RefreshFrame();
    }
}

void AnimationView::GotoTotalBeat(CalChart::Beats whichBeat)
{
    mCurrentBeat = whichBeat;
    RefreshFrame();
}

auto AnimationView::AtEndOfShow() const -> bool
{
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    if (totalBeats) {
        return (mCurrentBeat == *totalBeats);
    }
    return false;
}

auto AnimationView::GetAnimationBoundingBox(bool zoomInOnMarchers) const -> std::pair<CalChart::Coord, CalChart::Coord>
{
    return mView->GetAnimationBoundingBox(zoomInOnMarchers, mCurrentBeat);
}

void AnimationView::UnselectAll()
{
    mView->UnselectAll();
}

void AnimationView::SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown)
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

// Keystroke command toggles the timer, but the timer
// lives in the Frame.  So we have this weird path...
void AnimationView::ToggleTimer()
{
    GetAnimationFrame()->ToggleTimer();
}

auto AnimationView::OnBeat() const -> bool
{
    return GetAnimationFrame()->OnBeat();
}

auto AnimationView::GetAnimationFrame() const -> AnimationPanel const*
{
    return static_cast<AnimationPanel const*>(GetFrame());
}

auto AnimationView::GetAnimationFrame() -> AnimationPanel*
{
    return static_cast<AnimationPanel*>(GetFrame());
}

auto AnimationView::GetShowFieldSize() const -> CalChart::Coord
{
    return mView->GetShowFieldSize();
}

auto AnimationView::GetMarcherInfo(int which) const -> std::optional<CalChart::Animate::Info>
{
    return mView->GetAnimationInfo(mCurrentBeat, which);
}

auto AnimationView::GetMarchersByDistance(float fromX, float fromY) const -> std::multimap<double, CalChart::Animate::Info>
{
    return mView->GetSelectedAnimationInfoWithDistanceFromPoint(mCurrentBeat, CalChart::Coord(fromX, fromY));
}
#endif
