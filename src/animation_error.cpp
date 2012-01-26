/*
 * animation_error.cpp
 * Animation error dialog
 */

/*
   Copyright (C) 1995-2012  Richard Michael Powell

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

#include "animation_error.h"

enum
{
	CALCHART__anim_update = 100
};


wxIMPLEMENT_DYNAMIC_CLASS(AnimErrorList, wxDialog)


BEGIN_EVENT_TABLE(AnimErrorList, wxDialog)
EVT_LISTBOX(CALCHART__anim_update, AnimErrorList::OnCmdUpdate)
END_EVENT_TABLE()


AnimErrorListView::AnimErrorListView()
{
}


AnimErrorListView::~AnimErrorListView()
{
}


void
AnimErrorListView::OnDraw(wxDC *dc)
{
}


void
AnimErrorListView::OnUpdate(wxView *sender, wxObject *hint)
{
	if (hint && hint->IsKindOf(CLASSINFO(CC_show_modified)))
	{
		static_cast<AnimErrorList*>(GetFrame())->Close();
	}
}


AnimErrorList::AnimErrorList()
{
	Init();
}


AnimErrorList::AnimErrorList(CC_show *show, const ErrorMarker error_markers[NUM_ANIMERR], unsigned num,
		wxWindow *parent, wxWindowID id, const wxString& caption,
		const wxPoint& pos, const wxSize& size,
		long style)
{
	Init();
	
	Create(show, error_markers, num, parent, id, caption, pos, size, style);
}


AnimErrorList::~AnimErrorList()
{
	delete mView;
}


bool
AnimErrorList::Create(CC_show *show, const ErrorMarker error_markers[NUM_ANIMERR], unsigned num,
		wxWindow *parent, wxWindowID id, const wxString& caption,
		const wxPoint& pos, const wxSize& size,
		long style)
{
	if (!wxDialog::Create(parent, id, caption, pos, size, style))
		return false;

	mShow = show;
	for (size_t i = 0; i < NUM_ANIMERR; ++i)
		mErrorMarkers[i] = error_markers[i];

	// give this a view so it can pick up document changes
	mView = new AnimErrorListView;
	mView->SetDocument(mShow);
	mView->SetFrame(this);

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}


void
AnimErrorList::Init()
{
}


void
AnimErrorList::CreateControls()
{
// create a sizer for laying things out top down:
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(topsizer);

	wxButton *closeBut = new wxButton(this, wxID_OK, wxT("Close"));
	topsizer->Add(closeBut);

	wxListBox* lst = new wxListBox(this, CALCHART__anim_update, wxDefaultPosition, wxSize(200,200), 0, NULL, wxLB_SINGLE);

	topsizer->Add(lst, wxSizerFlags().Expand().Border(5) );
}


bool
AnimErrorList::TransferDataToWindow()
{
	wxListBox* lst = (wxListBox*)FindWindow(CALCHART__anim_update);

	for (unsigned i = 0, j = 0; i < NUM_ANIMERR; i++)
	{
		if (!mErrorMarkers[i].pntgroup.empty())
		{
			lst->Append(animate_err_msgs[i]);
			pointsels[j++] = mErrorMarkers[i];
		}
	}
	return true;
}


void
AnimErrorList::OnCmdUpdate(wxCommandEvent& event)
{
	Update(event.IsSelection() ? event.GetSelection() : -1);
}


void
AnimErrorList::Unselect()
{
	wxListBox* lst = (wxListBox*)FindWindow(CALCHART__anim_update);
	int i = lst->GetSelection();

	if (i >= 0)
	{
		lst->Deselect(i);
	}
}


void
AnimErrorList::Update()
{
	wxListBox* lst = (wxListBox*)FindWindow(CALCHART__anim_update);
	Update(lst->GetSelection());
}


void
AnimErrorList::Update(int i)
{
	if (i >= 0)
	{
		CC_show::SelectionList select;
		for (unsigned j = 0; j < mShow->GetNumPoints(); j++)
		{
			if (pointsels[i].pntgroup.count(j))
			{
				select.insert(j);
			}
		}
		mShow->SetSelection(select);
	}
	mShow->SetCurrentSheet(sheetnum > mShow->GetNumSheets() ? mShow->GetNumSheets()-1 : sheetnum);
	mShow->AllViewGoToCont(pointsels[i].contnum, pointsels[i].line, pointsels[i].col);
}
