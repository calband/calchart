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

/**
 * Represents the position of a camera that is viewing the show through omniview.
 */
struct viewpoint_t
{
	/**
	 * Makes the viewpoint (camera position).
	 * @param x_ The x position of the viewpoint/camera.
	 * @param y_ The y position of the viewpoint/camera.
	 * @param z_ The z position of the viewpoint/camera.
	viewpoint_t(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f) : x(x_), y(y_), z(z_) {}
	/**
	 * The x position of the camera.
	 */
	float x;
	/**
	 * The y position of the camera.
	 */
	float y;
	/**
	 * The z position of the camera.
	 */
	float z;
};

/**
 * Information about a marcher used to draw that marcher in omniview.
 */
struct MarcherInfo
{
	/**
	 * Makes a structure to record information about a marcher.
	 * @param x_ The x position of the marcher.
	 * @param y_ The y position of the marcher.
	 * @param dir The direction in which the marcher is facing.
	 */
	MarcherInfo(float x_ = 0, float y_ = 0, float dir = 0) : direction(dir), x(x_), y(y_) {}
	/**
	 * An angle measured in degrees counter-clockwise from the positive x axis
	 * that represents the direction in which the marcher is facing.
	 */
	float direction;
	/**
	 * The x position of the marcher (where the XY plane is the plane
	 * representing the ground in omniview).
	 */
	float x;
	/**
	 * The y position of the marcher (where the XY plane is the plane
	 * representing the ground in omniview).
	 */
	float y;
};

/**
 * The canvas to which the show is drawn when displayed via omniview.
 */
class CCOmniView_Canvas: public wxGLCanvas
{
public:
	/**
	 * Makes the canvas.
	 * @param view The view through which the canvas can access the animation.
	 * @param frame The frame which this canvas is drawn to.
	 * @param size The size of the canvas.
	 */
	CCOmniView_Canvas(AnimationView *view, wxWindow *frame, const wxSize& size = wxDefaultSize);
	/**
	 * Cleanup.
	 */
	~CCOmniView_Canvas();

	/**
	 * Links this canvas to an animation view, which gives the canvas access to
	 * animation information.
	 * @param view The animation view to connect.
	 */
	void SetView(AnimationView *view);

	/**
	 * Called when the canvas is repainted.
	 * @param event Unused.
	 */
	void OnPaint(wxPaintEvent& event);
	/**
	 * Called when a character on the keyboard is pressed.
	 * @param event An event containing information about the key that was
	 * pressed.
	 */
	void OnChar(wxKeyEvent& event);
	/**
	 * Called when the mouse is moved.
	 * @param event An event containing mouse-related information.
	 */
	void OnMouseMove(wxMouseEvent& event);

	/**
	 * Called when the "Follow Marcher" command is selected from the Omniview
	 * menu of the Animation frame.
	 * @param which The index of the point that represents the marcher. An
	 * index of -1 will unfollow any marchers.
	 */
	void OnCmd_FollowMarcher(int which);
	/**
	 * Currently unused.
	 * @param which N/A.
	 */
	void OnCmd_SaveCameraAngle(size_t which);
	/**
	 * Currently unused.
	 * @param which N/A.
	 */
	void OnCmd_GoToCameraAngle(size_t which);
	/**
	 * Called when the "Toggle Crowd" command is selected from the Omniview
	 * menu of the Animation frame.
	 */
	void OnCmd_ToggleCrowd();
	/**
	 * Called when the "Toggle Marching" command is selected from the Omniview
	 * menu of the Animation frame.
	 */
	void OnCmd_ToggleMarching();
	/**
	 * Called when the "Toggle Show Selected" command is selected from the
	 * Omniview menu of the Animation frame.
	 */
	void OnCmd_ToggleShowOnlySelected();

private:
	/**
	 * Builds and returns a MarcherInfo structure for a point.
	 * @param which The index of the point to build a MarcherInfo structure
	 * for.
	 * @return A MarcherInfo structure containing information about the point
	 * with index [which].
	 */
	MarcherInfo GetMarcherInfo(size_t which) const;
	/**
	 * Returns a map which contains a MarcherInfo value for every marcher
	 * that should be drawn, and where the key for each MarcherInfo value
	 * in the map is the distance from the camera/viewpoint to that marcher.
	 * The keys are sorted in increasing order, so that farther marchers
	 * are found at the back of the map.
	 * @return A map where the keys are the distance from the
	 * viewport/camera to a marcher that should be drawn, and the values
	 * are MarcherInfo entries representing the marchers that should
	 * be drawn.
	 */
	std::multimap<double, MarcherInfo> ParseAndDraw3dMarchers() const;

	/**
	 * The OpenGL Rendering Context used to draw to this canvas using
	 * OpenGL.
	 */
	boost::shared_ptr<CCOmniView_GLContext> m_glContext;
	/**
	 * The animation view that gives the canvas access to animation information.
	 */
	AnimationView *mAnimationView;
	/**
	 * The location of the omniview camera.
	 */
	viewpoint_t mViewPoint;

	/**
	 * The index of the point representing the marcher that is being followed.
	 * This is -1 if no marcher is being followed.
	 */
	int mFollowMarcher;
	/**
	 * True if the crowd should be drawn; false otherwise.
	 */
	bool mCrowdOn;
	/**
	 * True if only the selected marchers will be displayed; false if all
	 * marchers should be displayed.
	 */
	bool mShowOnlySelected;
	/**
	 * True if marchers should be animated to look like they are marching.
	 */
	bool mShowMarching;

	/**
	 * The angle at which the camera is oriented in the left/right directions
	 * (about the camera's local z axis).
	 */
	float mViewAngle;
	/**
	 * The angle at which the camera is oriented in the up/down directions
	 * (about the camera's local x axis).
	 */
	float mViewAngleZ;
	/**
	 * The field of view angle for the camera, in the y direction.
	 */
	float mFOV;
	
	// for mouse camera move:
	/**
	 * True if the camera position/orientation was changed while the shift
	 * key was down.
	 */
	bool mShiftMoving;
	/**
	 * The location of the camera before the camera was moved while the shift
	 * key was down.
	 */
	wxPoint mStartShiftMoveMousePosition;
	/**
	 * The camera's view angle before the camera was moved while the shift key
	 * was down.
	 */
	float mStartShiftMoveViewAngle;
	/**
	 * The camera's z view angle before the camera was moved while the shift key
	 * was down.
	 */
	float mStartShiftMoveViewAngleZ;
	
	wxDECLARE_EVENT_TABLE();
};

#endif // _CC_ONMIVIEW_CANVAS_H_
