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

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION 1
#endif

#include "CCOmniviewCanvas.h"
#include "AnimationView.h"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartShowMode.h"
#include "CalChartUtils.h"
#include "basic_ui.h"
#include "cc_omniview_constants.h"
#include "platconf.h"

#include <wx/dcbuffer.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

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

auto GetImageDir()
{
#if defined(__APPLE__) && (__APPLE__)
    const static wxString kImageDir = "CalChart.app/Contents/Resources/";
#else
    const static wxString kImageDir = wxFileName(::wxStandardPaths::Get().GetExecutablePath()).GetPath().Append(PATH_SEPARATOR "Resources" PATH_SEPARATOR);
#endif
    return kImageDir;
}

static constexpr auto kStartingViewPoint = CCOmniviewCanvas::ViewPoint{ kViewPoint_x_1, kViewPoint_y_1, kViewPoint_z_1 };
static constexpr auto kStartingViewAngle = kViewAngle_1;
static constexpr auto kStartingViewAngleZ = kViewAngle_z_1;

enum WhichImage {
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
};

std::map<WhichImage, std::string> ListOfImageFiles = {
    { WhichImage::kF0, "f0.tga" },
    { WhichImage::kF1, "f1.tga" },
    { WhichImage::kF2, "f2.tga" },
    { WhichImage::kFL0, "fl0.tga" },
    { WhichImage::kFL1, "fl1.tga" },
    { WhichImage::kFL2, "fl2.tga" },
    { WhichImage::kL0, "l0.tga" },
    { WhichImage::kL1, "l1.tga" },
    { WhichImage::kL2, "l2.tga" },
    { WhichImage::kBL0, "bl0.tga" },
    { WhichImage::kBL1, "bl1.tga" },
    { WhichImage::kBL2, "bl2.tga" },
    { WhichImage::kB0, "b0.tga" },
    { WhichImage::kB1, "b1.tga" },
    { WhichImage::kB2, "b2.tga" },
    { WhichImage::kBR0, "br0.tga" },
    { WhichImage::kBR1, "br1.tga" },
    { WhichImage::kBR2, "br2.tga" },
    { WhichImage::kR0, "r0.tga" },
    { WhichImage::kR1, "r1.tga" },
    { WhichImage::kR2, "r2.tga" },
    { WhichImage::kFR0, "fr0.tga" },
    { WhichImage::kFR1, "fr1.tga" },
    { WhichImage::kFR2, "fr2.tga" },
    { WhichImage::kField, "field.tga" },
    { WhichImage::kLines, "lines.tga" },
    { WhichImage::kDirection, "Direction.tga" },
    { WhichImage::kBleachers, "bleachers.tga" },
    { WhichImage::kWall, "wall.tga" },
    { WhichImage::kSky, "sky.tga" },
    { WhichImage::kCalband, "calband.tga" },
    { WhichImage::kC, "C.tga" },
    { WhichImage::kCal, "cal.tga" },
    { WhichImage::kCalifornia, "california.tga" },
    { WhichImage::kEECS, "eecs.tga" },
    { WhichImage::kPressbox, "pressbox.tga" },
    { WhichImage::kCrowd, "crowd.tga" },
    { WhichImage::kGoalpost, "goalpost.tga" },
    { WhichImage::kEndOfShow, "endofshow.tga" }
};

enum class WhichMarchingStyle {
    kClosed,
    kLeftHSHup,
    kRightHSHup,
    kLeftMilitaryPoof,
    kRightMilitaryPoof,
    kLeftGrapeVine,
    kRightGrapeVine,
};

BEGIN_EVENT_TABLE(CCOmniviewCanvas, wxGLCanvas)
EVT_CHAR(CCOmniviewCanvas::OnChar)
EVT_MOTION(CCOmniviewCanvas::OnMouseMove)
EVT_PAINT(CCOmniviewCanvas::OnPaint)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

static void CheckGLError()
{
    GLenum errLast = GL_NO_ERROR;

    for (;;) {
        GLenum err = glGetError();
        if (err == GL_NO_ERROR)
            return;

        // normally the error is reset by the call to glGetError() but if
        // glGetError() itself returns an error, we risk looping forever here
        // so check that we get a different error than the last time
        if (err == errLast) {
            wxLogError(wxT("OpenGL error state couldn't be reset."));
            return;
        }

        errLast = err;

        wxLogError(wxT("OpenGL error %d"), err);
    }
}

static auto LoadTextureWithImage(wxImage const& image, GLuint const& texture)
{
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (image.HasAlpha()) {
        const auto width = image.GetWidth();
        const auto height = image.GetHeight();
        std::vector<uint8_t> mixedAlpha(width * height * 4);
        for (auto y = 0; y < height; ++y) {
            for (auto x = 0; x < width; ++x) {
                mixedAlpha.at(x * 4 + (y * width * 4) + 0) = image.GetRed(x, y);
                mixedAlpha.at(x * 4 + (y * width * 4) + 1) = image.GetGreen(x, y);
                mixedAlpha.at(x * 4 + (y * width * 4) + 2) = image.GetBlue(x, y);
                mixedAlpha.at(x * 4 + (y * width * 4) + 3) = image.GetAlpha(x, y);
            }
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &mixedAlpha[0]);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.GetWidth(), image.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, image.GetData());
    }
    return true;
}

