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

ShowUndoMove::ShowUndoMove(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum), num(0) {
  unsigned i;
  
  num = sheet->GetNumSelectedPoints();
  if (num > 0) {
    elems = new ShowUndoMoveElem[num];
  }
  for (num=0,i=0; i < sheet->show->GetNumPoints(); i++) {
    if (sheet->show->IsSelected(i)) {
      elems[num].idx = i;
      elems[num].pos = sheet->pts[i].pos;
      num++;
    }
  }
}

ShowUndoMove::~ShowUndoMove() {
  if (num) {
    delete [] elems;
  }
}

int ShowUndoMove::Undo(CC_show *show)
{
  unsigned i;
  CC_sheet *sheet = show->GetNthSheet(sheetidx);
  for (i = 0; i < num; i++) {
    sheet->pts[elems[i].idx].pos = elems[i].pos;
  }
  show->winlist->UpdatePointsOnSheet(sheetidx);
  return (int)sheetidx;
}

unsigned ShowUndoMove::Size() {
  return sizeof(ShowUndoMoveElem) * num + sizeof(*this);
}

char *ShowUndoMove::UndoDescription() { return "Undo movement"; }

ShowUndoSym::ShowUndoSym(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum), num(0) {
  unsigned i;
  
  num = sheet->GetNumSelectedPoints();
  if (num > 0) {
    elems = new ShowUndoSymElem[num];
  }
  for (num=0,i=0; i < sheet->show->GetNumPoints(); i++) {
    if (sheet->show->IsSelected(i)) {
      elems[num].idx = i;
      elems[num].sym = sheet->pts[i].sym;
      elems[num].cont = sheet->pts[i].cont;
      num++;
    }
  }
}

ShowUndoSym::~ShowUndoSym() {
  if (num) {
    delete [] elems;
  }
}

int ShowUndoSym::Undo(CC_show *show)
{
  unsigned i;
  CC_sheet *sheet = show->GetNthSheet(sheetidx);
  for (i = 0; i < num; i++) {
    sheet->pts[elems[i].idx].sym = elems[i].sym;
    sheet->pts[elems[i].idx].cont = elems[i].cont;
  }
  show->winlist->UpdatePointsOnSheet(sheetidx);
  return (int)sheetidx;
}

unsigned ShowUndoSym::Size() {
  return sizeof(ShowUndoSymElem) * num + sizeof(*this);
}

char *ShowUndoSym::UndoDescription() { return "Undo symbol change"; }

ShowUndoLbl::ShowUndoLbl(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum), num(0) {
  unsigned i;
  
  num = sheet->GetNumSelectedPoints();
  if (num > 0) {
    elems = new ShowUndoLblElem[num];
  }
  for (num=0,i=0; i < sheet->show->GetNumPoints(); i++) {
    if (sheet->show->IsSelected(i)) {
      elems[num].idx = i;
      elems[num].right = sheet->pts[i].GetFlip();
      num++;
    }
  }
}

ShowUndoLbl::~ShowUndoLbl() {
  if (num) {
    delete [] elems;
  }
}

int ShowUndoLbl::Undo(CC_show *show)
{
  unsigned i;
  CC_sheet *sheet = show->GetNthSheet(sheetidx);
  for (i = 0; i < num; i++) {
    sheet->pts[elems[i].idx].Flip(elems[i].right);
  }
  show->winlist->UpdatePointsOnSheet(sheetidx);
  return (int)sheetidx;
}

unsigned ShowUndoLbl::Size() {
  return sizeof(ShowUndoLblElem) * num + sizeof(*this);
}

char *ShowUndoLbl::UndoDescription() { return "Undo label location"; }

ShowUndoCopy::ShowUndoCopy(unsigned sheetnum)
:ShowUndo(sheetnum) {
}

ShowUndoCopy::~ShowUndoCopy() {
}

int ShowUndoCopy::Undo(CC_show *show) {
  show->DeleteNthSheet(sheetidx);
  if (sheetidx > 0) {
    return (sheetidx-1);
  } else {
    return 0;
  }
}

unsigned ShowUndoCopy::Size() { return sizeof(*this); }

char *ShowUndoCopy::UndoDescription() { return "Undo copy stuntsheet"; }

ShowUndoDelete::ShowUndoDelete(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum), deleted_sheet(sheet) {
}

ShowUndoDelete::~ShowUndoDelete() {
  if (deleted_sheet) delete deleted_sheet;
}

int ShowUndoDelete::Undo(CC_show *show) {
  show->InsertSheet(deleted_sheet, sheetidx);
  deleted_sheet = NULL; // So it isn't deleted
  return (int)sheetidx;
}

unsigned ShowUndoDelete::Size() {
  return sizeof(*this) + sizeof(*deleted_sheet);
}

char *ShowUndoDelete::UndoDescription() { return "Undo delete stuntsheet"; }

ShowUndoName::ShowUndoName(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum) {
  
  name = sheet->GetName();
}

ShowUndoName::~ShowUndoName() {
}

