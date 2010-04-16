/* show_ui.cpp
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

#ifdef __GNUG__
#pragma implementation
#endif

#include "show_ui.h"
#include "modes.h"
#include <ctype.h>

extern ShowModeList *modelist;

BEGIN_EVENT_TABLE(PointPicker, wxFrame)
EVT_CLOSE(PointPicker::OnCloseWindow)
EVT_BUTTON(PointPicker_PointPickerClose,PointPicker::PointPickerClose)
EVT_BUTTON(PointPicker_PointPickerAll,PointPicker::PointPickerAll)
EVT_BUTTON(PointPicker_PointPickerNone,PointPicker::PointPickerNone)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(ShowInfoReq, wxFrame)
EVT_CLOSE(ShowInfoReq::OnCloseWindow)
EVT_BUTTON(ShowInfoReq_ShowInfoClose,ShowInfoReq::ShowInfoClose)
EVT_BUTTON(ShowInfoReq_ShowInfoSetNum,ShowInfoReq::ShowInfoSetNum)
EVT_BUTTON(ShowInfoReq_ShowInfoSetLabels,ShowInfoReq::ShowInfoSetLabels)
EVT_CHOICE(ShowInfoReq_ShowInfoModeChoice,ShowInfoReq::ShowInfoModeChoice)
END_EVENT_TABLE()

CC_WinNodePointPicker::CC_WinNodePointPicker(CC_WinList *lst,
PointPicker *req)
: CC_WinNode(lst), picker(req) {}

void CC_WinNodePointPicker::SetShow(CC_show *shw)
{
	picker->show = shw;
	picker->Update();
}


void CC_WinNodePointPicker::ChangeNumPoints(wxWindow*)
{
	picker->Update();
}


void CC_WinNodePointPicker::ChangePointLabels(wxWindow*)
{
	picker->Update();
}


void CC_WinNodePointPicker::UpdateSelections(wxWindow *win, int point)
{
	if (win != picker)
	{
		if (point < 0)
		{
			picker->UpdateSelections();
		}
		else
		{
			picker->Set(point, picker->show->IsSelected(point));
		}
	}
}


CC_WinNodeInfo::CC_WinNodeInfo(CC_WinList *lst, ShowInfoReq *req)
: CC_WinNode(lst), inforeq(req) {}

void CC_WinNodeInfo::SetShow(CC_show *shw)
{
	inforeq->Update(true, shw);
}


void CC_WinNodeInfo::ChangeNumPoints(wxWindow *win)
{
	if (win != inforeq)
	{
// make sure changes came from a different window
		inforeq->UpdateNumPoints();
	}
}


void CC_WinNodeInfo::ChangePointLabels(wxWindow *win)
{
	if (win != inforeq)
	{
// make sure changes came from a different window
		inforeq->UpdateLabels();
	}
}


void CC_WinNodeInfo::ChangeShowMode(wxWindow *win)
{
	if (win != inforeq)
	{
// make sure changes came from a different window
		inforeq->UpdateMode();
	}
}


void CC_WinNodeInfo::FlushDescr()
{
	inforeq->FlushDescr();
}


void CC_WinNodeInfo::SetDescr(wxWindow* win)
{
	if (win != inforeq)
	{
		inforeq->UpdateDescr(true);
	}
}


void PointPicker::PointPickerClose(wxCommandEvent&)
{
	Close();
}


void PointPicker::PointPickerAll(wxCommandEvent&)
{
	for (unsigned i=0; i < show->GetNumPoints(); i++)
	{
		Set(i, true);
	}
	show->winlist->UpdateSelections(this);
}


void PointPicker::PointPickerNone(wxCommandEvent&)
{
	for (unsigned i=0; i < show->GetNumPoints(); i++)
	{
		Set(i, false);
	}
	show->winlist->UpdateSelections(this);
}


PointPicker::PointPicker(CC_show *shw, CC_WinList *lst,
bool multi, wxFrame *frame,
const wxString& title,
int x, int y, int width, int height):
wxFrame(frame, -1, title, wxPoint(x, y), wxSize(width, height)),
show(shw)
{
// Give it an icon
	SetBandIcon(this);

	panel = new wxPanel(this);

// create a sizer for laying things out top down:
	wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

// add buttons to the top row
	wxBoxSizer *top_button_sizer = new wxBoxSizer( wxHORIZONTAL );
	wxButton *closeBut = new wxButton(panel, PointPicker_PointPickerClose, wxT("&Close"));
	closeBut->SetDefault();
	top_button_sizer->Add(closeBut, 0, wxALL, 5 );

	if (multi)
	{
		wxButton *setnumBut = new wxButton(panel, PointPicker_PointPickerAll, wxT("&All"));
		top_button_sizer->Add(setnumBut, 0, wxALL, 5 );

		wxButton *setlabBut = new wxButton(panel, PointPicker_PointPickerNone, wxT("&None"));
		top_button_sizer->Add(setlabBut, 0, wxALL, 5 );
	}
	topsizer->Add(top_button_sizer, 0, wxALIGN_CENTER );

	list = new wxListBox(panel, -1, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED);
	topsizer->Add(list, 0, wxALL, 0 );
	SetListBoxEntries();

	node = new CC_WinNodePointPicker(lst, this);

	panel->SetSizer( topsizer );

	Center();
	Show(true);
}


PointPicker::~PointPicker()
{
	if (node)
	{
		node->Remove();
		delete node;
	}
}


void PointPicker::OnCloseWindow(wxCloseEvent& event)
{
	Destroy();
}


void PointPicker::Update()
{
	list->Clear();
	SetListBoxEntries();
}


void PointPicker::UpdateSelections()
{
	unsigned n;

	for (n = 0; n < show->GetNumPoints(); n++)
	{
		list->SetSelection(n, show->IsSelected(n));
	}
}


void PointPicker::SetListBoxEntries()
{
	unsigned n;

	list->Set(show->GetNumPoints(), show->GetPointLabels());
	for (n = 0; n < show->GetNumPoints(); n++)
	{
		list->SetSelection(n, show->IsSelected(n));
	}
}


void ShowInfoReq::ShowInfoClose(wxCommandEvent& )
{
	Close();
}


void ShowInfoReq::ShowInfoSetNum(wxCommandEvent& )
{
	unsigned num;

	num = GetNumPoints();
	if (num != show->GetNumPoints())
	{
		if(wxMessageBox(wxT("Changing the number of points is not undoable.\nProceed?"),
			wxT("Change number of points"), wxYES_NO) == wxYES)
		{
			if (num > show->GetNumPoints())
				if (num > 10)
					show->SetNumPoints(num, GetColumns());
			else
				show->SetNumPoints(num, 10);
			else
				show->SetNumPoints(num, 1);
			SetLabels();
			show->winlist->ChangeNumPoints(this);
		}
	}
}


void ShowInfoReq::ShowInfoSetLabels(wxCommandEvent&)
{
	SetLabels();
	show->winlist->ChangePointLabels(this);
}


void ShowInfoReq::ShowInfoModeChoice(wxCommandEvent&)
{
	ShowMode *newmode;

	newmode = modelist->Find(GetChoiceStrSelection());
	if (newmode)
	{
		show->SetMode(newmode);
		show->winlist->ChangeShowMode(this);
	}
}


static void CalculateLabels(CC_show *show, bool letters[26],
bool& use_letters, int& maxnum)
{
	unsigned i;

	for (i = 0; i < 26; i++) letters[i] = false;
	use_letters = false;
	maxnum = 1;
	for (i = 0; i < show->GetNumPoints(); i++)
	{
		const wxString& tmp = show->GetPointLabel(i);
		if (!isdigit(tmp[0]))
		{
			letters[tmp[0]-'A'] = true;
			if ((tmp[1]-'0') >= maxnum)
				maxnum = tmp[1]-'0'+1;
			use_letters = true;
		}
	}
	if (use_letters == false)
	{
		maxnum = 10;
		for (i = 0; i < 26; i++) letters[i] = true;
	}
}


ShowInfoReq::ShowInfoReq(CC_show *shw, CC_WinList *lst,
wxFrame *frame, const wxString& title,
int x, int y, int width, int height):
wxFrame(frame, -1, title, wxPoint(x, y), wxSize(width, height)),
show(shw)
{
	unsigned i;
	wxString buf;
	wxString strs[2];
	bool letters[26];
	bool use_letters;
	int maxnum;
	wxStaticBoxSizer *boxsizer;

// Give it an icon
	SetBandIcon(this);

	CalculateLabels(show, letters, use_letters, maxnum);

	panel = new wxPanel(this, -1);

// create a sizer for laying things out top down:
	wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

// add buttons to the top row
	wxBoxSizer *top_button_sizer = new wxBoxSizer( wxHORIZONTAL );
	wxButton *closeBut = new wxButton(panel, ShowInfoReq_ShowInfoClose, wxT("&Close"));
	closeBut->SetDefault();
	top_button_sizer->Add(closeBut, 0, wxALL, 10 );

	wxButton *setnumBut = new wxButton(panel, ShowInfoReq_ShowInfoSetNum, wxT("Set &Num Points"));
	top_button_sizer->Add(setnumBut, 0, wxALL, 10 );

	wxButton *setlabBut = new wxButton(panel, ShowInfoReq_ShowInfoSetLabels, wxT("&Set Labels"));
	top_button_sizer->Add(setlabBut, 0, wxALL, 10 );

	topsizer->Add(top_button_sizer, 0, wxALIGN_CENTER );

// now we add a left and a right side, putting boxes around things
// special attention about box sizers
// "the order in which you create
// new controls is important. Create your wxStaticBox control before any
// siblings that are to appear inside the wxStaticBox in order to preserve
// the correct Z-Order of controls."
	wxBoxSizer *middle_sizer = new wxBoxSizer( wxHORIZONTAL );
	wxBoxSizer *left_middle_sizer = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer *right_middle_sizer = new wxBoxSizer( wxVERTICAL );
	boxsizer = new wxStaticBoxSizer(new wxStaticBox(panel, -1, wxT("&Letters")), wxHORIZONTAL);
	labels = new wxListBox(panel, -1, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED);
	for (i = 0; i < 26; i++)
	{
		buf = wxT("A");
		buf.SetChar(0, buf.GetChar(0)+i);
		labels->InsertItems(1, &buf, i);
	}
	labels->Select(0);
	boxsizer->Add(labels, 0, wxALL, 0 );
	left_middle_sizer->Add(boxsizer, 0, wxALL, 0 );

	middle_sizer->Add(left_middle_sizer, 0, wxALIGN_CENTER );

// now add right side
	buf.sprintf(wxT("%u"), show->GetNumPoints());
	boxsizer = new wxStaticBoxSizer(new wxStaticBox(panel, -1, wxT("&Points")), wxHORIZONTAL);
	numpnts = new wxTextCtrl(panel, -1, buf);
	boxsizer->Add(numpnts, 0, wxALL, 0 );
	right_middle_sizer->Add(boxsizer, 0, wxALL, 10 );

	strs[0] = wxT("Numbers");
	strs[1] = wxT("Letters");
	label_type = new wxRadioBox(panel, -1, wxT("&Labels"), wxDefaultPosition, wxDefaultSize, 2, strs, 2);
	label_type->SetSelection(use_letters);
	right_middle_sizer->Add(label_type, 0, wxALL, 10 );

	wxArrayString modeStrings;
	ShowModeList::Iter mode = modelist->Begin();
	while (mode != modelist->End())
	{
		modeStrings.Add((*mode)->GetName());
		++mode;
	}
	boxsizer = new wxStaticBoxSizer( new wxStaticBox(panel, -1, wxT("Show &mode")), wxHORIZONTAL);
	choice = new wxChoice(panel, ShowInfoReq_ShowInfoModeChoice, wxDefaultPosition, wxDefaultSize, modeStrings, wxCAPTION, wxDefaultValidator, wxT("Show &mode"));
	UpdateMode();
	boxsizer->Add(choice, 0, wxALL, 0 );
	right_middle_sizer->Add(boxsizer, 0, wxALL, 10 );

	middle_sizer->Add(right_middle_sizer, 0, wxALIGN_CENTER );
	topsizer->Add(middle_sizer, 0, wxALIGN_CENTER );

	boxsizer = new wxStaticBoxSizer( new wxStaticBox(panel, -1, wxT("P&oints per letter")), wxHORIZONTAL);
	lettersize = new wxSlider(panel, -1, maxnum, 1, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	boxsizer->Add(lettersize, 0, wxALIGN_CENTER|wxEXPAND );
	topsizer->Add(boxsizer, 0, wxALIGN_CENTER );

	node = new CC_WinNodeInfo(lst, this);
	panel->SetSizer( topsizer );

	Center();
	Show(true);
}


ShowInfoReq::~ShowInfoReq()
{
	if (node)
	{
		node->Remove();
		delete node;
	}
}


void ShowInfoReq::OnCloseWindow(wxCloseEvent& event)
{
	FlushDescr();
	Destroy();
}


void ShowInfoReq::UpdateLabels()
{
	int i;
	bool letters[26];
	bool use_letters;
	int maxnum;

	CalculateLabels(show, letters, use_letters, maxnum);

	label_type->SetSelection(use_letters);
	lettersize->SetValue(maxnum);
	for (i = 0; i < 26; i++)
	{
		if (!letters[i] || !labels->IsSelected(i))// so Motif version works
		{
			labels->SetSelection(i, letters[i]);
		}
	}
}


void ShowInfoReq::UpdateNumPoints()
{
	wxString buf;

	buf.sprintf(wxT("%u"), show->GetNumPoints());
	numpnts->SetValue(buf);
	UpdateLabels();
}


void ShowInfoReq::UpdateMode()
{
	choice->SetStringSelection(show->GetMode().GetName());
}


void ShowInfoReq::UpdateDescr(bool quick)
{
	wxString c;

	text->Clear();
	if (quick)
	{
		c = show->GetDescr();
	}
	else
	{
		c = show->UserGetDescr();
	}
	text->WriteText(c);
	text->SetInsertionPoint(0);
}


void ShowInfoReq::Update(bool quick, CC_show *shw)
{
	if (shw != NULL) show = shw;
	UpdateDescr(quick);
	UpdateMode();
	UpdateNumPoints();
}


void ShowInfoReq::FlushDescr()
{
	wxString descr;

//  descr = text->GetContents();
	if (descr != show->GetDescr())
	{
		show->UserSetDescr(descr, this);
	}
}


unsigned ShowInfoReq::GetNumPoints()
{
	long i;
	wxString buf;

	if (!numpnts->GetValue().ToLong(&i) || i < 0)
	{
		numpnts->SetValue(wxT("0"));
		i = 0;
	}
	if (i > MAX_POINTS)
	{
		buf.sprintf(wxT("%u"), MAX_POINTS);
		numpnts->SetValue(buf);
		i = MAX_POINTS;
	}
	return (unsigned)i;
}


unsigned ShowInfoReq::GetColumns()
{
	wxString s;
	long i;

	s = wxGetTextFromUser(wxT("Enter the number of columns for new points"),
		wxT("New points"), wxT("10"), this);
	if (s.empty() || !s.ToLong(&i))
	{
		i = 10;
	}
	if (i < 1)
	{
		i = 1;
	}
	return (unsigned)i;
}


void ShowInfoReq::SetLabels()
{
	unsigned i, j, num, numlabels, letr;
	bool letters[26];

	if (GetLabelType() == 0)
	{
// Numbers
		for (i = 0; i < show->GetNumPoints(); i++)
		{
// Begin with 1, not 0
			show->GetPointLabel(i).Printf(wxT("%u"), i+1);
		}
	}
	else
	{
// Letters
		num = GetLetterSize();
		numlabels = 0;
		for (i = 0; i < 26; i++)
		{
			if (GetLetter(i))
			{
				letters[i] = true;
				numlabels++;
			}
			else
			{
				letters[i] = false;
			}
		}
		if (num*numlabels < show->GetNumPoints())
		{
			wxMessageBox(wxT("There are not enough labels defined."), wxT("Set labels"));
		}
		else
		{
			j = 0;
			letr = 0;
			while (!letters[letr]) letr++;
			for (i = 0; i < show->GetNumPoints(); i++, j++)
			{
				if (j >= num)
				{
					j = 0;
					do
					{
						letr++;
					} while (!letters[letr]);
				}
				show->GetPointLabel(i).Printf(wxT("%c%u"), letr+'A', j);
			}
		}
	}
}
