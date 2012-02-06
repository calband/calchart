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

#include "cc_omniview_canvas.h"
#include "anim_ui.h"
#include "confgr.h"
#include "modes.h"
#include "animation_frame.h"

#include <GLUT/glut.h>

#include <wx/dcbuffer.h>

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

// Note about drawing:
// openGL uses a coordinate system that is like:
//
//    ^ y
//    |  x
// <--+--->
//    |
//    v -y
//
// where as calchart uses a system like:
//
//    ^ -y
//    |  x
// <--+--->
//    |
//    v y
//
// care must be taken to getting these correct

typedef struct { WhichImageEnum mImageId; wxString mImageFilename; } id_string_t;

#if defined(__APPLE__) && (__APPLE__)
const static wxString kImageDir = wxT("CalChart.app/image/");
#else
const static wxString kImageDir = wxT("image/");
#endif

static const viewpoint_t KStartingViewPoint = viewpoint_t(0, -16, 2.5);
static const float kStartingViewAngle = M_PI/2;
static const float kStartingViewAngleZ = 0;

id_string_t ListOfImageFiles[] = {
	{ kF0, wxT("f0.tga") },
	{ kF1, wxT("f1.tga") },
	{ kF2, wxT("f2.tga") },
	{ kFL0, wxT("fl0.tga") },
	{ kFL1, wxT("fl1.tga") },
	{ kFL2, wxT("fl2.tga") },
	{ kL0, wxT("l0.tga") },
	{ kL1, wxT("l1.tga") },
	{ kL2, wxT("l2.tga") },
	{ kBL0, wxT("bl0.tga") },
	{ kBL1, wxT("bl1.tga") },
	{ kBL2, wxT("bl2.tga") },
	{ kB0, wxT("b0.tga") },
	{ kB1, wxT("b1.tga") },
	{ kB2, wxT("b2.tga") },
	{ kBR0, wxT("br0.tga") },
	{ kBR1, wxT("br1.tga") },
	{ kBR2, wxT("br2.tga") },
	{ kR0, wxT("r0.tga") },
	{ kR1, wxT("r1.tga") },
	{ kR2, wxT("r2.tga") },
	{ kFR0, wxT("fr0.tga") },
	{ kFR1, wxT("fr1.tga") },
	{ kFR2, wxT("fr2.tga") },
	{ kField, wxT("field.tga") },
	// lines are created when needed for each show.
//	{ kLines, wxT("lines.tga") },
	{ kDirection, wxT("Direction.tga") },
	{ kBleachers, wxT("bleachers.tga") },
	{ kWall, wxT("wall.tga") },
	{ kSky, wxT("sky.tga") },
	{ kCalband, wxT("calband.tga") },
	{ kC, wxT("C.tga") },
	{ kCal, wxT("cal.tga") },
	{ kCalifornia, wxT("california.tga") },
	{ kEECS, wxT("eecs.tga") },
	{ kPressbox, wxT("pressbox.tga") },
	{ kCrowd, wxT("crowd.tga") },
	{ kGoalpost, wxT("goalpost.tga") },
	{ kEndOfShow, wxT("endofshow.tga") }
};


BEGIN_EVENT_TABLE(CCOmniView_Canvas, wxGLCanvas)
EVT_CHAR(CCOmniView_Canvas::OnChar)
EVT_MOTION(CCOmniView_Canvas::OnMouseMove)
EVT_PAINT(CCOmniView_Canvas::OnPaint)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

static void
CheckGLError()
{
    GLenum errLast = GL_NO_ERROR;
	
    for ( ;; )
    {
        GLenum err = glGetError();
        if ( err == GL_NO_ERROR )
            return;
		
        // normally the error is reset by the call to glGetError() but if
        // glGetError() itself returns an error, we risk looping forever here
        // so check that we get a different error than the last time
        if ( err == errLast )
        {
            wxLogError(wxT("OpenGL error state couldn't be reset."));
            return;
        }
		
        errLast = err;
		
        wxLogError(wxT("OpenGL error %d"), err);
    }
}

