/* undo.h
 * Definitions for the undo classes
 *
 * Modification history:
 * 8-30-95    Garrick Meeker              Created
 *
 */

#ifndef _UNDO_H_
#define _UNDO_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "show.h"

class ShowUndo {
public:
  ShowUndo(unsigned sheetnum): sheetidx(sheetnum) {}
  virtual ~ShowUndo();

  // returns the sheet to go to
  virtual int Undo(CC_show *show) = 0;
  // returns amount of memory used
  virtual unsigned Size() = 0;
  // returns description of this event
  virtual char *UndoDescription() = 0;

  ShowUndo *next;
  unsigned sheetidx;
  Bool was_modified;
};

// Point movement
struct ShowUndoMoveElem {
  unsigned idx;
  CC_coord pos;
};
class ShowUndoMove : public ShowUndo {
public:
  ShowUndoMove(unsigned sheetnum, CC_sheet *sheet);
  ~ShowUndoMove();

  virtual int Undo(CC_show *show);
  virtual unsigned Size();
  virtual char *UndoDescription();
private:
  unsigned num;
  ShowUndoMoveElem *elems;
};

// Point symbol changes
struct ShowUndoSymElem {
  unsigned idx;
  unsigned char sym;
};
class ShowUndoSym : public ShowUndo {
public:
  ShowUndoSym(unsigned sheetnum, CC_sheet *sheet);
  ~ShowUndoSym();

  virtual int Undo(CC_show *show);
  virtual unsigned Size();
  virtual char *UndoDescription();
private:
  unsigned num;
  ShowUndoSymElem *elems;
};

// Point label changes
struct ShowUndoLblElem {
  unsigned idx;
  Bool right;
};
class ShowUndoLbl : public ShowUndo {
public:
  ShowUndoLbl(unsigned sheetnum, CC_sheet *sheet);
  ~ShowUndoLbl();

  virtual int Undo(CC_show *show);
  virtual unsigned Size();
  virtual char *UndoDescription();
private:
  unsigned num;
  ShowUndoLblElem *elems;
};

// Copied stuntsheet
class ShowUndoCopy : public ShowUndo {
public:
  ShowUndoCopy(unsigned sheetnum);
  ~ShowUndoCopy();

  virtual int Undo(CC_show *show);
  virtual unsigned Size();
  virtual char *UndoDescription();
};

// Deleted stuntsheet
class ShowUndoDelete : public ShowUndo {
public:
  ShowUndoDelete(unsigned sheetnum, CC_sheet *sheet);
  ~ShowUndoDelete();

  virtual int Undo(CC_show *show);
  virtual unsigned Size();
  virtual char *UndoDescription();
private:
  CC_sheet *deleted_sheet;
};

// Stuntsheet name changes
class ShowUndoName : public ShowUndo {
public:
  ShowUndoName(unsigned sheetnum, CC_sheet *sheet);
  ~ShowUndoName();

  virtual int Undo(CC_show *show);
  virtual unsigned Size();
  virtual char *UndoDescription();
private:
  wxString name;
};

// Stuntsheet # of beat changes
class ShowUndoBeat : public ShowUndo {
public:
  ShowUndoBeat(unsigned sheetnum, CC_sheet *sheet);
  ~ShowUndoBeat();

  virtual int Undo(CC_show *show);
  virtual unsigned Size();
  virtual char *UndoDescription();
private:
  unsigned short beats;
};

// Stuntsheet animation continuity changes
class ShowUndoCont : public ShowUndo {
public:
  ShowUndoCont(unsigned sheetnum, unsigned contnum, CC_sheet *sheet);
  ~ShowUndoCont();

  virtual int Undo(CC_show *show);
  virtual unsigned Size();
  virtual char *UndoDescription();
private:
  unsigned cont;
  wxString conttext;
};

// Show description changes
class ShowUndoDescr : public ShowUndo {
public:
  ShowUndoDescr(CC_show *show);
  ~ShowUndoDescr();

  virtual int Undo(CC_show *show);
  virtual unsigned Size();
  virtual char *UndoDescription();
private:
  wxString descrtext;
};

class ShowUndoList {
public:
  ShowUndoList(CC_show *shw, int lim = -1);
  ~ShowUndoList();

  int Undo(CC_show *show);
  void Add(ShowUndo *undo);
  char *UndoDescription();
  inline void Limit(int val) { limit=val; Clean(); }
  void EraseAll();

private:
  ShowUndo *Pop();
  void Push(ShowUndo *undo);
  void Clean();

  CC_show *show;
  ShowUndo *list;
  int limit;
};

#endif
