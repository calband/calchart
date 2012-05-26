/*
 * cc_onmiview_canvas.h
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

#ifndef _CC_ONMIVIEW_CANVAS_H_
#define _CC_ONMIVIEW_CANVAS_H_

#include <wx/glcanvas.h>

#include <boost/shared_ptr.hpp>

#include <map>

class AnimationView;
class CCOmniView_GLContext;

struct viewpoint_t
{
	viewpoint_t(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f) : x(x_), y(y_), z(z_) {}
	float x, y, z;
};

struct MarcherInfo
{
	MarcherInfo(float x_ = 0, float y_ = 0, float dir = 0) : direction(dir), x(x_), y(y_) {}
	float direction;
	float x;
	float y;
};

class CCOmniView_Canvas: public wxGLCanvas
{
public:
	CCOmniView_Canvas(AnimationView *view, wxFrame *frame, const wxSize& size = wxDefaultSize);
	~CCOmniView_Canvas();

	void SetView(AnimationView *view);

	void OnPaint(wxPaintEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnMouseMove(wxMouseEvent& event);

	// negative -1 is to stop following
	void OnCmd_FollowMarcher(int which);
	void OnCmd_SaveCameraAngle(size_t which);
	void OnCmd_GoToCameraAngle(size_t which);
	void OnCmd_ToggleCrowd();
	void OnCmd_ToggleMarching();
	void OnCmd_ToggleShowOnlySelected();

private:
	MarcherInfo GetMarcherInfo(size_t which) const;
	std::multimap<double, MarcherInfo> ParseAndDraw3dMarchers() const;

	boost::shared_ptr<CCOmniView_GLContext> m_glContext;
	AnimationView *mView;
	viewpoint_t mViewPoint;

	// a -1 means not following any marcher
	int mFollowMarcher;
	bool mCrowdOn;
	bool mShowOnlySelected;
	bool mShowMarching;
	float mViewAngle, mViewAngleZ;
	float mFOV;
	
	// for mouse camera move:
	bool mShiftMoving;
	wxPoint mStartShiftMoveMousePosition;
	float mStartShiftMoveViewAngle, mStartShiftMoveViewAngleZ;
	
	wxDECLARE_EVENT_TABLE();
};

#endif // _CC_ONMIVIEW_CANVAS_H_
