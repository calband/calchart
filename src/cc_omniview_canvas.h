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

#include <map>

class AnimationView;

typedef enum
{
	kImageFirst = 0,
	kF0 = kImageFirst,
	kFL0,
	kL0,
	kBL0,
	kB0,
	kBR0,
	kR0,
	kFR0,
	kF1,
	kFL1,
	kL1,
	kBL1,
	kB1,
	kBR1,
	kR1,
	kFR1,
	kF2,
	kFL2,
	kL2,
	kBL2,
	kB2,
	kBR2,
	kR2,
	kFR2,
	kField,
	kLines,
	kDirection,
	kBleachers,
	kWall,
	kSky,
	kCalband,
	kC,
	kCal,
	kCalifornia,
	kEECS,
	kPressbox,
	kCrowd,
	kGoalpost,
	kEndOfShow,
	kImageLast
} WhichImageEnum;

struct viewpoint_t
{
	viewpoint_t(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f) : x(x_), y(y_), z(z_) {}
	float x, y, z;
};

struct MarcherInfo
{
	float direction;
	float x;
	float y;
};

// the rendering context used by all GL canvases
class CCOmniView_GLContext : public wxGLContext
{
public:
    CCOmniView_GLContext(wxGLCanvas *canvas);
	
    void DrawField(float xangle, float yangle, const wxSize &ClientSize, bool crowdOn);
	void Draw3dMarcher(const MarcherInfo &info, const viewpoint_t &viewpoint);
	
private:

	WhichImageEnum GetMarcherTexture(float cameraAngleToMarcher, float marcherDirection);
    // textures for the cube faces
    GLuint m_textures[kImageLast];
};

class CCOmniView_Canvas: public wxGLCanvas
{
public:
	CCOmniView_Canvas(wxFrame *frame, AnimationView *view);
	~CCOmniView_Canvas();

	void SetView(AnimationView *view);

	void Spin(float xSpin, float ySpin);
	void OnPaint(wxPaintEvent& event);
	void OnChar(wxKeyEvent& event);

private:
	std::multimap<double, MarcherInfo> ParseAndDraw3dMarchers();

	AnimationView *mView;
	float m_xangle, m_yangle;
	viewpoint_t mViewPoint;

	bool mFollowMarcher;
	bool mCrowdOn;
	float mViewAngle, mViewAngleZ;
	float mFOV;
	
	wxDECLARE_EVENT_TABLE();
};

#endif // _CC_ONMIVIEW_CANVAS_H_
