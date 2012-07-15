/*
 * show_ui.cpp
 * Classes for interacting with shows
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include "show_ui.h"
#include <ctype.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>

static const size_t kMaxPoints = 1000;

enum
{
	PointPicker_PointPickerAll=1100,
	PointPicker_PointPickerNone,
	PointPicker_PointPickerList,
};


BEGIN_EVENT_TABLE(PointPicker, wxDialog)
EVT_BUTTON(PointPicker_PointPickerAll,PointPicker::PointPickerAll)
EVT_BUTTON(PointPicker_PointPickerNone,PointPicker::PointPickerNone)
EVT_LISTBOX(PointPicker_PointPickerList,PointPicker::PointPickerSelect)
EVT_LISTBOX_DCLICK(PointPicker_PointPickerList,PointPicker::PointPickerAll)
END_EVENT_TABLE()

PointPickerView::PointPickerView() {}
PointPickerView::~PointPickerView() {}

void PointPickerView::OnDraw(wxDC *dc) {}
void PointPickerView::OnUpdate(wxView *sender, wxObject *hint)
{
	static_cast<PointPicker*>(GetFrame())->Update();
}

PointPicker::PointPicker()
{
	Init();
}

PointPicker::PointPicker(CC_show *shw,
	wxWindow *parent, wxWindowID id,
	const wxString& caption,
	const wxPoint& pos,
	const wxSize& size,
	long style )
{
	Init();
	
	Create(shw, parent, id, caption, pos, size, style);
}

PointPicker::~PointPicker()
{
	if (mView)
		delete mView;
}

void PointPicker::Init()
{
}


bool PointPicker::Create(CC_show *shw,
		wxWindow *parent, wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style)
{
	if (!wxDialog::Create(parent, id, caption, pos, size, style))
		return false;

	show = shw;

	// give this a view so it can pick up document changes
	mView = new PointPickerView;
	mView->SetDocument(show);
	mView->SetFrame(this);

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}

void PointPicker::CreateControls()
{
// Give it an icon
// create a sizer for laying things out top down:
	wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
	SetSizer( topsizer );

// add buttons to the top row
	wxBoxSizer *top_button_sizer = new wxBoxSizer( wxHORIZONTAL );
	wxButton *closeBut = new wxButton(this, wxID_OK, wxT("&Close"));
	closeBut->SetDefault();
	top_button_sizer->Add(closeBut, 0, wxALL, 5 );

	wxButton *setnumBut = new wxButton(this, PointPicker_PointPickerAll, wxT("&All"));
	top_button_sizer->Add(setnumBut, 0, wxALL, 5 );

	wxButton *setlabBut = new wxButton(this, PointPicker_PointPickerNone, wxT("&None"));
	top_button_sizer->Add(setlabBut, 0, wxALL, 5 );

	topsizer->Add(top_button_sizer, 0, wxALIGN_CENTER );

	mList = new wxListBox(this, PointPicker_PointPickerList, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED);
	topsizer->Add(mList, 0, wxGROW|wxALL, 0 );
	Update();
}


void PointPicker::PointPickerAll(wxCommandEvent&)
{
	show->SelectAll();
}


void PointPicker::PointPickerNone(wxCommandEvent&)
{
	show->UnselectAll();
}


void PointPicker::PointPickerSelect(wxCommandEvent&)
{
	wxArrayInt selections;
	size_t n = mList->GetSelections(selections);

	mCachedSelection.clear();
	for (size_t i = 0; i < n; ++i)
		mCachedSelection.insert(selections[i]);
	show->SetSelection(mCachedSelection);
}


void PointPicker::Update()
{
	std::vector<wxString> showLabels = show->GetPointLabels();
	if (mCachedLabels != showLabels)
	{
		mCachedLabels = showLabels;
		mList->Clear();
		mList->Set(mCachedLabels.size(), &mCachedLabels[0]);
	}
	CC_show::SelectionList showSelectionList = show->GetSelectionList();
	if (mCachedSelection != showSelectionList)
	{
		mList->DeselectAll();
		mCachedSelection = showSelectionList;
		for (CC_show::SelectionList::const_iterator n = mCachedSelection.begin(); n != mCachedSelection.end(); ++n)
		{
			mList->SetSelection(*n);
		}
	}
}


static void CalculateLabels(const CC_show& show, std::set<unsigned>& letters, bool& use_letters, int& maxnum)
{
	use_letters = false;
	maxnum = 1;
	for (unsigned i = 0; i < show.GetNumPoints(); ++i)
	{
		wxString tmp = show.GetPointLabel(i);
		size_t letterIndex = 0;
		if (!isdigit(tmp[0]))
		{
			use_letters = true;
			letterIndex += tmp[0]-'A';
			tmp.Remove(0, 1);
			if (!isdigit(tmp[0]))
			{
				tmp.Remove(0, 1);
				letterIndex += 26;
			}
		}
		long num = 0;
		if (tmp.ToLong(&num))
		{
			maxnum = std::max<int>(maxnum,num + 1);
		}
		if (use_letters)
		{
			letters.insert(letterIndex);
		}
	}
	if (use_letters == false)
	{
		maxnum = 10;
	}
}


enum
{
	ShowInfoReq_ID_POINTS_SPIN=1000,
	ShowInfoReq_ID_COLUMNS_SPIN,
	ShowInfoReq_ID_LABEL_TYPE,
	ShowInfoReq_ID_POINTS_PER_LETTER,
	ShowInfoReq_ID_LABEL_LETTERS,
	ShowInfoReq_ID_RESET,
};

BEGIN_EVENT_TABLE(ShowInfoReq, wxDialog)
EVT_BUTTON( wxID_RESET, ShowInfoReq::OnReset)
END_EVENT_TABLE()

IMPLEMENT_CLASS( ShowInfoReq, wxDialog )

ShowInfoReq::ShowInfoReq()
{
	Init();
}

ShowInfoReq::ShowInfoReq(CC_show *shw,
	wxWindow *parent, wxWindowID id,
	const wxString& caption,
	const wxPoint& pos,
	const wxSize& size,
	long style )
{
	Init();
	
	Create(shw, parent, id, caption, pos, size, style);
}

ShowInfoReq::~ShowInfoReq()
{
}

void ShowInfoReq::Init()
{
}


bool ShowInfoReq::Create(CC_show *shw,
		wxWindow *parent, wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	if (!wxDialog::Create(parent, id, caption, pos, size, style))
		return false;
	
	mShow = shw;
	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}


// Layout is to give a consistent show layout to both wizard and dialog
// layout requires a parent
//  wxSlider *lettersize;
void LayoutShowInfo(wxWindow *parent, bool putLastRowButtons)
{
// create a sizer for laying things out top down:
	wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
	parent->SetSizer( topsizer );

// now we add a left and a right side, putting boxes around things
// special attention about box sizers
// "the order in which you create
// new controls is important. Create your wxStaticBox control before any
// siblings that are to appear inside the wxStaticBox in order to preserve
// the correct Z-Order of controls."
	wxBoxSizer *middle_sizer = new wxBoxSizer( wxHORIZONTAL );
	wxBoxSizer *left_middle_sizer = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer *right_middle_sizer = new wxBoxSizer( wxVERTICAL );
	wxStaticBoxSizer *boxsizer;

	// add the left side
	wxBoxSizer *horizontal_sizer = new wxBoxSizer( wxHORIZONTAL );
	right_middle_sizer->Add(horizontal_sizer, 0, wxALL, 10 );
	wxStaticText* pointLabel = new wxStaticText(parent, wxID_STATIC, wxT("&Points:"), wxDefaultPosition, wxDefaultSize, 0);
	horizontal_sizer->Add(pointLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	wxSpinCtrl* numPoints = new wxSpinCtrl(parent, ShowInfoReq_ID_POINTS_SPIN, wxEmptyString, wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS, 1, kMaxPoints, 10);
	horizontal_sizer->Add(numPoints, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	horizontal_sizer = new wxBoxSizer( wxHORIZONTAL );
	right_middle_sizer->Add(horizontal_sizer, 0, wxALL, 10 );
	pointLabel = new wxStaticText(parent, wxID_STATIC, wxT("&Columns:"), wxDefaultPosition, wxDefaultSize, 0);
	horizontal_sizer->Add(pointLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	wxSpinCtrl* numberColumns = new wxSpinCtrl(parent, ShowInfoReq_ID_COLUMNS_SPIN, wxEmptyString, wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS, 0, kMaxPoints, 10);
	horizontal_sizer->Add(numberColumns, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// changing from radio box to choice
	wxString strs[2] = { wxT("Numbers"), wxT("Letters") };
	wxChoice* label_type = new wxChoice(parent, ShowInfoReq_ID_LABEL_TYPE, wxDefaultPosition, wxDefaultSize, 2, strs);
	label_type->SetSelection(0);
	right_middle_sizer->Add(label_type, 0, wxALL, 10 );

	horizontal_sizer = new wxBoxSizer( wxHORIZONTAL );
	right_middle_sizer->Add(horizontal_sizer, 0, wxALL, 10 );
	wxStaticText* label = new wxStaticText(parent, wxID_STATIC, wxT("P&oints per letter"), wxDefaultPosition, wxDefaultSize, 0);
	horizontal_sizer->Add(label, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	wxSpinCtrl* lettersize = new wxSpinCtrl(parent, ShowInfoReq_ID_POINTS_PER_LETTER, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 99, 10);
	horizontal_sizer->Add(lettersize, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	
	middle_sizer->Add(right_middle_sizer, 0, wxALIGN_CENTER );

	// now add right side
	boxsizer = new wxStaticBoxSizer(new wxStaticBox(parent, -1, wxT("&Letters")), wxHORIZONTAL);
	wxListBox *labels = new wxListBox(parent, ShowInfoReq_ID_LABEL_LETTERS, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED);
	for (int i = 0; i < 26; ++i)
	{
		wxString buf(static_cast<char>('A' + i));
		buf += wxString(static_cast<char>('A' + i));
		labels->InsertItems(1, &buf, i);
	}
	for (int i = 0; i < 26; ++i)
	{
		wxString buf(static_cast<char>('A' + i));
		labels->InsertItems(1, &buf, i);
	}
	boxsizer->Add(labels, 0, wxGROW|wxALL, 0 );
	left_middle_sizer->Add(boxsizer, 0, wxALL, 5 );
	
	middle_sizer->Add(left_middle_sizer, 0, wxALIGN_CENTER );
	
	topsizer->Add(middle_sizer, 0, wxALIGN_CENTER );

	if (putLastRowButtons)
	{
		// put a line between the controls and the buttons
		wxStaticLine* line = new wxStaticLine(parent, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
		topsizer->Add(line, 0, wxGROW|wxALL, 5);

		// the buttons on the bottom
		wxBoxSizer *okCancelBox = new wxBoxSizer( wxHORIZONTAL );
		topsizer->Add(okCancelBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

		wxButton *reset = new wxButton(parent, wxID_RESET, wxT("&Reset"));
		okCancelBox->Add(reset, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

		wxButton *ok = new wxButton(parent, wxID_OK);
		ok->SetDefault();
		okCancelBox->Add(ok, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

		wxButton *cancel = new wxButton(parent, wxID_CANCEL);
		okCancelBox->Add(cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	}
}

void ShowInfoReq::CreateControls()
{
	LayoutShowInfo(this, true);
}


bool ShowInfoReq::TransferDataToWindow()
{
	std::set<unsigned> letters;
	bool use_letters;
	int maxnum;
	CalculateLabels(*mShow, letters, use_letters, maxnum);

	wxSpinCtrl* pointsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_SPIN);
	wxSpinCtrl* columnsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_COLUMNS_SPIN);
	wxChoice* labelType = (wxChoice*) FindWindow(ShowInfoReq_ID_LABEL_TYPE);
	wxSpinCtrl* pointsPerLine = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
	wxListBox* label_letters = (wxListBox*) FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

	pointsCtrl->SetValue(mShow->GetNumPoints());
	columnsCtrl->SetValue(10);
	labelType->SetSelection(use_letters);
	pointsPerLine->SetValue(maxnum);
	label_letters->DeselectAll();
	for (unsigned i = 0; i < label_letters->GetCount(); ++i)
	{
		if (letters.count(i))
		{
			label_letters->SetSelection(i);
		}
	}
	return true;
}


bool ShowInfoReq::TransferDataFromWindow()
{
	wxSpinCtrl* pointsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_SPIN);
	wxSpinCtrl* columnsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_COLUMNS_SPIN);
	wxChoice* labelType = (wxChoice*) FindWindow(ShowInfoReq_ID_LABEL_TYPE);
	wxSpinCtrl* pointsPerLine = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
	wxListBox* label_letters = (wxListBox*) FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

	mNumberPoints = pointsCtrl->GetValue();
	mNumberColumns = columnsCtrl->GetValue();
	mLabels.clear();

	if (labelType->GetSelection() == 0)
	{
// Numbers
		for (unsigned i = 0; i < mNumberPoints; ++i)
		{
// Begin with 1, not 0
			wxString buffer;
			buffer.Printf(wxT("%u"), i);
			mLabels.push_back(buffer);
		}
	}
	else
	{
// Letters
		int numPerLetter = pointsPerLine->GetValue();
		int num = pointsCtrl->GetValue();
		unsigned letr = 0;
		while (num > 0)
		{
			if (label_letters->IsSelected(letr))
			{
				int n = std::min(num, numPerLetter);
				for (int i = 0; i < n; ++i)
				{
					wxString buffer;
					buffer.Printf(wxT("%s%u"), label_letters->GetString(letr), i);
					mLabels.push_back(buffer);
				}
				num -= n;
			}
			++letr;
		}
	}
	return true;
}


bool ShowInfoReq::Validate()
{
	wxChoice* labelType = (wxChoice*) FindWindow(ShowInfoReq_ID_LABEL_TYPE);
	if (labelType->GetSelection() == 0)
	{
		return true;
	}
	wxSpinCtrl* pointsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_SPIN);
	wxSpinCtrl* pointsPerLine = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
	wxListBox* label_letters = (wxListBox*) FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

	int num = pointsPerLine->GetValue();
	int numlabels = 0;
	for (unsigned i = 0; i < label_letters->GetCount(); i++)
	{
		if (label_letters->IsSelected(i))
		{
			++numlabels;
		}
	}
	bool canDo = (num*numlabels >= pointsCtrl->GetValue());
	if (!canDo)
		wxMessageBox(wxT("There are not enough labels defined."), wxT("Set labels"));
	return canDo;
}


void ShowInfoReq::OnReset(wxCommandEvent&)
{
	Init();
	TransferDataToWindow();
}


IMPLEMENT_CLASS( ShowInfoReqWizard, wxWizardPageSimple )

ShowInfoReqWizard::ShowInfoReqWizard(wxWizard *parent) : wxWizardPageSimple(parent),
mTransferDataToWindowFirstTime(true),
mNumberPoints(1),
mNumberColumns(1)
{
	LayoutShowInfo(this, false);
	GetSizer()->Fit(this);
}

bool ShowInfoReqWizard::TransferDataToWindow()
{
	// on first time, we need to set up values
	if (mTransferDataToWindowFirstTime)
	{
		wxSpinCtrl* pointsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_SPIN);
		wxSpinCtrl* columnsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_COLUMNS_SPIN);
		wxChoice* labelType = (wxChoice*) FindWindow(ShowInfoReq_ID_LABEL_TYPE);
		wxSpinCtrl* pointsPerLine = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
		wxListBox* label_letters = (wxListBox*) FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

		pointsCtrl->SetValue(mNumberPoints);
		columnsCtrl->SetValue(mNumberColumns);
		labelType->SetSelection(false);
		pointsPerLine->SetValue(10);
		label_letters->DeselectAll();
		mTransferDataToWindowFirstTime = false;
	}
	return true;
}

bool ShowInfoReqWizard::TransferDataFromWindow()
{
	wxSpinCtrl* pointsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_SPIN);
	wxSpinCtrl* columnsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_COLUMNS_SPIN);
	wxChoice* labelType = (wxChoice*) FindWindow(ShowInfoReq_ID_LABEL_TYPE);
	wxSpinCtrl* pointsPerLine = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
	wxListBox* label_letters = (wxListBox*) FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

	mNumberPoints = pointsCtrl->GetValue();
	mNumberColumns = columnsCtrl->GetValue();
	mLabels.clear();

	if (labelType->GetSelection() == 0)
	{
// Numbers
		for (unsigned i = 0; i < mNumberPoints; ++i)
		{
// Begin with 1, not 0
			wxString buffer;
			buffer.Printf(wxT("%u"), i);
			mLabels.push_back(buffer);
		}
	}
	else
	{
// Letters
		int numPerLetter = pointsPerLine->GetValue();
		int num = pointsCtrl->GetValue();
		unsigned letr = 0;
		while (num > 0)
		{
			if (label_letters->IsSelected(letr))
			{
				int n = std::min(num, numPerLetter);
				for (int i = 0; i < n; ++i)
				{
					wxString buffer;
					buffer.Printf(wxT("%s%u"), label_letters->GetString(letr), i);
					mLabels.push_back(buffer);
				}
				num -= n;
			}
			++letr;
		}
	}
	return true;
}


bool ShowInfoReqWizard::Validate()
{
	wxChoice* labelType = (wxChoice*) FindWindow(ShowInfoReq_ID_LABEL_TYPE);
	if (labelType->GetSelection() == 0)
	{
		return true;
	}
	wxSpinCtrl* pointsCtrl = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_SPIN);
	wxSpinCtrl* pointsPerLine = (wxSpinCtrl*) FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
	wxListBox* label_letters = (wxListBox*) FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

	int num = pointsPerLine->GetValue();
	int numlabels = 0;
	for (unsigned i = 0; i < label_letters->GetCount(); i++)
	{
		if (label_letters->IsSelected(i))
		{
			++numlabels;
		}
	}
	bool canDo = (num*numlabels >= pointsCtrl->GetValue());
	if (!canDo)
		wxMessageBox(wxT("There are not enough labels defined."), wxT("Set labels"));
	return canDo;
}

