/* print_ui.h
 * Dialox box for printing
 *
 * Modification history:
 * 7-16-95    Garrick Meeker              Created
 *
 */

#ifndef _PRINT_UI_H_
#define _PRINT_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <wx.h>
#include "show.h"

class ShowPrintDialog;

class CC_WinNodePrint : public CC_WinNode {
public:
  CC_WinNodePrint(CC_WinList *lst, ShowPrintDialog *req);

  virtual void SetShow(CC_show *shw);
  virtual void ChangePrint(wxWindow* win);

private:
  ShowPrintDialog *printreq;
};

class ShowPrintDialog : public wxDialogBox
{
public:
  ShowPrintDialog(CC_descr *dcr, CC_WinList *lst, Bool printEPS,
		  wxFrame *parent, char *title,
		  Bool isModal = FALSE,
		  int x = -1, int y = -1,
		  int width = -1, int height = -1);
  ~ShowPrintDialog();
  Bool OnClose(void);

  inline Bool Okay() { return ok; };
  void Update();

  CC_descr *show_descr;
  Bool eps;
  wxText *text_cmd, *text_opts, *text_view_cmd, *text_view_opts;
  wxText *text_x, *text_y, *text_width, *text_height;
  wxRadioBox *radio_orient, *radio_method;
  wxCheckBox *check_cont, *check_pages, *check_overview;
  wxFrame *frame;
  CC_WinNodePrint *node;

private:
  Bool ok;
};

#endif
