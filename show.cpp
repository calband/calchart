/* show.cc
 * Member functions for show classes
 *
 * Modification history:
 * 4-16-95    Garrick Meeker              Created from previous CalPrint
 * 7-28-95    Garrick Meeker              Added continuity parser from
 *                                           previous CalPrint
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <wx_utils.h>
#include "show.h"
#include "undo.h"
#include "confgr.h"
#include "modes.h"

#include "ingl.h"

#include <ctype.h>
#include <math.h>

static char nomem_str[] = "Out of memory!";
static char nofile_str[] = "Unable to open file";
static char badfile_mas_str[] = "Error reading master file";
static char badfile_pnt_str[] = "Error reading points file";
static char badfile_cnt_str[] = "Error reading animation continuity file";
static char badanimcont_str[] = "Error in animation continuity file";
static char badcont_str[] = "Error in continuity file";
static char contnohead_str[] = "Continuity file doesn't begin with header";
static char nosheets_str[] = "No sheets found";
static char writeerr_str[] = "Write error: check disk media";

static char *contnames[] = {
  "Plain",
  "Sol",
  "Bksl",
  "Sl",
  "X",
  "Solbksl",
  "Solsl",
  "Solx"
};

extern ShowModeList *modelist;

CC_WinNode::CC_WinNode(CC_WinList *lst)
: list(lst) {
  list->Add(this);
}

CC_WinNode::~CC_WinNode() {}

void CC_WinNode::Remove() {
  list->Remove(this);
}

void CC_WinNode::SetShow(CC_show*) {}
void CC_WinNode::ChangeName() {}
void CC_WinNode::UpdateSelections(wxWindow*, int) {}
void CC_WinNode::UpdatePoints() {}
void CC_WinNode::UpdatePointsOnSheet(unsigned, int) {}
void CC_WinNode::ChangeNumPoints(wxWindow*) {}
void CC_WinNode::ChangePointLabels(wxWindow*) {}
void CC_WinNode::ChangeShowMode(wxWindow*) {}
void CC_WinNode::UpdateStatusBar() {}
void CC_WinNode::GotoSheet(unsigned) {}
void CC_WinNode::AddSheet(unsigned) {}
void CC_WinNode::DeleteSheet(unsigned) {}
void CC_WinNode::AppendSheets() {}
void CC_WinNode::RemoveSheets(unsigned) {}
void CC_WinNode::ChangeTitle(unsigned) {}
void CC_WinNode::SelectSheet(wxWindow*, unsigned) {}
void CC_WinNode::AddContinuity(unsigned, unsigned) {}
void CC_WinNode::DeleteContinuity(unsigned, unsigned) {}
void CC_WinNode::FlushContinuity() {}
void CC_WinNode::SetContinuity(wxWindow*, unsigned, unsigned) {}
void CC_WinNode::ChangePrint(wxWindow*) {}
void CC_WinNode::FlushDescr() {}
void CC_WinNode::SetDescr(wxWindow*) {}

CC_WinList::CC_WinList()
: list(NULL) {}

CC_WinList::~CC_WinList() {}

Bool CC_WinList::MultipleWindows() {
  if (list == NULL) return FALSE;
  if (list->next == NULL) return FALSE;
  return TRUE;
}

void CC_WinList::Add(CC_WinNode *node) {
  node->next = list;
  list = node;
}

void CC_WinList::Remove(CC_WinNode *node) {
  CC_WinNode *n;
#if 0
  unsigned i;

  for (i = 0, n = list; n != NULL; n = n->next, i++);
  fprintf(stderr, "Deleting window from list of %u elements.\n", i);
#endif
  if (list == node) {
    list = list->next;
    if (list == NULL) Empty();
  } else {
    for (n = list; n != NULL; n = n->next) {
      if (n->next == node) {
	n->next = n->next->next;
	break;
      }
    }
  }
}

void CC_WinList::Empty() {}

void CC_WinList::SetShow(CC_show *shw) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->SetShow(shw);
  }
}
void CC_WinList::ChangeName() {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->ChangeName();
  }
}
void CC_WinList::UpdateSelections(wxWindow* win, int point) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->UpdateSelections(win, point);
  }
}
void CC_WinList::UpdatePoints() {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->UpdatePoints();
  }
}
void CC_WinList::UpdatePointsOnSheet(unsigned sht, int ref) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->UpdatePointsOnSheet(sht, ref);
  }
}
void CC_WinList::ChangeNumPoints(wxWindow *win) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->ChangeNumPoints(win);
  }
}
void CC_WinList::ChangePointLabels(wxWindow *win) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->ChangePointLabels(win);
  }
}
void CC_WinList::ChangeShowMode(wxWindow *win) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->ChangeShowMode(win);
  }
}
void CC_WinList::UpdateStatusBar() {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->UpdateStatusBar();
  }
}
void CC_WinList::GotoSheet(unsigned sht) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->GotoSheet(sht);
  }
}
void CC_WinList::AddSheet(unsigned sht) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->AddSheet(sht);
  }
}
void CC_WinList::DeleteSheet(unsigned sht) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->DeleteSheet(sht);
  }
}
void CC_WinList::AppendSheets() {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->AppendSheets();
  }
}
void CC_WinList::RemoveSheets(unsigned num) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->RemoveSheets(num);
  }
}
void CC_WinList::ChangeTitle(unsigned sht) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->ChangeTitle(sht);
  }
}
void CC_WinList::SelectSheet(wxWindow* win, unsigned sht) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->SelectSheet(win, sht);
  }
}
void CC_WinList::AddContinuity(unsigned sht, unsigned cont) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->AddContinuity(sht, cont);
  }
}
void CC_WinList::DeleteContinuity(unsigned sht, unsigned cont) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->DeleteContinuity(sht, cont);
  }
}
void CC_WinList::FlushContinuity() {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->FlushContinuity();
  }
}
void CC_WinList::SetContinuity(wxWindow* win, unsigned sht, unsigned cont) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->SetContinuity(win, sht, cont);
  }
}
void CC_WinList::ChangePrint(wxWindow* win) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->ChangePrint(win);
  }
}
void CC_WinList::FlushDescr() {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->FlushDescr();
  }
}
void CC_WinList::SetDescr(wxWindow* win) {
  CC_WinNode *n;

  for (n = list; n != NULL; n = n->next) {
    n->SetDescr(win);
  }
}

CC_WinListShow::CC_WinListShow(CC_show *shw)
: show(shw) {}

void CC_WinListShow::Empty() {
  delete show;
}

CC_continuity::CC_continuity()
: next(NULL), num(0) {}

CC_continuity::~CC_continuity() {
}

void CC_continuity::SetName(const char* s) {
  name = s;
  name.Capitalize();
}

void CC_continuity::SetText(const char* s) {
  text = s;
}

void CC_continuity::AppendText(const char* s) {
  text.Append(s);
}

// Get magnitude of vector
float CC_coord::Magnitude() const {
  float x_f, y_f;

  x_f = COORD2FLOAT(x);
  y_f = COORD2FLOAT(y);
  return sqrt(x_f*x_f + y_f*y_f);
}

// Get magnitude, but check for diagonal military
float CC_coord::DM_Magnitude() const {
  if ((x == y) || (x == -y)) {
    return COORD2FLOAT(ABS(x));
  } else {
    return Magnitude();
  }
}

// Get direction of this vector
float CC_coord::Direction() const {
  float ang;

  if (*this == 0) return 0.0;

  ang = acos(COORD2FLOAT(x)/Magnitude()); // normalize
  ang *= 180.0/PI; // convert to degrees
  if (y > 0) ang += 180.0; // check for > PI

  return ang;
}

// Get direction from this coord to another
float CC_coord::Direction(const CC_coord& c) const {
  CC_coord vect = c - *this;

  return vect.Direction();
}

// Set a coordinate from an old format disk coord
CC_coord& CC_coord::operator = (const cc_oldcoord& old) {
  x = ((get_lil_word(&old.x)+2) << (COORD_SHIFT-3)) - INT2COORD(88);
  y = (((get_lil_word(&old.y)+2) << COORD_SHIFT) / 6) - INT2COORD(50);
  return *this;
}

CC_sheet::CC_sheet(CC_show *shw)
: next(NULL), continuity(NULL), animcont(NULL), show(shw),
  numanimcont(0), beats(0), picked(TRUE) {
    pts = new CC_point[show->GetNumPoints()];
}

CC_sheet::CC_sheet(CC_show *shw, const char *newname)
: next(NULL), continuity(NULL), animcont(NULL), show(shw),
  numanimcont(0), beats(0), picked(TRUE), pts(NULL), name(newname) {
    pts = new CC_point[show->GetNumPoints()];
}

CC_sheet::CC_sheet(CC_sheet *sht)
: next(NULL), continuity(NULL), show(sht->show),
  beats(0), picked(sht->picked), name(sht->name), number(sht->number)
{
  int i;

  pts = new CC_point[show->GetNumPoints()];
  for (i = 0; i < show->GetNumPoints(); i++) {
    pts[i] = sht->pts[i];
    pts[i].sym = SYMBOL_PLAIN;
    pts[i].cont = 0;
  }
  animcont = new CC_continuity;
  animcont->SetName(contnames[0]);
  numanimcont = 1;
}

CC_sheet::~CC_sheet() {
  cc_text *tmp;
  CC_continuity *conttmp;

  if (pts) delete [] pts;
  if (animcont) {
    while (animcont) {
      conttmp = animcont->next;
      delete animcont;
      animcont = conttmp;
    }
  }
  while (continuity) {
    while (continuity->more) {
      tmp = continuity->more->more;
      delete continuity->more;
      continuity->more = tmp;
    }
    tmp = continuity->next;
    delete continuity;
    continuity = tmp;
  }
}

// Return the number of selected points
unsigned CC_sheet::GetNumSelectedPoints() {
  unsigned i,num;
  for (i=0,num=0; i<show->GetNumPoints(); i++) {
    if (show->IsSelected(i)) num++;
  }
  return num;
}

// Find point at certain coords
int CC_sheet::FindPoint(Coord x, Coord y, unsigned ref) {
  unsigned i;
  CC_coord c;
  Coord w = FLOAT2COORD(dot_ratio);
  for (i = 0; i < show->GetNumPoints(); i++) {
    c = GetPosition(i, ref);
    if (((x+w) >= c.x) && ((x-w) <= c.x) && ((y+w) >= c.y) && ((y-w) <= c.y)) {
      return i;
    }
  }
  return -1;
}

// Select points within rectangle
Bool CC_sheet::SelectPointsInRect(CC_coord c1, CC_coord c2) {
  unsigned i;
  Bool changed = FALSE;
  Coord n;

  if (c1.x > c2.x) {
    n = c2.x;
    c2.x = c1.x;
    c1.x = n;
  }
  if (c1.y > c2.y) {
    n = c2.y;
    c2.y = c1.y;
    c1.y = n;
  }
  for (i = 0; i < show->GetNumPoints(); i++) {
    if ((pts[i].pos.x >= c1.x) && (pts[i].pos.x <= c2.x) &&
	(pts[i].pos.y >= c1.y) && (pts[i].pos.y <= c2.y)) {
      if (!show->IsSelected(i)) {
	show->Select(i);
	changed = TRUE;
      }
    }
  }
  return changed;
}

Bool CC_sheet::SelectContinuity(unsigned i) {
  unsigned j;
  Bool changed = FALSE;

  for (j = 0; j < show->GetNumPoints(); j++) {
    if (pts[j].cont == i) {
      if (!show->IsSelected(j)) {
	show->Select(j, TRUE);
	changed = TRUE;
      }
    } else {
      if (show->IsSelected(j)) {
	show->Select(j, FALSE);
	changed = TRUE;
      }
    }
  }
  return changed;
}

void CC_sheet::SetContinuity(unsigned i) {
  unsigned j;

  // Create undo entry (sym also does continuity)
  show->undolist->Add(new ShowUndoSym(show->GetSheetPos(this), this, TRUE));

  for (j = 0; j < show->GetNumPoints(); j++) {
    if (show->IsSelected(j)) {
      pts[j].cont = i;
    }
  }
}

void CC_sheet::SetNumPoints(unsigned num, unsigned columns) {
  CC_point *newpts;
  unsigned i, j, cpy, col;
  CC_coord c, coff(show->mode->FieldOffset());
  CC_continuity *plaincont;

  newpts = new CC_point[num];
  cpy = MIN(show->GetNumPoints(), num);
  for (i = 0; i < cpy; i++) {
    newpts[i] = pts[i];
  }
  for (c = coff, col = 0; i < num; i++, col++, c.x += INT2COORD(2)) {
    plaincont = GetStandardContinuity(SYMBOL_PLAIN);
    if (col >= columns) {
      c.x = coff.x;
      c.y += INT2COORD(2);
      col = 0;
    }
    newpts[i].flags = 0;
    newpts[i].sym = SYMBOL_PLAIN;
    newpts[i].cont = plaincont->num;
    newpts[i].pos = c;
    for (j = 0; j < NUM_REF_PNTS; j++) {
      newpts[i].ref[j] = newpts[i].pos;
    }
  }
  delete [] pts;
  pts = newpts;
}

void CC_sheet::RelabelSheet(unsigned *table) {
  CC_point *newpts;

  newpts = new CC_point[show->GetNumPoints()];
  for (unsigned i = 0; i < show->GetNumPoints(); i++) {
    newpts[i] = pts[table[i]];
  }
  delete [] pts;
  pts = newpts;
}

CC_continuity *CC_sheet::GetNthContinuity(unsigned i) {
  CC_continuity *c;

  c = animcont;
  while ((i > 0) && c) {
    i--;
    c = c->next;
  }
  return c;
}

CC_continuity *CC_sheet::UserGetNthContinuity(unsigned i) {
  show->winlist->FlushContinuity();
  return GetNthContinuity(i);
}

void CC_sheet::SetNthContinuity(const char *text, unsigned i) {
  CC_continuity *c;

  c = GetNthContinuity(i);
  if (c) {
    c->SetText(text);
  }
}

void CC_sheet::UserSetNthContinuity(const char *text, unsigned i,
				    wxWindow *win) {
  CC_continuity *c;

  c = GetNthContinuity(i);
  if (c) {
    // Create undo entry
    show->undolist->Add(new ShowUndoCont(show->GetSheetPos(this), i, this));
    c->SetText(text);
    show->winlist->SetContinuity(win, show->GetSheetPos(this), i);
  }
}

CC_continuity *CC_sheet::RemoveNthContinuity(unsigned i) {
  CC_continuity *cont = animcont;
  CC_continuity *tmp;
  unsigned idx;

  if (i > 0) {
    idx = i;
    while (--idx) {
      cont = cont->next;
    }
    tmp = cont->next;
    cont->next = tmp->next;
    cont = tmp;
  } else {
    animcont = animcont->next;
  }
  numanimcont--;
  cont->next = NULL;
  show->winlist->DeleteContinuity(show->GetSheetPos(this), i);
  return cont;
}

void CC_sheet::UserDeleteContinuity(unsigned i) {
  CC_continuity *cont = RemoveNthContinuity(i);
  show->undolist->Add(new ShowUndoDeleteContinuity(show->GetSheetPos(this),
						   i, cont));
}

void CC_sheet::InsertContinuity(CC_continuity *newcont, unsigned i) {
  CC_continuity *cont = animcont;
  unsigned idx;

  if (i > 0) {
    idx = i;
    while (--idx) {
      cont = cont->next;
    }
    newcont->next = cont->next;
    cont->next = newcont;
  } else {
    newcont->next = animcont;
    animcont = newcont;
  }
  numanimcont++;
  show->winlist->AddContinuity(show->GetSheetPos(this), i);
}

void CC_sheet::AppendContinuity(CC_continuity *newcont) {
  CC_continuity *last;

  if (animcont == NULL) {
    animcont = newcont;
  } else {
    last = animcont;
    while (last->next != NULL) last = last->next;
    last->next = newcont;
  }
  numanimcont++;
}

CC_continuity *CC_sheet::UserNewContinuity(const char *name) {
  CC_continuity *newcont;
  unsigned newcontnum;

  newcont = new CC_continuity;
  newcont->SetName(name);
  newcontnum = NextUnusedContinuityNum();
  newcont->num = newcontnum;
  AppendContinuity(newcont);
  show->winlist->AddContinuity(show->GetSheetPos(this), numanimcont-1);
  show->undolist->Add(new ShowUndoAddContinuity(show->GetSheetPos(this),
						numanimcont-1));
  return newcont;
}

unsigned CC_sheet::NextUnusedContinuityNum() {
  unsigned i = 0;
  Bool found;
  CC_continuity *c;

  do {
    found = FALSE;
    for (c = animcont; c != NULL; c = c->next) {
      if (c->num == i) {
	found = TRUE;
	i++;
	break;
      }
    }
  } while (found);
  return i;
}

CC_continuity *CC_sheet::GetStandardContinuity(SYMBOL_TYPE sym) {
  CC_continuity *c;
  unsigned i,idx;

  for (c = animcont; c != NULL; c = c->next) {
    if (c->name.CompareTo(contnames[sym], wxString::ignoreCase) == 0) {
      break;
    }
  }
  if (c == NULL) {
    i = (unsigned)sym;
    idx = 0;
    // Put in correct postion
    while (i > 0) {
      idx = FindContinuityByName(contnames[--i]);
      if (idx != 0) break;
    }
    c = new CC_continuity;
    c->SetName(contnames[sym]);
    c->num = NextUnusedContinuityNum();
    InsertContinuity(c, idx);
    show->undolist->Add(new ShowUndoAddContinuity(show->GetSheetPos(this),
						  idx));
  }
  return c;
}

unsigned CC_sheet::FindContinuityByName(const char *name) {
  unsigned idx;
  CC_continuity *c;

  for (idx = 1, c = animcont; c != NULL; idx++, c = c->next) {
    if (c->name.CompareTo(name, wxString::ignoreCase) == 0) {
      break;
    }
  }
  if (c == NULL) {
    idx = 0;
  }
  return idx;
}

Bool CC_sheet::ContinuityInUse(unsigned idx) {
  unsigned i;
  CC_continuity *c = GetNthContinuity(idx);
 
  for (i = 0; i < show->GetNumPoints(); i++) {
    if (pts[i].cont == c->num) return TRUE;
  }
  return FALSE;
}

void CC_sheet::UserSetName(const char *newname) {
  // Create undo entry
  show->undolist->Add(new ShowUndoName(show->GetSheetPos(this), this));
  SetName(newname);
  show->winlist->ChangeTitle(show->GetSheetPos(this));
}

void CC_sheet::UserSetBeats(unsigned short b) {
  // Create undo entry
  show->undolist->Add(new ShowUndoBeat(show->GetSheetPos(this), this));
  beats = b;
  show->winlist->ChangeTitle(show->GetSheetPos(this));
}

// Set point symbols
Bool CC_sheet::SetPointsSym(SYMBOL_TYPE sym) {
  unsigned i;
  Bool change = FALSE;
  CC_continuity *c;

  if (GetNumSelectedPoints() <= 0) return FALSE;

  // Create undo entries
  show->undolist->StartMulti();
  c = GetStandardContinuity(sym);
  show->undolist->Add(new ShowUndoSym(show->GetSheetPos(this), this));
  show->undolist->EndMulti();

  for (i = 0; i < show->GetNumPoints(); i++) {
    if (show->IsSelected(i)) {
      if (pts[i].sym != sym) {
	pts[i].sym = sym;
	pts[i].cont = c->num;
	change = TRUE;
      }
    }
  }
  return change;
}

// Set point labels
Bool CC_sheet::SetPointsLabel(Bool right) {
  unsigned i;
  Bool change = FALSE;

  if (GetNumSelectedPoints() <= 0) return FALSE;

  // Create undo entry
  show->undolist->Add(new ShowUndoLbl(show->GetSheetPos(this), this));

  for (i = 0; i < show->GetNumPoints(); i++) {
    if (show->IsSelected(i)) {
      pts[i].Flip(right);
      change = TRUE;
    }
  }
  return change;
}

// Set point labels
Bool CC_sheet::SetPointsLabelFlip() {
  unsigned i;
  Bool change = FALSE;

  if (GetNumSelectedPoints() <= 0) return FALSE;

  // Create undo entry
  show->undolist->Add(new ShowUndoLbl(show->GetSheetPos(this), this));

  for (i = 0; i < show->GetNumPoints(); i++) {
    if (show->IsSelected(i)) {
      pts[i].FlipToggle();
      change = TRUE;
    }
  }
  return change;
}

// Set a point from an old format disk point
void CC_sheet::SetPoint(const cc_oldpoint& val, unsigned i) {
  pts[i].flags = (val.flags & OLD_FLAG_FLIP) ? PNT_LABEL:0;
  if ((val.sym < 111) || (val.sym > 118))
    pts[i].sym = SYMBOL_PLAIN;
  else pts[i].sym = (SYMBOL_TYPE)(val.sym - 111);
  pts[i].cont = get_lil_word(&val.cont);
  SetAllPositions(val.pos, i);
  for (unsigned j = 0; j < 3; j++) {
    // -1 means undefined (endian doesn't matter)
    if ((val.ref[j].x == 0xFFFF) && (val.ref[j].y == 0xFFFF)) {
      SetPosition(val.pos, i, j+1);
    } else {
      SetPosition(val.ref[j], i, j+1);
    }
  }
}

// Get position of point
const CC_coord& CC_sheet::GetPosition(unsigned i, unsigned ref) const {
  if (ref == 0)
    return pts[i].pos;
  else
    return pts[i].ref[ref-1];
}

// Set position of point and all refs
void CC_sheet::SetAllPositions(const CC_coord& val, unsigned i) {
  unsigned j;

  pts[i].pos = val;
  for (j = 0; j < NUM_REF_PNTS; j++) {
    pts[i].ref[j] = val;
  }
}

// Set position of point
void CC_sheet::SetPosition(const CC_coord& val, unsigned i, unsigned ref) {
  unsigned j;
  if (ref == 0) {
    for (j=0; j<NUM_REF_PNTS; j++) {
      if (pts[i].ref[j] == pts[i].pos) {
	pts[i].ref[j] = val;
      }
    }
    pts[i].pos = val;
  }
  else {
    pts[i].ref[ref-1] = val;
  }
}

// Move points
Bool CC_sheet::TranslatePoints(CC_coord delta, unsigned ref) {
  unsigned i;
  Bool change = FALSE;

  if (((delta.x == 0) && (delta.y == 0)) ||
      (GetNumSelectedPoints() <= 0)) return FALSE;

  // Create undo entry
  show->undolist->Add(new ShowUndoMove(show->GetSheetPos(this), this, ref));

  for (i = 0; i < show->GetNumPoints(); i++) {
    if (show->IsSelected(i)) {
      SetPosition(GetPosition(i, ref) + delta, i, ref);
      change = TRUE;
    }
  }
  return change;
}

// Create a new show
CC_show::CC_show(unsigned npoints)
:numpoints(npoints), numsheets(1), sheets(new CC_sheet(this, "1")),
 modified(FALSE), print_landscape(FALSE), print_do_cont(TRUE),
 print_do_cont_sheet(TRUE) {
  winlist = new CC_WinListShow(this);
  undolist = new ShowUndoList(this, undo_buffer_size);
  mode = modelist->Default();
  if (npoints) {
    pt_labels = new char[npoints][4];
    if (pt_labels == NULL) {
      // Out of mem!
      error = nomem_str;
      return;
    }
    for (unsigned int i = 0; i < npoints; i++) {
      sprintf(pt_labels[i], "%u", i);
    }
    selections = new Bool[npoints];
    if (selections == NULL) {
      // Out of mem!
      error = nomem_str;
      return;
    }
    UnselectAll();
  } else {
    pt_labels = NULL;
    selections = NULL;
  }
  sheets->animcont = new CC_continuity;
  sheets->animcont->SetName(contnames[0]);
  sheets->numanimcont = 1;

  error = NULL;
}

#define INGL_SHOW MakeINGLid('S','H','O','W')
#define INGL_SHET MakeINGLid('S','H','E','T')
#define INGL_SIZE MakeINGLid('S','I','Z','E')
#define INGL_LABL MakeINGLid('L','A','B','L')
#define INGL_MODE MakeINGLid('M','O','D','E')
#define INGL_DESC MakeINGLid('D','E','S','C')
#define INGL_NAME MakeINGLid('N','A','M','E')
#define INGL_DURA MakeINGLid('D','U','R','A')
#define INGL_POS  MakeINGLid('P','O','S',' ')
#define INGL_SYMB MakeINGLid('S','Y','M','B')
#define INGL_TYPE MakeINGLid('T','Y','P','E')
#define INGL_REFP MakeINGLid('R','E','F','P')
#define INGL_CONT MakeINGLid('C','O','N','T')
#define INGL_PCNT MakeINGLid('P','C','N','T')

// Only exists so SHOW chunk is recognized
static char* load_show_SHOW(INGLchunk*) {
  return NULL;
}

static char* load_show_SHET(INGLchunk* chunk) {
  CC_show *show = (CC_show*)chunk->prev->userdata;
  CC_sheet *sheet = new CC_sheet(show);

  show->InsertSheetInternal(sheet, show->GetNumSheets());
  chunk->userdata = sheet;

  return NULL;
}

static char* load_show_SIZE(INGLchunk* chunk) {
  CC_show *show = (CC_show*)chunk->prev->userdata;
  
  if ((show->GetNumPoints() > 0) || (show->GetSheet() != NULL)) {
    return "Duplicate SIZE chunks in file";
  }
  if (chunk->size != 4) {
    return "Bad SIZE chunk";
  }
  show->SetNumPointsInternal(get_big_long(chunk->data));
  return NULL;
}

static char* load_show_LABL(INGLchunk* chunk) {
  unsigned i;
  char *str = (char*)chunk->data;
  CC_show *show = (CC_show*)chunk->prev->userdata;
  for (i = 0; i < show->GetNumPoints(); i++) {
    strncpy(show->GetPointLabel(i), str, 3);
    show->GetPointLabel(i)[3] = 0;
    str += strlen(str)+1;
  }
  return NULL;
}

static char* load_show_MODE(INGLchunk* /*chunk*/) {
  return NULL;
}

