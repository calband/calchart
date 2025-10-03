#pragma once
/*
 * AnimationPanel.h
 * Header for AnimationPanel
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

#include "AnimationSprites.hpp"
#include "CalChartDrawCommand.h"
#include <wx/wx.h>
#include <wxUI/wxUI.hpp>

class AnimationCanvas;
class CalChartView;
class wxBitmapToggleButton;
class wxStaticText;
class wxSpinCtrl;
class wxButton;
class CCOmniviewCanvas;

namespace CalChart {
class Configuration;
}

class AnimationPanel : public wxPanel {
    using super = wxPanel;
    wxDECLARE_EVENT_TABLE();

public:
    AnimationPanel(
        CalChart::Configuration& config,
        wxWindow* parent,
        bool miniMode,
        wxWindowID winid = wxID_ANY,
        wxPoint const& pos = wxDefaultPosition,
        wxSize const& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        wxString const& name = wxPanelNameStr);
    ~AnimationPanel() override;

    AnimationPanel(AnimationPanel const&) = delete;
    AnimationPanel(AnimationPanel&&) = delete;
    auto operator=(AnimationPanel&) -> AnimationPanel& = delete;
    auto operator=(AnimationPanel&&) -> AnimationPanel& = delete;

    void OnUpdate(); // Refresh from the View
    void UpdatePanel(); // specfically to update the controls
    void SetView(CalChartView* view);

    void SetPlayState(bool playState);
    void ToggleTimer();
    auto OnBeat() const -> bool;
    auto TimerOn() const { return mTimerOn; }
    void UnselectAll();
    void SelectMarchersInBox(wxPoint const& mouseStart, wxPoint const& mouseEnd, bool altDown);

    // info
    [[nodiscard]] auto GetTotalNumberBeats() const -> CalChart::Beats;
    [[nodiscard]] auto GetNumSheets() const -> size_t;
    [[nodiscard]] auto BeatToSheetOffsetAndBeat(CalChart::Beats beat) const -> std::optional<std::tuple<size_t, CalChart::Beats>>;
    [[nodiscard]] auto BeatForSheet(int sheet) const -> CalChart::Beats;
    [[nodiscard]] auto GetSheetBeatSheetFromTotalCurrentBeat() const -> CalChart::Beats;

    [[nodiscard]] auto GetAnimationBoundingBox(bool zoomInOnMarchers) const -> std::pair<CalChart::Coord, CalChart::Coord>;
    [[nodiscard]] auto GetShowFieldSize() const -> CalChart::Coord;
    [[nodiscard]] auto GetMarcherInfo(int which) const -> std::optional<CalChart::Animate::Info>;
    [[nodiscard]] auto GetMarchersByDistance(float fromX, float fromY) const -> std::multimap<double, CalChart::Animate::Info>;

    void PrevBeat();
    void NextBeat();
    void GotoTotalBeat(CalChart::Beats whichBeat);
    void GotoSheetBeat(int whichSheet, CalChart::Beats whichBeat);
    [[nodiscard]] auto AtEndOfShow() const -> bool;

    [[nodiscard]] auto GenerateDrawCommands() -> std::vector<CalChart::Draw::DrawCommand>;

private:
    void Init();
    void CreateControls();

    // Event Handlers
    void OnCmd_anim_next_beat_timer(wxTimerEvent& event);
    void OnSlider_anim_tempo(wxSpinEvent& event);
    void OnSlider_anim_gotobeat(wxScrollEvent& event);
    void OnSlider_anim_gotosheet(wxScrollEvent& event);
    void OnCmd_PlayButton();
    void OnCmd_ToggleAnimOmni();

    // Internals
    // timer stuff:
    void StartTimer();
    void StopTimer();
    auto GetTempo() const { return mTempo; }
    void SetTempo(unsigned tempo) { mTempo = tempo; }

    CalChartView* mView{};
    AnimationCanvas* mCanvas{};
    CCOmniviewCanvas* mOmniCanvas{};
    wxUI::Text::Proxy mTempoLabel{};
    wxUI::SpinCtrl::Proxy mTempoCtrl{};
    wxUI::Slider::Proxy mBeatSlider{};
    wxUI::Slider::Proxy mSheetSlider{};
    wxUI::CheckBox::Proxy mSpritesCheckbox{};
    wxUI::CheckBox::Proxy mZoomCheckbox{};
    wxUI::CheckBox::Proxy mCollisionCheckbox{};
    wxUI::BitmapToggleButton::Proxy mPlayPauseButton{};
    wxUI::Button::Proxy mAnimateOmniToggle{};
    wxUI::Button::Proxy mOmniHelpButton{};
    std::vector<wxWindow*> mItemsToHide;

    wxTimer* mTimer{};
    unsigned mTempo{};
    bool mTimerOn{};
    const bool mInMiniMode{};
    bool mShowOmni{};

    CalChart::Beats mCurrentBeat{};
    bool mDrawCollisionWarning = true;
    bool mPlayCollisionWarning = false;

    CalChart::Configuration& mConfig;
    AnimationSprites mSprites;
};
