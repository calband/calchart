/* print_ui.h
 * Dialox box for printing
 *
 * Modification history:
 * 7-16-95    Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1995-2008  Garrick Brian Meeker

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _PRINT_UI_H_
#define _PRINT_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <wx/wx.h>
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

class ShowPrintDialog : public wxDialog
{
public:
  ShowPrintDialog(CC_descr *dcr, CC_WinList *lst, bool printEPS,
		  wxFrame *parent, const wxString& title,
		  bool isModal = false,
		  int x = -1, int y = -1,
		  int width = -1, int height = -1);
  ~ShowPrintDialog();
  void OnCloseWindow(wxCloseEvent& event);

  inline bool Okay() { return ok; };
  void Update();

  CC_descr *show_descr;
  bool eps;
  wxTextCtrl *text_cmd, *text_opts, *text_view_cmd, *text_view_opts;
  wxTextCtrl *text_x, *text_y, *text_width, *text_height;
  wxTextCtrl *text_minyards;
  wxRadioBox *radio_orient, *radio_method;
  wxCheckBox *check_cont, *check_pages, *check_overview;
  wxFrame *frame;
  CC_WinNodePrint *node;

private:
  bool ok;

  DECLARE_EVENT_TABLE()
};

#endif