static char* load_show_DESC(INGLchunk* chunk) {
  CC_show *show = (CC_show*)chunk->prev->userdata;

  show->SetDescr((char*)chunk->data);

  return NULL;
}

static char* load_show_NAME(INGLchunk* chunk) {
  CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
  
  sheet->SetName((char*)chunk->data);

  return NULL;
}

static char* load_show_DURA(INGLchunk* chunk) {
  CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
  
  if (chunk->size != 4) {
    return "Bad DURA chunk";
  }
  sheet->beats = get_big_long(chunk->data);

  return NULL;
}

static char* load_show_POS(INGLchunk* chunk) {
  CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
  unsigned i;
  unsigned char *data;
  CC_coord c;

  if (chunk->size != (unsigned long)sheet->show->GetNumPoints()*4) {
    return "Bad POS chunk";
  }
  data = (unsigned char*)chunk->data;
  for (i = 0; i < sheet->show->GetNumPoints(); i++) {
    c.x = get_big_word(data);
    data += 2;
    c.y = get_big_word(data);
    data += 2;
    sheet->SetAllPositions(c, i);
  }

  return NULL;
}

static char* load_show_SYMB(INGLchunk* chunk) {
  CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
  unsigned i;
  unsigned char *data;

  if (chunk->size != sheet->show->GetNumPoints()) {
    return "Bad SYMB chunk";
  }
  data = (unsigned char *)chunk->data;
  for (i = 0; i < sheet->show->GetNumPoints(); i++) {
    sheet->GetPoint(i).sym = (SYMBOL_TYPE)(*(data++));
  }

  return NULL;
}