static auto LoadTexture(wxString const& filename, GLuint const& texture)
{
    wxImage image;
    if (!image.LoadFile(filename)) {
        wxLogError(wxT("Couldn't load image from ") + filename + wxT("."));
        return false;
    }
    return LoadTextureWithImage(image, texture);
}

// We always draw from the upper left, upper right, lower right, lower left
static void DrawTextureOnBox(double const points[4][3], size_t repeat_x, size_t repeat_y, GLuint const& texture)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(points[0][0], points[0][1], points[0][2]);
    glTexCoord2f(repeat_x, 0);
    glVertex3f(points[1][0], points[1][1], points[1][2]);
    glTexCoord2f(repeat_x, repeat_y);
    glVertex3f(points[2][0], points[2][1], points[2][2]);
    glTexCoord2f(0, repeat_y);
    glVertex3f(points[3][0], points[3][1], points[3][2]);
    glEnd();
}

static void DrawBox(double const points[4][3])
{
    glBegin(GL_QUADS);
    glVertex3f(points[0][0], points[0][1], points[0][2]);
    glVertex3f(points[1][0], points[1][1], points[1][2]);
    glVertex3f(points[2][0], points[2][1], points[2][2]);
    glVertex3f(points[3][0], points[3][1], points[3][2]);
    glEnd();
}

auto GetMarcherTextureAndPoints(CalChart::Radian cameraAngleToMarcher, CalChart::Radian marcherDirection, float& x1, float& x2, float& y1, float& y2)
{
    // Returns which direction they are facing in regards to the camera.
    auto relativeAngle = marcherDirection - cameraAngleToMarcher; // convert to relative angle;
    relativeAngle = CalChart::NormalizeAngle(relativeAngle);

    if (cameraAngleToMarcher >= 1.0 * CalChart::pi / 8.0 && cameraAngleToMarcher < 3.0 * CalChart::pi / 8.0) {
        y1 += 0.45f;
        y2 -= 0.45f;
        x1 -= 0.45f;
        x2 += 0.45f;
    } else if (cameraAngleToMarcher >= 3.0 * CalChart::pi / 8.0 && cameraAngleToMarcher < 5.0 * CalChart::pi / 8.0) {
        x1 -= 0.45f;
        x2 += 0.45f;
    } else if (cameraAngleToMarcher >= 5.0 * CalChart::pi / 8.0 && cameraAngleToMarcher < 7.0 * CalChart::pi / 8.0) {
        y1 -= 0.45f;
        y2 += 0.45f;
        x1 -= 0.45f;
        x2 += 0.45f;
    } else if (cameraAngleToMarcher >= 7.0 * CalChart::pi / 8.0 && cameraAngleToMarcher < 9.0 * CalChart::pi / 8.0) {
        y1 -= 0.45f;
        y2 += 0.45f;
    } else if (cameraAngleToMarcher >= 9.0 * CalChart::pi / 8.0 && cameraAngleToMarcher < 11.0 * CalChart::pi / 8.0) {
        y1 -= 0.45f;
        y2 += 0.45f;
        x1 += 0.45f;
        x2 -= 0.45f;
    } else if (cameraAngleToMarcher >= 11.0 * CalChart::pi / 8.0 && cameraAngleToMarcher < 13.0 * CalChart::pi / 8.0) {
        x1 += 0.45f;
        x2 -= 0.45f;
    } else if (cameraAngleToMarcher >= 13.0 * CalChart::pi / 8.0 && relativeAngle < 15.0 * CalChart::pi / 8.0) {
        y1 += 0.45f;
        y2 -= 0.45f;
        x1 += 0.45f;
        x2 -= 0.45f;
    } else // if (cameraAngleToMarcher >= 15.0*CalChart::pi/8.0 || cameraAngleToMarcher <
    // 1.0*CalChart::pi/8.0) // and everything else
    {
        y1 += 0.45f;
        y2 -= 0.45f;
    }

    if (relativeAngle >= 1.0 * CalChart::pi / 8.0 && relativeAngle < 3.0 * CalChart::pi / 8.0) {
        return WhichImage::kBR0;
    } else if (relativeAngle >= 3.0 * CalChart::pi / 8.0 && relativeAngle < 5.0 * CalChart::pi / 8.0) {
        return WhichImage::kR0;
    } else if (relativeAngle >= 5.0 * CalChart::pi / 8.0 && relativeAngle < 7.0 * CalChart::pi / 8.0) {
        return WhichImage::kFR0;
    } else if (relativeAngle >= 7.0 * CalChart::pi / 8.0 && relativeAngle < 9.0 * CalChart::pi / 8.0) {
        return WhichImage::kF0;
    } else if (relativeAngle >= 9.0 * CalChart::pi / 8.0 && relativeAngle < 11.0 * CalChart::pi / 8.0) {
        return WhichImage::kFL0;
    } else if (relativeAngle >= 11.0 * CalChart::pi / 8.0 && relativeAngle < 13.0 * CalChart::pi / 8.0) {
        return WhichImage::kL0;
    } else if (relativeAngle >= 13.0 * CalChart::pi / 8.0 && relativeAngle < 15.0 * CalChart::pi / 8.0) {
        return WhichImage::kBL0;
    } else // if (relativeAngle >= 15.0*CalChart::pi/8.0 || relativeAngle < 1.0*CalChart::pi/8.0)
    // // and everything else
    {
        return WhichImage::kB0;
    }
}

