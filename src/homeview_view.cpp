/*
 * homeview_view.h
 * view for viewer only version
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "homeview_view.h"

#include "calchartapp.h"
#include "confgr.h"
#include "field_frame.h"
#include "top_frame.h"
#include "animate.h"
#include "show_ui.h"
#include "cc_command.h"
#include "cc_shapes.h"
#include "setup_wizards.h"

#include <wx/wizard.h>

IMPLEMENT_DYNAMIC_CLASS(HomeViewView, wxView)

HomeViewView::HomeViewView() :
mFrame(NULL)
{
}

HomeViewView::~HomeViewView()
{
}

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool
HomeViewView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
	mShow = static_cast<CC_show*>(doc);
	mFrame = new FieldFrame(doc, this, wxStaticCast(wxGetApp().GetTopWindow(), wxDocParentFrame), wxPoint(50, 50),
							wxSize(GetConfiguration_FieldFrameWidth(), GetConfiguration_FieldFrameHeight()));
	
	mFrame->Show(true);
	Activate(true);
	return true;
}

// Sneakily gets used for default print/preview
// as well as drawing on the screen.
void
HomeViewView::OnDraw(wxDC *dc)
{
}

void
HomeViewView::OnUpdate(wxView *WXUNUSED(sender), wxObject *hint)
{
	if (hint && hint->IsKindOf(CLASSINFO(CC_show_setup)))
	{
		// Set up everything else
		OnWizardSetup(*show);
	}
	else
	{
		if (mFrame)
		{
			mFrame->UpdatePanel();
			wxString buf;
			GetDocument()->GetPrintableName(buf);
			mFrame->SetTitle(buf);
		}
	}
}

// Clean up windows used for displaying the view.
bool
HomeViewView::OnClose(bool deleteWindow)
{
	SetFrame((wxFrame*)NULL);
	
	Activate(false);
	
	if (!GetDocument()->Close())
		return false;
	
	if (deleteWindow)
	{
		delete mFrame;
	}
	return true;
}


void
HomeViewView::OnWizardSetup(CC_show& show)
{
	wxWizard *wizard = new wxWizard(mFrame, wxID_ANY, wxT("New Show Setup Wizard"));
	// page 1:
	// set the number of points and the labels
	ShowInfoReqWizard *page1 = new ShowInfoReqWizard(wizard);
	
	// page 2:
	// choose the show mode
	ChooseShowModeWizard *page2 = new ChooseShowModeWizard(wizard);
	
	// page 3:
	// and maybe a description
	SetDescriptionWizard *page3 = new SetDescriptionWizard(wizard);
	// page 4:
	
	wxWizardPageSimple::Chain(page1, page2);
	wxWizardPageSimple::Chain(page2, page3);
	
	wizard->GetPageAreaSizer()->Add(page1);
	if (wizard->RunWizard(page1))
	{
		show.SetNumPoints(page1->GetNumberPoints(), page1->GetNumberColumns());
		show.SetPointLabel(page1->GetLabels());
		ShowMode *newmode = ShowModeList_Find(wxGetApp().GetModeList(), page2->GetValue());
		if (newmode)
		{
			show.SetMode(newmode);
		}
		show.SetDescr(page3->GetValue());
	}
	else
	{
		wxMessageBox(
					 wxT("Show setup not completed.\n")
					 wxT("You can change the number of marchers\n")
					 wxT("and show mode via the menu options"), wxT("Show not setup"), wxICON_INFORMATION|wxOK);
	}
	wizard->Destroy();
}

int
HomeViewView::FindPoint(CC_coord pos) const
{
	return mShow->GetCurrentSheet()->FindPoint(pos.x, pos.y, 0);
}

CC_coord
HomeViewView::PointPosition(int which) const
{
	return mShow->GetCurrentSheet()->GetPosition(which, 0);
}

void
HomeViewView::GoToSheet(size_t which)
{
	if (which < mShow->GetNumSheets())
	{
		mShow->SetCurrentSheet(which);
	}
}

void
HomeViewView::GoToNextSheet()
{
	GoToSheet(mShow->GetCurrentSheetNum() + 1);
}

void
HomeViewView::GoToPrevSheet()
{
	GoToSheet(mShow->GetCurrentSheetNum() - 1);
}

// toggle selection means toggle it as selected to unselected
// otherwise, always select it
void
HomeViewView::SelectWithLasso(const CC_lasso* lasso, bool toggleSelected)
{
	mShow->SelectWithLasso(*lasso, toggleSelected, 0);
}

// Select points within rectangle
void
HomeViewView::SelectPointsInRect(const CC_coord& c1, const CC_coord& c2, bool toggleSelected)
{
	CC_lasso lasso(c1);
	lasso.Append(CC_coord(c1.x, c2.y));
	lasso.Append(c2);
	lasso.Append(CC_coord(c2.x, c1.y));
	lasso.End();
	SelectWithLasso(&lasso, toggleSelected);
}