static float
NormalizeAngle(float angle)
{
	while (angle > 2*M_PI)
	{
		angle -= 2*M_PI;
	}
	while (angle < 0.0)
	{
		angle += 2*M_PI;
	}
	return angle;
}

static bool
LoadTextureWithImage(const wxImage &image, GLuint& texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (image.HasAlpha())
	{
		const size_t width = image.GetWidth();
		const size_t height = image.GetHeight();
		std::vector<uint8_t> mixedAlpha(width * height * 4);
		for (size_t y = 0; y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				mixedAlpha.at(x*4 + (y * width * 4) + 0) = image.GetRed(x, y);
				mixedAlpha.at(x*4 + (y * width * 4) + 1) = image.GetGreen(x, y);
				mixedAlpha.at(x*4 + (y * width * 4) + 2) = image.GetBlue(x, y);
				mixedAlpha.at(x*4 + (y * width * 4) + 3) = image.GetAlpha(x, y);
			}
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &mixedAlpha[0]);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.GetWidth(), image.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, image.GetData());
	}
	return true;
}

static bool
LoadTexture(const wxString &filename, GLuint& texture)
{
	wxImage image;
	if ( !image.LoadFile(filename) )
	{
		wxLogError(wxT("Couldn't load image from ") + filename + wxT("."));
		return false;
	}
	return LoadTextureWithImage(image, texture);
}

// We always draw from the upper left, upper right, lower right, lower left
static void
DrawTextureOnBox(const float points[4][3], size_t repeat_x, size_t repeat_y, const GLuint &texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0); glVertex3f(points[0][0], points[0][1], points[0][2]);
	glTexCoord2f(repeat_x,0); glVertex3f(points[1][0], points[1][1], points[1][2]);
	glTexCoord2f(repeat_x,repeat_y); glVertex3f(points[2][0], points[2][1], points[2][2]);
	glTexCoord2f(0,repeat_y); glVertex3f(points[3][0], points[3][1], points[3][2]);
	glEnd();
}

static void
DrawBox(const float points[4][3])
{
	glBegin(GL_QUADS);
	glVertex3f(points[0][0], points[0][1], points[0][2]);
	glVertex3f(points[1][0], points[1][1], points[1][2]);
	glVertex3f(points[2][0], points[2][1], points[2][2]);
	glVertex3f(points[3][0], points[3][1], points[3][2]);
	glEnd();
}

