/*
 * animation_canvas.h
 * Header for animation canvas interface
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

#ifndef _ANIMATION_CANVAS_H_
#define _ANIMATION_CANVAS_H_

#include <wx/wx.h>

class AnimationView;
class CC_coord;

/**
 * The canvas to which the 2D animation is drawn. The actual drawing is done
 * by the AnimationView.
 */
class AnimationCanvas: public wxPanel
{
public:
	AnimationCanvas(AnimationView *view, wxWindow *parent, const wxSize& size = wxDefaultSize);
	~AnimationCanvas();

	void SetView(AnimationView *view);

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnLeftDownMouseEvent(wxMouseEvent& event);
	void OnLeftUpMouseEvent(wxMouseEvent& event);
	void OnRightUpMouseEvent(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnChar(wxKeyEvent& event);

private:
	AnimationView *mAnimationView;

	static const size_t kDefaultAnimSize = 3;
	float mUserScale;
	wxCoord mUserOriginX;
	static float CalcUserScale(const CC_coord& showSize, const wxSize& windowSize);
	static wxCoord CalcUserOriginX(const CC_coord& showSize, const wxSize& windowSize);

	// for mouse and drawing
	bool mMouseDown;
	long mMouseXStart, mMouseYStart;
	long mMouseXEnd, mMouseYEnd;
	
	wxDECLARE_EVENT_TABLE();
};

#endif // _ANIMATION_CANVAS_H_
