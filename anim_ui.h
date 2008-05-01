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

  void OnPaint();
  void OnEvent(wxMouseEvent& event);
  void OnChar(wxKeyEvent& event);

  inline unsigned GetTempo() { return tempo; }
  void SetTempo(unsigned t);

  inline void Redraw() { RedrawBuffer(); OnPaint(); }
  void RedrawBuffer();
  void UpdateText();
  void Refresh();
  void Generate();
  void FreeAnim();

  // Select colliding points and redraw
  void SelectCollisions();

  // true if changes made
  inline bool PrevBeat() {
    if (anim) { if (anim->PrevBeat()) { Refresh(); return true; } }
    return false;
  }
  inline bool NextBeat() {
    if (anim) { if (anim->NextBeat()) { Refresh(); return true; } }
    return false;
  }
  inline void GotoBeat(unsigned i) {
    if (anim) { anim->GotoBeat(i); Refresh(); }
  }
  inline bool PrevSheet() {
    if (anim) { if (anim->PrevSheet()) { Refresh(); return true; } }
    return false;
  }
  inline bool NextSheet() {
    if (anim) { if (anim->NextSheet()) { Refresh(); return true; } }
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
  const char * GeneratePOVFiles(const char *filebasename);
#endif
#ifdef ANIM_OUTPUT_RIB
  const char * GenerateRIBFrame();
  const char * GenerateRIBFile(const char *filename, bool all = true);
#endif

  Animation* anim;
  AnimationTimer* timer;
  bool timeron;
private:
  CC_descr *show_descr;
  AnimationFrame *ourframe;
  unsigned tempo;
};

class AnimationSlider: public wxSlider {
public:
  AnimationSlider(wxPanel *parent, wxWindowID id,
		  int value, int min_value, int max_value, int width):
  wxSlider(parent, id, value, min_value, max_value),
  canvas(NULL) {}

  AnimationCanvas *canvas;
};

class AnimationFrame: public wxFrameWithStuffSized {
public:
  AnimationFrame(wxFrame *frame, CC_descr *dcr, CC_WinList *lst);
  ~AnimationFrame();

  void OnMenuCommand(int id);
  void OnMenuSelect(int id);

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
};

enum {
  CALCHART__ANIM_REANIMATE = 100,
  CALCHART__ANIM_SELECT_COLL,
#ifdef ANIM_OUTPUT_POVRAY
  CALCHART__ANIM_POVRAY,
#endif
#ifdef ANIM_OUTPUT_RIB
  CALCHART__ANIM_RIB_FRAME,
  CALCHART__ANIM_RIB,
#endif
  CALCHART__ANIM_CLOSE
};

class AnimErrorList: public wxFrame {
public:
  AnimErrorList(AnimateCompile *comp, CC_WinList *lst, unsigned num,
		wxFrame *frame, const wxChar *title,
		int x = -1, int y = -1, int width = 300, int height = 300);
  ~AnimErrorList();
  bool OnClose(void);
  void OnSize(int w, int h);

  inline bool Okay() { return ok; };

  void Unselect();
  void Update();

  CC_show *show;
private:
  bool ok;
  unsigned sheetnum;
  wxPanel *panel;
  GoodListBox *list;
  ErrorMarker pointsels[NUM_ANIMERR];
  CC_WinNodeAnimErrors *node;
};

#endif
