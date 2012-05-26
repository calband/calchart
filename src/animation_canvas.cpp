/*
 * animation_canvas.cpp
 * Animation canvas interface
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

#include "animation_canvas.h"
#include "animation_view.h"
#include "confgr.h"

#include <wx/dcbuffer.h>


BEGIN_EVENT_TABLE(AnimationCanvas, wxPanel)
EVT_CHAR(AnimationCanvas::OnChar)
EVT_LEFT_DOWN(AnimationCanvas::OnLeftDownMouseEvent)
EVT_LEFT_UP(AnimationCanvas::OnLeftUpMouseEvent)
EVT_RIGHT_UP(AnimationCanvas::OnRightUpMouseEvent)
EVT_MOTION(AnimationCanvas::OnMouseMove)
EVT_PAINT(AnimationCanvas::OnPaint)
EVT_SIZE(AnimationCanvas::OnSize)
END_EVENT_TABLE()


AnimationCanvas::AnimationCanvas(AnimationView *view, wxWindow *parent,
								 const wxSize& size) :
wxPanel(parent, wxID_ANY, wxDefaultPosition, size),
mView(view),
mUserScale(CalcUserScale((view) ? view->GetShowSize() : CC_coord(0, 0), size)),
mMouseDown(false)
{
}


AnimationCanvas::~AnimationCanvas()
{
}


void
AnimationCanvas::SetView(AnimationView *view)
{
	mView = view;
}


void
AnimationCanvas::OnPaint(wxPaintEvent& event)
{
	wxBufferedPaintDC dc(this);

	dc.SetUserScale(mUserScale, mUserScale);
	dc.SetBackground(*CalChartBrushes[COLOR_FIELD]);
	dc.Clear();
	if (mMouseDown)
	{
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(*CalChartPens[COLOR_SHAPES]);
		dc.DrawRectangle(mMouseXStart, mMouseYStart,
						  mMouseXEnd - mMouseXStart, mMouseYEnd - mMouseYStart);
	}
	if (mView)
	{
		mView->OnDraw(&dc);
	}
}


float
AnimationCanvas::CalcUserScale(const CC_coord& showSize, const wxSize& windowSize)
{
	float newX = windowSize.x;
	float newY = windowSize.y;
	float x = (showSize.x) ? showSize.x : newX;
	float y = (showSize.y) ? showSize.y : newY;
	
	float showAspectRatio = x/y;
	float newSizeRatio = newX/newY;
	float newvalue = 1.0;
	// always choose x when the new aspect ratio is smaller than the show.
	// This will keep the whole field on in the canvas
	if (newSizeRatio < showAspectRatio)
	{
		newvalue = newX / (float)Coord2Int(x);
	}
	else
	{
		newvalue = newY / (float)Coord2Int(y);
	}
	return (newvalue * (Coord2Int(1 << 16)/65536.0));
}


void
AnimationCanvas::OnSize(wxSizeEvent& event)
{
	mUserScale = CalcUserScale((mView) ? mView->GetShowSize() : CC_coord(0, 0), event.GetSize());
	Refresh();
}


void
AnimationCanvas::OnLeftDownMouseEvent(wxMouseEvent& event)
{
	wxClientDC dc(this);
	dc.SetUserScale(mUserScale, mUserScale);
	long x,y;
	event.GetPosition(&x, &y);
	x = dc.DeviceToLogicalX( x );
	y = dc.DeviceToLogicalY( y );

	if (!event.AltDown() && !event.ShiftDown())
	{
		mView->UnselectMarchers();
	}

	mMouseXEnd = mMouseXStart = x;
	mMouseYEnd = mMouseYStart = y;
	mMouseDown = true;
}


void
AnimationCanvas::OnLeftUpMouseEvent(wxMouseEvent& event)
{
	wxClientDC dc(this);
	dc.SetUserScale(mUserScale, mUserScale);
	long x,y;
	event.GetPosition(&x, &y);
	x = dc.DeviceToLogicalX( x );
	y = dc.DeviceToLogicalY( y );
	mMouseXEnd = x;
	mMouseYEnd = y;
	mMouseDown = false;

	// if mouse lifted very close to where clicked, then it is a previous beat move
	if ((std::abs(mMouseXEnd - mMouseXStart) < Int2Coord(1)/2) && (std::abs(mMouseYEnd - mMouseYStart) < Int2Coord(1)/2))
	{
		if (mView)
		{
			mView->PrevBeat();
		}
	}
	else
	{
		if (mView)
		{
			mView->SelectMarchersInBox(mMouseXStart, mMouseYStart, mMouseXEnd, mMouseYEnd, event.AltDown());
		}
	}
	Refresh();
}


void
AnimationCanvas::OnRightUpMouseEvent(wxMouseEvent& event)
{
	if (mView)
	{
		mView->NextBeat();
	}
}


void
AnimationCanvas::OnMouseMove(wxMouseEvent& event)
{
	wxClientDC dc(this);
	dc.SetUserScale(mUserScale, mUserScale);
	long x,y;
	event.GetPosition(&x, &y);
	x = dc.DeviceToLogicalX( x );
	y = dc.DeviceToLogicalY( y );
	mMouseXEnd = x;
	mMouseYEnd = y;
	if (event.Dragging())
	{
		Refresh();
	}
}


void AnimationCanvas::OnChar(wxKeyEvent& event)
{
	if (mView)
	{
		if (event.GetKeyCode() == WXK_LEFT)
			mView->PrevBeat();
		else if (event.GetKeyCode() == WXK_RIGHT)
			mView->NextBeat();
		else if (event.GetKeyCode() == WXK_SPACE)
		{
			mView->ToggleTimer();
		}
		else
		{
			event.Skip();
		}
	}
	else
	{
		event.Skip();
	}
}