// Returns the angle in regards to the camera.
static auto GetAngle(float x, float y, CCOmniviewCanvas::ViewPoint const& viewpoint)
{
    auto v = CCOmniviewCanvas::ViewPoint{ x - viewpoint.x, y - viewpoint.y, 0.f };
    auto mag = sqrt(v.x * v.x + v.y * v.y);
    auto ang = acos(v.x / mag); // normalize
    return CalChart::Radian{ (v.y < 0) ? -ang : ang };
}

// the rendering context used by CCOmniviewCanvas
class CCOmniView_GLContext : public wxGLContext {
public:
    CCOmniView_GLContext(wxGLCanvas* canvas);

    void DrawField(float FieldEW, float FieldNS, bool crowdOn);
    void Draw3dMarcher(AnimationView::MarcherInfo const& info, const CCOmniviewCanvas::ViewPoint& viewpoint, WhichMarchingStyle style);

    bool UseForLines(wxImage const& lines);

private:
    // textures for the cube faces
    GLuint m_textures[static_cast<int>(WhichImage::kImageLast)];
};

CCOmniView_GLContext::CCOmniView_GLContext(wxGLCanvas* canvas)
    : wxGLContext(canvas)
{
    SetCurrent(*canvas);

    // set up the parameters we want to use
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND); // to allow alpha values to be considered
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // to allow alpha values to be considered
    glClearColor(0.289f, 0.941f, 1.0f, 1.0f); // Default Background Color

    glGenTextures(WXSIZEOF(m_textures), m_textures);

    for (auto i : ListOfImageFiles) {
        if (!LoadTexture(GetImageDir() + i.second, m_textures[i.first])) {
            wxLogError(wxT("Could not load ") + i.second);
        }
    }

    CheckGLError();
}

bool CCOmniView_GLContext::UseForLines(wxImage const& lines)
{
    return LoadTextureWithImage(lines, m_textures[WhichImage::kLines]);
}

