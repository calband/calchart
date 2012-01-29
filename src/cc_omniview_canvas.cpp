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

#include <GLUT/glut.h>

#include <wx/dcbuffer.h>

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#define DO_CCVIEW 1

typedef struct { WhichImageEnum mImageId; wxString mImageFilename; } id_string_t;

const static wxString kImageDir = wxT("../../../resources/image/");

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
	{ kLines, wxT("lines.tga") },
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
EVT_PAINT(CCOmniView_Canvas::OnPaint)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

static void CheckGLError()
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

bool LoadTexture(const wxString &filename, GLuint& texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	wxImage image;
	if ( !image.LoadFile(filename) )
	{
		wxLogError(wxT("Couldn't load image from '%s'."), filename.c_str());
		return false;
	}

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


CCOmniView_GLContext::CCOmniView_GLContext(wxGLCanvas *canvas) :
wxGLContext(canvas)
{
	SetCurrent(*canvas);
	
    // set up the parameters we want to use
    glEnable(GL_TEXTURE_2D);
	
    // set viewing projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);
	
    // create the textures to use for cube sides: they will be reused by all
    // canvases (which is probably not critical in the case of simple textures
    // we use here but could be really important for a real application where
    // each texture could take many megabytes)
    glGenTextures(WXSIZEOF(m_textures), m_textures);

	wxInitAllImageHandlers();
	for (size_t i = 0; i < sizeof(ListOfImageFiles)/sizeof(ListOfImageFiles[0]); ++i)
	{
		LoadTexture(kImageDir + ListOfImageFiles[i].mImageFilename, m_textures[ListOfImageFiles[i].mImageId]);
	}

	glEnable(GL_BLEND);			// to allow alpha values to be considered
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // to allow alpha values to be considered
	glClearColor(0.289,0.941,1.0,1.0);  // Default Background Color
    CheckGLError();

}


void
DrawBox(const std::pair<float,float> &start, const std::pair<float,float> &end, const std::pair<float,float> z)
{
	glBegin (GL_POLYGON);
	glVertex3f(start.first, start.second, z.first);
	glVertex3f(end.first, start.second, z.first);
	glVertex3f(end.first, end.second, z.second);
	glVertex3f(start.first, end.second, z.second);
	glEnd ();
}


void
DrawTextureOnBox(const float points[4][3], size_t repeat_x, size_t repeat_y, const GLuint &texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0,repeat_y); glVertex3f(points[0][0], points[0][1], points[0][2]);
	glTexCoord2f(repeat_x,repeat_y); glVertex3f(points[1][0], points[1][1], points[1][2]);
	glTexCoord2f(repeat_x,0); glVertex3f(points[2][0], points[2][1], points[2][2]);
	glTexCoord2f(0,0); glVertex3f(points[3][0], points[3][1], points[3][2]);
	glEnd();
}


void
DrawBox(const float points[4][3])
{
	glBegin(GL_QUADS);
	glVertex3f(points[0][0], points[0][1], points[0][2]);
	glVertex3f(points[1][0], points[1][1], points[1][2]);
	glVertex3f(points[2][0], points[2][1], points[2][2]);
	glVertex3f(points[3][0], points[3][1], points[3][2]);
	glEnd();
}


void
CCOmniView_GLContext::DrawField(float xangle, float yangle, const wxSize &ClientSize, bool crowdOn)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
#if 0//!DO_CCVIEW
	
