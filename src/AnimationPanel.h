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

#include <wx/docview.h>
#include <wx/wx.h>
#include <wxUI/wxUI.h>

class AnimationView;
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

// Animiation Panel is a little bit different because it has it's own view.
// When we are animating a show we are taking an CalChart animation object
// and having a copy that animates on it's own.  So AnimiationView handles
// that complexity of updating state of this panel.

class AnimationPanel : public wxPanel {
    using super = wxPanel;
    wxDECLARE_EVENT_TABLE();

public:
    AnimationPanel(CalChart::Configuration& config, wxWindow* parent, wxWindowID winid = wxID_ANY, wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER, wxString const& name = wxPanelNameStr);
    ~AnimationPanel() override;

    void OnUpdate(); // Refresh from the View
    void SetView(CalChartView* view);

    // Called by the view
    void SetPlayState(bool playState);
    void ToggleTimer();
    void UpdatePanel(); // specfically to update the controls
    bool OnBeat() const;
    auto TimerOn() const { return mTimerOn; }

    auto GetInMiniMode() const { return mInMiniMode; }
    void SetInMiniMode(bool);

private:
    void Init();
    void CreateControls();

    // Event Handlers
    void OnCmd_anim_next_beat_timer(wxTimerEvent& event);
    void OnSlider_anim_tempo(wxSpinEvent& event);
    void OnSlider_anim_gotobeat(wxScrollEvent& event);
    void OnCmd_PlayButton();
    void OnCmd_ToggleAnimOmni();

    // Internals
    // timer stuff:
    void StartTimer();
    void StopTimer();
    auto GetTempo() const { return mTempo; }
    void SetTempo(unsigned tempo) { mTempo = tempo; }

    AnimationView* mView{};
    AnimationCanvas* mCanvas{};
    CCOmniviewCanvas* mOmniCanvas{};
    wxUI::Text::Proxy mTempoLabel{};
    wxUI::SpinCtrl::Proxy mTempoCtrl{};
    wxUI::Slider::Proxy mBeatSlider{};
    wxUI::CheckBox::Proxy mSpritesCheckbox{};
    wxUI::CheckBox::Proxy mZoomCheckbox{};
    wxUI::CheckBox::Proxy mCollisionCheckbox{};
    wxUI::BitmapToggleButton::Proxy mPlayPauseButton{};
    wxUI::Button::Proxy mAnimateOmniToggle{};
    wxUI::Button::Proxy mOmniHelpButton{};
    wxTimer* mTimer{};
    unsigned mTempo{};
    bool mTimerOn{};
    bool mInMiniMode{};
    bool mShowOmni{};
    std::vector<wxWindow*> mItemsToHide;
    CalChart::Configuration& mConfig;
};