static char* load_show_TYPE(INGLchunk* chunk) {
  CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
  unsigned i;
  unsigned char *data;

  if (chunk->size != sheet->show->GetNumPoints()) {
    return "Bad TYPE chunk";
  }
  data = (unsigned char *)chunk->data;
  for (i = 0; i < sheet->show->GetNumPoints(); i++) {
    sheet->GetPoint(i).cont = *(data++);
  }

  return NULL;
}

static char* load_show_REFP(INGLchunk* chunk) {
  CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
  unsigned i, ref;
  unsigned char *data;
  CC_coord c;

  if (chunk->size != (unsigned long)sheet->show->GetNumPoints()*4+2) {
    return "Bad REFP chunk";
  }
  data = (unsigned char*)chunk->data;
  ref = get_big_word(data);
  data += 2;
  for (i = 0; i < sheet->show->GetNumPoints(); i++) {
    c.x = get_big_word(data);
    data += 2;
    c.y = get_big_word(data);
    data += 2;
    sheet->SetPosition(c, i, ref);
  }

  return NULL;
}

static char* load_show_SHET_LABL(INGLchunk* chunk) {
  CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
  unsigned i;
  unsigned char *data;

  if (chunk->size != sheet->show->GetNumPoints()) {
    return "Bad LABL chunk";
  }
  data = (unsigned char *)chunk->data;
  for (i = 0; i < sheet->show->GetNumPoints(); i++) {
    if (*(data++)) {
      sheet->GetPoint(i).Flip();
    }
  }

  return NULL;
}

