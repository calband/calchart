/* show_ui.h
 * Classes for interacting with shows
 *
 * Modification history:
 * 8-7-95     Garrick Meeker              Created
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

#ifndef _SHOW_UI_H_
#define _SHOW_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "show.h"
#include "basic_ui.h"

class PointPicker;
class ShowInfoReq;

class CC_WinNodePointPicker : public CC_WinNode
{
public:
	CC_WinNodePointPicker(CC_WinList *lst, PointPicker *req);

	virtual void SetShow(CC_show *shw);
	virtual void UpdateSelections(wxWindow* win, int point = -1);
	virtual void ChangeNumPoints(wxWindow *win);
	virtual void ChangePointLabels(wxWindow *win);

private:
	PointPicker *picker;
};

enum
{
	PointPicker_PointPickerClose=1100,
	PointPicker_PointPickerAll,
	PointPicker_PointPickerNone,
};

class PointPicker : public wxFrame
{
public:
	PointPicker(CC_show *shw, CC_WinList *lst,
		bool multi, wxFrame *frame, const wxString& title,
		int x = -1, int y = -1, int width = 250, int height = 300);
	~PointPicker();
	void OnCloseWindow(wxCloseEvent& event);

	inline bool Okay() { return ok; };

	inline bool Get(unsigned n) { return list->IsSelected(n); }
	inline void Set(unsigned n, bool v = true)
	{
		list->SetSelection(n,v);
		show->Select(n,v);
	}
	void Update();
	void UpdateSelections();

	CC_show *show;
private:
	void SetListBoxEntries();
	void PointPickerClose(wxCommandEvent&);
	void PointPickerAll(wxCommandEvent&);
	void PointPickerNone(wxCommandEvent&);

	bool ok;
	wxPanel *panel;
	wxListBox *list;
	CC_WinNodePointPicker *node;

	DECLARE_EVENT_TABLE()
};

class ShowInfoReq : public wxDialog
{
	DECLARE_CLASS( ShowInfoReq )
	DECLARE_EVENT_TABLE()

public:
	ShowInfoReq();
	ShowInfoReq(CC_show *shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Show Info"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~ShowInfoReq( );

	void Init();

	bool Create(CC_show *shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Show Info"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

	void UpdateLabels();
	void UpdateNumPoints();
	void UpdateMode();
	void UpdateDescr();	  // quick doesn't flush other windows

	void FlushDescr();						  // Flush changes in description text window

	inline wxString GetChoiceStrSelection() { return choice->GetStringSelection(); }
	inline int GetChoiceSelection() { return choice->GetSelection(); }
	unsigned GetNumPoints();
	unsigned GetColumns();
	inline int GetLabelType() { return label_type->GetSelection(); }
	inline int GetLetterSize() { return lettersize->GetValue(); }
	inline bool GetLetter(unsigned i) { return labels->IsSelected(i); }
	void SetLabels();

	CC_show *show;
private:
	wxPanel *panel;
	void ShowInfoSetNum(wxCommandEvent&);
	void ShowInfoSetLabels(wxCommandEvent&);
	void ShowInfoModeChoice(wxCommandEvent&);
	void ShowInfoSetDescription(wxCommandEvent&);
	wxTextCtrl *numpnts;
	wxRadioBox *label_type;
	wxListBox *labels;
	wxChoice *choice;
	wxSlider *lettersize;
	FancyTextWin *text;
};
#endif