//    glLoadIdentity();
//    glTranslatef(0.0f, 0.0f, -2.0f);
//    glRotatef(xangle, 1.0f, 0.0f, 0.0f);
//    glRotatef(yangle, 0.0f, 1.0f, 0.0f);
	
    // draw six faces of a cube of size 1 centered at (0, 0, 0)
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glBegin(GL_QUADS);
	glNormal3f( 0.0f, 0.0f, 1.0f);
	glTexCoord2f(0, 0); glVertex3f( 0.5f, 0.5f, 0.5f);
	glTexCoord2f(1, 0); glVertex3f(-0.5f, 0.5f, 0.5f);
	glTexCoord2f(1, 1); glVertex3f(-0.5f,-0.5f, 0.5f);
	glTexCoord2f(0, 1); glVertex3f( 0.5f,-0.5f, 0.5f);
    glEnd();
	
    glBindTexture(GL_TEXTURE_2D, m_textures[1]);
    glBegin(GL_QUADS);
	glNormal3f( 0.0f, 0.0f,-1.0f);
	glTexCoord2f(0, 0); glVertex3f(-0.5f,-0.5f,-0.5f);
	glTexCoord2f(1, 0); glVertex3f(-0.5f, 0.5f,-0.5f);
	glTexCoord2f(1, 1); glVertex3f( 0.5f, 0.5f,-0.5f);
	glTexCoord2f(0, 1); glVertex3f( 0.5f,-0.5f,-0.5f);
    glEnd();
	
    glBindTexture(GL_TEXTURE_2D, m_textures[2]);
    glBegin(GL_QUADS);
	glNormal3f( 0.0f, 1.0f, 0.0f);
	glTexCoord2f(0, 0); glVertex3f( 0.5f, 0.5f, 0.5f);
	glTexCoord2f(1, 0); glVertex3f( 0.5f, 0.5f,-0.5f);
	glTexCoord2f(1, 1); glVertex3f(-0.5f, 0.5f,-0.5f);
	glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f, 0.5f);
    glEnd();
	
    glBindTexture(GL_TEXTURE_2D, m_textures[3]);
    glBegin(GL_QUADS);
	glNormal3f( 0.0f,-1.0f, 0.0f);
	glTexCoord2f(0, 0); glVertex3f(-0.5f,-0.5f,-0.5f);
	glTexCoord2f(1, 0); glVertex3f( 0.5f,-0.5f,-0.5f);
	glTexCoord2f(1, 1); glVertex3f( 0.5f,-0.5f, 0.5f);
	glTexCoord2f(0, 1); glVertex3f(-0.5f,-0.5f, 0.5f);
    glEnd();
	
    glBindTexture(GL_TEXTURE_2D, m_textures[4]);
    glBegin(GL_QUADS);
	glNormal3f( 1.0f, 0.0f, 0.0f);
	glTexCoord2f(0, 0); glVertex3f( 0.5f, 0.5f, 0.5f);
	glTexCoord2f(1, 0); glVertex3f( 0.5f,-0.5f, 0.5f);
	glTexCoord2f(1, 1); glVertex3f( 0.5f,-0.5f,-0.5f);
	glTexCoord2f(0, 1); glVertex3f( 0.5f, 0.5f,-0.5f);
    glEnd();
	
    glBindTexture(GL_TEXTURE_2D, m_textures[5]);
    glBegin(GL_QUADS);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(0, 0); glVertex3f(-0.5f,-0.5f,-0.5f);
	glTexCoord2f(1, 0); glVertex3f(-0.5f,-0.5f, 0.5f);
	glTexCoord2f(1, 1); glVertex3f(-0.5f, 0.5f, 0.5f);
	glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f,-0.5f);
    glEnd();
	