static char badcontchunk[] = "Bad CONT chunk";
static char* load_show_CONT(INGLchunk* chunk) {
  CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
  CC_continuity *newcont;
  unsigned num;
  char *name;
  char *text;

  if (chunk->size < 3) { // one byte num + two nils minimum
    return badcontchunk;
  }
  if (((char*)chunk->data)[chunk->size-1] != '\0') { // make sure we have a nil
    return badcontchunk;
  }
  name = (char *)chunk->data + 1;
  num = strlen(name);
  if (chunk->size < num + 3) { // check for room for text string
    return badcontchunk;
  }
  text = (char *)chunk->data + 2 + strlen(name);

  newcont = new CC_continuity;
  newcont->num = *((unsigned char *)chunk->data);
  newcont->SetName(name);
  newcont->SetText(text);
  sheet->AppendContinuity(newcont);

  return NULL;
}

static char* load_show_PCNT(INGLchunk* /*chunk*/) {
  return NULL;
}

static INGLhandler load_show_handlers[] = {
  { INGL_SHOW, 0, load_show_SHOW },
  { INGL_SHET, INGL_SHOW, load_show_SHET },
  { INGL_SIZE, INGL_SHOW, load_show_SIZE },
  { INGL_LABL, INGL_SHOW, load_show_LABL },
  { INGL_MODE, INGL_SHOW, load_show_MODE },
  { INGL_DESC, INGL_SHOW, load_show_DESC },
  { INGL_NAME, INGL_SHET, load_show_NAME },
  { INGL_DURA, INGL_SHET, load_show_DURA },
  { INGL_POS , INGL_SHET, load_show_POS  },
  { INGL_SYMB, INGL_SHET, load_show_SYMB },
  { INGL_TYPE, INGL_SHET, load_show_TYPE },
  { INGL_REFP, INGL_SHET, load_show_REFP },
  { INGL_LABL, INGL_SHET, load_show_SHET_LABL },
  { INGL_CONT, INGL_SHET, load_show_CONT },
  { INGL_PCNT, INGL_SHET, load_show_PCNT }
};

