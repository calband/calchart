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

#include "animate.h"
#include "cont.h"
#include <wx_utils.h>

extern int parsecontinuity();
extern const char *yyinputbuffer;
extern ContProcedure *ParsedContinuity;

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

Bool AnimateCommand::Begin(AnimatePoint&) {
  beat = 0;
  return TRUE;
}

Bool AnimateCommand::End(AnimatePoint&) {
  beat = numbeats;
  return TRUE;
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
}

void AnimateCommand::ApplyBackward(AnimatePoint&) {
}

void AnimateCommand::ClipBeats(unsigned beats) {
  numbeats = beats;
}

AnimateCommandMT::AnimateCommandMT(unsigned beats, AnimateDir direction)
: AnimateCommand(beats), dir(direction) {
}

AnimateDir AnimateCommandMT::Direction() { return dir; }

AnimateCommandMove::AnimateCommandMove(unsigned beats, CC_coord movement)
: AnimateCommandMT(beats, AnimGetDirFromVector(movement)), vector(movement) {
}

Bool AnimateCommandMove::Begin(AnimatePoint& pt) {
  if (numbeats == 0) {
    pt.pos += vector;
    return FALSE;
  } else {
    return AnimateCommand::Begin(pt);
  }
}

Bool AnimateCommandMove::End(AnimatePoint& pt) {
  if (numbeats == 0) {
    pt.pos -= vector;
    return FALSE;
  } else {
    return AnimateCommand::End(pt);
  }
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
  pt.pos += vector;
}

void AnimateCommandMove::ApplyBackward(AnimatePoint& pt) {
  pt.pos -= vector;
}

void AnimateCommandMove::ClipBeats(unsigned beats) {
  AnimateCommand::ClipBeats(beats);
}

AnimateCommandRotate::AnimateCommandRotate(unsigned beats, CC_coord cntr,
					   float rad, float ang1, float ang2)
: AnimateCommand(beats), origin(cntr), r(rad), ang_start(ang1), ang_end(ang2) {
}

Bool AnimateCommandRotate::Begin(AnimatePoint& pt) {
  if (numbeats == 0) {
    ApplyForward(pt);
    return FALSE;
  } else {
    return AnimateCommand::Begin(pt);
  }
}

Bool AnimateCommandRotate::End(AnimatePoint& pt) {
  if (numbeats == 0) {
    ApplyBackward(pt);
    return FALSE;
  } else {
    return AnimateCommand::End(pt);
  }
}

Bool AnimateCommandRotate::NextBeat(AnimatePoint& pt) {
  Bool b = AnimateCommand::NextBeat(pt);
  float curr_ang = ((ang_end - ang_start) * beat / numbeats + ang_start)
    * PI / 180.0;
  pt.pos.x = CLIPCOORD(origin.x + cos(curr_ang)*r);
  pt.pos.y = CLIPCOORD(origin.y - sin(curr_ang)*r);
  return b;
}

Bool AnimateCommandRotate::PrevBeat(AnimatePoint& pt) {
  if (AnimateCommand::PrevBeat(pt)) {
    float curr_ang = ((ang_end - ang_start) * beat / numbeats + ang_start)
      * PI / 180.0;
    pt.pos.x = CLIPCOORD(origin.x + cos(curr_ang)*r);
    pt.pos.y = CLIPCOORD(origin.y - sin(curr_ang)*r);
    return TRUE;
  } else {
    return FALSE;
  }
}

void AnimateCommandRotate::ApplyForward(AnimatePoint& pt) {
  pt.pos.x = CLIPCOORD(origin.x + cos(ang_end*PI/180.0)*r);
  pt.pos.y = CLIPCOORD(origin.y - sin(ang_end*PI/180.0)*r);
}

void AnimateCommandRotate::ApplyBackward(AnimatePoint& pt) {
  pt.pos.x = CLIPCOORD(origin.x + cos(ang_start*PI/180.0)*r);
  pt.pos.y = CLIPCOORD(origin.y - sin(ang_start*PI/180.0)*r);
}