void CCOmniView_GLContext::DrawField(float FieldEW, float FieldNS, bool crowdOn)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    // because we look up to see the sky, make sure the texture is pointing down:
    double const points00[][3] = { { 1000, -1000, 30 },
        { -1000, -1000, 30 },
        { -1000, 1000, 30 },
        { 1000, 1000, 30 } };
    DrawTextureOnBox(points00, 20, 20, m_textures[WhichImage::kSky]);
    double const points01[][3] = {
        { -1 * (FieldNS / 2.0 + 25), (FieldEW / 2.0 + 10), 0 },
        { (FieldNS / 2.0 + 25), (FieldEW / 2.0 + 10), 0 },
        { (FieldNS / 2.0 + 25), -1 * (FieldEW / 2.0 + 10), 0 },
        { -1 * (FieldNS / 2.0 + 25), -1 * (FieldEW / 2.0 + 10), 0 }
    };
    DrawTextureOnBox(points01, 2, 2, m_textures[WhichImage::kField]);
    double const points02[][3] = { { FieldNS / 2.0 + 16, +FieldEW / 2.0, 0 },
        { FieldNS / 2.0 + 16, -FieldEW / 2.0, 0 },
        { FieldNS / 2.0, -FieldEW / 2.0, 0 },
        { FieldNS / 2.0, FieldEW / 2.0, 0 } };
    DrawTextureOnBox(points02, 1, 1, m_textures[WhichImage::kCalifornia]);
    double const points03[][3] = {
        { -1 * (FieldNS / 2.0 + 16), -1 * (FieldEW / 2.0), 0 },
        { -1 * (FieldNS / 2.0 + 16), -1 * (-FieldEW / 2.0), 0 },
        { -1 * (FieldNS / 2.0), -1 * (-FieldEW / 2.0), 0 },
        { -1 * (FieldNS / 2.0), -1 * (FieldEW / 2.0), 0 }
    };
    DrawTextureOnBox(points03, 1, 1, m_textures[WhichImage::kEECS]);
    double const points04[][3] = { { -FieldNS / 2.0, FieldEW / 2.0, 0 },
        { FieldNS / 2.0, FieldEW / 2.0, 0 },
        { FieldNS / 2.0, -FieldEW / 2.0, 0 },
        { -FieldNS / 2.0, -FieldEW / 2.0, 0 } };
    DrawTextureOnBox(points04, 1, 1, m_textures[WhichImage::kLines]);
    double const points05[][3] = {
        { -10, 10, 0 }, { 10, 10, 0 }, { 10, -10, 0 }, { -10, -10, 0 }
    };
    DrawTextureOnBox(points05, 1, 1, m_textures[WhichImage::kCalband]);

    // stands:
    double const points06[][3] = { { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 30, 25 },
        { FieldNS / 2.0 + 25, FieldEW / 2.0 + 30, 25 },
        { FieldNS / 2.0 + 25, FieldEW / 2.0 + 10, 0 },
        { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 10, 0 } };
    DrawTextureOnBox(points06, 5, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
    double const points07[][3] = {
        { -(-FieldNS / 2.0 - 25), -(FieldEW / 2.0 + 30), 25 },
        { -(FieldNS / 2.0 + 25), -(FieldEW / 2.0 + 30), 25 },
        { -(FieldNS / 2.0 + 25), -(FieldEW / 2.0 + 10), 0 },
        { -(-FieldNS / 2.0 - 25), -(FieldEW / 2.0 + 10), 0 }
    };
    DrawTextureOnBox(points07, 5, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
    double const points08[][3] = { { -FieldNS / 2.0 - 25, -FieldEW / 2.0 - 10, 0 },
        { -FieldNS / 2.0 - 25, -FieldEW / 2.0 - 10, 0 },
        { -FieldNS / 2.0 - 50, -FieldEW / 2.0 - 10, 25 },
        { -FieldNS / 2.0 - 25, -FieldEW / 2.0 - 30, 25 } };
    DrawTextureOnBox(points08, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
    double const points09[][3] = { { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 10, 0 },
        { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 10, 0 },
        { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 30, 25 },
        { -FieldNS / 2.0 - 50, FieldEW / 2.0 + 10, 25 } };
    DrawTextureOnBox(points09, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
    double const points10[][3] = { { FieldNS / 2.0 + 25, -FieldEW / 2.0 - 10, 0 },
        { FieldNS / 2.0 + 25, -FieldEW / 2.0 - 10, 0 },
        { FieldNS / 2.0 + 25, -FieldEW / 2.0 - 30, 25 },
        { FieldNS / 2.0 + 50, -FieldEW / 2.0 - 10, 25 } };
    DrawTextureOnBox(points10, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
    double const points11[][3] = { { FieldNS / 2.0 + 25, FieldEW / 2.0 + 10, 0 },
        { FieldNS / 2.0 + 25, FieldEW / 2.0 + 10, 0 },
        { FieldNS / 2.0 + 50, FieldEW / 2.0 + 10, 25 },
        { FieldNS / 2.0 + 25, FieldEW / 2.0 + 30, 25 } };
    DrawTextureOnBox(points11, 1, 10, m_textures[crowdOn ? kCrowd : kBleachers]);
    double const points12[][3] = { { -FieldNS / 2.0 - 50, -FieldEW / 2.0 - 10, 25 },
        { -FieldNS / 2.0 - 50, FieldEW / 2.0 + 10, 25 },
        { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 10, 0 },
        { -FieldNS / 2.0 - 25, -FieldEW / 2.0 - 10, 0 } };
    DrawTextureOnBox(points12, 4, 5, m_textures[crowdOn ? kCrowd : kBleachers]);
    double const points13[][3] = {
        { -(-FieldNS / 2.0 - 50), -(-FieldEW / 2.0 - 10), 25 },
        { -(-FieldNS / 2.0 - 50), -(FieldEW / 2.0 + 10), 25 },
        { -(-FieldNS / 2.0 - 25), -(FieldEW / 2.0 + 10), 0 },
        { -(-FieldNS / 2.0 - 25), -(-FieldEW / 2.0 - 10), 0 }
    };
    DrawTextureOnBox(points13, 4, 5, m_textures[crowdOn ? kCrowd : kBleachers]);

    glColor3f(0.0, 0.0, 0.0);
    double const points14[][3] = { { FieldNS / 2.0 + 25, 8, 0 },
        { FieldNS / 2.0 + 25, -8, 0 },
        { FieldNS / 2.0 + 32, -8, 6 },
        { FieldNS / 2.0 + 32, 8, 6 } };
    DrawBox(points14); // North Tunnel
    double const points15[][3] = { { -FieldNS / 2.0 - 25, 8, 0 },
        { -FieldNS / 2.0 - 25, -8, 0 },
        { -FieldNS / 2.0 - 32, -8, 6 },
        { -FieldNS / 2.0 - 32, 8, 6 } };
    DrawBox(points15); // South Tunnel
    glColor3f(1.0, 1.0, 1.0);

    double const points16[][3] = { { -40, FieldEW / 2.0 + 30, 35 },
        { 40, FieldEW / 2.0 + 30, 35 },
        { 40, FieldEW / 2.0 + 30, 25 },
        { -40, FieldEW / 2.0 + 30, 25 } };
    DrawTextureOnBox(points16, 1, 1, m_textures[WhichImage::kPressbox]);

    double const points17[][3] = { { 10, -(FieldEW / 2.0 + 30), 25 },
        { -10, -(FieldEW / 2.0 + 30), 25 },
        { -10, -(FieldEW / 2.0 + 10), 0 },
        { 10, -(FieldEW / 2.0 + 10), 0 } };
    DrawTextureOnBox(points17, 1, 1, m_textures[WhichImage::kC]);

    double const points18[][3] = { { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 10, 0 },
        { FieldNS / 2.0 + 25, FieldEW / 2.0 + 10, 0 },
        { FieldNS / 2.0 + 25, FieldEW / 2.0 + 10, 3 },
        { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 10, 3 } };
    DrawTextureOnBox(points18, 5, 1, m_textures[WhichImage::kWall]);
    double const points19[][3] = { { -FieldNS / 2.0 - 25, -FieldEW / 2.0 - 10, 0 },
        { FieldNS / 2.0 + 25, -FieldEW / 2.0 - 10, 0 },
        { FieldNS / 2.0 + 25, -FieldEW / 2.0 - 10, 3 },
        { -FieldNS / 2.0 - 25, -FieldEW / 2.0 - 10, 3 } };
    DrawTextureOnBox(points19, 5, 1, m_textures[WhichImage::kWall]);
    double const points20[][3] = { { -FieldNS / 2.0 - 25, 8, 0 },
        { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 10, 0 },
        { -FieldNS / 2.0 - 25, FieldEW / 2.0 + 10, 3 },
        { -FieldNS / 2.0 - 25, 8, 3 } };
    DrawTextureOnBox(points20, 2, 1, m_textures[WhichImage::kWall]);
    double const points21[][3] = { { -FieldNS / 2.0 - 25, -FieldEW / 2.0 - 10, 0 },
        { -FieldNS / 2.0 - 25, -8, 0 },
        { -FieldNS / 2.0 - 25, -8, 3 },
        { -FieldNS / 2.0 - 25, -FieldEW / 2.0 - 10, 3 } };
    DrawTextureOnBox(points21, 2, 1, m_textures[WhichImage::kWall]);
    double const points22[][3] = { { FieldNS / 2.0 + 25, 8, 0 },
        { FieldNS / 2.0 + 25, FieldEW / 2.0 + 10, 0 },
        { FieldNS / 2.0 + 25, FieldEW / 2.0 + 10, 3 },
        { FieldNS / 2.0 + 25, 8, 3 } };
    DrawTextureOnBox(points22, 2, 1, m_textures[WhichImage::kWall]);
    double const points23[][3] = { { FieldNS / 2.0 + 25, -FieldEW / 2.0 - 10, 0 },
        { FieldNS / 2.0 + 25, -8, 0 },
        { FieldNS / 2.0 + 25, -8, 3 },
        { FieldNS / 2.0 + 25, -FieldEW / 2.0 - 10, 3 } };
    DrawTextureOnBox(points23, 2, 1, m_textures[WhichImage::kWall]);

    glFlush();

    CheckGLError();
}

void CCOmniView_GLContext::Draw3dMarcher(AnimationView::MarcherInfo const& info, CCOmniviewCanvas::ViewPoint const& viewpoint, WhichMarchingStyle style)
{
    auto ang = CalChart::NormalizeAngle(GetAngle(info.x, info.y, viewpoint));
    auto dir = info.direction;

    auto x2 = info.x;
    auto x1 = x2;
    auto y2 = info.y;
    auto y1 = y2;
    auto z1 = 3.f;
    auto z2 = 0.f;

    auto face = GetMarcherTextureAndPoints(ang, dir, x1, x2, y1, y2);

    // if we want to modify the marching:
    switch (style) {
    case WhichMarchingStyle::kLeftHSHup:
        face = static_cast<WhichImage>(face + kF1);
        break;
    case WhichMarchingStyle::kRightHSHup:
        face = static_cast<WhichImage>(face + kF2);
        break;
    default:
        break;
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    double const points[][3] = {
        { x1, y1, z1 }, { x2, y2, z1 }, { x2, y2, z2 }, { x1, y1, z2 }
    };
    DrawTextureOnBox(points, 1, 1, m_textures[face]);
}

CCOmniviewCanvas::CCOmniviewCanvas(wxWindow* parent, CalChartConfiguration& config_, wxSize const& size)
    : wxGLCanvas(parent, wxID_ANY, NULL, wxDefaultPosition, size, wxFULL_REPAINT_ON_RESIZE)
    , m_glContext(new CCOmniView_GLContext(this))
    , config(config_)
    , mViewPoint(kStartingViewPoint)
    , mViewAngle(kStartingViewAngle)
    , mViewAngleZ(kStartingViewAngleZ)
{
    Init();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
}

void CCOmniviewCanvas::Init()
{
}

void CCOmniviewCanvas::CreateControls()
{
    SetSizer(VStack([](auto) {}));
}

void CCOmniviewCanvas::SetView(AnimationView* view)
{
    mView = view;
    // line drawing for CCOmniView doesn't look right on Mac. See issue #305
    //#if defined(__APPLE__) && (__APPLE__)
    //    m_glContext->UseForLines(GetOmniLinesImage(config, mView->GetShowMode()));
    //#endif
}

// rolling my own gluperspective
// http://nehe.gamedev.net/article/replacement_for_gluperspective/21002/
static void myGLUPerspective(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    auto fH = tan(fovY / 360.0 * std::numbers::pi) * zNear;
    auto fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

template <typename Float>
static void NormalizeVector(Float v[3])
{
    auto mag = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] /= mag;
    v[1] /= mag;
    v[2] /= mag;
}

template <typename Float>
static void CrossVector(Float dst[3], Float a[3], Float b[3])
{
    dst[0] = a[1] * b[2] - a[2] * b[1];
    dst[1] = a[2] * b[0] - a[0] * b[2];
    dst[2] = a[0] * b[1] - a[1] * b[0];
}

// rolling my own gluLookAt from http://www.opengl.org/wiki/GluLookAt_code
static void mygluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ)
{
    GLfloat m[16];
    float forward[3];
    //------------------
    forward[0] = centerX - eyeX;
    forward[1] = centerY - eyeY;
    forward[2] = centerZ - eyeZ;
    NormalizeVector(forward);

    float upVector[3];
    upVector[0] = upX;
    upVector[1] = upY;
    upVector[2] = upZ;

    float side[3];
    CrossVector(side, forward, upVector);
    NormalizeVector(side);

    float up[3];
    CrossVector(up, side, forward);
    NormalizeVector(up);

    //------------------
    m[0] = side[0];
    m[4] = side[1];
    m[8] = side[2];
    m[12] = 0.0;
    //------------------
    m[1] = up[0];
    m[5] = up[1];
    m[9] = up[2];
    m[13] = 0.0;
    //------------------
    m[2] = -forward[0];
    m[6] = -forward[1];
    m[10] = -forward[2];
    m[14] = 0.0;
    //------------------
    m[3] = 0;
    m[7] = 0;
    m[11] = 0;
    m[15] = 1.0;
    glMultMatrixf(m);
    glTranslated(-eyeX, -eyeY, -eyeZ);
}

void CCOmniviewCanvas::OnPaint(wxPaintEvent&)
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
    const wxSize ClientSize = GetClientSize() * GetContentScaleFactor();

    glViewport(0, 0, ClientSize.x, ClientSize.y);

    CalChart::Coord fieldSize = mView ? mView->GetShowMode().FieldSize() : CalChart::Coord(160, 80);
    auto FieldEW = CalChart::CoordUnits2Float(fieldSize.y);
    auto FieldNS = CalChart::CoordUnits2Float(fieldSize.x);

    glClear(GL_COLOR_BUFFER_BIT);
    // set our view point:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    myGLUPerspective(mFOV, static_cast<float>(ClientSize.x) / static_cast<float>(ClientSize.y), 0.1, 2 * FieldNS);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (mFollowMarcher != -1 && mView) {
        auto info = mView->GetMarcherInfo(mFollowMarcher);
        mViewPoint.x = info.x;
        mViewPoint.y = info.y;
        mViewPoint.z = 2;
        mViewAngle = info.direction;
        mViewAngleZ = CalChart::Radian{};
    }

    mygluLookAt(mViewPoint.x, mViewPoint.y, mViewPoint.z, mViewPoint.x + cos(mViewAngle), mViewPoint.y + sin(mViewAngle), mViewPoint.z + sin(mViewAngleZ), 0.0, 0.0, 1.0);

    // Render the graphics and swap the buffers.
    m_glContext->DrawField(FieldEW, FieldNS, mCrowdOn);
    if (mView) {
        auto marchers = mView->GetMarchersByDistance(mViewPoint.x, mViewPoint.y);
        for (auto i = marchers.rbegin(); i != marchers.rend(); ++i) {
            m_glContext->Draw3dMarcher(i->second, mViewPoint, mShowMarching ? (mView->OnBeat() ? WhichMarchingStyle::kLeftHSHup : WhichMarchingStyle::kRightHSHup) : WhichMarchingStyle::kClosed);
        }
    }

    SwapBuffers();
}

void CCOmniviewCanvas::OnChar(wxKeyEvent& event)
{
    auto const stepIncr = 0.3f * 3;
    auto const AngleStepIncr = CalChart::Radian{ 0.1f } * 3;
    switch (event.GetKeyCode()) {
    // predetermined camera angles:
    case '1':
    case '!':
        OnCmd_FollowMarcher(-1);
        mViewPoint = ViewPoint{ kViewPoint_x_1, kViewPoint_y_1, kViewPoint_z_1 };
        mViewAngle = kViewAngle_1;
        mViewAngleZ = kViewAngle_z_1;
        if (event.GetKeyCode() == '!') {
            mViewPoint.y *= -1;
            mViewPoint.x *= -1;
            mViewAngle = CalChart::NormalizeAngle(mViewAngle + CalChart::pi);
        }
        break;
    case '2':
    case '@':
        OnCmd_FollowMarcher(-1);
        mViewPoint = ViewPoint{ kViewPoint_x_2, kViewPoint_y_2, kViewPoint_z_2 };
        mViewAngle = kViewAngle_2;
        mViewAngleZ = kViewAngle_z_2;
        if (event.GetKeyCode() == '@') {
            mViewPoint.y *= -1;
            mViewPoint.x *= -1;
            mViewAngle = CalChart::NormalizeAngle(mViewAngle + CalChart::pi);
        }
        break;
    case '3':
    case '#':
        OnCmd_FollowMarcher(-1);
        mViewPoint = ViewPoint{ kViewPoint_x_3, kViewPoint_y_3, kViewPoint_z_3 };
        mViewAngle = kViewAngle_3;
        mViewAngleZ = kViewAngle_z_3;
        if (event.GetKeyCode() == '#') {
            mViewPoint.y *= -1;
            mViewPoint.x *= -1;
            mViewAngle = CalChart::NormalizeAngle(mViewAngle + CalChart::pi);
        }
        break;

    case '4':
        OnCmd_FollowMarcher(-1);
        mViewPoint = ViewPoint{ config.Get_OmniViewPoint_X_4(),
            config.Get_OmniViewPoint_Y_4(),
            config.Get_OmniViewPoint_Z_4() };
        mViewAngle = CalChart::Radian{ config.Get_OmniViewAngle_4() };
        mViewAngleZ = CalChart::Radian{ config.Get_OmniViewAngle_Z_4() };
        break;
    case '5':
        mViewPoint = ViewPoint{ config.Get_OmniViewPoint_X_5(),
            config.Get_OmniViewPoint_Y_5(),
            config.Get_OmniViewPoint_Z_5() };
        mViewAngle = CalChart::Radian{ config.Get_OmniViewAngle_5() };
        mViewAngleZ = CalChart::Radian{ config.Get_OmniViewAngle_Z_5() };
        break;
    case '6':
        mViewPoint = ViewPoint{ config.Get_OmniViewPoint_X_6(),
            config.Get_OmniViewPoint_Y_6(),
            config.Get_OmniViewPoint_Z_6() };
        mViewAngle = CalChart::Radian{ config.Get_OmniViewAngle_6() };
        mViewAngleZ = CalChart::Radian{ config.Get_OmniViewAngle_Z_6() };
        break;

    case '$':
        OnCmd_FollowMarcher(-1);
        if (wxMessageBox(wxT("Set Custom Viewpoint 4?"), wxT("Custom Viewpoint"), wxYES_NO) == wxYES) {
            config.Set_OmniViewPoint_X_4(mViewPoint.x);
            config.Set_OmniViewPoint_Y_4(mViewPoint.y);
            config.Set_OmniViewPoint_Z_4(mViewPoint.z);
            config.Set_OmniViewAngle_4(mViewAngle.getValue());
            config.Set_OmniViewAngle_Z_4(mViewAngleZ.getValue());
        }
        break;
    case '%':
        OnCmd_FollowMarcher(-1);
        if (wxMessageBox(wxT("Set Custom Viewpoint 5?"), wxT("Custom Viewpoint"), wxYES_NO) == wxYES) {
            config.Set_OmniViewPoint_X_5(mViewPoint.x);
            config.Set_OmniViewPoint_Y_5(mViewPoint.y);
            config.Set_OmniViewPoint_Z_5(mViewPoint.z);
            config.Set_OmniViewAngle_5(mViewAngle.getValue());
            config.Set_OmniViewAngle_Z_5(mViewAngleZ.getValue());
        }
        break;
    case '^':
        OnCmd_FollowMarcher(-1);
        if (wxMessageBox(wxT("Set Custom Viewpoint 6?"), wxT("Custom Viewpoint"), wxYES_NO) == wxYES) {
            config.Set_OmniViewPoint_X_6(mViewPoint.x);
            config.Set_OmniViewPoint_Y_6(mViewPoint.y);
            config.Set_OmniViewPoint_Z_6(mViewPoint.z);
            config.Set_OmniViewAngle_6(mViewAngle.getValue());
            config.Set_OmniViewAngle_Z_6(mViewAngleZ.getValue());
        }
        break;

    // up and down
    case '-':
        OnCmd_FollowMarcher(-1);
        mViewPoint.z -= stepIncr;
        mViewPoint.z = std::max(mViewPoint.z, 0.3f);
        break;
    case '+':
        OnCmd_FollowMarcher(-1);
        mViewPoint.z += stepIncr;
        break;

    // change view, do not move
    case 'q':
        OnCmd_FollowMarcher(-1);
        mViewAngle += AngleStepIncr;
        mViewAngle = CalChart::NormalizeAngle(mViewAngle);
        break;
    case 'e':
        OnCmd_FollowMarcher(-1);
        mViewAngle -= AngleStepIncr;
        mViewAngle = CalChart::NormalizeAngle(mViewAngle);
        break;
    case 'r':
        OnCmd_FollowMarcher(-1);
        mViewAngleZ += AngleStepIncr;
        break;
    case 'f':
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

    // move, following the FPS model
    case 'a':
        OnCmd_FollowMarcher(-1);
        mViewPoint.x += -stepIncr * cos(mViewAngle - CalChart::pi / 2);
        mViewPoint.y += -stepIncr * sin(mViewAngle - CalChart::pi / 2);
        break;
    case 'd':
        OnCmd_FollowMarcher(-1);
        mViewPoint.x += stepIncr * cos(mViewAngle - CalChart::pi / 2);
        mViewPoint.y += stepIncr * sin(mViewAngle - CalChart::pi / 2);
        break;
    case 's':
        OnCmd_FollowMarcher(-1);
        mViewPoint.x += -stepIncr * cos(mViewAngle);
        mViewPoint.y += -stepIncr * sin(mViewAngle);
        mViewPoint.z += -stepIncr * sin(mViewAngleZ);
        mViewPoint.z = std::max(mViewPoint.z, 0.3f);
        break;
    case 'w':
        OnCmd_FollowMarcher(-1);
        mViewPoint.x += stepIncr * cos(mViewAngle);
        mViewPoint.y += stepIncr * sin(mViewAngle);
        mViewPoint.z += stepIncr * sin(mViewAngleZ);
        mViewPoint.z = std::max(mViewPoint.z, 0.3f);
        break;

    case WXK_LEFT:
        mView->PrevBeat();
        break;
    case WXK_RIGHT:
        mView->NextBeat();
        break;
    case WXK_SPACE:
        mView->ToggleTimer();
        break;

    default:
        event.Skip();
    }

    Refresh();
}

void CCOmniviewCanvas::OnMouseMove(wxMouseEvent& event)
{
    if (event.ShiftDown()) {
        wxPoint thisPos = event.GetPosition();
        if (!mShiftMoving) {
            mShiftMoving = true;
            mStartShiftMoveMousePosition = thisPos;
            mStartShiftMoveViewAngle = mViewAngle;
            mStartShiftMoveViewAngleZ = mViewAngleZ;
        }
        const wxSize ClientSize = GetClientSize();
        wxPoint delta = thisPos - mStartShiftMoveMousePosition;
        mViewAngle = mStartShiftMoveViewAngle + CalChart::Radian{ sin(delta.x / static_cast<float>(ClientSize.x)) };
        mViewAngleZ = mStartShiftMoveViewAngleZ + CalChart::Radian{ sin(delta.y / static_cast<float>(ClientSize.y)) };
        Refresh();
    } else {
        mShiftMoving = false;
    }
}

void CCOmniviewCanvas::OnCmd_FollowMarcher(int which)
{
    mFollowMarcher = which;
    Refresh();
}

void CCOmniviewCanvas::OnCmd_SaveCameraAngle(size_t) { }

void CCOmniviewCanvas::OnCmd_GoToCameraAngle(size_t) { }

void CCOmniviewCanvas::OnCmd_ToggleCrowd()
{
    mCrowdOn = !mCrowdOn;
    Refresh();
}

void CCOmniviewCanvas::OnCmd_ToggleMarching()
{
    mShowMarching = !mShowMarching;
    Refresh();
}

void CCOmniviewCanvas::OnCmd_ShowKeyboardControls()
{
    wxMessageDialog dialog(
        this,
        wxT("1, 2, 3 : Select different camera angles (student, field, upper corner)\n")
            wxT("shift-1, shift-2, shift-3 : Select ALUMNI different camera angles (Alumni, field viewing east, upper corner)\n")
                wxT("4, 5, 6 : Select custom camera angles (set to student, field, upper corner by default)\n")
                    wxT("shift-4, shift-5, shift-6 : Set custom camera angle\n")
                        wxT("+ : Move camera up\n")
                            wxT("- : Move camera down\n")
                                wxT("w : Move camera forward\n")
                                    wxT("s : Move camera backward\n")
                                        wxT("a : Move camera left\n")
                                            wxT("d : Move camera right\n")
                                                wxT("q : Pan camera left\n")
                                                    wxT("e : Pan camera right\n")
                                                        wxT("r : Pan camera up\n")
                                                            wxT("f : Pan camera down\n")
                                                                wxT("< : Decrease Field Of View\n")
                                                                    wxT("> : Increase Field Of View\n")
                                                                        wxT("o : Toggle Crowd\n")
                                                                            wxT("left arrow : Back 1 beat\n")
                                                                                wxT("right arrow : Forward 1 beat\n")
                                                                                    wxT("space : Toggle Marching\n"),
        wxT("Keyboard Commands"),
        wxOK);
    dialog.ShowModal();
    return;
}