#define SHOWBUFSIZE 128
// Load a show
CC_show::CC_show(const char *file)
:selections(NULL), numpoints(0), numsheets(0), sheets(NULL),
 pt_labels(NULL), modified(FALSE), print_landscape(FALSE), print_do_cont(TRUE),
 print_do_cont_sheet(TRUE) {
  cc_oldpoint *diskpts;

  // These are for really old reference point format
  cc_oldpoint conv_diskpt;
  Bool reallyoldpnts;
  int record_len;

  int namelen;
  char *namebuf;
  Bool old_format = FALSE;
  Bool old_format_uppercase = FALSE;
  FILE *fp;
  unsigned int i, j, k;
  unsigned int off;
  CC_sheet *curr_sheet = NULL;
  CC_continuity *newanimcont;
  unsigned numanimcont;
  char tempbuf[SHOWBUFSIZE];
  char sheetnamebuf[SHOWBUFSIZE];

  winlist = new CC_WinListShow(this);
  undolist = new ShowUndoList(this, undo_buffer_size);
  mode = modelist->Default();
  error = NULL;

  namelen = strlen(file);
  if (namelen > 4) {
    if (strcmp(".mas", file+namelen-4) == 0) old_format = TRUE;
    if (strcmp(".MAS", file+namelen-4) == 0) {
      old_format = TRUE;
      old_format_uppercase = TRUE;
    }
  }

  if (old_format) {
    namebuf = copystring(file);
    if (!namebuf) {
      error = nomem_str;
      return;
    }
    fp = fopen(namebuf, "r");
    if (!fp) {
      error = nofile_str;
      return;
    }
    if (fgets(tempbuf, SHOWBUFSIZE, fp) == NULL) {
      error = badfile_mas_str;
      return;
    }
    if (sscanf(tempbuf, " %u ", &i) != 1) {
      error = badfile_mas_str;
      return;
    }
    if (i != 1024) {
      error = badfile_mas_str;
      return;
    }

    if (fgets(tempbuf, SHOWBUFSIZE, fp) == NULL) {
      error = badfile_mas_str;
      return;
    }
    if (sscanf(tempbuf, " %hu , %hu ", &numsheets, &numpoints) != 2) {
      error = badfile_mas_str;
      return;
    }
    pt_labels = new char[numpoints][4];
    selections = new Bool[numpoints];
    UnselectAll();

    for (i = 0; i < numsheets; i++) {
      if (fgets(tempbuf, SHOWBUFSIZE, fp) == NULL) {
	error = badfile_mas_str;
	return;
      }
      if (sscanf(tempbuf, " \"%[^\"]\" , %u , %u\n",
		 sheetnamebuf, &j, &off) != 3) {
	error = badfile_mas_str;
	return;
      }
      off = strlen(sheetnamebuf);
      while (off > 0) {
	off--;
	if (sheetnamebuf[off] != ' ') {
	  sheetnamebuf[off+1] = 0;
	  break;
	}
      }

      if (curr_sheet == NULL) {
	sheets = new CC_sheet(this, sheetnamebuf);
	if (!sheets) {
	  error = nomem_str;
	  return;
	}
	curr_sheet = sheets;
      } else {
	curr_sheet->next = new CC_sheet(this, sheetnamebuf);
	if (!sheets) {
	  error = nomem_str;
	  return;
	}
	curr_sheet = curr_sheet->next;
      }
      curr_sheet->beats = j;
    }
    fclose(fp);

    diskpts = new cc_oldpoint[numpoints];
    if (!diskpts) {
      error = nomem_str;
      return;
    }
    for (k = 1, curr_sheet = sheets;
	 k <= numsheets;
	 k++, curr_sheet = curr_sheet->next) {
      sprintf(namebuf+namelen-3, "%c%u",
	      old_format_uppercase ? 'S':'s', k);
      fp = fopen(namebuf, "rb");
      if (!fp) {
	error = nofile_str;
	return;
      }
      record_len = fread(diskpts, numpoints, sizeof(cc_oldpoint), fp);
      if (record_len == sizeof(cc_oldpoint)) {
	reallyoldpnts = FALSE;
      } else if (record_len == sizeof(cc_reallyoldpoint)) {
	reallyoldpnts = TRUE;
      } else {
	error = badfile_pnt_str;
	return;
      }
      fclose(fp);

      for (i = 0; i < numpoints; i++) {
	if (reallyoldpnts) {
	  short refidx;
	  unsigned short refloc;

	  conv_diskpt.sym = ((cc_reallyoldpoint*)diskpts)[i].sym;
	  conv_diskpt.flags = ((cc_reallyoldpoint*)diskpts)[i].flags;
	  conv_diskpt.pos = ((cc_reallyoldpoint*)diskpts)[i].pos;
	  conv_diskpt.color = ((cc_reallyoldpoint*)diskpts)[i].color;
	  conv_diskpt.code[0] = ((cc_reallyoldpoint*)diskpts)[i].code[0];
	  conv_diskpt.code[1] = ((cc_reallyoldpoint*)diskpts)[i].code[1];
	  conv_diskpt.cont = ((cc_reallyoldpoint*)diskpts)[i].cont;
	  refidx = get_lil_word(&((cc_reallyoldpoint*)diskpts)[i].refnum);
	  if (refidx >= 0) {
	    refloc =
	      get_lil_word(&((cc_reallyoldpoint*)diskpts)[refidx].pos.x) +
	      (short)get_lil_word(&((cc_reallyoldpoint*)diskpts)[i].ref.x);
	    put_lil_word(&conv_diskpt.ref[0].x, refloc);

	    refloc =
	      get_lil_word(&((cc_reallyoldpoint*)diskpts)[refidx].pos.y) +
	      (short)get_lil_word(&((cc_reallyoldpoint*)diskpts)[i].ref.y);
	    put_lil_word(&conv_diskpt.ref[0].y, refloc);
	  } else {
	    conv_diskpt.ref[0].x = 0xFFFF;
	    conv_diskpt.ref[0].x = 0xFFFF;
	  }
	  conv_diskpt.ref[1].x = 0xFFFF;
	  conv_diskpt.ref[1].x = 0xFFFF;
	  conv_diskpt.ref[2].x = 0xFFFF;
	  conv_diskpt.ref[2].x = 0xFFFF;
	  curr_sheet->SetPoint(conv_diskpt, i);
	} else {
	  curr_sheet->SetPoint(diskpts[i], i);
	}

	if (k == 1) {
	  // Build table for point labels
	  unsigned int label_idx = 0;
	  for (j = 0; j < 2; j++) {
	    // Convert to ascii
	    if ((diskpts[i].code[j] >= '0') && (diskpts[i].code[j] != '?')) {
	      if (diskpts[i].code[j] <= 'Z') {
		// normal ascii
		pt_labels[i][label_idx++] = diskpts[i].code[j];
	      } else {
		if (diskpts[i].code[j] > 100) {
		  // digit (0-9)
		  pt_labels[i][label_idx++] = diskpts[i].code[j] - 101 + '0';
		} else {
		  // '10' to '19' range
		  sprintf(&pt_labels[i][label_idx], "%u",
			  diskpts[i].code[j] - 81);
		  label_idx = strlen(pt_labels[i]);
		}
	      }
	    }
	  }
	  pt_labels[i][label_idx] = 0;
	}
      }
      // Now load animation continuity
      // We need to use fgets and sscanf so blank lines aren't skipped
      sprintf(namebuf+namelen-3, "%c%u",
	      old_format_uppercase ? 'F':'f', k);
      fp = fopen(namebuf, "r");
      if (!fp) {
	error = nofile_str;
	return;
      }
      if (my_fgets(tempbuf, SHOWBUFSIZE, fp) == NULL) {
	error = badfile_cnt_str;
	return;
      }
      if (strcmp("'NEW", tempbuf) != 0) {
	error = badanimcont_str;
	return;
      }
      if (fgets(tempbuf, SHOWBUFSIZE, fp) == NULL) {
	error = badanimcont_str;
	return;
      }
      if (sscanf(tempbuf, " %u", &numanimcont) != 1) {
	error = badanimcont_str;
	return;
      }
      for (i = 0; i < numanimcont; i++) {
	newanimcont = new CC_continuity;

	// Skip blank line
	if (fgets(tempbuf, SHOWBUFSIZE, fp) == NULL) {
	  error = badanimcont_str;
	  return;
	}
	if (fgets(tempbuf, SHOWBUFSIZE, fp) == NULL) {
	  error = badanimcont_str;
	  return;
	}
	if (sscanf(tempbuf, " \"%[^\"]\" , %u , %u\n",
		   tempbuf, &newanimcont->num, &j) != 3) {
	  error = badanimcont_str;
	  return;
	}
	newanimcont->SetName(tempbuf);
	while (j > 0) {
	  j--;
	  if (my_fgets(tempbuf, SHOWBUFSIZE, fp) == NULL) {
	    error = badanimcont_str;
	  }
	  off = strlen(tempbuf);
	  while (off > 0) {
	    off--;
	    if (tempbuf[off] != ' ') {
	      off++;
	      break;
	    }
	  }
	  tempbuf[off] = '\n';
	  tempbuf[off+1] = 0;
	  newanimcont->AppendText(tempbuf);
	}
	curr_sheet->AppendContinuity(newanimcont);
      }
      fclose(fp);
    }
    delete diskpts;
    // now load continuity file if it exists
    /* This is the format for each sheet:
     * %%str      where str is the string printed for the stuntsheet number
     * normal ascii text possibly containing the following codes:
     * \bs \be \is \ie for bold start, bold end, italics start, italics end
     * \po plainman
     * \pb backslashman
     * \ps slashman
     * \px xman
     * \so solidman
     * \sb solidbackslashman
     * \ss solidslashman
     * \sx solidxman
     * a line may begin with these symbols in order: <>~
     * < don't print continuity on individual sheets
     * > don't print continuity on master sheet
     * ~ center this line
     * also, there are three tab stops set for standard continuity format
     */
    strcpy(namebuf+namelen-3, old_format_uppercase ? "TXT":"txt");
    fp = fopen(namebuf, "r");
    if (fp) {
      cc_text *last_text, *start_text;
      cc_text *new_text;
      unsigned pos;
      Bool do_bs, do_sym, do_tab;
      Bool on_sheet, on_main, center;
      enum PSFONT_TYPE currfontnum, lastfontnum;
      char lineotext[SHOWBUFSIZE];
      char c1, c2;

      curr_sheet = NULL;
      currfontnum = lastfontnum = PSFONT_NORM;
      last_text = NULL;
      while (TRUE) {
	if (my_fgets(tempbuf, SHOWBUFSIZE, fp) == NULL) break;
	if ((tempbuf[0] == '%') && (tempbuf[1] == '%')) {
	  if (curr_sheet) curr_sheet = curr_sheet->next;
	  else curr_sheet = sheets;
	  if (!curr_sheet) break;
	  if (tempbuf[2]) {
	    curr_sheet->SetNumber(&tempbuf[2]);
	  }
	  last_text = NULL;
	} else {
	  if (curr_sheet == NULL) {
	    // Continuity doesn't begin with a sheet header
	    error = contnohead_str;
	    return;
	  }
	  on_main = TRUE;
	  on_sheet = TRUE;
	  pos = 0;
	  if (tempbuf[pos] == '<') {
	    on_sheet = FALSE;
	    pos++;
	  } else {
	    if (tempbuf[pos] == '>') {
	      on_main = FALSE;
	      pos++;
	    }
	  }
	  if (tempbuf[pos] == '~') {
	    center = TRUE;
	    pos++;
	  } else {
	    center = FALSE;
	  }
	  do_bs = FALSE;
	  do_tab = FALSE;
	  start_text = NULL;
	  do {
	    do_sym = FALSE;
	    for (i=0; tempbuf[pos] && !(do_sym); pos++) {
	      if (do_bs) {
		c1 = tolower(tempbuf[pos]);
		switch (c1) {
		case 'p':
		  do_sym = TRUE;
		  c2 = tolower(tempbuf[++pos]);
		  switch (c2) {
		  case 'o':
		    lineotext[i++] = 'A';
		    currfontnum = PSFONT_SYMBOL;
		    break;
		  case 'b':
		    lineotext[i++] = 'C';
		    currfontnum = PSFONT_SYMBOL;
		    break;
		  case 's':
		    lineotext[i++] = 'D';
		    currfontnum = PSFONT_SYMBOL;
		    break;
		  case 'x':
		    lineotext[i++] = 'E';
		    currfontnum = PSFONT_SYMBOL;
		    break;
		  default:
		    // code not recognized
		    error = badcont_str;
		    return;
		    break;
		  }
		  break;
		case 's':
		  do_sym = TRUE;
		  c2 = tolower(tempbuf[++pos]);
		  switch (c2) {
		  case 'o':
		    lineotext[i++] = 'B';
		    currfontnum = PSFONT_SYMBOL;
		    break;
		  case 'b':
		    lineotext[i++] = 'F';
		    currfontnum = PSFONT_SYMBOL;
		    break;
		  case 's':
		    lineotext[i++] = 'G';
		    currfontnum = PSFONT_SYMBOL;
		    break;
		  case 'x':
		    lineotext[i++] = 'H';
		    currfontnum = PSFONT_SYMBOL;
		    break;
		  default:
		    // code not recognized
		    error = badcont_str;
		    return;
		    break;
		  }
		  break;
		case 'b':
		  c2 = tolower(tempbuf[pos+1]);
		  switch (c2) {
		  case 's':
		    pos++;
		    switch (currfontnum) {
		    case PSFONT_NORM:
		      currfontnum = lastfontnum = PSFONT_BOLD;
		      break;
		    case PSFONT_ITAL:
		      currfontnum = lastfontnum = PSFONT_BOLDITAL;
		      break;
		    default:
		      break;
		    }
		    break;
		  case 'e':
		    pos++;
		    switch (currfontnum) {
		    case PSFONT_BOLD:
		      currfontnum = lastfontnum = PSFONT_NORM;
		      break;
		    case PSFONT_BOLDITAL:
		      currfontnum = lastfontnum = PSFONT_ITAL;
		      break;
		    default:
		      break;
		    }
		    break;
		  default:
		    // code not recognized
		    error = badcont_str;
		    return;
		    break;
		  }
		  break;
		case 'i':
		  c2 = tolower(tempbuf[pos+1]);
		  switch (c2) {
		  case 's':
		    pos++;
		    switch (currfontnum) {
		    case PSFONT_NORM:
		      currfontnum = lastfontnum = PSFONT_ITAL;
		      break;
		    case PSFONT_BOLD:
		      currfontnum = lastfontnum = PSFONT_BOLDITAL;
		      break;
		    default:
		      break;
		    }
		    break;
		  case 'e':
		    pos++;
		    switch (currfontnum) {
		    case PSFONT_ITAL:
		      currfontnum = lastfontnum = PSFONT_NORM;
		      break;
		    case PSFONT_BOLDITAL:
		      currfontnum = lastfontnum = PSFONT_BOLD;
		      break;
		    default:
		      break;
		    }
		    break;
		  default:
		    // code not recognized
		    error = badcont_str;
		    return;
		    break;
		  }
		  break;
		default:
		  lineotext[i++] = '\\';
		  lineotext[i++] = c1;
		}
		do_bs = FALSE;
	      } else {
		if (do_tab) {
		  currfontnum = PSFONT_TAB;
		  do_tab = FALSE;
		  break;
		} else {
		  if (tempbuf[pos] == '\\') {
		    do_bs = TRUE;
		    pos++;
		    break;
		  } else {
		    if (tempbuf[pos] == '\t') {
		      do_tab = TRUE;
		      pos++;
		      break;
		    } else {
		      lineotext[i++] = tempbuf[pos];
		    }
		  }
		}
	      }
	    }
	    // Add any remaining text
	    // Empty text only okay if line is blank
	    if ((i > 0) || (currfontnum == PSFONT_TAB) ||
		(!tempbuf[pos] && (start_text == NULL))) {
	      lineotext[i] = 0;
	      new_text = new cc_text;
	      if (new_text == NULL) {
		error = nomem_str;
		return;
	      }
	      if (start_text) start_text->more = new_text;
	      else {
		if (last_text) last_text->next = new_text;
		else curr_sheet->continuity = new_text;
		last_text = new_text;
	      }
	      start_text = new_text;
	      new_text->next = NULL;
	      new_text->more = NULL;
	      new_text->on_main = on_main;
	      new_text->on_sheet = on_sheet;
	      new_text->center = center;
	      new_text->font = currfontnum;
	      new_text->text = lineotext;
	    }
	    // restore to previous font (used for symbols and tabs)
	    currfontnum = lastfontnum;
	  } while (tempbuf[pos]);
	}
      }
      fclose(fp);
    }
    strcpy(namebuf+namelen-3, "shw");
    SetName(namebuf);
    delete namebuf;
  } else {
    INGLread readhnd(file);
    if (!readhnd.Okay()) {
      error = nofile_str;
    } else {
      readhnd.ParseFile(load_show_handlers,
			sizeof(load_show_handlers)/sizeof(INGLhandler),
			&error, this);
      SetName(file);
    }
  }
  if (sheets == NULL) {
    error = nosheets_str;
  }
}

