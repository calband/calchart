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

// Number of variables in continuity language (A B C D X Y Z DOF DOH)
enum {
  CONTVAR_A,
  CONTVAR_B,
  CONTVAR_C,
  CONTVAR_D,
  CONTVAR_X,
  CONTVAR_Y,
  CONTVAR_Z,
  CONTVAR_DOF,
  CONTVAR_DOH,
  NUMCONTVARS
};

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
  ANIMERR_DIVISION_ZERO,
  ANIMERR_UNDEFINED,
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
  virtual float RealDirection() = 0;
  virtual float MotionDirection();
  virtual void ClipBeats(unsigned beats);

  AnimateCommand *next, *prev;
  unsigned numbeats;
protected:
  unsigned beat;
};

class AnimateCommandMT : public AnimateCommand {
public:
  AnimateCommandMT(unsigned beats, float direction);

  virtual AnimateDir Direction();
  virtual float RealDirection();
protected:
  AnimateDir dir;
  float realdir;
};

class AnimateCommandMove : public AnimateCommandMT {
public:
  AnimateCommandMove(unsigned beats, CC_coord movement);
  AnimateCommandMove(unsigned beats, CC_coord movement, float direction);

  virtual Bool NextBeat(AnimatePoint& pt);
  virtual Bool PrevBeat(AnimatePoint& pt);

  virtual void ApplyForward(AnimatePoint& pt);
  virtual void ApplyBackward(AnimatePoint& pt);

  virtual float MotionDirection();
  virtual void ClipBeats(unsigned beats);
private:
  CC_coord vector;
};

class AnimateCommandRotate : public AnimateCommand {
public:
  AnimateCommandRotate(unsigned beats, CC_coord cntr, float rad,
		       float ang1, float ang2);

  virtual Bool NextBeat(AnimatePoint& pt);
  virtual Bool PrevBeat(AnimatePoint& pt);

  virtual void ApplyForward(AnimatePoint& pt);
  virtual void ApplyBackward(AnimatePoint& pt);

  virtual AnimateDir Direction();
  virtual float RealDirection();
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

enum CollisionWarning {
  COLLISION_NONE,
  COLLISION_SHOW,
  COLLISION_BEEP
};

class wxFrame;
class Animation {
public:
  Animation(CC_show *show, wxFrame *frame, CC_WinList *winlist);
  ~Animation();

  // Returns TRUE if changes made
  Bool PrevSheet();
  Bool NextSheet();

  void GotoBeat(unsigned i);

  Bool PrevBeat();
  Bool NextBeat();

  void GotoSheet(unsigned i);

  inline void EnableCollisions(CollisionWarning col) { check_collis = col; }
  void CheckCollisions();

  AnimatePoint *pts;
  AnimateSheet *curr_sheet;
  unsigned curr_sheetnum;
  unsigned numsheets;
  unsigned curr_beat;
  unsigned numpts;
  AnimateCommand **curr_cmds;
  Bool *collisions;
private:
  void BeginCmd(unsigned i);
  void EndCmd(unsigned i);

  void RefreshSheet();

  AnimateSheet *sheets;
  CollisionWarning check_collis;
};

class AnimateVariable {
private:
  float v;
  Bool valid;
public:
  AnimateVariable(): v(0.0), valid(FALSE) {}
  inline Bool IsValid() const { return valid; }
  inline float GetValue() const { return v; }
  inline void SetValue(float newv) { v = newv; valid = TRUE; }
  inline void ClearValue() { v = 0.0; valid = FALSE; }
};

class ContProcedure;
class AnimateCompile {
public:
  AnimateCompile();
  ~AnimateCompile();

  // Compile a point
  void Compile(unsigned pt_num, ContProcedure* proc);
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

  float GetVarValue(int varnum);
  void SetVarValue(int varnum, float value);

  AnimatePoint pt;
  AnimateCommand *cmds;
  AnimateCommand *curr_cmd;
  CC_show *show;
  CC_sheet *curr_sheet;
  unsigned curr_pt;
  unsigned beats_rem;
  Bool *error_markers[NUM_ANIMERR];
private:
  AnimateVariable *vars[NUMCONTVARS];
  void MakeErrorMarker(AnimateError err);
  Bool okay;
};

#endif
