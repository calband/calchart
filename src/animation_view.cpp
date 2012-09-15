/*
 * AnimationView.cpp
 * Animation user interface
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

#include "animation_view.h"
#include "animation_frame.h"
#include "basic_ui.h"
#include "modes.h"
#include "confgr.h"
#include "cc_shapes.h"
#include "calchartapp.h"
#include "top_frame.h"

#include <wx/dcbuffer.h>

#include <boost/bind.hpp>
#include <fstream>


//IMPLEMENT_DYNAMIC_CLASS(AnimationView, wxView)


AnimationView::AnimationView() :
mErrorOccurred(false),
mCollisionWarningType(COLLISION_SHOW)
{
}


AnimationView::~AnimationView()
{
}


void
AnimationView::OnDraw(wxDC *dc)
{
	dc->SetPen(*CalChartPens[COLOR_FIELD_DETAIL]);
	GetShow()->GetMode().DrawAnim(*dc);
	const bool checkForCollision = mCollisionWarningType != COLLISION_NONE;
	if (mAnimation)
	{
		for (unsigned i = 0; i < GetShow()->GetNumPoints(); ++i)
		{
			if (checkForCollision && mAnimation->IsCollision(i))
			{
				dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_COLLISION]);
				dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_COLLISION]);
			}
			else if (GetShow()->IsSelected(i))
			{
				switch (mAnimation->Direction(i))
				{
					case ANIMDIR_SW:
					case ANIMDIR_W:
					case ANIMDIR_NW:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_HILIT_BACK]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_HILIT_BACK]);
						break;
					case ANIMDIR_SE:
					case ANIMDIR_E:
					case ANIMDIR_NE:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_HILIT_FRONT]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_HILIT_FRONT]);
						break;
					default:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_HILIT_SIDE]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_HILIT_SIDE]);
				}
			}
			else
			{
				switch (mAnimation->Direction(i))
				{
					case ANIMDIR_SW:
					case ANIMDIR_W:
					case ANIMDIR_NW:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_BACK]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_BACK]);
						break;
					case ANIMDIR_SE:
					case ANIMDIR_E:
					case ANIMDIR_NE:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_FRONT]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_FRONT]);
						break;
					default:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_SIDE]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_SIDE]);
				}
			}
			CC_coord position = mAnimation->Position(i);
			unsigned long x = position.x+GetShow()->GetMode().Offset().x;
			unsigned long y = position.y+GetShow()->GetMode().Offset().y;
			dc->DrawRectangle(x - Int2Coord(1)/2, y - Int2Coord(1)/2, Int2Coord(1), Int2Coord(1));
		}
	}
}


void
AnimationView::OnUpdate(wxView *sender, wxObject *hint)
{
    wxView::OnUpdate(sender, hint);
	if (hint && hint->IsKindOf(CLASSINFO(CC_show_modified)))
	{
		if (mAnimation)
		{
			mAnimation.reset();
			RefreshFrame();
		}
	}
	if (hint && hint->IsKindOf(CLASSINFO(CC_show_FinishedLoading)))
	{
		Generate();
	}
}


void
AnimationView::RefreshFrame()
{
	GetAnimationFrame()->UpdatePanel();
	GetAnimationFrame()->Refresh();
}


void
AnimationView::SetCollisionType(CollisionWarning col)
{
	mCollisionWarningType = col;
	if (mAnimation)
	{
		mAnimation->SetCollisionAction(col == COLLISION_BEEP ? wxBell : NULL);
	}
}


void
AnimationView::SelectCollisions()
{
	if (mAnimation)
	{
		CC_show::SelectionList select;
		for (unsigned i = 0; i < GetShow()->GetNumPoints(); i++)
		{
			if (mAnimation->IsCollision(i))
			{
				select.insert(i);
			}
		}
		GetShow()->SetSelection(select);
	}
}


void
AnimationView::Generate()
{
	GetAnimationFrame()->SetStatusText(wxT("Compiling..."));
	mAnimation.reset();
	
	// flush out the show
	GetShow()->FlushAllTextWindows();		  // get all changes in text windows
	
	// set our error indicator:
	mErrorOccurred = false;
	// (wxMessageBox(wxT("Ignore errors?"), wxT("Animate"), wxYES_NO) != wxYES);
	NotifyStatus notifyStatus = boost::bind(&AnimationView::OnNotifyStatus, this, _1);
	NotifyErrorList notifyErrorList = boost::bind(&AnimationView::OnNotifyErrorList, this, _1, _2, _3);
	mAnimation.reset(new Animation(GetShow(), notifyStatus, notifyErrorList));
	if (mAnimation && (mAnimation->GetNumberSheets() == 0))
	{
		mAnimation.reset();
	}
	if (mAnimation)
	{
#if defined(GENERATE_SHOW_DUMP) && GENERATE_SHOW_DUMP
		std::ofstream output("/tmp/animate.txt");
		mAnimation->GotoSheet(0);
		do
		{
			output<<mAnimation->GetCurrentInfo();
		} while (mAnimation->NextBeat());
#endif // GENERATE_SHOW_DUMP

		mAnimation->SetCollisionAction(mCollisionWarningType == COLLISION_BEEP ? wxBell : NULL);
		mAnimation->GotoSheet(GetShow()->GetCurrentSheetNum());
	}
	GetAnimationFrame()->UpdatePanel();
	GetAnimationFrame()->SetStatusText(mErrorOccurred ? wxT("Error during compilation") : wxT("Ready"));
	GetAnimationFrame()->Refresh();
	if (mErrorOccurred)
	{
		wxMessageBox(wxT("Error during compilation.  See Error pulldown below."), wxT("Errors occurred"), wxOK);
	}
}


// true if changes made
bool
AnimationView::PrevBeat()
{
	if (mAnimation && mAnimation->PrevBeat())
	{
		RefreshFrame();
		return true;
	}
	return false;
}


bool
AnimationView::NextBeat()
{
	if (mAnimation && mAnimation->NextBeat())
	{
		RefreshFrame();
		return true;
	}
	return false;
}


void
AnimationView::GotoBeat(unsigned i)
{
	if (mAnimation)
	{
		mAnimation->GotoBeat(i);
		RefreshFrame();
	}
}


bool
AnimationView::PrevSheet()
{
	if (mAnimation && mAnimation->PrevSheet())
	{
		RefreshFrame();
		return true;
	}
	return false;
}


bool
AnimationView::NextSheet()
{
	if (mAnimation && mAnimation->NextSheet())
	{
		RefreshFrame();
		return true;
	}
	return false;
}


void
AnimationView::GotoSheet(unsigned i)
{
	if (mAnimation)
	{
		mAnimation->GotoSheet(i);
		RefreshFrame();
	}
}


void
AnimationView::SetSelection(const CC_show::SelectionList& sl)
{
	GetShow()->SetSelection(sl);
}


int
AnimationView::GetNumberSheets() const
{
	return (mAnimation) ? mAnimation->GetNumberSheets() : 0;
}


int
AnimationView::GetCurrentSheet() const
{
	return (mAnimation) ? mAnimation->GetCurrentSheet() : 0;
}


int
AnimationView::GetNumberBeats() const
{
	return (mAnimation) ? mAnimation->GetNumberBeats() : 0;
}


int
AnimationView::GetCurrentBeat() const
{
	return (mAnimation) ? mAnimation->GetCurrentBeat() : 0;
}


wxString
AnimationView::GetStatusText() const
{
	wxString tempbuf;
	
	if (mAnimation)
	{
		tempbuf.Printf(wxT("Beat %u of %u  Sheet %d of %d \"%.32s\""),
					   mAnimation->GetCurrentBeat(), mAnimation->GetNumberBeats(),
					   mAnimation->GetCurrentSheet()+1, mAnimation->GetNumberSheets(),
					   mAnimation->GetCurrentSheetName().c_str());
	}
	else
	{
		tempbuf = wxT("No animation available");
	}
	return tempbuf;
}


const CC_coord&
AnimationView::GetShowSize() const
{
	return GetShow()->GetMode().Size();
}

void
AnimationView::UnselectMarchers()
{
	GetShow()->UnselectAll();
}

void
AnimationView::SelectMarchersInBox(long mouseXStart, long mouseYStart,
								   long mouseXEnd, long mouseYEnd, bool altDown)
{
	// otherwise, Select points within rectangle
	Coord x_off = GetShow()->GetMode().Offset().x;
	Coord y_off = GetShow()->GetMode().Offset().y;
	CC_lasso lasso(CC_coord(mouseXStart-x_off, mouseYStart-y_off));
	lasso.Append(CC_coord(mouseXStart-x_off, mouseYEnd-y_off));
	lasso.Append(CC_coord(mouseXEnd-x_off, mouseYEnd-y_off));
	lasso.Append(CC_coord(mouseXEnd-x_off, mouseYStart-y_off));
	lasso.End();
	CC_show::SelectionList pointlist;
	for (unsigned i = 0; i < GetShow()->GetNumPoints(); ++i)
	{
		CC_coord position = mAnimation->Position(i);
		if (lasso.Inside(position))
		{
			pointlist.insert(i);
		}
	}
	if (altDown)
	{
		GetShow()->ToggleSelection(pointlist);
	}
	else
	{
		GetShow()->AddToSelection(pointlist);
	}
}


// Keystroke command toggles the timer, but the timer
// lives in the Frame.  So we have this weird path...
void
AnimationView::ToggleTimer()
{
	GetAnimationFrame()->ToggleTimer();
}


bool
AnimationView::OnBeat() const
{
	return GetAnimationFrame()->OnBeat();
}

CC_continuity
AnimationView::GetContinuityOnSheet(unsigned whichSheet, unsigned whichContinuity) const
{
	CC_show::const_CC_sheet_iterator_t current_sheet = GetShow()->GetNthSheet(whichSheet);
	return current_sheet->GetNthContinuity(whichContinuity);
}

void
AnimationView::OnNotifyStatus(const wxString& status)
{
	GetAnimationFrame()->SetStatusText(status);
}


bool
AnimationView::OnNotifyErrorList(const std::vector<ErrorMarker>& error_markers, unsigned sheetnum, const wxString& message)
{
	GetAnimationFrame()->OnNotifyErrorList(error_markers, sheetnum, message);
	mErrorOccurred = true;
	return false;
}

boost::shared_ptr<Animation>
AnimationView::GetAnimation()
{
	return mAnimation;
}

AnimationFrame*
AnimationView::GetAnimationFrame()
{
	return static_cast<AnimationFrame*>(GetFrame());
}


const AnimationFrame*
AnimationView::GetAnimationFrame() const
{
	return static_cast<const AnimationFrame*>(GetFrame());
}


CC_show*
AnimationView::GetShow()
{
	return static_cast<CC_show*>(GetDocument());
}


const CC_show*
AnimationView::GetShow() const
{
	return static_cast<const CC_show*>(GetDocument());
}

