/* undo.cc
 * Handle wxWindows interface
 *
 * Modification history:
 * 8-30-95    Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "undo.h"
#include <wx_utils.h>

ShowUndo::~ShowUndo() {}

ShowUndoMany::ShowUndoMany(ShowUndo *undolist)
  :ShowUndo(0), list(undolist) {}

ShowUndoMany::~ShowUndoMany() {
  ShowUndo *next;

  while (list) {
    next = list->next;
    delete list;
    list = next;
  }
}

int ShowUndoMany::Undo(CC_show *show, ShowUndo** newundo) {
  ShowUndo *undo;
  ShowUndo *newnode;
  ShowUndo *newlist;
  int i;

  newlist = newnode = NULL;
  i = -1;
  for (undo = list; undo != NULL; undo = undo->next) {
    if (newlist) {
      undo->Undo(show, &newnode->next);
      if (newnode->next) newnode = newnode->next;
    } else {
      i = undo->Undo(show, &newlist);
      newnode = newlist;
    }
  }
  *newundo = new ShowUndoMany(newlist);
  return i;
}

unsigned ShowUndoMany::Size() {
  ShowUndo *undo;
  unsigned i;

  for (i = sizeof(*this), undo = list; undo != NULL; undo = undo->next) {
    i += undo->Size();
  }
  return i;
}

char *ShowUndoMany::UndoDescription() {
  if (list) return list->UndoDescription();
  else return NULL;
}
char *ShowUndoMany::RedoDescription() {
  if (list) return list->RedoDescription();
  else return NULL;
}

ShowUndoMove::ShowUndoMove(unsigned sheetnum, CC_sheet *sheet, unsigned ref)
:ShowUndo(sheetnum), refnum(ref) {
  unsigned i, j;
  
  num = sheet->GetNumSelectedPoints();
  if (num > 0) {
    elems = new ShowUndoMoveElem[num];
  }
  for (num=0,i=0; i < sheet->show->GetNumPoints(); i++) {
    if (sheet->show->IsSelected(i)) {
      elems[num].idx = i;
      elems[num].pos = sheet->GetPosition(i, refnum);
      elems[num].refmask = 1<<ref;
      if (refnum == 0) for (j = 1; j <= NUM_REF_PNTS; j++) {
	if (sheet->GetPosition(i, j) == sheet->GetPosition(i, 0)) {
	  elems[num].refmask |= 1<<j;
	}
      }
      num++;
    }
  }
}

ShowUndoMove::ShowUndoMove(ShowUndoMove* old, CC_sheet *sheet)
:ShowUndo(old->sheetidx), num(old->num), refnum(old->refnum) {
  unsigned i;

  if (num > 0) {
    elems = new ShowUndoMoveElem[num];
  }
  for (i = 0; i < num; i++) {
    elems[i].idx = old->elems[i].idx;
    elems[i].refmask = old->elems[i].refmask;
    elems[i].pos = sheet->GetPosition(elems[i].idx, refnum);
  }
}
    
ShowUndoMove::~ShowUndoMove() {
  if (num) {
    delete [] elems;
  }
}

int ShowUndoMove::Undo(CC_show *show, ShowUndo** newundo)
{
  unsigned i, j;
  CC_sheet *sheet = show->GetNthSheet(sheetidx);

  *newundo = new ShowUndoMove(this, sheet);
  for (i = 0; i < num; i++) {
    for (j = 0; j <= NUM_REF_PNTS; j++) {
      if (elems[i].refmask & (1<<j)) {
	sheet->SetPositionQuick(elems[i].pos, elems[i].idx, j);
      }
    }
  }
  show->winlist->UpdatePointsOnSheet(sheetidx, refnum);
  return (int)sheetidx;
}

unsigned ShowUndoMove::Size() {
  return sizeof(ShowUndoMoveElem) * num + sizeof(*this);
}

char *ShowUndoMove::UndoDescription() { return "Undo movement"; }
char *ShowUndoMove::RedoDescription() { return "Redo movement"; }

ShowUndoSym::ShowUndoSym(unsigned sheetnum, CC_sheet *sheet, Bool contchanged)
:ShowUndo(sheetnum), contchange(contchanged) {
  unsigned i;
  
  num = sheet->GetNumSelectedPoints();
  if (num > 0) {
    elems = new ShowUndoSymElem[num];
  }
  for (num=0,i=0; i < sheet->show->GetNumPoints(); i++) {
    if (sheet->show->IsSelected(i)) {
      elems[num].idx = i;
      elems[num].sym = sheet->GetPoint(i).sym;
      elems[num].cont = sheet->GetPoint(i).cont;
      num++;
    }
  }
}

ShowUndoSym::ShowUndoSym(ShowUndoSym* old, CC_sheet *sheet)
:ShowUndo(old->sheetidx), num(old->num), contchange(old->contchange) {
  unsigned i;

  if (num > 0) {
    elems = new ShowUndoSymElem[num];
  }
  for (i = 0; i < num; i++) {
    elems[i].idx = old->elems[i].idx;
    elems[i].sym = sheet->GetPoint(elems[i].idx).sym;
    elems[i].cont = sheet->GetPoint(elems[i].idx).cont;
  }
}
    
ShowUndoSym::~ShowUndoSym() {
  if (num) {
    delete [] elems;
  }
}

int ShowUndoSym::Undo(CC_show *show, ShowUndo** newundo)
{
  unsigned i;
  CC_sheet *sheet = show->GetNthSheet(sheetidx);

  *newundo = new ShowUndoSym(this, sheet);
  for (i = 0; i < num; i++) {
    sheet->GetPoint(elems[i].idx).sym = elems[i].sym;
    sheet->GetPoint(elems[i].idx).cont = elems[i].cont;
  }
  show->winlist->UpdatePointsOnSheet(sheetidx);
  return (int)sheetidx;
}

unsigned ShowUndoSym::Size() {
  return sizeof(ShowUndoSymElem) * num + sizeof(*this);
}

char *ShowUndoSym::UndoDescription() {
  return contchange ? "Undo continuity assignment" : "Undo symbol change";
}
char *ShowUndoSym::RedoDescription() {
  return contchange ? "Redo continuity assignment" : "Redo symbol change";
}

ShowUndoLbl::ShowUndoLbl(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum) {
  unsigned i;
  
  num = sheet->GetNumSelectedPoints();
  if (num > 0) {
    elems = new ShowUndoLblElem[num];
  }
  for (num=0,i=0; i < sheet->show->GetNumPoints(); i++) {
    if (sheet->show->IsSelected(i)) {
      elems[num].idx = i;
      elems[num].right = sheet->GetPoint(i).GetFlip();
      num++;
    }
  }
}

ShowUndoLbl::ShowUndoLbl(ShowUndoLbl* old, CC_sheet *sheet)
:ShowUndo(old->sheetidx), num(old->num) {
  unsigned i;

  if (num > 0) {
    elems = new ShowUndoLblElem[num];
  }
  for (i = 0; i < num; i++) {
    elems[i].idx = old->elems[i].idx;
    elems[i].right = sheet->GetPoint(elems[i].idx).GetFlip();
  }
}
    
ShowUndoLbl::~ShowUndoLbl() {
  if (num) {
    delete [] elems;
  }
}

int ShowUndoLbl::Undo(CC_show *show, ShowUndo** newundo)
{
  unsigned i;
  CC_sheet *sheet = show->GetNthSheet(sheetidx);

  *newundo = new ShowUndoLbl(this, sheet);
  for (i = 0; i < num; i++) {
    sheet->GetPoint(elems[i].idx).Flip(elems[i].right);
  }
  show->winlist->UpdatePointsOnSheet(sheetidx);
  return (int)sheetidx;
}

unsigned ShowUndoLbl::Size() {
  return sizeof(ShowUndoLblElem) * num + sizeof(*this);
}

char *ShowUndoLbl::UndoDescription() { return "Undo label location"; }
char *ShowUndoLbl::RedoDescription() { return "Redo label location"; }

ShowUndoCopy::ShowUndoCopy(unsigned sheetnum)
:ShowUndo(sheetnum) {
}

ShowUndoCopy::~ShowUndoCopy() {
}

int ShowUndoCopy::Undo(CC_show *show, ShowUndo** newundo) {
  CC_sheet *sheet = show->RemoveNthSheet(sheetidx);
  *newundo = new ShowUndoDelete(sheetidx, sheet);
  if (sheetidx > 0) {
    return (sheetidx-1);
  } else {
    return 0;
  }
}

unsigned ShowUndoCopy::Size() { return sizeof(*this); }

char *ShowUndoCopy::UndoDescription() { return "Undo copy stuntsheet"; }
char *ShowUndoCopy::RedoDescription() { return "Redo delete stuntsheet"; }

ShowUndoDelete::ShowUndoDelete(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum), deleted_sheet(sheet) {
}

ShowUndoDelete::~ShowUndoDelete() {
  if (deleted_sheet) delete deleted_sheet;
}

int ShowUndoDelete::Undo(CC_show *show, ShowUndo** newundo) {
  show->InsertSheet(deleted_sheet, sheetidx);
  *newundo = new ShowUndoCopy(sheetidx);
  deleted_sheet = NULL; // So it isn't deleted
  return (int)sheetidx;
}

unsigned ShowUndoDelete::Size() {
  return sizeof(*this) + sizeof(*deleted_sheet);
}

char *ShowUndoDelete::UndoDescription() { return "Undo delete stuntsheet"; }
char *ShowUndoDelete::RedoDescription() { return "Redo copy stuntsheet"; }

ShowUndoAppendSheets::ShowUndoAppendSheets(unsigned sheetnum)
:ShowUndo(sheetnum) {
}

ShowUndoAppendSheets::~ShowUndoAppendSheets() {
}

int ShowUndoAppendSheets::Undo(CC_show *show, ShowUndo** newundo) {
  CC_sheet *sheet = show->RemoveLastSheets(sheetidx);
  *newundo = new ShowUndoDeleteAppendSheets(sheet);
  return -1;
}

unsigned ShowUndoAppendSheets::Size() { return sizeof(*this); }

char *ShowUndoAppendSheets::UndoDescription() { return "Undo append sheets"; }
char *ShowUndoAppendSheets::RedoDescription() { return ""; }

ShowUndoDeleteAppendSheets::ShowUndoDeleteAppendSheets(CC_sheet *sheet)
:ShowUndo(0), deleted_sheets(sheet) {
}

ShowUndoDeleteAppendSheets::~ShowUndoDeleteAppendSheets() {
  CC_sheet *sheet;

  while (deleted_sheets) {
    sheet = deleted_sheets->next;
    delete deleted_sheets;
    deleted_sheets = sheet;
  }
}

int ShowUndoDeleteAppendSheets::Undo(CC_show *show, ShowUndo** newundo) {
  *newundo = new ShowUndoAppendSheets(show->GetNumSheets());
  show->Append(deleted_sheets);
  deleted_sheets = NULL; // So they aren't deleted
  return -1;
}

unsigned ShowUndoDeleteAppendSheets::Size() {
  unsigned i;
  CC_sheet *sheet;

  for (i = sizeof(*this), sheet = deleted_sheets;
       sheet != NULL;
       sheet = sheet->next) {
    i += sizeof(deleted_sheets);
  }
  return i;
}

char *ShowUndoDeleteAppendSheets::UndoDescription() { return ""; }
char *ShowUndoDeleteAppendSheets::RedoDescription() { return "Redo append sheets"; }

ShowUndoName::ShowUndoName(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum) {
  
  name = sheet->GetName();
}

ShowUndoName::~ShowUndoName() {
}

int ShowUndoName::Undo(CC_show *show, ShowUndo** newundo)
{
  CC_sheet *sheet = show->GetNthSheet(sheetidx);

  *newundo = new ShowUndoName(sheetidx, sheet);
  sheet->SetName(name);
  show->winlist->ChangeTitle(sheetidx);
  return (int)sheetidx;
}

unsigned ShowUndoName::Size() {
  return strlen(name) + sizeof(*this);
}

char *ShowUndoName::UndoDescription() { return "Undo change stuntsheet name"; }
char *ShowUndoName::RedoDescription() { return "Redo change stuntsheet name"; }

ShowUndoBeat::ShowUndoBeat(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum), beats(sheet->beats) {}

ShowUndoBeat::~ShowUndoBeat() {}

int ShowUndoBeat::Undo(CC_show *show, ShowUndo** newundo)
{
  CC_sheet *sheet = show->GetNthSheet(sheetidx);

  *newundo = new ShowUndoBeat(sheetidx, sheet);
  sheet->UserSetBeats(beats);
  return (int)sheetidx;
}

unsigned ShowUndoBeat::Size() {
  return sizeof(*this);
}

char *ShowUndoBeat::UndoDescription() { return "Undo set number of beats"; }
char *ShowUndoBeat::RedoDescription() { return "Redo set number of beats"; }

ShowUndoCont::ShowUndoCont(unsigned sheetnum, unsigned contnum,
			   CC_sheet *sheet)
:ShowUndo(sheetnum), cont(contnum) {

  conttext = sheet->GetNthContinuity(cont)->text;
}

ShowUndoCont::~ShowUndoCont() {
}

int ShowUndoCont::Undo(CC_show *show, ShowUndo** newundo)
{
  CC_sheet *sheet = show->GetNthSheet(sheetidx);

  *newundo = new ShowUndoCont(sheetidx, cont, sheet);
  sheet->SetNthContinuity(conttext, cont);
  show->winlist->SetContinuity(NULL, sheetidx, cont);
  return (int)sheetidx;
}

unsigned ShowUndoCont::Size() {
  return strlen(conttext) + sizeof(*this);
}

char *ShowUndoCont::UndoDescription() { return "Undo edit continuity"; }
char *ShowUndoCont::RedoDescription() { return "Redo edit continuity"; }

ShowUndoAddContinuity::ShowUndoAddContinuity(unsigned sheetnum,
					     unsigned contnum)
:ShowUndo(sheetnum), addcontnum(contnum) {
}

ShowUndoAddContinuity::~ShowUndoAddContinuity() {
}

int ShowUndoAddContinuity::Undo(CC_show *show, ShowUndo** newundo) {
  CC_continuity *cont;

  cont = show->GetNthSheet(sheetidx)->RemoveNthContinuity(addcontnum);
  *newundo = new ShowUndoDeleteContinuity(sheetidx, addcontnum, cont);
  return (int)sheetidx;
}

unsigned ShowUndoAddContinuity::Size() { return sizeof(*this); }

char *ShowUndoAddContinuity::UndoDescription() {
  return "Undo add continuity";
}
char *ShowUndoAddContinuity::RedoDescription() {
  return "Redo delete continuity";
}

ShowUndoDeleteContinuity::ShowUndoDeleteContinuity(unsigned sheetnum,
						   unsigned contnum,
						   CC_continuity *cont)
:ShowUndo(sheetnum), deleted_cont(cont), delcontnum(contnum) {
}

ShowUndoDeleteContinuity::~ShowUndoDeleteContinuity() {
  if (deleted_cont) delete deleted_cont;
}

int ShowUndoDeleteContinuity::Undo(CC_show *show, ShowUndo** newundo) {
  show->GetNthSheet(sheetidx)->InsertContinuity(deleted_cont, delcontnum);
  *newundo = new ShowUndoAddContinuity(sheetidx, delcontnum);
  deleted_cont = NULL; // So it isn't deleted
  return (int)sheetidx;
}

unsigned ShowUndoDeleteContinuity::Size() {
  return sizeof(*this) + sizeof(*deleted_cont);
}

char *ShowUndoDeleteContinuity::UndoDescription() {
  return "Undo delete continuity";
}
char *ShowUndoDeleteContinuity::RedoDescription() {
  return "Redo add continuity";
}

ShowUndoDescr::ShowUndoDescr(CC_show *show)
:ShowUndo(0) {
  
  descrtext = show->GetDescr();
}

ShowUndoDescr::~ShowUndoDescr() {
}

int ShowUndoDescr::Undo(CC_show *show, ShowUndo** newundo)
{
  *newundo = new ShowUndoDescr(show);
  show->SetDescr(descrtext);
  show->winlist->SetDescr(NULL);
  return -1; // don't goto another sheet
}

unsigned ShowUndoDescr::Size() {
  return strlen(descrtext) + sizeof(*this);
}

char *ShowUndoDescr::UndoDescription() { return "Undo edit show description"; }
char *ShowUndoDescr::RedoDescription() { return "Redo edit show description"; }

ShowUndoList::ShowUndoList(CC_show *shw, int lim)
:show(shw), list(NULL), redolist(NULL), limit(lim) {}

ShowUndoList::~ShowUndoList() {
  EraseAll();
}

// Execute one undo
int ShowUndoList::Undo(CC_show *show) {
  unsigned i;
  ShowUndo *undo = Pop();
  ShowUndo *redo;

  if (undo) {
    redo = NULL;
    i = undo->Undo(show, &redo);
    if (redo) PushRedo(redo);
    show->SetModified(undo->was_modified);
    delete undo;
    return (int)i;
  } else {
    return -1;
  }
}

// Execute one redo
int ShowUndoList::Redo(CC_show *show) {
  unsigned i;
  ShowUndo *undo = PopRedo();
  ShowUndo *newundo;

  if (undo) {
    newundo = NULL;
    i = undo->Undo(show, &newundo);
    if (newundo) Push(newundo);
    show->SetModified(TRUE);
    delete undo;
    return (int)i;
  } else {
    return -1;
  }
}

// Add an entry to list and clean if necessary
void ShowUndoList::Add(ShowUndo *undo) {
  EraseAllRedo();
  Push(undo);
  Clean();
}

// Prepare to add multiple actions
void ShowUndoList::StartMulti() {
  oldlist = list;
}

// End adding multiple actions
void ShowUndoList::EndMulti() {
  ShowUndo *newlist, *tmplist;

  newlist = NULL;
  for (tmplist = list; tmplist != NULL; tmplist = tmplist->next) {
    if (tmplist->next == oldlist) {
      newlist = list;
      list = oldlist;
      tmplist->next = NULL;
      break;
    }
  }
  if (newlist) {
    Add(new ShowUndoMany(newlist));
  }
}

// Return description of undo stack
char *ShowUndoList::UndoDescription() {
  if (list) {
    return list->UndoDescription();
  } else {
    return "No undo information";
  }
}

// Return description of redo stack
char *ShowUndoList::RedoDescription() {
  if (redolist) {
    return redolist->RedoDescription();
  } else {
    return "No redo information";
  }
}

// Pop one entry off list
ShowUndo *ShowUndoList::Pop() {
  ShowUndo *undo;

  undo = list;
  if (undo) {
    list = list->next;
  }
  return undo;
}

// Push one entry onto list
void ShowUndoList::Push(ShowUndo *undo) {
  undo->next = list;
  undo->was_modified = show->Modified();
  show->SetModified(TRUE); // Show is now modified
  list = undo;
}

// Pop one entry off list
ShowUndo *ShowUndoList::PopRedo() {
  ShowUndo *undo;

  undo = redolist;
  if (undo) {
    redolist = redolist->next;
  }
  return undo;
}

// Push one entry onto list
void ShowUndoList::PushRedo(ShowUndo *undo) {
  undo->next = redolist;
  undo->was_modified = TRUE; // Show will be modified
  redolist = undo;
}

// Remove everything from list
void ShowUndoList::EraseAll() {
  ShowUndo *undo;

  EraseAllRedo();
  while ((undo = Pop()) != NULL) {
    delete undo;
  }
}

// Remove everything from list
void ShowUndoList::EraseAllRedo() {
  ShowUndo *undo;

  while ((undo = PopRedo()) != NULL) {
    delete undo;
  }
}

// Make sure list memory requirements
void ShowUndoList::Clean() {
  ShowUndo *ptr, *tmpptr;
  unsigned size, total;

  // < 0 means no limit
  // 0 means no list
  // > 0 means try to make total less than or equal to limit, but always
  //     allow one event
  ptr = list;
  if (ptr != NULL) {
    if (limit >= 0) {
      if (limit > 0) {
	total = ptr->Size();
	tmpptr = ptr;
	ptr = ptr->next;
      } else {
	total = 0;
	tmpptr = NULL;
	list = NULL;
      }
      while (ptr != NULL) {
	size = ptr->Size();
	if ((total + size) <= (unsigned)limit) {
	  total += size;
	  tmpptr = ptr;
	  ptr = ptr->next;
	} else break;
      }
      if (tmpptr != NULL) {
	tmpptr->next = NULL;
      }
      // Delete rest of list
      while (ptr != NULL) {
	tmpptr = ptr->next;
	delete ptr;
	ptr = tmpptr;
      }
    }
  }
}

ostream& operator<< (ostream& s, const ShowUndoList& list) {
  ShowUndo *elem;

  elem = list.list;
  while (elem) {
    s << elem->UndoDescription() << endl;
    elem = elem->next;
  }
  s << "*** End of undo stack." << endl;
  elem = list.redolist;
  while (elem) {
    s << elem->RedoDescription() << endl;
    elem = elem->next;
  }
  s << "*** End of redo stack." << endl;
  return s;
}
