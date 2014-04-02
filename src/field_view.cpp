/*
 * field_view.cpp
 * Header for field view
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

#include "field_view.h"
#include "field_canvas.h"
#include "field_frame.h"

#include "calchartapp.h"
#include "confgr.h"
#include "top_frame.h"
#include "animate.h"
#include "show_ui.h"
#include "cc_command.h"
#include "cc_shapes.h"
#include "setup_wizards.h"
#include "animation_frame.h"
#include "draw.h"
#include "animatecommand.h"
#include "cc_sheet.h"

#include <wx/wizard.h>
#include <wx/textfile.h>

IMPLEMENT_DYNAMIC_CLASS(FieldView, wxView)

FieldView::FieldView() :
mFrame(NULL),
mDrawPaths(false),
mCurrentReferencePoint(0)
{
}

FieldView::~FieldView()
{
}

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool
FieldView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
	mShow = static_cast<CalChartDoc*>(doc);
#if defined(BUILD_FOR_VIEWER) && (BUILD_FOR_VIEWER != 0)
	mFrame = new AnimationFrame(NULL, doc, this, wxStaticCast(wxGetApp().GetTopWindow(), wxDocParentFrame));
#else
	mFrame = new FieldFrame(doc, this, wxStaticCast(wxGetApp().GetTopWindow(), wxDocParentFrame), wxPoint(50, 50), wxSize(GetConfiguration_FieldFrameWidth(), GetConfiguration_FieldFrameHeight()));
#endif
	
	mFrame->Show(true);
	Activate(true);
	return true;
}

// Sneakily gets used for default print/preview
// as well as drawing on the screen.
void
FieldView::OnDraw(wxDC *dc)
{
	if (mShow)
	{
		// draw the field
		dc->SetPen(GetCalChartPen(COLOR_FIELD_DETAIL));
		dc->SetTextForeground(GetCalChartPen(COLOR_FIELD_TEXT).GetColour());
		mShow->GetMode().Draw(*dc);
		
		CC_show::const_CC_sheet_iterator_t sheet = mShow->GetCurrentSheet();
		if (sheet != mShow->GetSheetEnd())
		{
			if (mCurrentReferencePoint > 0)
			{
				Draw(*dc, *mShow, *mShow->GetCurrentSheet(), 0, false);
				Draw(*dc, *mShow, *mShow->GetCurrentSheet(), mCurrentReferencePoint, true);
			}
			else
			{
				Draw(*dc, *mShow, *mShow->GetCurrentSheet(), mCurrentReferencePoint, true);
			}
			DrawPaths(*dc, *sheet);
		}
		CC_sheet* ghostSheet = mGhostModule.getGhostSheet();
		if (ghostSheet != nullptr) {
			DrawGhostSheet(*dc, *mShow, *ghostSheet, 0);
		}
	}
}

void
FieldView::OnUpdate(wxView *WXUNUSED(sender), wxObject *hint)
{
	if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_setup)))
	{
		// give our show a first page
		CalChartDoc* show = static_cast<CalChartDoc*>(GetDocument());
		
		show->SetupNewShow();

		// Set up everything else
		OnWizardSetup(*show);
		
		// make the show modified so it gets repainted
		show->Modify(true);
	}
	else if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_modified)))
	{
		GeneratePaths();
	}
	
	if (mFrame)
	{
		mFrame->UpdatePanel();
		mFrame->Refresh();
	}
	mGhostModule.update(mShow, GetCurrentSheetNum());
}

// Clean up windows used for displaying the view.
bool
FieldView::OnClose(bool deleteWindow)
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
FieldView::OnWizardSetup(CalChartDoc& show)
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
		auto labels = page1->GetLabels();
		std::vector<std::string> tlabels(labels.begin(), labels.end());
		show.SetPointLabel(tlabels);
		ShowMode *newmode = ShowModeList_Find(wxGetApp().GetModeList(), page2->GetValue());
		if (newmode)
		{
			show.SetMode(newmode);
		}
		show.SetDescr(page3->GetValue().ToStdString());
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

bool
FieldView::DoTranslatePoints(const CC_coord& delta)
{
	if (((delta.x == 0) && (delta.y == 0)) ||
		(mShow->GetSelectionList().size() == 0))
		return false;
	GetDocument()->GetCommandProcessor()->Submit(new TranslatePointsByDeltaCommand(*mShow, delta, mCurrentReferencePoint), true);
	return true;
}

bool
FieldView::DoTransformPoints(const Matrix& transmat)
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new TransformPointsCommand(*mShow, transmat, mCurrentReferencePoint), true);
	return true;
}

bool
FieldView::DoMovePointsInLine(const CC_coord& start, const CC_coord& second)
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new TransformPointsInALineCommand(*mShow, start, second, mCurrentReferencePoint), true);
	return true;
}

bool
FieldView::DoResetReferencePoint()
{
	GetDocument()->GetCommandProcessor()->Submit(new SetReferencePointToRef0(*mShow, mCurrentReferencePoint), true);
	return true;
}

bool
FieldView::DoSetPointsSymbol(SYMBOL_TYPE sym)
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new SetSymbolCommand(*mShow, sym), true);
	return true;
}

bool
FieldView::DoSetDescription(const wxString& descr)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetDescriptionCommand(*mShow, descr), true);
	return true;
}

void
FieldView::DoSetMode(const wxString& mode)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetModeCommand(*mShow, mode), true);
}

void
FieldView::DoSetShowInfo(unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetShowInfoCommand(*mShow, numPoints, numColumns, labels), true);
}

bool
FieldView::DoSetSheetTitle(const wxString& descr)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetSheetTitleCommand(*mShow, descr), true);
	return true;
}

bool
FieldView::DoSetSheetBeats(unsigned short beats)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetSheetBeatsCommand(*mShow, beats), true);
	return true;
}

bool
FieldView::DoSetPointsLabel(bool right)
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new SetLabelRightCommand(*mShow, right), true);
	return true;
}

bool
FieldView::DoSetPointsLabelFlip()
{
	if (mShow->GetSelectionList().size() == 0) return false;
	GetDocument()->GetCommandProcessor()->Submit(new SetLabelFlipCommand(*mShow), true);
	return true;
}

bool
FieldView::DoInsertSheets(const CC_show::CC_sheet_container_t& sht, unsigned where)
{
	GetDocument()->GetCommandProcessor()->Submit(new AddSheetsCommand(*mShow, sht, where), true);
	return true;
}

bool
FieldView::DoDeleteSheet(unsigned where)
{
	GetDocument()->GetCommandProcessor()->Submit(new RemoveSheetsCommand(*mShow, where), true);
	return true;
}

bool
FieldView::DoImportPrintableContinuity(const wxString& file)
{
	wxTextFile fp;
	fp.Open(file);
	if (!fp.IsOpened())
	{
		return wxT("Unable to open file");
	}
	// read the file into a vector
	std::vector<std::string> lines;
	for (size_t line = 0; line < fp.GetLineCount(); ++line)
	{
		lines.push_back(fp.GetLine(line).ToStdString());
	}
	auto hasCont = mShow->AlreadyHasPrintContinuity();
	if (hasCont)
	{
		// prompt the user to find out if they would like to continue
		int userchoice = wxMessageBox(
									  wxT("This show already has some Printable Continuity.")
									  wxT("Would you like to continue Importing Printable Continuity and overwrite it?"),
									  wxT("Overwrite Printable Continuity?"), wxYES_NO|wxCANCEL);
		if (userchoice != wxYES)
		{
			return true;
		}
	}
	auto result = GetDocument()->GetCommandProcessor()->Submit(new ImportPrintContinuityCommand(*mShow, lines));
	return result;
}

int
FieldView::FindPoint(CC_coord pos) const
{
	return mShow->GetCurrentSheet()->FindPoint(pos.x, pos.y, Float2Coord(GetConfiguration_DotRatio()), mCurrentReferencePoint);
}

CC_coord
FieldView::PointPosition(int which) const
{
	return mShow->GetCurrentSheet()->GetPosition(which, mCurrentReferencePoint);
}

CC_coord
FieldView::GetShowFieldOffset() const
{
	return mShow->GetMode().Offset();
}

CC_coord
FieldView::GetShowFieldSize() const
{
	return mShow->GetMode().Size();
}

void
FieldView::GoToSheet(size_t which)
{
	if (which < mShow->GetNumSheets())
	{
		mShow->SetCurrentSheet(which);
	}
}

void
FieldView::GoToNextSheet()
{
	GoToSheet(mShow->GetCurrentSheetNum() + 1);
}

void
FieldView::GoToPrevSheet()
{
	GoToSheet(mShow->GetCurrentSheetNum() - 1);
}

void
FieldView::SetReferencePoint(unsigned which)
{
	mCurrentReferencePoint = which;
	OnUpdate(this);
}

void
FieldView::AddToSelection(const SelectionList& sl)
{
	mShow->AddToSelection(sl);
}

void
FieldView::ToggleSelection(const SelectionList& sl)
{
	mShow->ToggleSelection(sl);
}

// toggle selection means toggle it as selected to unselected
// otherwise, always select it
void
FieldView::SelectWithLasso(const CC_lasso* lasso, bool toggleSelected)
{
	mShow->SelectWithLasso(*lasso, toggleSelected, mCurrentReferencePoint);
}

// Select points within rectangle
void
FieldView::SelectPointsInRect(const CC_coord& c1, const CC_coord& c2, bool toggleSelected)
{
	CC_lasso lasso(c1);
	lasso.Append(CC_coord(c1.x, c2.y));
	lasso.Append(c2);
	lasso.Append(CC_coord(c2.x, c1.y));
	lasso.End();
	SelectWithLasso(&lasso, toggleSelected);
}

void
FieldView::OnEnableDrawPaths(bool enable)
{
	mDrawPaths = enable;
	if (mDrawPaths)
	{
		GeneratePaths();
	}
	mFrame->Refresh();
}

void
FieldView::DrawPaths(wxDC& dc, const CC_sheet& sheet)
{
	if (mDrawPaths && mAnimation && mAnimation->GetNumberSheets() && (static_cast<unsigned>(mAnimation->GetNumberSheets()) > mShow->GetCurrentSheetNum()))
	{
		CC_coord origin = GetShowFieldOffset();
		mAnimation->GotoSheet(mShow->GetCurrentSheetNum());
		for (auto point = mShow->GetSelectionList().begin(); point != mShow->GetSelectionList().end(); ++point)
		{
			DrawPath(dc, mAnimation->GenPathToDraw(*point, origin), mAnimation->EndPosition(*point, origin));
		}
	}
}

void
FieldView::GeneratePaths()
{
	mAnimation = mShow->NewAnimation(NotifyStatus(), NotifyErrorList());
}

