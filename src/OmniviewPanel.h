#pragma once
/*
 * animation_frame.h
 * Header for AnimationFrame
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

#include "AnimationView.h"
#include "animate_types.h"

#include <wx/docview.h>
#include <wx/wx.h>

#include <functional>
#include <map>

class AnimationView;
class AnimationCanvas;
class CCOmniView_Canvas;
class FancyTextWin;
class wxSplitterWindow;
class CalChartConfiguration;

class AnimationFrame : public wxFrame {
public:
    using super = wxFrame;

    AnimationFrame(std::function<void()> onClose, wxDocument* doc,
        CalChartConfiguration& config, wxView* view, wxFrame* parent,
        const wxSize& size);
    ~AnimationFrame();

    void OnSize(wxSizeEvent& event);

    void OnCmdReanimate(wxCommandEvent& event);
    void OnCmdSelectCollisions(wxCommandEvent& event);
    void OnCmdClose(wxCommandEvent& event) { Close(); }
    void OnCmdClose(wxCloseEvent& event);

    void OnCmd_anim_stop(wxCommandEvent& event) { StopTimer(); }
    void OnCmd_anim_play(wxCommandEvent& event) { StartTimer(); }
    void OnCmd_anim_prev_beat(wxCommandEvent& event) { mAnimationView.PrevBeat(); }
    void OnCmd_anim_next_beat(wxCommandEvent& event) { mAnimationView.NextBeat(); }
    void OnCmd_anim_next_beat_timer(wxTimerEvent& event);
    void OnCmd_anim_prev_sheet(wxCommandEvent& event) { mAnimationView.PrevSheet(); }
    void OnCmd_anim_next_sheet(wxCommandEvent& event) { mAnimationView.NextSheet(); }
    void OnCmd_anim_collisions(wxCommandEvent& event);
    void OnCmd_anim_errors(wxCommandEvent& event);
    void OnSlider_anim_tempo(wxSpinEvent& event);
    void OnSlider_anim_gotosheet(wxScrollEvent& event);
    void OnSlider_anim_gotobeat(wxScrollEvent& event);

    void OnCmd_FollowMarcher(wxCommandEvent& event);
    void OnCmd_SaveCameraAngle(wxCommandEvent& event);
    void OnCmd_GoToCameraAngle(wxCommandEvent& event);
    void OnCmd_ShowKeyboardControls(wxCommandEvent& event);
    void OnCmd_ToggleCrowd(wxCommandEvent& event);
    void OnCmd_ToggleMarching(wxCommandEvent& event);
    void OnCmd_ToggleShowOnlySelected(wxCommandEvent& event);

    // Called by the view
    void ToggleTimer();
    void UpdatePanel();

    bool OnBeat() const;

    void
    OnNotifyErrorList(const std::map<CalChart::AnimateError, CalChart::ErrorMarker>& error_markers,
        unsigned sheetnum, const wxString& message);

    // controlling how the screen splits between views
    void OnCmd_SplitViewHorizontal(wxCommandEvent& event);
    void OnCmd_SplitViewVertical(wxCommandEvent& event);
    void OnCmd_SplitViewUnsplit(wxCommandEvent& event);
    void OnCmd_SwapAnimateAndOmni(wxCommandEvent& event);

    // update UI when these happen
    void OnCmd_UpdateUIHorizontal(wxUpdateUIEvent& event);
    void OnCmd_UpdateUIVertical(wxUpdateUIEvent& event);
    void OnCmd_UpdateUIUnsplit(wxUpdateUIEvent& event);

private:
    bool OnSlider_isNextBeatEvent(wxScrollEvent& event);
    bool OnSlider_shouldTransitionToNextSheet(wxScrollEvent& event);
    bool OnSlider_isPreviousBeatEvent(wxScrollEvent& event);
    bool OnSlider_shouldTransitionToPreviousSheet(wxScrollEvent& event);
    void TransitionToPreviousSheet();
    void TransitionToNextSheet();

    AnimationView mAnimationView;
    CalChartConfiguration& config;
    // we really do need one of each.  We can't do inheritance because they have
    // different base classes
    AnimationCanvas* mCanvas;
    CCOmniView_Canvas* mOmniViewCanvas;
    wxSlider* mSheetSlider;
    wxSlider* mBeatSlider;

    wxSplitterWindow* mSplitter;
    // these are just the observers we use to manipulate the split
    wxWindow* mSplitA;
    wxWindow* mSplitB;

    // continuity errors:
    wxChoice* mErrorList;
    std::vector<std::pair<CalChart::ErrorMarker, unsigned>> mErrorMarkers;
    FancyTextWin* mErrorText;

    // timer stuff:
    void StartTimer();
    void StopTimer();
    auto GetTempo() const { return mTempo; }
    void SetTempo(unsigned tempo) { mTempo = tempo; }
    wxTimer* mTimer;
    unsigned mTempo;
    bool mTimerOn;

    // when we go, let people know
    std::function<void()> mWhenClosed;
    wxDECLARE_EVENT_TABLE();
};