static WhichImageEnum
GetMarcherTextureAndPoints(float cameraAngleToMarcher, float marcherDirection, float &x1, float &x2, float &y1, float &y2)
{
	// Returns which direction they are facing in regards to the camera.
	float relativeAngle = marcherDirection - cameraAngleToMarcher; // convert to relative angle;
	relativeAngle = NormalizeAngle(relativeAngle);
	
	if (cameraAngleToMarcher >= 1.0*M_PI/8.0 && cameraAngleToMarcher < 3.0*M_PI/8.0) 
	{
		y1 += 0.45;
		y2 -= 0.45;
		x1 -= 0.45;
		x2 += 0.45;
	}
	else if (cameraAngleToMarcher >= 3.0*M_PI/8.0 && cameraAngleToMarcher < 5.0*M_PI/8.0)
	{
		x1 -= 0.45;
		x2 += 0.45;
	}
	else if (cameraAngleToMarcher >= 5.0*M_PI/8.0 && cameraAngleToMarcher < 7.0*M_PI/8.0)
	{
		y1 -= 0.45;
		y2 += 0.45;
		x1 -= 0.45;
		x2 += 0.45;
	}
	else if (cameraAngleToMarcher >= 7.0*M_PI/8.0 && cameraAngleToMarcher < 9.0*M_PI/8.0)
	{
		y1 -= 0.45;
		y2 += 0.45;
	}
	else if (cameraAngleToMarcher >= 9.0*M_PI/8.0 && cameraAngleToMarcher < 11.0*M_PI/8.0)
	{
		y1 -= 0.45;
		y2 += 0.45;
		x1 += 0.45;
		x2 -= 0.45;
	}
	else if (cameraAngleToMarcher >= 11.0*M_PI/8.0 && cameraAngleToMarcher < 13.0*M_PI/8.0)
	{
		x1 += 0.45;
		x2 -= 0.45;
	}
	else if (cameraAngleToMarcher >= 13.0*M_PI/8.0 && relativeAngle < 15.0*M_PI/8.0)
	{
		y1 += 0.45;
		y2 -= 0.45;
		x1 += 0.45;
		x2 -= 0.45;
	}
	else // if (cameraAngleToMarcher >= 15.0*M_PI/8.0 || cameraAngleToMarcher < 1.0*M_PI/8.0) // and everything else
	{
		y1 += 0.45;
		y2 -= 0.45;
	}

	if (relativeAngle >= 1.0*M_PI/8.0 && relativeAngle < 3.0*M_PI/8.0) 
	{
		return kBR0;
	}
	else if (relativeAngle >= 3.0*M_PI/8.0 && relativeAngle < 5.0*M_PI/8.0)
	{
		return kR0;
	}
	else if (relativeAngle >= 5.0*M_PI/8.0 && relativeAngle < 7.0*M_PI/8.0)
	{
		return kFR0;
	}
	else if (relativeAngle >= 7.0*M_PI/8.0 && relativeAngle < 9.0*M_PI/8.0)
	{
		return kF0;
	}
	else if (relativeAngle >= 9.0*M_PI/8.0 && relativeAngle < 11.0*M_PI/8.0)
	{
		return kFL0;
	}
	else if (relativeAngle >= 11.0*M_PI/8.0 && relativeAngle < 13.0*M_PI/8.0)
	{
		return kL0;
	}
	else if (relativeAngle >= 13.0*M_PI/8.0 && relativeAngle < 15.0*M_PI/8.0)
	{
		return kBL0;
	}
	else // if (relativeAngle >= 15.0*M_PI/8.0 || relativeAngle < 1.0*M_PI/8.0) // and everything else
	{
		return kB0;
	}
}

// Returns the angle in regards to the camera.
static float
GetAngle(float x, float y, const viewpoint_t &viewpoint)
{
	viewpoint_t v = viewpoint_t(x - viewpoint.x, y - viewpoint.y);
	float mag = (sqrt(v.x*v.x + v.y*v.y));
	float ang = acos(v.x/mag); // normalize
	return (v.y < 0) ? -ang : ang;
}


CCOmniView_GLContext::CCOmniView_GLContext(wxGLCanvas *canvas) :
wxGLContext(canvas)
{
	SetCurrent(*canvas);
	
    // set up the parameters we want to use
    glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);			// to allow alpha values to be considered
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // to allow alpha values to be considered
	glClearColor(0.289,0.941,1.0,1.0);  // Default Background Color
	
    glGenTextures(WXSIZEOF(m_textures), m_textures);

	wxInitAllImageHandlers();
	for (size_t i = 0; i < sizeof(ListOfImageFiles)/sizeof(ListOfImageFiles[0]); ++i)
	{
		if (!LoadTexture(kImageDir + ListOfImageFiles[i].mImageFilename, m_textures[ListOfImageFiles[i].mImageId]))
		{
			wxLogError(wxT("Could not load ") + ListOfImageFiles[i].mImageFilename);
		}
	}

    CheckGLError();
}

bool
CCOmniView_GLContext::UseForLines(const wxImage &lines)
{
	return LoadTextureWithImage(lines, m_textures[kLines]);
}

