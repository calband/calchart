/* anim_ui.h
 * Header for animation user interface
 *
 * Modification history:
 * 1-4-95     Garrick Meeker              Created
 *
 */

#ifndef _ANIM_UI_H_
#define _ANIM_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "basic_ui.h"
#include "show.h"
#include "animate.h"
#include <wx_timer.h>
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

class AnimationTimer: public wxTimer {
public:
  AnimationTimer(AnimationCanvas* c): canvas(c) {}

  void Notify();
private:
  AnimationCanvas* canvas;
};

class AnimationCanvas: public wxCanvas {
public:
  AnimationCanvas(wxFrame *frame, CC_descr *dcr);
  ~AnimationCanvas();

  void OnPaint();
  void OnEvent(wxMouseEvent& event);
  void OnChar(wxKeyEvent& event);

  inline unsigned GetTempo() { return tempo; }
  void SetTempo(unsigned t);

  void UpdateText();
  inline void Refresh() { OnPaint(); UpdateText(); }
  void Generate();
  void FreeAnim();

  // TRUE if changes made
  inline Bool PrevBeat() {
    if (anim) { if (anim->PrevBeat()) { Refresh(); return TRUE; } }
    return FALSE;
  }
  inline Bool NextBeat() {
    if (anim) { if (anim->NextBeat()) { Refresh(); return TRUE; } }
    return FALSE;
  }
  inline Bool PrevSheet() {
    if (anim) { if (anim->PrevSheet()) { Refresh(); return TRUE; } }
    return FALSE;
  }
  inline Bool NextSheet() {
    if (anim) { if (anim->NextSheet()) { Refresh(); return TRUE; } }
    return FALSE;
  }

  Animation* anim;
  AnimationTimer* timer;
private:
  CC_descr *show_descr;
  wxFrame *ourframe;
  unsigned tempo;
};

class AnimationSlider: public wxSlider {
public:
  AnimationSlider(wxPanel *parent, wxFunction func, char *label,
		  int value, int min_value, int max_value, int width):
  wxSlider(parent, func, label, value, min_value, max_value, width),
  canvas(NULL) {}

  AnimationCanvas *canvas;
};

class AnimationFrame: public wxFrameWithStuffSized {
public:
  AnimationFrame(wxFrame *frame, CC_descr *dcr, CC_WinList *lst);
  ~AnimationFrame();

  void OnMenuCommand(int id);
  void OnMenuSelect(int id);

  AnimationCanvas *canvas;
private:
  CC_WinNodeAnim *node;
};

enum {
  CALCHART__ANIM_REANIMATE = 100,
  CALCHART__ANIM_CLOSE
};

#endif