#else
	
	float FieldEW = 80;
	float FieldNS = 160;

	const float points00[][3] = { { -1000, -1000, 30 }, { 1000, -1000, 30 }, { 1000, 1000, 30 }, { -1000, 1000, 30} };
	DrawTextureOnBox(points00, 1, 1, m_textures[kSky]);
	const float points01[][3] = { { -FieldNS/2.0-25, -FieldEW/2.0-10, 0 }, { FieldNS/2.0+25, -FieldEW/2.0-10, 0 }, { FieldNS/2.0+25, FieldEW/2.0+10, 0 }, { -FieldNS/2.0-25, FieldEW/2.0+10, 0 } };
	DrawTextureOnBox(points01, 1, 1, m_textures[kField]);
	const float points02[][3] = { { FieldNS/2.0, +FieldEW/2.0, 0 }, { FieldNS/2.0, -FieldEW/2.0, 0 }, { FieldNS/2.0+16, -FieldEW/2.0, 0 }, { FieldNS/2.0+16, +FieldEW/2.0, 0 } };
	DrawTextureOnBox(points02, 1, 1, m_textures[kCalifornia]);
	const float points03[][3] = { { -FieldNS/2.0, -FieldEW/2.0, 0 }, { -FieldNS/2.0, +FieldEW/2.0, 0 }, { -FieldNS/2.0-16, +FieldEW/2.0, 0 }, { -FieldNS/2.0-16, -FieldEW/2.0, 0 } };
	DrawTextureOnBox(points03, 1, 1, m_textures[kEECS]);
	const float points04[][3] = { { -FieldNS/2.0, -FieldEW/2.0, 0 }, { FieldNS/2.0, -FieldEW/2.0, 0 }, { FieldNS/2.0, FieldEW/2.0, 0 }, { -FieldNS/2.0, FieldEW/2.0, 0 } };
	DrawTextureOnBox(points04, 1, 1, m_textures[kLines]);
	const float points05[][3] = { { -10, -10, 0 }, { 10, -10, 0 }, { 10, 10, 0 }, { -10, 10, 0 } };
	DrawTextureOnBox(points05, 1, 1, m_textures[kCalband]);
	
	// stands:
	const float points06[][3] = { { -FieldNS/2.0-25, FieldEW/2.0 + 10, 0 }, { FieldNS/2.0 + 25, FieldEW/2.0 + 10, 0 }, { FieldNS/2.0 + 25, FieldEW/2.0 + 30, 25 }, { -FieldNS/2.0 - 25, FieldEW/2.0 + 30, 25 } };
	DrawTextureOnBox(points06, 5, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points07[][3] = { { -FieldNS/2.0-25, -FieldEW/2.0 - 10, 0 }, { FieldNS/2.0+25, -FieldEW/2.0 - 10, 0 }, { FieldNS/2.0 + 25, -FieldEW/2.0 - 30, 25 }, { -FieldNS/2.0 - 25, -FieldEW/2.0 - 30, 25 } };
	DrawTextureOnBox(points07, 5, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points08[][3] = { { -FieldNS/2.0-25, -FieldEW/2.0-10, 0 }, { -FieldNS/2.0-25, -FieldEW/2.0-10, 0 }, { -FieldNS/2.0-50, -FieldEW/2.0-10, 25 }, { -FieldNS/2.0-25, -FieldEW/2.0-30, 25 } };
	DrawTextureOnBox(points08, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points09[][3] = { { -FieldNS/2.0-25, FieldEW/2.0 + 10, 0 }, { -FieldNS/2.0-25, FieldEW/2.0 + 10, 0 }, { -FieldNS/2.0-25, FieldEW/2.0 + 30, 25 }, { -FieldNS/2.0-50, FieldEW/2.0 + 10, 25 } };
	DrawTextureOnBox(points09, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points10[][3] = { { FieldNS/2.0+25, -FieldEW/2.0-10, 0 }, { FieldNS/2.0+25, -FieldEW/2.0-10, 0 }, { FieldNS/2.0+25, -FieldEW/2.0-30, 25 }, { FieldNS/2.0+50, -FieldEW/2.0-10, 25 } };
	DrawTextureOnBox(points10, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points11[][3] = { { FieldNS/2.0+25, FieldEW/2.0 + 10, 0 }, { FieldNS/2.0+25, FieldEW/2.0 + 10, 0 }, { FieldNS/2.0+50, FieldEW/2.0 + 10, 25 }, { FieldNS/2.0+25, FieldEW/2.0 + 30, 25 } };
	DrawTextureOnBox(points11, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points12[][3] = { { -FieldNS/2.0 - 25, -FieldEW/2.0 - 10, 0 }, { -FieldNS/2.0 - 25, FieldEW/2.0 + 10, 0 }, { -FieldNS/2.0 - 50, FieldEW/2.0 + 10, 25 }, { -FieldNS/2.0 - 50, -FieldEW/2.0 - 10, 25 } };
	DrawTextureOnBox(points12, 4, 5, m_textures[crowdOn ? kCrowd : kBleachers]);
	const float points13[][3] = { { FieldNS/2.0 + 25, FieldEW/2.0 + 10, 0 }, { FieldNS/2.0 + 25, -FieldEW/2.0 - 10, 0 }, { FieldNS/2.0 + 50, -FieldEW/2.0 - 10, 25 }, { FieldNS/2.0 + 50, +FieldEW/2.0 + 10, 25 } };
	DrawTextureOnBox(points13, 4, 5, m_textures[crowdOn ? kCrowd : kBleachers]);

	glColor3f(0.0,0.0,0.0);
	const float points14[][3] = { { FieldNS/2.0 + 25, 8, 0 }, { FieldNS/2.0 + 25, -8, 0 }, { FieldNS/2.0 + 32, -8, 6 }, { FieldNS/2.0 + 32, 8, 6 } };
	DrawBox(points14); // North Tunnel
	const float points15[][3] = { { -FieldNS/2.0 - 25, 8, 0 }, { -FieldNS/2.0 - 25, -8, 0 }, { -FieldNS/2.0 - 32, -8, 6 }, { -FieldNS/2.0 - 32, 8, 6 } };
	DrawBox(points15); // South Tunnel
	glColor3f(1.0,1.0,1.0);
	
	const float points16[][3] = { { -40, FieldEW/2.0 + 30, 25 }, { 40, FieldEW/2.0 + 30, 25 }, { 40, FieldEW/2.0 + 30, 35 }, { -40, FieldEW/2.0 + 30, 35 } };
	DrawTextureOnBox(points16, 1, 1, m_textures[kPressbox]);
	
	const float points17[][3] = { { +10, -FieldEW/2.0 - 10, 0 }, { -10, -FieldEW/2.0 - 10, 0 }, { -10, -FieldEW/2.0 - 30, 25 }, { +10, -FieldEW/2.0 - 30, 25 } };
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
	const float points24[][3] = { { +10, -FieldEW/2.0 - 10, 0 }, { -10, -FieldEW/2.0 - 10, 0 }, { -10, -FieldEW/2.0 - 30, 25 }, { +10, -FieldEW/2.0 - 30, 25 } };
	DrawTextureOnBox(points24, 2, 1, m_textures[kWall]);
	
#endif
    glFlush();
	
    CheckGLError();
}

WhichImageEnum
CCOmniView_GLContext::GetMarcherTexture(float cameraAngleToMarcher, float marcherDirection)
{
	// Returns which direction they are facing in regards to the camera.
	float relativeAngle = cameraAngleToMarcher - marcherDirection; // convert to relative angle;
	while (relativeAngle < 0.0)
		relativeAngle += 360.0;
	while (relativeAngle > 360.0)
		relativeAngle -= 360.0;
	if (relativeAngle >= 22.5 && relativeAngle < 67.5) 
		return kBR0;
	else if (relativeAngle >= 67.5 && relativeAngle < 112.5)
		return kR0;
	else if (relativeAngle >= 112.5 && relativeAngle < 157.5)
		return kFR0;
	else if (relativeAngle >= 157.5 && relativeAngle < 202.5)
		return kF0;
	else if (relativeAngle >= 202.5 && relativeAngle < 247.5)
		return kFL0;
	else if (relativeAngle >= 247.5 && relativeAngle < 292.5)
		return kL0;
	else if (relativeAngle >= 292.5 && relativeAngle < 337.5)
		return kBL0;
	else if (relativeAngle >= 337.5 || relativeAngle < 22.5)
		return kB0;
	else {// Default in case something wacko... 
		return kB0;
	}
}


// Returns the angle in regards to the camera.
float GetAngle(float x, float y, const viewpoint_t &viewpoint){
	viewpoint_t v = viewpoint_t(x - viewpoint.x, y - viewpoint.y);
	float mag = (sqrt(v.x*v.x + v.y*v.y));
	float ang = acos(v.x/mag); // normalize
	ang *= 180.0/M_PI; // convert to degrees
	if (v.y > 0) ang = (-ang);
	
	// using GURK's coordinate system;
	if (ang < 0.0)
		ang = ang + 360;
	return ang;
}

void
CCOmniView_GLContext::Draw3dMarcher(const MarcherInfo &info, const viewpoint_t &viewpoint)
{
	float ang = GetAngle(info.x, info.y, viewpoint);
	float dir = info.direction;
	size_t face = GetMarcherTexture(ang, dir);
	
	glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
	
	float x1, x2, y1, y2;
	x1 = x2 = info.x;
	y1 = y2 = info.y;
	float z1 = 0, z2 = 3;
	
	if ((dir < 45 || dir > 315) || (dir > 135 && dir < 225)){ 
		if (face == kF0 || face == kB0) {
			y1 -= 0.45;
			y2 += 0.45;
		}
		else {
			x1 -= 0.45;
			x2 += 0.45;
		}
		if (ang > 157.5 && ang < 337.5 ) {
			const float points[][3] = { { x1, y1, z1 }, { x2, y2, z1 }, { x2, y2, z2 }, { x1, y1, z2 } };
			DrawTextureOnBox(points, 1, 1, m_textures[face]);
		}
		else {
			const float points[][3] = { { x2, y2, z1 }, { x1, y1, z1 }, { x1, y1, z2 }, { x2, y2, z2 } };
			DrawTextureOnBox(points, 1, 1, m_textures[face]);
		}
	}
	else {
		if (face == kF0 || face == kB0) {
			x1 -= 0.45;
			x2 += 0.45;
		}
		else {
			y1 -= 0.45;
			y2 += 0.45;
		}
		if (ang > 112.5 && ang < 292.5 ) {
			const float points[][3] = { { x1, y1, z1 }, { x2, y2, z1 }, { x2, y2, z2 }, { x1, y1, z2 } };
			DrawTextureOnBox(points, 1, 1, m_textures[face]);
		}
		else {
			const float points[][3] = { { x2, y2, z1 }, { x1, y1, z1 }, { x1, y1, z2 }, { x2, y2, z2 } };
			DrawTextureOnBox(points, 1, 1, m_textures[face]);
		}
	}
}



CCOmniView_Canvas::CCOmniView_Canvas(wxFrame *frame, AnimationView *view) :
wxGLCanvas(frame, wxID_ANY, NULL, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
m_xangle(0), m_yangle(0),
mViewPoint(-1, -1, 2),
mFollowMarcher(false),
mCrowdOn(false),
mViewAngle(M_PI/2),
mViewAngleZ(0),
mFOV(60)
{
	mView = view;
}


CCOmniView_Canvas::~CCOmniView_Canvas()
{
}


void
CCOmniView_Canvas::SetView(AnimationView *view)
{
	mView = view;
}

CCOmniView_GLContext *m_glContext = NULL;

CCOmniView_GLContext& GetContext1(wxGLCanvas *canvas)
{
    if ( !m_glContext )
    {
        // Create the OpenGL context for the first window which needs it:
        // subsequently created windows will all share the same context.
        m_glContext = new CCOmniView_GLContext(canvas);
    }
	
    m_glContext->SetCurrent(*canvas);
	
    return *m_glContext;
}

std::multimap<double, MarcherInfo>
CCOmniView_Canvas::ParseAndDraw3dMarchers()
{
	std::multimap<double, MarcherInfo> result;
	for (size_t i = 0; i < mView->GetShow()->GetNumPoints(); ++i)
	{
		MarcherInfo info;
		info.direction = mView->mAnimation->RealDirection(i);

		CC_coord position = mView->mAnimation->Position(i);
		float x = info.x = Coord2Float(position.x);//+mView->GetShow()->GetMode().Offset().x;
		float y = info.y = Coord2Float(position.y);//+mView->GetShow()->GetMode().Offset().y;

		float distance = sqrt(pow(mViewPoint.x-x,2)+pow(mViewPoint.y-y,2));
		result.insert(std::pair<double, MarcherInfo>(distance, info));
	}
	return result;
}

void
CCOmniView_Canvas::OnPaint(wxPaintEvent& event)
{
    // This is required even though dc is not used otherwise.
    wxPaintDC dc(this);

    // Set the OpenGL viewport according to the client size of this canvas.
    // This is done here rather than in a wxSizeEvent handler because our
    // OpenGL rendering context (and thus viewport setting) is used with
    // multiple canvases: If we updated the viewport in the wxSizeEvent
    // handler, changing the size of one canvas causes a viewport setting that
    // is wrong when next another canvas is repainted.
    const wxSize ClientSize = GetClientSize();
	
    CCOmniView_GLContext& context = GetContext1(this);
    glViewport(0, 0, ClientSize.x, ClientSize.y);

#if DO_CCVIEW
	float FieldEW = 80;
	float FieldNS = 160;
	
	glClear(GL_COLOR_BUFFER_BIT);
	// set our view point:
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(mFOV, static_cast<float>(ClientSize.x) / static_cast<float>(ClientSize.y), 0.1 ,2*FieldNS);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
	gluLookAt(mViewPoint.x, mViewPoint.y, mViewPoint.z, mViewPoint.x + cos(mViewAngle), mViewPoint.y + sin(mViewAngle), mViewPoint.z + sin(mViewAngleZ), 0.0, 0.0, 1.0);

#endif
	
    // Render the graphics and swap the buffers.
    context.DrawField(m_xangle, m_yangle, ClientSize, mCrowdOn);
	if (mView && mView->mAnimation)
	{
		std::multimap<double, MarcherInfo> marchers = ParseAndDraw3dMarchers();
		for (std::multimap<double, MarcherInfo>::reverse_iterator i = marchers.rbegin(); i != marchers.rend(); ++i)
		{
			context.Draw3dMarcher(i->second, mViewPoint);
		}
	}

    SwapBuffers();
}


void CCOmniView_Canvas::Spin(float xSpin, float ySpin)
{
    m_xangle += xSpin;
    m_yangle += ySpin;
	
    Refresh(false);
}


#if DO_CCVIEW
void CCOmniView_Canvas::OnChar(wxKeyEvent& event)
{
	const float stepIncr = 10;
	const float AngleStepIncr = 0.1;
	switch (event.GetKeyCode())
	{
		case 'f':
			mFollowMarcher = true;
			break;
		case 'b':
			mFollowMarcher = false;
			break;

			// predetermined camera angles:
		case '1':
			mFollowMarcher = false;
			mViewPoint = viewpoint_t(0, 0, 2.5);
			mViewAngle = M_PI/2;
			mViewAngleZ = 0;
			mFOV = 30;
			break;
		case '2':
			mFollowMarcher = false;
			mViewPoint = viewpoint_t(0, -67, 20);
			mViewAngle = M_PI/2;
			mViewAngleZ = -M_PI/8;
			mFOV = 90;
			break;
			
		case 'c':
			mViewPoint.z -= stepIncr;
			mViewPoint.z = std::max(mViewPoint.z, 0.3f);
			break;
		case 'v':
			mViewPoint.z += stepIncr;
			break;
		case 'q':
			mViewAngle -= AngleStepIncr;
			break;
		case 'w':
			mViewAngle += AngleStepIncr;
			break;
		case 'a':
			mViewAngleZ -= AngleStepIncr;
			break;
		case 's':
			mViewAngleZ += AngleStepIncr;
			break;
		case WXK_LEFT:
			mViewPoint.x += -stepIncr*cos(mViewAngle - M_PI/2);
			mViewPoint.y += -stepIncr*sin(mViewAngle - M_PI/2);
			break;
		case WXK_RIGHT:
			mViewPoint.x += stepIncr*cos(mViewAngle - M_PI/2);
			mViewPoint.y += stepIncr*sin(mViewAngle - M_PI/2);
			break;
		case WXK_DOWN:
			mViewPoint.x += -stepIncr*cos(mViewAngle);
			mViewPoint.y += -stepIncr*sin(mViewAngle);
			mViewPoint.z += -stepIncr*sin(mViewAngleZ);
			mViewPoint.z = std::max(mViewPoint.z, 0.3f);
			break;
		case WXK_UP:
			mViewPoint.x += stepIncr*cos(mViewAngle);
			mViewPoint.y += stepIncr*sin(mViewAngle);
			mViewPoint.z += stepIncr*sin(mViewAngleZ);
			mViewPoint.z = std::max(mViewPoint.z, 0.3f);
			break;
//		case 'i': printf("View Point:");viewPoint->PrintVertex(); printf(", Angle: %f, AngleZ: %f\n", viewAngle, viewAngleZ);printf("\n");break;
//		case ' ': {	
//			for (int i = 0; i < Marchers.size(); i++) {	 // progress step
//				//printf("stepping\n");
//				Marchers[i]->Step();
//			}
//			break;
//		}
//		case 'm': 
//			DRAWFIELD = !DRAWFIELD; 
//			break;
//		case 'h': printf("Basic commands:\nw\tforward\ns\tbackward\nd\tsidestep-right\na\tsidestep-left\nv\tup vertically\nc\tdown\nleft-click\tmouse-look\nright-click\tmenu\n"); break;
//		case 'p': animate = !animate; break;
//		case 'r': Marchers = loadMarcher(); break;
//		case '=': tempo = tempo+10; break;
//		case '+': tempo = tempo+10; break;
//		case '-': tempo = tempo-10; break;
//		case '>': if (FOV < 160) FOV += 5; break;
//		case '<': if (FOV > 10) FOV -= 5; break;
//		case 't': crowdOn = !crowdOn;
//		case '#': 
//			customViewPoint->x = (float) (viewPoint->x);
//			customViewPoint->y = (float) (viewPoint->y);
//			customViewPoint->z = (float) (viewPoint->z);
//			customViewAngle = viewAngle;
//			customViewAngleZ = viewAngleZ;
//			customFOV = FOV;
//			break;
//		case '3':
//			viewPoint->x = (float) (customViewPoint->x);
//			viewPoint->y = (float) (customViewPoint->y);
//			viewPoint->z = (float) (customViewPoint->z);
//			viewAngle = customViewAngle;
//			viewAngleZ = customViewAngleZ;
//			FOV = customFOV;
//			break;
		default:
			event.Skip();
	}

	Refresh();
//	if (move) {
//		viewPoint->x = viewPoint->x+changeX;
//		viewPoint->y = viewPoint->y+changeY;
//		viewPoint->z = viewPoint->z+changeZ;
//	}
}


#else
void CCOmniView_Canvas::OnChar(wxKeyEvent& event)
{
    float angle = 5.0;
	if (mView)
	{
		if (event.GetKeyCode() == WXK_LEFT)
		{
            Spin( 0.0, angle );
			mView->PrevBeat();
		}
		else if (event.GetKeyCode() == WXK_RIGHT)
		{
            Spin( 0.0, -angle );
			mView->NextBeat();
		}
		else if (event.GetKeyCode() == WXK_UP)
		{
            Spin( angle, 0.0 );
			mView->NextBeat();
		}
		else if (event.GetKeyCode() == WXK_DOWN)
		{
            Spin( -angle, 0.0 );
			mView->NextBeat();
		}
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
#endif

