/* anim_ui.h
 * Header for animation user interface
 *
 * Modification history:
 * 1-4-95     Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1995-2008  Garrick Brian Meeker

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

#ifndef _ANIM_UI_H_
#define _ANIM_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "basic_ui.h"
#include "show.h"
#include "animate.h"
#include <wx/timer.h>
#define DEFAULT_ANIM_SIZE 3

class AnimationCanvas;
class AnimationFrame;

class CC_WinNodeAnim : public CC_WinNode {
public:
  CC_WinNodeAnim(CC_WinList *lst, AnimationFrame *frm);

  virtual void SetShow(CC_show *shw);
  virtual void UpdateSelections(wxWindow* win = NULL, int point = -1);
  virtual void ChangeNumPoints(wxWindow *win);

private:
  AnimationFrame *frame;
};

class AnimErrorList;

class CC_WinNodeAnimErrors : public CC_WinNode {
public:
  CC_WinNodeAnimErrors(CC_WinList *lst, AnimErrorList *err);

  virtual void SetShow(CC_show *shw);
  virtual void UpdateSelections(wxWindow* win = NULL, int point = -1);
  virtual void ChangeNumPoints(wxWindow *win);

private:
  AnimErrorList *errlist;
};

class AnimationTimer: public wxTimer {
public:
  AnimationTimer(AnimationCanvas* c): canvas(c) {}

  void Notify();
private:
  AnimationCanvas* canvas;
};

class AnimationFrame;
class AnimationCanvas: public AutoScrollCanvas {
public:
  AnimationCanvas(AnimationFrame *frame, CC_descr *dcr);
  ~AnimationCanvas();

  void OnEraseBackground(wxEraseEvent& event);
  void OnPaint(wxPaintEvent& event);
  void OnLeftMouseEvent(wxMouseEvent& event);
  void OnRightMouseEvent(wxMouseEvent& event);
  void OnChar(wxKeyEvent& event);

  inline unsigned GetTempo() { return tempo; }
  void SetTempo(unsigned t);

  inline void Redraw() { RedrawBuffer(); Refresh(); }
  void RedrawBuffer();
  void UpdateText();
  void RefreshCanvas();
  void Generate();
  void FreeAnim();

  // Select colliding points and redraw
  void SelectCollisions();

  // true if changes made
  inline bool PrevBeat() {
    if (anim) { if (anim->PrevBeat()) { RefreshCanvas(); return true; } }
    return false;
  }
  inline bool NextBeat() {
    if (anim) { if (anim->NextBeat()) { RefreshCanvas(); return true; } }
    return false;
  }
  inline void GotoBeat(unsigned i) {
    if (anim) { anim->GotoBeat(i); RefreshCanvas(); }
  }
  inline bool PrevSheet() {
    if (anim) { if (anim->PrevSheet()) { RefreshCanvas(); return true; } }
    return false;
  }
  inline bool NextSheet() {
    if (anim) { if (anim->NextSheet()) { RefreshCanvas(); return true; } }
    return false;
  }
  inline void GotoSheet(unsigned i) {
    if (anim) { anim->GotoSheet(i); Refresh(); }
  }

  void StartTimer();
  inline void StopTimer() {
    timer->Stop(); timeron = false;
  }
#ifdef ANIM_OUTPUT_POVRAY
  wxString GeneratePOVFiles(const wxString& filebasename);
#endif
#ifdef ANIM_OUTPUT_RIB
  wxString GenerateRIBFrame();
  wxString GenerateRIBFile(const wxString& filename, bool all = true);
#endif

  Animation* anim;
  AnimationTimer* timer;
  bool timeron;
private:
  CC_descr *show_descr;
  AnimationFrame *ourframe;
  unsigned tempo;

  DECLARE_EVENT_TABLE()
};

class AnimationFrame: public wxFrame {
public:
  AnimationFrame(wxFrame *frame, CC_descr *dcr, CC_WinList *lst);
  ~AnimationFrame();

  void OnCmdReanimate(wxCommandEvent& event);
  void OnCmdSelectCollisions(wxCommandEvent& event);
#ifdef ANIM_OUTPUT_POVRAY
  void OnCmdPOV(wxCommandEvent& event);
#endif
#ifdef ANIM_OUTPUT_RIB
  void OnCmdRIBFrame(wxCommandEvent& event);
  void OnCmdRIBAll(wxCommandEvent& event);
  void OnCmdRIB(wxCommandEvent& event, bool allframes);
#endif
  void OnCmdClose(wxCommandEvent& event);

  void OnCmd_anim_stop(wxCommandEvent& event);
  void OnCmd_anim_play(wxCommandEvent& event);
  void OnCmd_anim_prev_beat(wxCommandEvent& event);
  void OnCmd_anim_next_beat(wxCommandEvent& event);
  void OnCmd_anim_prev_sheet(wxCommandEvent& event);
  void OnCmd_anim_next_sheet(wxCommandEvent& event);
  void OnCmd_anim_collisions(wxCommandEvent& event);
  void OnSlider_anim_tempo(wxScrollEvent& event);
  void OnSlider_anim_gotosheet(wxScrollEvent& event);
  void OnSlider_anim_gotobeat(wxScrollEvent& event);

  void UpdatePanel();

  inline CollisionWarning CollisionType() {
    return (CollisionWarning)collis->GetSelection(); }

  AnimationCanvas *canvas;
private:
  CC_WinNodeAnim *node;
  wxChoice *collis;
  wxSlider *sheet_slider;
  wxSlider *beat_slider;

  friend class AnimationCanvas;

  DECLARE_EVENT_TABLE()
};

enum {
  CALCHART__ANIM_REANIMATE = 1,
  CALCHART__ANIM_SELECT_COLL,
#ifdef ANIM_OUTPUT_POVRAY
  CALCHART__ANIM_POVRAY,
#endif
#ifdef ANIM_OUTPUT_RIB
  CALCHART__ANIM_RIB_FRAME,
  CALCHART__ANIM_RIB,
#endif

  CALCHART__anim_stop,
  CALCHART__anim_play,
  CALCHART__anim_prev_beat,
  CALCHART__anim_next_beat,
  CALCHART__anim_prev_sheet,
  CALCHART__anim_next_sheet,
  CALCHART__anim_collisions,
  CALCHART__anim_tempo,
  CALCHART__anim_gotosheet,
  CALCHART__anim_gotobeat,

  CALCHART__anim_update
};

class AnimErrorList: public wxFrame {
public:
  AnimErrorList(AnimateCompile *comp, CC_WinList *lst, unsigned num,
		wxFrame *frame, const wxString& title,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(300, 300));
  ~AnimErrorList();
  void OnCloseWindow(wxCloseEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnCmdClose(wxCommandEvent& event);
  void OnCmdUpdate(wxCommandEvent& event);

  inline bool Okay() { return ok; };

  void Unselect();
  void Update();
  void Update(int i);

  CC_show *show;
private:
  bool ok;
  unsigned sheetnum;
  wxListBox *list;
  ErrorMarker pointsels[NUM_ANIMERR];
  CC_WinNodeAnimErrors *node;

  DECLARE_EVENT_TABLE()
};

#endif
