/* animate.cc
 * Classes for animating shows
 *
 * Modification history:
 * 12-29-95   Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "anim_ui.h"
#include "cont.h"
#include <math.h>

extern int parsecontinuity();
extern const char *yyinputbuffer;
extern ContProcedure *ParsedContinuity;

const char *animate_err_msgs[] = {
  "Ran out of time",
  "Not enough to do",
  "Didn't make it to position",
  "Invalid countermarch",
  "Invalid fountain",
  "Division by zero",
  "Undefined value"
};

AnimateDir AnimGetDirFromVector(CC_coord& vector) {
  if (vector.y < vector.x/2) {
    if (vector.y < -vector.x/2) {
      if (vector.y/2 < vector.x) {
	if (-vector.y/2 < vector.x) {
	  return ANIMDIR_W;
	} else {
	  return ANIMDIR_SW;
	}
      } else {
	return ANIMDIR_NW;
      }
    } else {
      return ANIMDIR_S;
    }
  } else {
    if (vector.y < -vector.x/2) {
      return ANIMDIR_N;
    } else {
      if (vector.y/2 < vector.x) {
	return ANIMDIR_SE;
      } else {
	if (-vector.y/2 < vector.x) {
	  return ANIMDIR_NE;
	} else {
	  return ANIMDIR_E;
	}
      }
    }
  }
}

AnimateDir AnimGetDirFromAngle(float ang) {
  while (ang >= 360.0) ang -= 360.0;
  while (ang < 0.0) ang += 360.0;
  if (ang > 22.5) {
    if (ang > 67.5) {
      if (ang > 112.5) {
	if (ang > 157.5) {
	  if (ang > 202.5) {
	    if (ang > 247.5) {
	      if (ang > 292.5) {
		if (ang > 337.5) {
		  return ANIMDIR_N;
		} else {
		  return ANIMDIR_NE;
		}
	      } else {
		return ANIMDIR_E;
	      }
	    } else {
	      return ANIMDIR_SE;
	    }
	  } else {
	    return ANIMDIR_S;
	  }
	} else {
	  return ANIMDIR_SW;
	}
      } else {
	return ANIMDIR_W;
      }
    } else {
      return ANIMDIR_NW;
    }
  } else {
    return ANIMDIR_N;
  }
}


AnimateCommand::AnimateCommand(unsigned beats)
: next(NULL), prev(NULL), numbeats(beats) {}

Bool AnimateCommand::Begin(AnimatePoint& pt) {
  beat = 0;
  if (numbeats == 0) {
    ApplyForward(pt);
    return FALSE;
  }
  else return TRUE;
}

Bool AnimateCommand::End(AnimatePoint& pt) {
  beat = numbeats;
  if (numbeats == 0) {
    ApplyBackward(pt);
    return FALSE;
  }
  else return TRUE;
}

Bool AnimateCommand::NextBeat(AnimatePoint&) {
  beat++;
  if (beat >= numbeats) return FALSE;
  else return TRUE;
}

Bool AnimateCommand::PrevBeat(AnimatePoint&) {
  if (beat == 0) return FALSE;
  else {
    beat--;
    return TRUE;
  }
}

void AnimateCommand::ApplyForward(AnimatePoint&) {
  beat = numbeats;
}

void AnimateCommand::ApplyBackward(AnimatePoint&) {
  beat = 0;
}

void AnimateCommand::ClipBeats(unsigned beats) {
  numbeats = beats;
}

AnimateCommandMT::AnimateCommandMT(unsigned beats, float direction)
: AnimateCommand(beats), dir(AnimGetDirFromAngle(direction)), realdir(direction) {
}

AnimateDir AnimateCommandMT::Direction() { return dir; }

float AnimateCommandMT::RealDirection() { return realdir; }

AnimateCommandMove::AnimateCommandMove(unsigned beats, CC_coord movement)
: AnimateCommandMT(beats, movement.Direction()), vector(movement) {
}

AnimateCommandMove::AnimateCommandMove(unsigned beats, CC_coord movement, float direction)
: AnimateCommandMT(beats, direction), vector(movement) {
}

Bool AnimateCommandMove::NextBeat(AnimatePoint& pt) {
  Bool b = AnimateCommand::NextBeat(pt);
  pt.pos.x +=
    ((long)beat * vector.x / (short)numbeats) -
      ((long)(beat-1) * vector.x / (short)numbeats);
  pt.pos.y +=
    ((long)beat * vector.y / (short)numbeats) -
      ((long)(beat-1) * vector.y / (short)numbeats);
  return b;
}

Bool AnimateCommandMove::PrevBeat(AnimatePoint& pt) {
  if (AnimateCommand::PrevBeat(pt)) {
    pt.pos.x +=
      ((long)beat * vector.x / (short)numbeats) -
	((long)(beat+1) * vector.x / (short)numbeats);
    pt.pos.y +=
      ((long)beat * vector.y / (short)numbeats) -
	((long)(beat+1) * vector.y / (short)numbeats);
    return TRUE;
  } else {
    return FALSE;
  }
}

void AnimateCommandMove::ApplyForward(AnimatePoint& pt) {
  AnimateCommand::ApplyForward(pt);
  pt.pos += vector;
}

void AnimateCommandMove::ApplyBackward(AnimatePoint& pt) {
  AnimateCommand::ApplyBackward(pt);
  pt.pos -= vector;
}

void AnimateCommandMove::ClipBeats(unsigned beats) {
  AnimateCommand::ClipBeats(beats);
}

AnimateCommandRotate::AnimateCommandRotate(unsigned beats, CC_coord cntr,
					   float rad, float ang1, float ang2)
: AnimateCommand(beats), origin(cntr), r(rad), ang_start(ang1), ang_end(ang2) {
}

Bool AnimateCommandRotate::NextBeat(AnimatePoint& pt) {
  Bool b = AnimateCommand::NextBeat(pt);
  float curr_ang = ((ang_end - ang_start) * beat / numbeats + ang_start)
    * PI / 180.0;
  pt.pos.x = (Coord)CLIPFLOAT(origin.x + cos(curr_ang)*r);
  pt.pos.y = (Coord)CLIPFLOAT(origin.y - sin(curr_ang)*r);
  return b;
}

Bool AnimateCommandRotate::PrevBeat(AnimatePoint& pt) {
  if (AnimateCommand::PrevBeat(pt)) {
    float curr_ang = ((ang_end - ang_start) * beat / numbeats + ang_start)
      * PI / 180.0;
    pt.pos.x = (Coord)CLIPFLOAT(origin.x + cos(curr_ang)*r);
    pt.pos.y = (Coord)CLIPFLOAT(origin.y - sin(curr_ang)*r);
    return TRUE;
  } else {
    return FALSE;
  }
}

void AnimateCommandRotate::ApplyForward(AnimatePoint& pt) {
  AnimateCommand::ApplyForward(pt);
  pt.pos.x = (Coord)CLIPFLOAT(origin.x + cos(ang_end*PI/180.0)*r);
  pt.pos.y = (Coord)CLIPFLOAT(origin.y - sin(ang_end*PI/180.0)*r);
}

void AnimateCommandRotate::ApplyBackward(AnimatePoint& pt) {
  AnimateCommand::ApplyBackward(pt);
  pt.pos.x = (Coord)CLIPFLOAT(origin.x + cos(ang_start*PI/180.0)*r);
  pt.pos.y = (Coord)CLIPFLOAT(origin.y - sin(ang_start*PI/180.0)*r);
}

AnimateDir AnimateCommandRotate::Direction() {
  return AnimGetDirFromAngle(RealDirection());
}

float AnimateCommandRotate::RealDirection() {
  float curr_ang = (ang_end - ang_start) * beat / numbeats + ang_start;
  if (ang_end > ang_start) {
    return curr_ang + 90;
  } else {
    return curr_ang - 90;
  }
}

void AnimateCommandRotate::ClipBeats(unsigned beats) {
  AnimateCommand::ClipBeats(beats);
}

AnimateSheet::AnimateSheet(unsigned numpoints)
: next(NULL), prev(NULL), name(NULL), numpts(numpoints) {
  unsigned i;

  pts = new AnimatePoint[numpts];
  commands = new AnimateCommand*[numpts];
  end_cmds = new AnimateCommand*[numpts];
  for (i = 0; i < numpts; i++) {
    commands[i] = NULL;
    end_cmds[i] = NULL;
  }
}

AnimateSheet::~AnimateSheet() {
  AnimateCommand *cmd, *tmp;
  unsigned i;

  if (pts) delete [] pts;
  if (commands) {
    for (i = 0; i < numpts; i++) {
      cmd = commands[i];
      while (cmd) {
	tmp = cmd->next;
	delete cmd;
	cmd = tmp;
      }
    }
    delete commands;
  }
  if (end_cmds) delete end_cmds;
  if (name) delete name;
}

void AnimateSheet::SetName(const char *s) {
  if (name) delete name;
  name = copystring(s);
}

Animation::Animation(CC_show *show, wxFrame *frame, CC_WinList *winlist)
: curr_sheet(NULL), numsheets(0), numpts(show->GetNumPoints()), sheets(NULL) {
  unsigned i, j;
  ContProcedure* curr_proc;
  AnimateCompile comp;
  CC_continuity *currcont;
  wxString tempbuf;

  pts = new AnimatePoint[numpts];
  curr_cmds = new AnimateCommand*[numpts];
  collisions = new Bool[numpts];
  for (i = 0; i < numpts; i++) {
    collisions[i] = FALSE;
  }

  // Now compile
  comp.show = show;
  comp.show->winlist->FlushContinuity(); // get all changes in text windows

  for (comp.curr_sheet = show->GetSheet(); comp.curr_sheet;
       comp.curr_sheet = comp.curr_sheet->next) {
    if (! comp.curr_sheet->IsInAnimation()) continue;
    if (curr_sheet) {
      curr_sheet->next = new AnimateSheet(numpts);
      curr_sheet->next->prev = curr_sheet;
      curr_sheet = curr_sheet->next;
    } else  {
      sheets = curr_sheet = new AnimateSheet(numpts);
    }
    numsheets++;
    curr_sheet->SetName(comp.curr_sheet->GetName());
    curr_sheet->numbeats = comp.curr_sheet->beats;
    for (i = 0; i < numpts; i++) {
      curr_sheet->pts[i].pos = comp.curr_sheet->GetPosition(i);
    }

    // Now parse continuity
    comp.SetStatus(TRUE);
    comp.FreeErrorMarkers();
    for (currcont = comp.curr_sheet->animcont; currcont != NULL;
	 currcont  = currcont->next) {
      if ((yyinputbuffer = currcont->text) != NULL) {
	tempbuf.sprintf("Compiling \"%.32s\" %.32s...",
			comp.curr_sheet->GetName(), currcont->name.GetData());
	frame->SetStatusText(tempbuf.GetData());
	parsecontinuity();
	for (j = 0; j < numpts; j++) {
	  if (comp.curr_sheet->GetPoint(j).cont == currcont->num) {
	    comp.Compile(j, ParsedContinuity);
	    curr_sheet->commands[j] = comp.cmds;
	      curr_sheet->end_cmds[j] = comp.curr_cmd;
	  }
	}
	while (ParsedContinuity) {
	  curr_proc = ParsedContinuity->next;
	  delete ParsedContinuity;
	  ParsedContinuity = curr_proc;
	}
      }
    }
    // Handle points that don't have continuity (shouldn't happen)
    tempbuf.sprintf("Compiling \"%.32s\"...", comp.curr_sheet->GetName());
    frame->SetStatusText(tempbuf.GetData());
    for (j = 0; j < numpts; j++) {
      if (curr_sheet->commands[j] == NULL) {
	comp.Compile(j, NULL);
	curr_sheet->commands[j] = comp.cmds;
	curr_sheet->end_cmds[j] = comp.curr_cmd;
      }
    }
    if (!comp.Okay()) {
      tempbuf.sprintf("Errors for \"%.32s\"", comp.curr_sheet->GetName());
      (void)new AnimErrorList(&comp, winlist, frame, tempbuf.GetData());
      if (wxMessageBox("Ignore errors?", "Animate", wxYES_NO) != wxYES) {
	break;
      }
    }
  }
}

Animation::~Animation() {
  AnimateSheet *sht, *tmp;

  if (pts) delete pts;
  sht = sheets;
  while (sht) {
    tmp = sht->next;
    delete sht;
    sht = tmp;
  }
  if (curr_cmds) delete curr_cmds;
}

Bool Animation::PrevSheet() {
  if (curr_beat == 0) {
    if (curr_sheet->prev) {
      curr_sheet = curr_sheet->prev;
      curr_sheetnum--;
    }
  }
  RefreshSheet();
  CheckCollisions();
  return TRUE;
}

Bool Animation::NextSheet() {
  if (curr_sheet->next) {
    curr_sheet = curr_sheet->next;
    curr_sheetnum++;
    RefreshSheet();
    CheckCollisions();
  } else {
    if (curr_beat >= curr_sheet->numbeats) {
      if (curr_sheet->numbeats == 0) {
	curr_beat = 0;
      } else {
	curr_beat = curr_sheet->numbeats-1;
      }
    }
    return FALSE;
  }
  return TRUE;
}

Bool Animation::PrevBeat() {
  unsigned i;

  if (curr_beat == 0) {
    if (curr_sheet->prev == NULL) return FALSE;
    curr_sheet = curr_sheet->prev;
    curr_sheetnum--;
    for (i = 0; i < numpts; i++) {
      curr_cmds[i] = curr_sheet->end_cmds[i];
      EndCmd(i);
    }
    curr_beat = curr_sheet->numbeats;
  }
  for (i = 0; i < numpts; i++) {
    if (!curr_cmds[i]->PrevBeat(pts[i])) {
      // Advance to prev command, skipping zero beat commands
      if (curr_cmds[i]->prev) {
	curr_cmds[i] = curr_cmds[i]->prev;
	EndCmd(i);
	// Set to next-to-last beat of this command
	// Should always return TRUE
	curr_cmds[i]->PrevBeat(pts[i]);
      }
    }
  }
  if (curr_beat > 0)
    curr_beat--;
  CheckCollisions();
  return TRUE;
}

Bool Animation::NextBeat() {
  unsigned i;

  curr_beat++;
  if (curr_beat >= curr_sheet->numbeats) {
    return NextSheet();
  }
  for (i = 0; i < numpts; i++) {
    if (curr_cmds[i]) {
      if (!curr_cmds[i]->NextBeat(pts[i])) {
	// Advance to next command, skipping zero beat commands
	if (curr_cmds[i]->next) {
	  curr_cmds[i] = curr_cmds[i]->next;
	  BeginCmd(i);
	}
      }
    }
  }
  CheckCollisions();
  return TRUE;
}

void Animation::GotoBeat(unsigned i) {
  while (curr_beat > i) {
    PrevBeat();
  }    
  while (curr_beat < i) {
    NextBeat();
  }    
}

void Animation::GotoSheet(unsigned i) {
  curr_sheetnum = i;
  curr_sheet = sheets;
  while (i > 0) {
    curr_sheet = curr_sheet->next;
    i--;
  }    
  CheckCollisions();
  RefreshSheet();
}

void Animation::BeginCmd(unsigned i) {
  if (curr_cmds[i] != NULL) {
    while (!curr_cmds[i]->Begin(pts[i])) {
      if (curr_cmds[i]->next == NULL) return;
      curr_cmds[i] = curr_cmds[i]->next;
    }
  }
}

void Animation::EndCmd(unsigned i) {
  if (curr_cmds[i] != NULL) {
    while (!curr_cmds[i]->End(pts[i])) {
      if (curr_cmds[i]->prev == NULL) return;
      curr_cmds[i] = curr_cmds[i]->prev;
    }
  }
}

void Animation::RefreshSheet() {
  unsigned i;

  for (i = 0; i < numpts; i++) {
    pts[i].pos = curr_sheet->pts[i].pos;
    curr_cmds[i] = curr_sheet->commands[i];
    BeginCmd(i);
  }
  curr_beat = 0;
}

void Animation::CheckCollisions() {
  unsigned i, j;

  for (i = 0; i < numpts; i++) {
    collisions[i] = FALSE;
  }
  if (check_collis) {
    for (i = 0; i < numpts; i++) {
      for (j = i+1; j < numpts; j++) {
	if (pts[i].pos.Collides(pts[j].pos)) {
	  collisions[i] = TRUE;
	  collisions[j] = TRUE;
	}
      }
    }
  }
}

AnimateCompile::AnimateCompile()
: okay(TRUE) {
  unsigned i;

  for (i = 0; i < NUM_ANIMERR; i++) {
    error_markers[i] = NULL;
  }
}

AnimateCompile::~AnimateCompile() {
  FreeErrorMarkers();
}

void AnimateCompile::Compile(unsigned pt_num, ContProcedure* proc) {
  unsigned i;
  CC_coord c;

  for (i = 0; i < NUMCONTVARS; i++) {
    vars[i] = 0.0;
    vars_valid[i] = FALSE;
  }

  pt.pos = curr_sheet->GetPosition(pt_num);
  cmds = curr_cmd = NULL;
  curr_pt = pt_num;
  beats_rem = curr_sheet->beats;

  if (proc == NULL) {
    // no continuity was specified
    CC_sheet *s;
    for (s = curr_sheet->next; s != NULL; s = s->next) {
      if (s->IsInAnimation()) {
	//use EVEN REM NP
	ContProcEven defcont(new ContValueFloat(beats_rem),
			     new ContNextPoint());
	defcont.Compile(this);
	break;
      }
    }
    if (s == NULL) {
      //use MTRM E
      ContProcMTRM defcont(new ContValueDefined(CC_E));
      defcont.Compile(this);
    }
  }

  for (; proc; proc = proc->next) {
    proc->Compile(this);
  }
  if (curr_sheet->next) {
    if (pt.pos !=
	curr_sheet->next->GetPosition(curr_pt)) {
      c = curr_sheet->next->GetPosition(curr_pt) - pt.pos;
      RegisterError(ANIMERR_WRONGPLACE);
      Append(new AnimateCommandMove(beats_rem, c));
    }
  }
  if (beats_rem) {
    RegisterError(ANIMERR_EXTRATIME);
    Append(new AnimateCommandMT(beats_rem, ANIMDIR_E));
  }
}

Bool AnimateCompile::Append(AnimateCommand *cmd) {
  Bool clipped;

  if (beats_rem < cmd->numbeats) {
    RegisterError(ANIMERR_OUTOFTIME);
    if (beats_rem == 0) {
      delete cmd;
      return FALSE;
    }
    cmd->ClipBeats(beats_rem);
    clipped = TRUE;
  } else {
    clipped = FALSE;
  }
  if (curr_cmd) {
    curr_cmd->next = cmd;
    cmd->prev = curr_cmd;
    curr_cmd = cmd;
  } else {
    cmds = curr_cmd = cmd;
  }
  beats_rem -= cmd->numbeats;

  cmd->ApplyForward(pt); // Move current point to new position
  vars[CONTVAR_DOF] = cmd->RealDirection();
  vars_valid[CONTVAR_DOF] = TRUE;
  return TRUE;
}

void AnimateCompile::RegisterError(AnimateError err) {
  MakeErrorMarker(err);
  error_markers[err][curr_pt] = TRUE;
  SetStatus(FALSE);
}

void AnimateCompile::FreeErrorMarkers() {
  unsigned i;
  
  for (i = 0; i < NUM_ANIMERR; i++) {
    if (error_markers[i]) {
      delete [] error_markers[i];
      error_markers[i] = NULL;
    }
  }
}

void AnimateCompile::MakeErrorMarker(AnimateError err) {
  unsigned i;

  if (!error_markers[err]) {
    error_markers[err] = new Bool[show->GetNumPoints()];
    for (i = 0; i < show->GetNumPoints(); i++) {
      error_markers[err][i] = FALSE;
    }
  }
}
