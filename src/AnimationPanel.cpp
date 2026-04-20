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
#include "tb_music.xbm"
#include "tb_music_mute.xbm"
#include "tb_play.xbm"
#include "tb_stop.xbm"
#include "ui_enums.h"

#include <wx/artprov.h>
#include <wx/filedlg.h>
#ifndef __WXGTK__
#include <wx/mediactrl.h>
#endif
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/stdpaths.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/wfstream.h>
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
#ifndef __WXGTK__
    , mMediaCtrl(new wxMediaCtrl(this, wxID_ANY))
#endif
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
#ifndef __WXGTK__
    // Hide the media control (we only use it for playback, not display)
    mMediaCtrl->Hide();
#endif
}

void AnimationPanel::CreateControls()
{
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
#ifndef __WXGTK__
            wxUI::BitmapToggleButton{ ScaleButtonBitmap(wxBitmap{ BITMAP_NAME(tb_music_mute) }), ScaleButtonBitmap(wxBitmap{ BITMAP_NAME(tb_music) }) }
                .bind([this] {
                    OnCmd_MusicButton();
                })
                .withProxy(mMusicButton),
#endif
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
                wxUI::Text{ std::to_string(mTempo) }.withProxy(mTempoValue),
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

#ifndef __WXGTK__
    *mMusicButton = !mMusicMuted; // Starts in muted state (false)
#endif

    mItemsToHide.push_back(mAnimateOmniToggle.control());
    mItemsToHide.push_back(mOmniHelpButton.control());
    mItemsToHide.push_back(mSpritesCheckbox.control());
    mItemsToHide.push_back(mZoomCheckbox.control());
    mItemsToHide.push_back(mCollisionCheckbox.control());
    mItemsToHide.push_back(mTempoLabel.control());
    mItemsToHide.push_back(mTempoValue.control());

    *mCollisionCheckbox = mDrawCollisionWarning;

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
    // Timer fired - advance to next beat
    NextBeat();
    if (AtEndOfShow()) {
        StopTimer();
    } else {
        // Schedule the next beat
        StartTimer();
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
    SetPlayState(!mTimerOn);
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
    auto beatsForCurrentSheet = BeatsForSheet(currentSheet) - 1; // gives us which shee we are on and beat

    if (mSheetSlider.control()) {
        updateController(mBeatSlider, static_cast<int>(beatsForCurrentSheet), static_cast<int>(sheetBeat));
    } else {
        updateController(mBeatSlider, static_cast<int>(totalBeats), static_cast<int>(currentBeat));
    }
    updateController(mSheetSlider, totalSheets + 1, currentSheet + 1, false);
    *mPlayPauseButton = mTimerOn;

    // Update tempo display
    if (mView && mTempoValue.control()) {
        auto currentTempo = mView->GetTempoForAnimationBeat(mCurrentBeat);
        *mTempoValue = std::to_string(currentTempo);
        mTempo = currentTempo;
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
    return mView->AnimationBeatToSheetOffsetAndBeat(beat);
}

auto AnimationPanel::BeatsForSheet(int sheet) const -> CalChart::Beats
{
    if (!mView) {
        return 0;
    }
    return mView->AnimationBeatsForSheet(sheet);
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
    GotoTotalBeat(mCurrentBeat - 1);
}

void AnimationPanel::NextBeat()
{
    if (!mView) {
        return;
    }
    auto totalBeats = mView->GetTotalNumberAnimationBeats();
    if (!totalBeats) {
        return;
    }
    if (mCurrentBeat >= (*totalBeats - 1)) {
        return;
    }
    GotoTotalBeat(mCurrentBeat + 1);
}

void AnimationPanel::GotoTotalBeat(CalChart::Beats whichBeat)
{
    mCurrentBeat = whichBeat;

    // If playing, adjust anchor time to maintain sync
    if (!mTimerOn) {
        UpdatePanel();
        return;
    }

    if (!mView) {
        UpdatePanel();
        return;
    }

    auto downbeatTimes = mView->GetDownbeatTimes();
    if (whichBeat >= downbeatTimes.size()) {
        UpdatePanel();
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto currentBeatTime = std::chrono::duration<float>(downbeatTimes[whichBeat]);
    mAnchorTime = now - std::chrono::duration_cast<std::chrono::steady_clock::duration>(currentBeatTime);

    UpdatePanel();
#ifndef __WXGTK__
    // Sync audio to new position
    if (mTimerOn && !mMusicMuted && !mMusicFilePath.IsEmpty()) {
        UpdateMedia();
    }
#endif
}

void AnimationPanel::GotoSheetBeat(int whichSheet, CalChart::Beats whichBeat)
{
    GotoTotalBeat(whichBeat + mView->GetTotalNumberAnimationBeatsUpTo(whichSheet));
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
    UpdatePanel();
}

void AnimationPanel::StartTimer()
{
    if (!mView) {
        mTimerOn = false;
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto downbeatTimes = mView->GetDownbeatTimes();

    // If no anchor time, set it based on current beat
    if (!mAnchorTime.has_value()) {
        // Calculate when beat 0 would have occurred
        if (mCurrentBeat < downbeatTimes.size()) {
            auto currentBeatTime = std::chrono::duration<float>(downbeatTimes[mCurrentBeat]);
            mAnchorTime = now - std::chrono::duration_cast<std::chrono::steady_clock::duration>(currentBeatTime);
        } else {
            mAnchorTime = now;
        }
    }

    // Calculate next beat index
    auto nextBeat = mCurrentBeat + 1;

    // Check if we have a next beat
    if (nextBeat >= downbeatTimes.size()) {
        mTimerOn = false;
        return;
    }

    // Calculate when the next beat should occur
    auto nextBeatTime = std::chrono::duration<float>(downbeatTimes[nextBeat]);
    auto nextBeatTimePoint = *mAnchorTime + std::chrono::duration_cast<std::chrono::steady_clock::duration>(nextBeatTime);

    // Calculate delay from now
    auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(nextBeatTimePoint - now);

    // Ensure delay is at least 1ms (avoid negative or zero delays)
    if (delay.count() < 1) {
        delay = std::chrono::milliseconds(1);
    }

    // Start one-shot timer
    if (!mTimer->Start(static_cast<int>(delay.count()), wxTIMER_ONE_SHOT)) {
        mTimerOn = false;
#ifndef __WXGTK__
        // Start audio if not muted and music is loaded
        if (!mMusicMuted && !mMusicFilePath.IsEmpty()) {
            UpdateMedia();
        }
#endif
        return;
    }
    mTimerOn = true;
#ifndef __WXGTK__
    // Start audio if not muted and music is loaded
    if (!mMusicMuted && !mMusicFilePath.IsEmpty()) {
        UpdateMedia();
    }
#endif
}

void AnimationPanel::StopTimer()
{
    mTimer->Stop();
    mTimerOn = false;
    mAnchorTime = std::nullopt;
#ifndef __WXGTK__
    // Stop audio as well
    mMediaCtrl->Stop();
#endif
}

void AnimationPanel::OnUpdate()
{
    if (!mView) {
        return;
    }
#ifndef __WXGTK__
    LoadMusicFileFromShow();
#endif
    GotoTotalBeat(mView->GetAnimationBeatForCurrentSheet());
}

void AnimationPanel::SetView(CalChartView* view)
{
    mView = view;
#ifndef __WXGTK__
    LoadMusicFileFromShow();
#endif
}

// Audio control methods

void AnimationPanel::OnCmd_MusicButton()
{
#ifndef __WXGTK__
    if (mMusicFilePath.IsEmpty()) {
        // No music loaded - open file dialog
        LoadMusicFile();
    } else {
        // Toggle mute state
        mMusicMuted = !mMusicMuted;
        if (mMusicMuted) {
            mMediaCtrl->Stop();
        } else {
            // Sync and play if animation is playing
            if (mTimerOn) {
                UpdateMedia();
            }
        }
    }
#endif
}

void AnimationPanel::LoadMusicFileFromShow()
{
#ifndef __WXGTK__
    // what we do is we tell the view to load the media, then we get the media vector of bytes.  Then we write that to a tmp file a the media file path.
    if (!mView) {
        return;
    }
    auto currentMediaVersion = mView->GetMediaVersion();
    if (currentMediaVersion == mMediaVersion) {
        // No change in media, no need to reload
        return;
    }
    mMediaVersion = currentMediaVersion;

    auto [mediaBytes, extension] = mView->GetMedia();
    if (mediaBytes.empty()) {
        return;
    }

    // Write to temp file
    auto tempDir = wxStandardPaths::Get().GetTempDir();
    auto tempFilePath = tempDir + "/calchart_temp_music_file" + extension;
    wxLogDebug("Writing music bytes to temp file: %s", tempFilePath);
    wxFileOutputStream output(tempFilePath);
    if (!output.IsOk()) {
        wxMessageBox("Failed to create temporary file for music", "Error", wxOK | wxICON_ERROR);
        return;
    }
    output.Write(mediaBytes.data(), mediaBytes.size());
    if (output.GetLastError() != wxSTREAM_NO_ERROR) {
        wxMessageBox("Failed to write music data to temporary file", "Error", wxOK | wxICON_ERROR);
        return;
    }
    mMusicFilePath = tempFilePath;

    if (!mMediaCtrl->Load(mMusicFilePath)) {
        wxMessageBox("Failed to load music file", "Error", wxOK | wxICON_ERROR);
        mMusicFilePath = "";
        return;
    }
#endif
}

void AnimationPanel::LoadMusicFile()
{
#ifndef __WXGTK__
    if (!mView) {
        return;
    }
    mView->OnSetMedia();
    LoadMusicFileFromShow();
    if (mMusicFilePath.IsEmpty()) {
        return;
    }

    // Successfully loaded - unmute and sync
    mMusicMuted = false;
    if (mMusicButton.control()) {
        mMusicButton.control()->SetValue(true); // Show as unmuted
    }
    if (mTimerOn) {
        UpdateMedia();
    }
#endif
}

void AnimationPanel::UpdateMedia()
{
#ifndef __WXGTK__
    if (mMusicFilePath.IsEmpty() || mMusicMuted) {
        return;
    }

    auto positionMs = CalculateAudioPositionMs();
    auto position = mMediaCtrl->Tell();
    if (auto offset = abs(position - positionMs);
        offset > 500) {
        wxLogDebug("Audio offsync by %ld ms, seeking to %ld ms", offset, position);
        mMediaCtrl->Seek(positionMs);
    }
    mMediaCtrl->Play();
#endif
}

auto AnimationPanel::CalculateAudioPositionMs() const -> long
{
#ifndef __WXGTK__
    if (!mView) {
        return 0;
    }

    auto downbeatTimes = mView->GetDownbeatTimes();
    if (mCurrentBeat >= downbeatTimes.size()) {
        return 0;
    }

    return static_cast<long>(downbeatTimes[mCurrentBeat].count() * 1000.0f);
#else
    return 0;
#endif
}