AnimateDir AnimateCommandRotate::Direction() {
  float curr_ang = (ang_end - ang_start) * beat / numbeats + ang_start;
  if (ang_end > ang_start) {
    return AnimGetDirFromAngle(curr_ang + 90);
  } else {
    return AnimGetDirFromAngle(curr_ang - 90);
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

Animation::Animation(CC_show *show)
: curr_sheet(NULL), numpts(show->GetNumPoints()), sheets(NULL) {
  unsigned i, j;
  ContProcedure* curr_proc;
  AnimateCompile comp;
  CC_continuity *currcont;

  pts = new AnimatePoint[numpts];
  curr_cmds = new AnimateCommand*[numpts];
  
  // Now compile
  comp.show = show;
  comp.show->winlist->FlushContinuity(); // get all changes in text windows

  for (comp.curr_sheet = show->GetSheet(); comp.curr_sheet;
       comp.curr_sheet = comp.curr_sheet->next) {
    if (curr_sheet) {
      curr_sheet->next = new AnimateSheet(numpts);
      curr_sheet->next->prev = curr_sheet;
      curr_sheet = curr_sheet->next;
    } else  {
      sheets = curr_sheet = new AnimateSheet(numpts);
    }
    curr_sheet->SetName(comp.curr_sheet->GetName());
    curr_sheet->numbeats = comp.curr_sheet->beats;
    for (i = 0; i < numpts; i++) {
      curr_sheet->pts[i].pos = comp.curr_sheet->pts[i].pos;
    }

    // Now parse continuity
    for (currcont = comp.curr_sheet->animcont; currcont != NULL;
	 currcont  = currcont->next) {
      if ((yyinputbuffer = currcont->text) != NULL) {
#ifndef wx_msw
	fprintf(stderr, "Parsing %s %s...\n", comp.curr_sheet->GetName(),
		(const char *)currcont->name);
#endif
	parsecontinuity();
	if (ParsedContinuity != NULL) {
	  for (j = 0; j < numpts; j++) {
	    if (comp.curr_sheet->pts[j].cont == currcont->num) {
	      comp.Init(j);
	      for (curr_proc = ParsedContinuity; curr_proc;
		   curr_proc = curr_proc->next) {
		curr_proc->Compile(&comp);
	      }
	      CC_coord c;
	      if (comp.curr_sheet->next) {
		if (comp.pt.pos !=
		    comp.curr_sheet->next->pts[comp.curr_pt].pos) {
		  c = comp.curr_sheet->next->pts[comp.curr_pt].pos -
		    comp.pt.pos;
		  comp.Append(new AnimateCommandMove(comp.beats_rem, c));
		}
	      }
	      if (comp.beats_rem) {
		comp.Append(new AnimateCommandMT(comp.beats_rem, ANIMDIR_E));
	      }
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
  if (curr_sheet->prev) {
    curr_sheet = curr_sheet->prev;
  }
  RefreshSheet();
  return TRUE;
}

Bool Animation::NextSheet() {
  if (curr_sheet->next) {
    curr_sheet = curr_sheet->next;
    RefreshSheet();
  } else {
    if ((curr_sheet == sheets) && (curr_beat != 0))
      RefreshSheet();
    else return FALSE;
  }
  return TRUE;
}

Bool Animation::PrevBeat() {
  unsigned i;

  if (curr_beat == 0) {
    if (curr_sheet->prev == NULL) return FALSE;
    curr_sheet = curr_sheet->prev;
    for (i = 0; i < numpts; i++) {
      curr_cmds[i] = curr_sheet->end_cmds[i];
      EndCmd(i);
    }
    curr_beat = curr_sheet->numbeats;
  }
  for (i = 0; i < numpts; i++) {
    while (curr_cmds[i] != NULL) {
      if (curr_cmds[i]->PrevBeat(pts[i])) break;
      // Advance to prev command, skipping zero beat commands
      curr_cmds[i] = curr_cmds[i]->prev;
      EndCmd(i);
    }
  }
  curr_beat--;
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
	curr_cmds[i] = curr_cmds[i]->next;
	BeginCmd(i);
      }
    }
  }
  return TRUE;
}

void Animation::GotoSheet(unsigned i) {
  curr_sheet = sheets;
  while (i > 0) {
    curr_sheet = curr_sheet->next;
    i--;
  }    
  RefreshSheet();
}

void Animation::BeginCmd(unsigned i) {
  while (curr_cmds[i] != NULL) {
    if (curr_cmds[i]->Begin(pts[i])) return;
    curr_cmds[i] = curr_cmds[i]->next;
  }
}

void Animation::EndCmd(unsigned i) {
  while (curr_cmds[i] != NULL) {
    if (curr_cmds[i]->End(pts[i])) return;
    curr_cmds[i] = curr_cmds[i]->next;
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

AnimateCompile::AnimateCompile()
: okay(TRUE) {
}

void AnimateCompile::Init(unsigned pt_num) {
  unsigned i;

  for (i = 0; i < NUMCONTVARS; i++) {
    vars[i] = 0.0;
  }
  pt.pos = curr_sheet->pts[pt_num].pos;
  cmds = curr_cmd = NULL;
  curr_pt = pt_num;
  beats_rem = curr_sheet->beats;
  okay = TRUE;
}

Bool AnimateCompile::Append(AnimateCommand *cmd) {
  Bool clipped;

  if (beats_rem < cmd->numbeats) {
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

  return TRUE;
}