// Destroy a show
CC_show::~CC_show() {
  CC_sheet *tmp;

#if 0
  fprintf(stderr, "Deleting show...\n");
#endif
  if (winlist) delete winlist;
  if (undolist) delete undolist;
  if (pt_labels) delete pt_labels;
  while (sheets) {
    tmp = sheets->next;
    delete sheets;
    sheets = tmp;
  }
}

void CC_show::Append(CC_show *shw) {
  CC_sheet *sht;

  if (numpoints == shw->GetNumPoints()) {
    if (sheets == NULL) {
      sheets = shw->sheets;
    } else {
      for (sht = sheets; sht->next != NULL; sht = sht->next);
      sht->next = shw->sheets;
    }
    numsheets += shw->numsheets;
    for (sht = shw->sheets; sht != NULL; sht = sht->next) {
      sht->show = this;
    }
    shw->sheets = NULL;
    shw->numsheets = 0;
    delete shw;
    winlist->AppendSheets();
  }
}

void CC_show::Append(CC_sheet *newsheets) {
  CC_sheet *sht;

  if (sheets == NULL) {
    sheets = newsheets;
  } else {
    for (sht = sheets; sht->next != NULL; sht = sht->next);
    sht->next = newsheets;
  }
  for (sht = newsheets; sht != NULL; sht = sht->next) {
    numsheets++;
  }
  winlist->AppendSheets();
}