void
CCOmniView_GLContext::DrawField(float FieldEW, float FieldNS, bool crowdOn)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
	
	// because we look up to see the sky, make sure the texture is pointing down:
	const float points00[][3] = { { 1000, -1000, 30 }, { -1000, -1000, 30 }, { -1000, 1000, 30 }, { 1000, 1000, 30} };
	DrawTextureOnBox(points00, 20, 20, m_textures[kSky]);
	const float points01[][3] = { { -1*(FieldNS/2.0+25), (FieldEW/2.0+10), 0 }, { (FieldNS/2.0+25), (FieldEW/2.0+10), 0 }, { (FieldNS/2.0+25), -1*(FieldEW/2.0+10), 0 }, { -1*(FieldNS/2.0+25), -1*(FieldEW/2.0+10), 0 } };
	DrawTextureOnBox(points01, 2, 2, m_textures[kField]);
	const float points02[][3] = { { FieldNS/2.0+16, +FieldEW/2.0, 0 }, { FieldNS/2.0+16, -FieldEW/2.0, 0 }, { FieldNS/2.0, -FieldEW/2.0, 0 }, { FieldNS/2.0, FieldEW/2.0, 0 } };
	DrawTextureOnBox(points02, 1, 1, m_textures[kCalifornia]);
	const float points03[][3] = { { -1*(FieldNS/2.0+16), -1*(FieldEW/2.0), 0 }, { -1*(FieldNS/2.0+16), -1*(-FieldEW/2.0), 0 }, { -1*(FieldNS/2.0), -1*(-FieldEW/2.0), 0 }, { -1*(FieldNS/2.0), -1*(FieldEW/2.0), 0 } };
	DrawTextureOnBox(points03, 1, 1, m_textures[kEECS]);
	const float points04[][3] = { { -FieldNS/2.0, FieldEW/2.0, 0 }, { FieldNS/2.0, FieldEW/2.0, 0 }, { FieldNS/2.0, -FieldEW/2.0, 0 }, { -FieldNS/2.0, -FieldEW/2.0, 0 } };
	DrawTextureOnBox(points04, 1, 1, m_textures[kLines]);
	const float points05[][3] = { { -10, 10, 0 }, { 10, 10, 0 }, { 10, -10, 0 }, { -10, -10, 0 } };
	DrawTextureOnBox(points05, 1, 1, m_textures[kCalband]);
	
	// stands:
	const float points06[][3] = { { -FieldNS/2.0 - 25, FieldEW/2.0 + 30, 25 } , { FieldNS/2.0 + 25, FieldEW/2.0 + 30, 25 } , { FieldNS/2.0 + 25, FieldEW/2.0 + 10, 0 }, { -FieldNS/2.0-25, FieldEW/2.0 + 10, 0 } };
	DrawTextureOnBox(points06, 5, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points07[][3] = { { -(-FieldNS/2.0 - 25), -(FieldEW/2.0 + 30), 25 } , { -(FieldNS/2.0 + 25), -(FieldEW/2.0 + 30), 25 } , { -(FieldNS/2.0 + 25), -(FieldEW/2.0 + 10), 0 }, { -(-FieldNS/2.0-25), -(FieldEW/2.0 + 10), 0 } };
	DrawTextureOnBox(points07, 5, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points08[][3] = { { -FieldNS/2.0-25, -FieldEW/2.0-10, 0 }, { -FieldNS/2.0-25, -FieldEW/2.0-10, 0 }, { -FieldNS/2.0-50, -FieldEW/2.0-10, 25 }, { -FieldNS/2.0-25, -FieldEW/2.0-30, 25 } };
	DrawTextureOnBox(points08, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points09[][3] = { { -FieldNS/2.0-25, FieldEW/2.0 + 10, 0 }, { -FieldNS/2.0-25, FieldEW/2.0 + 10, 0 }, { -FieldNS/2.0-25, FieldEW/2.0 + 30, 25 }, { -FieldNS/2.0-50, FieldEW/2.0 + 10, 25 } };
	DrawTextureOnBox(points09, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points10[][3] = { { FieldNS/2.0+25, -FieldEW/2.0-10, 0 }, { FieldNS/2.0+25, -FieldEW/2.0-10, 0 }, { FieldNS/2.0+25, -FieldEW/2.0-30, 25 }, { FieldNS/2.0+50, -FieldEW/2.0-10, 25 } };
	DrawTextureOnBox(points10, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points11[][3] = { { FieldNS/2.0+25, FieldEW/2.0 + 10, 0 }, { FieldNS/2.0+25, FieldEW/2.0 + 10, 0 }, { FieldNS/2.0+50, FieldEW/2.0 + 10, 25 }, { FieldNS/2.0+25, FieldEW/2.0 + 30, 25 } };
	DrawTextureOnBox(points11, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points12[][3] = { { -FieldNS/2.0 - 50, -FieldEW/2.0 - 10, 25 }, { -FieldNS/2.0 - 50, FieldEW/2.0 + 10, 25 }, { -FieldNS/2.0 - 25, FieldEW/2.0 + 10, 0 }, { -FieldNS/2.0 - 25, -FieldEW/2.0 - 10, 0 }  };
	DrawTextureOnBox(points12, 4, 5, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points13[][3] = { { -(-FieldNS/2.0 - 50), -(-FieldEW/2.0 - 10), 25 }, { -(-FieldNS/2.0 - 50), -(FieldEW/2.0 + 10), 25 }, { -(-FieldNS/2.0 - 25), -(FieldEW/2.0 + 10), 0 }, { -(-FieldNS/2.0 - 25), -(-FieldEW/2.0 - 10), 0 }  };
	DrawTextureOnBox(points13, 4, 5, m_textures[crowdOn ? kCrowd : kBleachers]);

	glColor3f(0.0,0.0,0.0);
	const float points14[][3] = { { FieldNS/2.0 + 25, 8, 0 }, { FieldNS/2.0 + 25, -8, 0 }, { FieldNS/2.0 + 32, -8, 6 }, { FieldNS/2.0 + 32, 8, 6 } };
	DrawBox(points14); // North Tunnel
	const float points15[][3] = { { -FieldNS/2.0 - 25, 8, 0 }, { -FieldNS/2.0 - 25, -8, 0 }, { -FieldNS/2.0 - 32, -8, 6 }, { -FieldNS/2.0 - 32, 8, 6 } };
	DrawBox(points15); // South Tunnel
	glColor3f(1.0,1.0,1.0);
	
	const float points16[][3] = { { -40, FieldEW/2.0 + 30, 35 } , { 40, FieldEW/2.0 + 30, 35 }, { 40, FieldEW/2.0 + 30, 25 }, { -40, FieldEW/2.0 + 30, 25 } };
	DrawTextureOnBox(points16, 1, 1, m_textures[kPressbox]);
	
	const float points17[][3] = { { 10, -(FieldEW/2.0 + 30), 25 }, { -10, -(FieldEW/2.0 + 30), 25 },{ -10, -(FieldEW/2.0 + 10), 0 }, { 10, -(FieldEW/2.0 + 10), 0 } };
	DrawTextureOnBox(points17, 1, 1, m_textures[kC]);

	const float points18[][3] = { { -FieldNS/2.0-25, FieldEW/2.0 + 10, 0 }, { FieldNS/2.0+25, FieldEW/2.0 + 10, 0 }, { FieldNS/2.0 + 25, FieldEW/2.0 + 10, 3 }, { -FieldNS/2.0 - 25, FieldEW/2.0 + 10, 3 } };
	DrawTextureOnBox(points18, 5, 1, m_textures[kWall]);
	const float points19[][3] = { { -FieldNS/2.0-25, -FieldEW/2.0-10, 0 }, { FieldNS/2.0+25, -FieldEW/2.0 - 10, 0 }, { FieldNS/2.0 + 25, -FieldEW/2.0 - 10, 3 }, { -FieldNS/2.0 - 25, -FieldEW/2.0 - 10, 3 } };
	DrawTextureOnBox(points19, 5, 1, m_textures[kWall]);
	const float points20[][3] = { { -FieldNS/2.0 - 25, 8, 0 }, { -FieldNS/2.0 - 25, FieldEW/2.0+10, 0 }, { -FieldNS/2.0 - 25, FieldEW/2.0 + 10, 3 }, { -FieldNS/2.0 - 25, 8, 3 } };
	DrawTextureOnBox(points20, 2, 1, m_textures[kWall]);
	const float points21[][3] = { { -FieldNS/2.0 - 25, -FieldEW/2.0-10, 0 }, { -FieldNS/2.0 - 25, -8, 0 }, { -FieldNS/2.0 - 25, -8, 3 }, { -FieldNS/2.0 - 25, -FieldEW/2.0 - 10, 3 } };
	DrawTextureOnBox(points21, 2, 1, m_textures[kWall]);
	const float points22[][3] = { { FieldNS/2.0 + 25, 8, 0 }, { FieldNS/2.0 + 25, FieldEW/2.0+10, 0 }, { FieldNS/2.0 + 25, FieldEW/2.0 + 10, 3 }, { FieldNS/2.0 + 25, 8, 3 } };
	DrawTextureOnBox(points22, 2, 1, m_textures[kWall]);
	const float points23[][3] = { { FieldNS/2.0 + 25, -FieldEW/2.0-10, 0 }, { FieldNS/2.0 + 25, -8, 0 }, { FieldNS/2.0 + 25, -8, 3 }, { FieldNS/2.0 + 25, -FieldEW/2.0 - 10, 3 } };
	DrawTextureOnBox(points23, 2, 1, m_textures[kWall]);
	
    glFlush();
	
    CheckGLError();
}

void
CCOmniView_GLContext::Draw3dMarcher(const MarcherInfo &info, const viewpoint_t &viewpoint, WhichMarchingStyle style)
{
	float ang = NormalizeAngle(GetAngle(info.x, info.y, viewpoint));
	float dir = info.direction;

	float x1, x2, y1, y2;
	x1 = x2 = info.x;
	y1 = y2 = info.y;
	float z1 = 3, z2 = 0;

	WhichImageEnum face = GetMarcherTextureAndPoints(ang, dir, x1, x2, y1, y2);

	// if we want to modify the marching:
	switch (style)
	{
		case kLeftHSHup:
			face = static_cast<WhichImageEnum>(face + kF1);
			break;
		case kRightHSHup:
			face = static_cast<WhichImageEnum>(face + kF2);
			break;
		default:
			break;
	}
	
	glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
	
	const float points[][3] = { { x1, y1, z1 }, { x2, y2, z1 }, { x2, y2, z2 }, { x1, y1, z2 } };
	DrawTextureOnBox(points, 1, 1, m_textures[face]);
}


CCOmniView_Canvas::CCOmniView_Canvas(wxFrame *frame, AnimationView *view) :
wxGLCanvas(frame, wxID_ANY, NULL, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
mView(view),
mFrame(static_cast<AnimationFrame*>(frame)),
mViewPoint(KStartingViewPoint),
mFollowMarcher(-1),
mCrowdOn(false),
mShowOnlySelected(false),
mShowMarching(true),
mViewAngle(kStartingViewAngle),
mViewAngleZ(kStartingViewAngleZ),
mFOV(60),
mShiftMoving(false)
{
	m_glContext = new CCOmniView_GLContext(this);
    m_glContext->UseForLines(mView->GetShow()->GetMode().GetOmniLinesImage());
}


CCOmniView_Canvas::~CCOmniView_Canvas()
{
	delete m_glContext;
}

void
CCOmniView_Canvas::SetView(AnimationView *view)
{
	mView = view;
}


MarcherInfo
CCOmniView_Canvas::GetMarcherInfo(size_t which)
{
	MarcherInfo info;
	if (mView && mView->mAnimation)
	{
		info.direction = NormalizeAngle((mView->mAnimation->RealDirection(which) * M_PI / 180.0));
		
		CC_coord position = mView->mAnimation->Position(which);
		info.x = Coord2Float(position.x);
		// because the coordinate system for continuity and OpenGL are different, correct here.
		info.y = -1.0 * Coord2Float(position.y);
	}
	return info;
}

std::multimap<double, MarcherInfo>
CCOmniView_Canvas::ParseAndDraw3dMarchers()
{
	std::multimap<double, MarcherInfo> result;
	for (size_t i = 0; (i < mView->GetShow()->GetNumPoints()); ++i)
	{
		if (mShowOnlySelected && !mView->GetShow()->IsSelected(i))
		{
			continue;
		}
		MarcherInfo info = GetMarcherInfo(i);
		float distance = sqrt(pow(mViewPoint.x-info.x,2)+pow(mViewPoint.y-info.y,2));
		result.insert(std::pair<double, MarcherInfo>(distance, info));
	}
	return result;
}

void
CCOmniView_Canvas::OnPaint(wxPaintEvent& event)
{
    // This is required even though dc is not used otherwise.
    wxPaintDC dc(this);
	SetCurrent(*m_glContext);

    // Set the OpenGL viewport according to the client size of this canvas.
    // This is done here rather than in a wxSizeEvent handler because our
    // OpenGL rendering context (and thus viewport setting) is used with
    // multiple canvases: If we updated the viewport in the wxSizeEvent
    // handler, changing the size of one canvas causes a viewport setting that
    // is wrong when next another canvas is repainted.
    const wxSize ClientSize = GetClientSize();
	
    glViewport(0, 0, ClientSize.x, ClientSize.y);

	CC_coord fieldSize = mView ? mView->GetShow()->GetMode().FieldSize() : CC_coord(160, 80);
	float FieldEW = Coord2Float(fieldSize.y);
	float FieldNS = Coord2Float(fieldSize.x);
	
	glClear(GL_COLOR_BUFFER_BIT);
	// set our view point:
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(mFOV, static_cast<float>(ClientSize.x) / static_cast<float>(ClientSize.y), 0.1, 2*FieldNS);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (mFollowMarcher != -1)
	{
		MarcherInfo info = GetMarcherInfo(mFollowMarcher);
		mViewPoint.x = info.x;
		mViewPoint.y = info.y;
		mViewPoint.z = 2;
		mViewAngle = info.direction;
		mViewAngleZ = 0;
	}
	gluLookAt(mViewPoint.x, mViewPoint.y, mViewPoint.z, mViewPoint.x + cos(mViewAngle), mViewPoint.y + sin(mViewAngle), mViewPoint.z + sin(mViewAngleZ), 0.0, 0.0, 1.0);
	
    // Render the graphics and swap the buffers.
    m_glContext->DrawField(FieldEW, FieldNS, mCrowdOn);
	if (mView && mView->mAnimation)
	{
		std::multimap<double, MarcherInfo> marchers = ParseAndDraw3dMarchers();
		for (std::multimap<double, MarcherInfo>::reverse_iterator i = marchers.rbegin(); i != marchers.rend(); ++i)
		{
			m_glContext->Draw3dMarcher(i->second, mViewPoint, mShowMarching ? (mView->OnBeat() ? kLeftHSHup : kRightHSHup) : kClosed);
		}
	}

    SwapBuffers();
}

void
CCOmniView_Canvas::OnChar(wxKeyEvent& event)
{
	const float stepIncr = 0.3 * 3;
	const float AngleStepIncr = 0.1 * 3;
	switch (event.GetKeyCode())
	{
			// predetermined camera angles:
		case '1':
			OnCmd_FollowMarcher(-1);
			mViewPoint = viewpoint_t(0, -16, 2.5);
			mViewAngle = M_PI/2;
			mViewAngleZ = 0;
			break;
		case '2':
			OnCmd_FollowMarcher(-1);
			mViewPoint = viewpoint_t(0, -60, 20);
			mViewAngle = M_PI/2;
			mViewAngleZ = -M_PI/8;
			break;
		case '3':
			OnCmd_FollowMarcher(-1);
			mViewPoint = viewpoint_t(60, -55, 20);
			mViewAngle = 11*M_PI/16;
			mViewAngleZ = -M_PI/8;
			break;

		case '-':
			OnCmd_FollowMarcher(-1);
			mViewPoint.z -= stepIncr;
			mViewPoint.z = std::max(mViewPoint.z, 0.3f);
			break;
		case '+':
			OnCmd_FollowMarcher(-1);
			mViewPoint.z += stepIncr;
			break;
		case 'q':
			OnCmd_FollowMarcher(-1);
			mViewAngle += AngleStepIncr;
			mViewAngle = NormalizeAngle(mViewAngle);
			break;
		case 'w':
			OnCmd_FollowMarcher(-1);
			mViewAngle -= AngleStepIncr;
			mViewAngle = NormalizeAngle(mViewAngle);
			break;
		case 'a':
			OnCmd_FollowMarcher(-1);
			mViewAngleZ += AngleStepIncr;
			break;
		case 'z':
			OnCmd_FollowMarcher(-1);
			mViewAngleZ -= AngleStepIncr;
			break;
		case '>':
			mFOV += 5;
			mFOV = std::min<float>(mFOV, 160);
			break;
		case '<':
			mFOV -= 5;
			mFOV = std::max<float>(mFOV, 10);
			break;

		case 'o':
			OnCmd_ToggleCrowd();
			break;

		case 't':
			OnCmd_ToggleShowOnlySelected();
			break;

		case WXK_LEFT:
			OnCmd_FollowMarcher(-1);
			mViewPoint.x += -stepIncr*cos(mViewAngle - M_PI/2);
			mViewPoint.y += -stepIncr*sin(mViewAngle - M_PI/2);
			break;
		case WXK_RIGHT:
			OnCmd_FollowMarcher(-1);
			mViewPoint.x += stepIncr*cos(mViewAngle - M_PI/2);
			mViewPoint.y += stepIncr*sin(mViewAngle - M_PI/2);
			break;
		case WXK_DOWN:
			OnCmd_FollowMarcher(-1);
			mViewPoint.x += -stepIncr*cos(mViewAngle);
			mViewPoint.y += -stepIncr*sin(mViewAngle);
			mViewPoint.z += -stepIncr*sin(mViewAngleZ);
			mViewPoint.z = std::max(mViewPoint.z, 0.3f);
			break;
		case WXK_UP:
			OnCmd_FollowMarcher(-1);
			mViewPoint.x += stepIncr*cos(mViewAngle);
			mViewPoint.y += stepIncr*sin(mViewAngle);
			mViewPoint.z += stepIncr*sin(mViewAngleZ);
			mViewPoint.z = std::max(mViewPoint.z, 0.3f);
			break;
		case WXK_SPACE:
			mView->ToggleTimer();
			break;
		default:
			event.Skip();
	}

	Refresh();
}

void
CCOmniView_Canvas::OnMouseMove(wxMouseEvent &event)
{
	if (event.ShiftDown())
	{
		wxPoint thisPos = event.GetPosition();
		if (!mShiftMoving)
		{
			mShiftMoving = true;
			mStartShiftMoveMousePosition = thisPos;
			mStartShiftMoveViewAngle = mViewAngle;
			mStartShiftMoveViewAngleZ = mViewAngleZ;
		}
		const wxSize ClientSize = GetClientSize();
		wxPoint delta = thisPos - mStartShiftMoveMousePosition;
		mViewAngle = mStartShiftMoveViewAngle + sin(delta.x/static_cast<float>(ClientSize.x));
		mViewAngleZ = mStartShiftMoveViewAngleZ + sin(delta.y/static_cast<float>(ClientSize.y));
		Refresh();
	}
	else
	{
		mShiftMoving = false;
	}
}

void
CCOmniView_Canvas::OnCmd_FollowMarcher(int which)
{
	mFollowMarcher = which;
	Refresh();
}

void
CCOmniView_Canvas::OnCmd_SaveCameraAngle(size_t which)
{
}

void
CCOmniView_Canvas::OnCmd_GoToCameraAngle(size_t which)
{
}

void
CCOmniView_Canvas::OnCmd_ToggleCrowd()
{
	mCrowdOn = !mCrowdOn;
	Refresh();
}

void
CCOmniView_Canvas::OnCmd_ToggleMarching()
{
	mShowMarching = !mShowMarching;
	Refresh();
}

void
CCOmniView_Canvas::OnCmd_ToggleShowOnlySelected()
{
	mShowOnlySelected = !mShowOnlySelected;
	Refresh();
}

