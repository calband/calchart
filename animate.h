/* animate.h
 * Classes for animating shows
 *
 * Modification history:
 * 12-29-95   Garrick Meeker              Created
 *
 */

#ifndef _ANIMATE_H_
#define _ANIMATE_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "show.h"

// Number of variables in continuity language (A B C D X Y Z DOF)
#define NUMCONTVARS 8

enum AnimateDir {
  ANIMDIR_N, ANIMDIR_NE, ANIMDIR_E, ANIMDIR_SE,
  ANIMDIR_S, ANIMDIR_SW, ANIMDIR_W, ANIMDIR_NW
};

enum AnimateError {
  ANIMERR_OUTOFTIME,
  ANIMERR_EXTRATIME,
  ANIMERR_WRONGPLACE,
  ANIMERR_INVALID_CM,
  ANIMERR_INVALID_FNTN,
  NUM_ANIMERR
};

extern const char *animate_err_msgs[];

AnimateDir AnimGetDirFromVector(CC_coord& vector);
AnimateDir AnimGetDirFromAngle(float ang);

class AnimatePoint {
public:
  CC_coord pos;
  int dir;
};

class AnimateCommand {
public:
  AnimateCommand(unsigned beats);

  // returns FALSE if end of command
  virtual Bool Begin(AnimatePoint& pt);
  virtual Bool End(AnimatePoint& pt);
  virtual Bool NextBeat(AnimatePoint& pt);
  virtual Bool PrevBeat(AnimatePoint& pt);

  // go through all beats at once
  virtual void ApplyForward(AnimatePoint& pt);
  virtual void ApplyBackward(AnimatePoint& pt);

  virtual AnimateDir Direction() = 0;
  virtual void ClipBeats(unsigned beats);

  AnimateCommand *next, *prev;
  unsigned numbeats;
protected:
  unsigned beat;
};

class AnimateCommandMT : public AnimateCommand {
public:
  AnimateCommandMT(unsigned beats, AnimateDir direction);

  virtual AnimateDir Direction();
protected:
  AnimateDir dir;
};

class AnimateCommandMove : public AnimateCommandMT {
public:
  AnimateCommandMove(unsigned beats, CC_coord movement);

  virtual Bool Begin(AnimatePoint& pt);
  virtual Bool End(AnimatePoint& pt);
  virtual Bool NextBeat(AnimatePoint& pt);
  virtual Bool PrevBeat(AnimatePoint& pt);

  virtual void ApplyForward(AnimatePoint& pt);
  virtual void ApplyBackward(AnimatePoint& pt);

  virtual void ClipBeats(unsigned beats);
private:
  CC_coord vector;
};

class AnimateCommandRotate : public AnimateCommand {
public:
  AnimateCommandRotate(unsigned beats, CC_coord cntr, float rad,
		       float ang1, float ang2);

  virtual Bool Begin(AnimatePoint& pt);
  virtual Bool End(AnimatePoint& pt);
  virtual Bool NextBeat(AnimatePoint& pt);
  virtual Bool PrevBeat(AnimatePoint& pt);

  virtual void ApplyForward(AnimatePoint& pt);
  virtual void ApplyBackward(AnimatePoint& pt);

  virtual AnimateDir Direction();
  virtual void ClipBeats(unsigned beats);
private:
  CC_coord origin;
  float r, ang_start, ang_end;
};

class AnimateSheet {
public:
  AnimateSheet(unsigned numpoints);
  ~AnimateSheet();
  void SetName(const char *s);

  AnimateSheet *next, *prev;
  AnimatePoint *pts;
  AnimateCommand **commands;
  AnimateCommand **end_cmds;
  char *name;
  unsigned numbeats;
private:
  unsigned numpts;
};

class wxFrame;
class Animation {
public:
  Animation(CC_show *show, wxFrame *frame, CC_WinList *winlist);
  ~Animation();

  // Returns TRUE if changes made
  Bool PrevSheet();
  Bool NextSheet();
  Bool PrevBeat();
  Bool NextBeat();

  void GotoSheet(unsigned i);

  AnimatePoint *pts;
  AnimateSheet *curr_sheet;
  unsigned curr_sheetnum;
  unsigned numsheets;
  unsigned curr_beat;
  unsigned numpts;
  AnimateCommand **curr_cmds;
private:
  void BeginCmd(unsigned i);
  void EndCmd(unsigned i);

  void RefreshSheet();

  AnimateSheet *sheets;
};

class AnimateCompile {
public:
  AnimateCompile();
  ~AnimateCompile();

  // Prepare for compiling a point
  void Init(unsigned pt_num);
  // TRUE if successful
  Bool Append(AnimateCommand *cmd);

  inline Bool Okay() { return okay; };
  inline void SetStatus(Bool s) { okay = s; };
  void RegisterError(AnimateError err);
  inline Bool *StealErrorMarker(unsigned i) {
    Bool *b = error_markers[i];
    error_markers[i] = NULL;
    return b;
  }
  void FreeErrorMarkers();

  float vars[NUMCONTVARS];
  AnimatePoint pt;
  AnimateCommand *cmds;
  AnimateCommand *curr_cmd;
  CC_show *show;
  CC_sheet *curr_sheet;
  unsigned curr_pt;
  unsigned beats_rem;
  Bool *error_markers[NUM_ANIMERR];
private:
  void MakeErrorMarker(AnimateError err);
  Bool okay;
};

#endif