char *CC_show::Save(const char *filename) {
  INGLwrite *handl = new INGLwrite(filename);
  INGLid id;
  unsigned i, j;
  Coord crd;
  unsigned char c;
  const char *str;
  CC_continuity *curranimcont;

  FlushAllTextWindows();

  if (!handl->Okay()) {
    delete handl;
    return nofile_str;
  }
  if (!handl->WriteHeader()) {
    return writeerr_str;
  }
  if (!handl->WriteGurk(INGL_SHOW)) {
    return writeerr_str;
  }

  // Handle show info
  i = GetNumPoints();
  put_big_long(&id, i);
  if (!handl->WriteChunk(INGL_SIZE, 4, &id)) {
    return writeerr_str;
  }

  id = 0;
  for (i = 0; i < GetNumPoints(); i++) {
    id += strlen(GetPointLabel(i))+1;
  }
  if (id > 0) {
    if (!handl->WriteChunkHeader(INGL_LABL, id)) {
      return writeerr_str;
    }
    for (i = 0; i < GetNumPoints(); i++) {
      if (!handl->WriteStr(GetPointLabel(i))) {
	return writeerr_str;
      }
    }
  }

  //handl->WriteChunkStr(INGL_MODE, mode->Name());

  // Description
  str = UserGetDescr();
  if (str[0] != '\0') {
    if (!handl->WriteChunkStr(INGL_DESC, str)) {
      return writeerr_str;
    }
  }

  // Handle sheets
  for (CC_sheet *curr_sheet = GetSheet();
       curr_sheet != NULL;
       curr_sheet = curr_sheet->next) {
    if (!handl->WriteGurk(INGL_SHET)) {
      return writeerr_str;
    }
    // Name
    if (!handl->WriteChunkStr(INGL_NAME, curr_sheet->GetName())) {
      return writeerr_str;
    }
    // Beats
    put_big_long(&id, curr_sheet->beats);
    if (!handl->WriteChunk(INGL_DURA, 4, &id)) {
      return writeerr_str;
    }
    
    // Point positions
    if (!handl->WriteChunkHeader(INGL_POS, GetNumPoints()*4)) {
      return writeerr_str;
    }
    for (i = 0; i < GetNumPoints(); i++) {
      put_big_word(&crd, curr_sheet->GetPosition(i).x);
      if (!handl->Write(&crd, 2)) {
	return writeerr_str;
      }
      put_big_word(&crd, curr_sheet->GetPosition(i).y);
      if (!handl->Write(&crd, 2)) {
	return writeerr_str;
      }
    }
    // Ref point positions
    for (j = 0; j < NUM_REF_PNTS; j++) {
      for (i = 0; i < GetNumPoints(); i++) {
	if (curr_sheet->GetPosition(i) != curr_sheet->GetPosition(i, j)) {
	  if (!handl->WriteChunkHeader(INGL_REFP, GetNumPoints()*4+2)) {
	    return writeerr_str;
	  }
	  put_big_word(&crd, j);
	  if (!handl->Write(&crd, 2)) {
	    return writeerr_str;
	  }
	  for (i = 0; i < GetNumPoints(); i++) {
	    put_big_word(&crd, curr_sheet->GetPosition(i, j).x);
	    if (!handl->Write(&crd, 2)) {
	      return writeerr_str;
	    }
	    put_big_word(&crd, curr_sheet->GetPosition(i, j).y);
	    if (!handl->Write(&crd, 2)) {
	      return writeerr_str;
	    }
	  }
	  break;
	}
      }
    }
    // Point symbols
    for (i = 0; i < GetNumPoints(); i++) {
      if (curr_sheet->GetPoint(i).sym != 0) {
	if (!handl->WriteChunkHeader(INGL_SYMB, GetNumPoints())) {
	  return writeerr_str;
	}
	for (i = 0; i < GetNumPoints(); i++) {
	  if (!handl->Write(&curr_sheet->GetPoint(i).sym, 1)) {
	    return writeerr_str;
	  }
	}
	break;
      }
    }
    // Point continuity types
    for (i = 0; i < GetNumPoints(); i++) {
      if (curr_sheet->GetPoint(i).cont != 0) {
	if (!handl->WriteChunkHeader(INGL_TYPE, GetNumPoints())) {
	  return writeerr_str;
	}
	for (i = 0; i < GetNumPoints(); i++) {
	  if (!handl->Write(&curr_sheet->GetPoint(i).cont, 1)) {
	    return writeerr_str;
	  }
	}
	break;
      }
    }
    // Point labels (left or right)
    for (i = 0; i < GetNumPoints(); i++) {
      if (curr_sheet->GetPoint(i).GetFlip()) {
	if (!handl->WriteChunkHeader(INGL_LABL, GetNumPoints())) {
	  return writeerr_str;
	}
	for (i = 0; i < GetNumPoints(); i++) {
	  if (curr_sheet->GetPoint(i).GetFlip()) {
	    c = TRUE;
	  } else {
	    c = FALSE;
	  }
	  if (!handl->Write(&c, 1)) {
	    return writeerr_str;
	  }
	}
	break;
      }
    }
    // Continuity text
    for (curranimcont = curr_sheet->animcont; curranimcont != NULL;
	 curranimcont  = curranimcont->next) {
      if (!handl->WriteChunkHeader(INGL_CONT,
				   1+curranimcont->name.Length()+1+
				   curranimcont->text.Length()+1)) {
	return writeerr_str;
      }
      if (!handl->Write(&curranimcont->num, 1)) {
	return writeerr_str;
      }
      if (!handl->WriteStr(curranimcont->name)) {
	return writeerr_str;
      }
      if (!handl->WriteStr(curranimcont->text)) {
	return writeerr_str;
      }
    }
    if (!handl->WriteEnd(INGL_SHET)) {
      return writeerr_str;
    }
  }

  if (!handl->WriteEnd(INGL_SHOW)) {
    return writeerr_str;
  }

  delete handl;

  return NULL;
}