int ShowUndoName::Undo(CC_show *show)
{
  CC_sheet *sheet = show->GetNthSheet(sheetidx);
  sheet->SetName(name);
  show->winlist->ChangeTitle(sheetidx);
  return (int)sheetidx;
}

unsigned ShowUndoName::Size() {
  return strlen(name) + sizeof(*this);
}

char *ShowUndoName::UndoDescription() { return "Undo change stuntsheet name"; }

ShowUndoBeat::ShowUndoBeat(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum), beats(sheet->beats) {}

ShowUndoBeat::~ShowUndoBeat() {}

int ShowUndoBeat::Undo(CC_show *show)
{
  CC_sheet *sheet = show->GetNthSheet(sheetidx);
  sheet->UserSetBeats(beats);
  return (int)sheetidx;
}

unsigned ShowUndoBeat::Size() {
  return sizeof(*this);
}

char *ShowUndoBeat::UndoDescription() { return "Undo set number of beats"; }

ShowUndoCont::ShowUndoCont(unsigned sheetnum, unsigned contnum,
			   CC_sheet *sheet)
:ShowUndo(sheetnum), cont(contnum) {

  conttext = sheet->GetNthContinuity(cont)->text;
}

ShowUndoCont::~ShowUndoCont() {
}

int ShowUndoCont::Undo(CC_show *show)
{
  CC_sheet *sheet = show->GetNthSheet(sheetidx);
  sheet->SetNthContinuity(conttext, cont);
  show->winlist->SetContinuity(NULL, sheetidx, cont);
  return (int)sheetidx;
}

unsigned ShowUndoCont::Size() {
  return strlen(conttext) + sizeof(*this);
}

char *ShowUndoCont::UndoDescription() { return "Undo edit continuity"; }

ShowUndoAddContinuity::ShowUndoAddContinuity(unsigned sheetnum,
					     unsigned contnum)
:ShowUndo(sheetnum), addcontnum(contnum) {
}

ShowUndoAddContinuity::~ShowUndoAddContinuity() {
}

int ShowUndoAddContinuity::Undo(CC_show *show) {
  delete show->GetNthSheet(sheetidx)->RemoveNthContinuity(addcontnum);
  return (int)sheetidx;
}

unsigned ShowUndoAddContinuity::Size() { return sizeof(*this); }

char *ShowUndoAddContinuity::UndoDescription() {
  return "Undo add continuity";
}

ShowUndoDeleteContinuity::ShowUndoDeleteContinuity(unsigned sheetnum,
						   unsigned contnum,
						   CC_continuity *cont)
:ShowUndo(sheetnum), deleted_cont(cont), delcontnum(contnum) {
}

ShowUndoDeleteContinuity::~ShowUndoDeleteContinuity() {
  if (deleted_cont) delete deleted_cont;
}

int ShowUndoDeleteContinuity::Undo(CC_show *show) {
  show->GetNthSheet(sheetidx)->InsertContinuity(deleted_cont, delcontnum);
  deleted_cont = NULL; // So it isn't deleted
  return (int)sheetidx;
}

unsigned ShowUndoDeleteContinuity::Size() {
  return sizeof(*this) + sizeof(*deleted_cont);
}

char *ShowUndoDeleteContinuity::UndoDescription() {
  return "Undo delete continuity";
}

ShowUndoDescr::ShowUndoDescr(CC_show *show)
:ShowUndo(0) {
  
  descrtext = show->GetDescr();
}

ShowUndoDescr::~ShowUndoDescr() {
}

int ShowUndoDescr::Undo(CC_show *show)
{
  show->SetDescr(descrtext);
  show->winlist->SetDescr(NULL);
  return -1; // don't goto another sheet
}

unsigned ShowUndoDescr::Size() {
  return strlen(descrtext) + sizeof(*this);
}

char *ShowUndoDescr::UndoDescription() { return "Undo edit show description"; }

ShowUndoList::ShowUndoList(CC_show *shw, int lim)
:show(shw), list(NULL), limit(lim) {}

ShowUndoList::~ShowUndoList() {
  EraseAll();
}

// Execute one undo
int ShowUndoList::Undo(CC_show *show) {
  unsigned i;
  ShowUndo *undo = Pop();

  if (undo) {
    i = undo->Undo(show);
    show->SetModified(undo->was_modified);
    delete undo;
    return (int)i;
  } else {
    return -1;
  }
}

// Add an entry to list and clean if necessary
void ShowUndoList::Add(ShowUndo *undo) {
  Push(undo);
  Clean();
}

// Return description of undo stack
char *ShowUndoList::UndoDescription() {
  if (list) {
    return list->UndoDescription();
  } else {
    return "No undo information";
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

// Remove everything from list
void ShowUndoList::EraseAll() {
  ShowUndo *undo;

  while ((undo = Pop()) != NULL) {
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
      }
      while (ptr != NULL) {
	size = ptr->Size();
	if ((total + size) <= (unsigned)limit) {
	  total += size;
	} else break;
	tmpptr = ptr;
	ptr = ptr->next;
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
  return s;
}