const char *CC_show::UserGetName() {
  if (name.Empty()) return "Untitled";
  else return wxFileNameFromPath((char *)name.GetData());
}

void CC_show::UserSetDescr(const char *newdescr, wxWindow *win) {
  // Create undo entry
  undolist->Add(new ShowUndoDescr(this));
  descr = newdescr;
  winlist->SetDescr(win);
}

CC_sheet *CC_show::GetNthSheet(unsigned n) {
  CC_sheet *nsheet = sheets;
  while (n && nsheet) {
    n--;
    nsheet = nsheet->next;
  }
  return nsheet;
}

unsigned CC_show::GetSheetPos(CC_sheet *sheet) {
  CC_sheet *nsheet = sheets;
  unsigned n = 0;
  while (nsheet!=sheet) {
    if (nsheet == NULL) return 0;
    nsheet = nsheet->next; n++;
  }
  return n;
}

CC_sheet *CC_show::RemoveNthSheet(unsigned sheetidx) {
  CC_sheet *sht = sheets;
  CC_sheet *tmp;
  unsigned idx;

  if (sheetidx > 0) {
    idx = sheetidx;
    while (--idx) {
      sht = sht->next;
    }
    tmp = sht->next;
    sht->next = tmp->next;
    sht = tmp;
  } else {
    sheets = sheets->next;
  }
  numsheets--;
  sht->next = NULL;
  winlist->DeleteSheet(sheetidx);
  return sht;
}

CC_sheet *CC_show::RemoveLastSheets(unsigned numtoremain) {
  CC_sheet *sht = sheets;
  CC_sheet *tmp;
  unsigned idx;

  if (numtoremain > 0) {
    idx = numtoremain;
    while (--idx) {
      sht = sht->next;
    }
    tmp = sht->next;
    sht->next = NULL;
    sht = tmp;
  } else {
    sheets = NULL;
  }
  numsheets = numtoremain;
  winlist->RemoveSheets(numtoremain);
  return sht;
}

void CC_show::DeleteNthSheet(unsigned sheetidx) {
  delete RemoveNthSheet(sheetidx);
}

void CC_show::UserDeleteSheet(unsigned sheetidx) {
  CC_sheet *sht = RemoveNthSheet(sheetidx);
  undolist->Add(new ShowUndoDelete(sheetidx, sht));
}

void CC_show::InsertSheetInternal(CC_sheet *nsheet, unsigned sheetidx) {
  CC_sheet *sht = sheets;
  unsigned idx;

  if (sheetidx > 0) {
    idx = sheetidx;
    while (--idx) {
      sht = sht->next;
    }
    nsheet->next = sht->next;
    sht->next = nsheet;
  } else {
    nsheet->next = sheets;
    sheets = nsheet;
  }
  numsheets++;
}

void CC_show::InsertSheet(CC_sheet *nsheet, unsigned sheetidx) {
  InsertSheetInternal(nsheet, sheetidx);
  winlist->AddSheet(sheetidx);
}

void CC_show::UserInsertSheet(CC_sheet *sht, unsigned sheetidx) {
  InsertSheet(sht, sheetidx);
  undolist->Add(new ShowUndoCopy(sheetidx));
}

void CC_show::SetNumPoints(unsigned num, unsigned columns) {
  unsigned i, cpy;
  CC_sheet *sht;

  char (*new_labels)[4];

  for (sht = sheets; sht != NULL; sht = sht->next) {
    sht->SetNumPoints(num, columns);
  }

  undolist->EraseAll(); // Remove all previously avail undo

  delete selections;
  selections = new Bool[num];
  new_labels = new char[num][4];
  for (i = 0; i < num; i++) {
    selections[i] = FALSE;
  }
  cpy = MIN(numpoints, num);
  for (i = 0; i < cpy; i++) {
    strcpy(new_labels[i], pt_labels[i]);
  }
  for (; i < num; i++) {
    strcpy(new_labels[i], "");
  }
  delete pt_labels;
  pt_labels = new_labels;

  numpoints = num;

  SetModified(TRUE);
}

void CC_show::SetNumPointsInternal(unsigned num) {
  numpoints = num;
  pt_labels = new char[numpoints][4];
  selections = new Bool[numpoints];
  for (unsigned i = 0; i < numpoints; i++) pt_labels[i][0] = 0;
  UnselectAll();
}

Bool CC_show::RelabelSheets(unsigned sht) {
  CC_sheet *sheet;
  unsigned i,j;
  unsigned *table;
  Bool *used_table;

  sheet = GetNthSheet(sht);
  if (sheet->next == NULL) return FALSE;
  table = new unsigned[GetNumPoints()];
  used_table = new Bool[GetNumPoints()];

  for (i = 0; i < GetNumPoints(); i++) {
    used_table[i] = FALSE;
  }
  for (i = 0; i < GetNumPoints(); i++) {
    for (j = 0; j < GetNumPoints(); j++) {
      if (!used_table[j]) {
	if (sheet->GetPosition(i) == sheet->next->GetPosition(i)) {
	  table[i] = j;
	  used_table[j] = TRUE;
	  break;
	}
      }
    }
    if (j == GetNumPoints()) {
      // didn't find a match
      delete [] table;
      delete [] used_table;
      return FALSE;
    }
  }
  while ((sheet = sheet->next) != NULL) {
    sheet->RelabelSheet(table);
  }

  delete [] table;
  return TRUE;
}

Bool CC_show::UnselectAll() {
  Bool changed = FALSE;
  for (unsigned i=0; i<numpoints; i++) {
    if (IsSelected(i)) {
      Select(i, FALSE);
      changed = TRUE;
    }
  }
  return changed;
}
