/*
 * modes.cpp
 * Handle show mode classes
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

#include <wx/dc.h>
#include <wx/utils.h>
#include "modes.h"
#include "confgr.h"

#include <algorithm>

extern wxFont* yardLabelFont;

ShowMode::ShowMode(const wxString& nam, const CC_coord& siz, const CC_coord& off,
const CC_coord& bord1, const CC_coord& bord2)
: offset(off), size(siz), border1(bord1), border2(bord2)
{
	size += border1 + border2;
	offset += border1;
	name = nam;
}


ShowMode::ShowMode(const wxString& nam, const CC_coord& siz,
const CC_coord& bord1, const CC_coord& bord2)
: offset(siz/2), size(siz), border1(bord1), border2(bord2)
{
	size += border1 + border2;
	offset += border1;
	name = nam;
}


ShowMode::~ShowMode()
{
}


CC_coord ShowMode::ClipPosition(const CC_coord& pos) const
{
	CC_coord clipped;
	CC_coord min = MinPosition();
	CC_coord max = MaxPosition();

	if (pos.x < min.x) clipped.x = min.x;
	else if (pos.x > max.x) clipped.x = max.x;
	else clipped.x = pos.x;
	if (pos.y < min.y) clipped.y = min.y;
	else if (pos.y > max.y) clipped.y = max.y;
	else clipped.y = pos.y;
	return clipped;
}


ShowModeStandard::ShowModeStandard(const wxString& nam,
CC_coord bord1, CC_coord bord2,
unsigned short hashw, unsigned short hashe)
: ShowMode(nam, CC_coord(INT2COORD(160),INT2COORD(84)),bord1, bord2),
hashw(hashw), hashe(hashe)
{}

ShowModeStandard::ShowModeStandard(const wxString& nam,
CC_coord bord1, CC_coord bord2,
CC_coord siz, CC_coord off,
unsigned short hashw, unsigned short hashe)
: ShowMode(nam, siz, off, bord1, bord2), hashw(hashw), hashe(hashe)
{}

ShowModeStandard::~ShowModeStandard() {}

SHOW_TYPE ShowModeStandard::GetType() const
{
	return SHOW_STANDARD;
}


void ShowModeStandard::Draw(wxDC *dc) const
{
	unsigned short i;
	Coord j, k;
	wxPoint points[5];
	wxCoord textw, texth, textd;
	CC_coord fieldsize, fieldedge;

	fieldsize = size - border1 - border2;
	fieldedge = offset - border1;

	points[0].x = 0;
	points[0].y = 0;
	points[1].x = fieldsize.x;
	points[1].y = 0;
	points[2].x = fieldsize.x;
	points[2].y = fieldsize.y;
	points[3].x = 0;
	points[3].y = fieldsize.y;
	points[4].x = 0;
	points[4].y = 0;

// Draw vertical lines
	dc->DrawLines(5, points, border1.x, border1.y);
	for (j = 0; j < fieldsize.x; j+=INT2COORD(8))
	{
		dc->DrawLine(j+border1.x, border1.y,
			j+border1.x, size.y - border2.y);
		for (k = 0; k < fieldsize.y; k+=INT2COORD(2))
		{
			dc->DrawLine(j+INT2COORD(4)+border1.x, k+border1.y,
				j+INT2COORD(4)+border1.x, k+INT2COORD(1)+border1.y);
		}
	}

	for (j = 0; j < fieldsize.x; j+=INT2COORD(8))
	{
// Draw horizontal lines
		for (i = 4; i < COORD2INT(fieldsize.y); i+=4)
		{
			if ((i != hashw) && (i != hashe))
			{
				dc->DrawLine(j+border1.x, INT2COORD(i)+border1.y,
					j+INT2COORD(1)+border1.x, INT2COORD(i)+border1.y);
				dc->DrawLine(j+INT2COORD(2)+border1.x, INT2COORD(i)+border1.y,
					j+INT2COORD(3)+border1.x, INT2COORD(i)+border1.y);
				dc->DrawLine(j+INT2COORD(4)+border1.x, INT2COORD(i)+border1.y,
					j+INT2COORD(5)+border1.x, INT2COORD(i)+border1.y);
				dc->DrawLine(j+INT2COORD(6)+border1.x, INT2COORD(i)+border1.y,
					j+INT2COORD(7)+border1.x, INT2COORD(i)+border1.y);
			}
		}
// Draw hashes
		dc->DrawLine(j+border1.x,
			INT2COORD(hashw)+border1.y,
			j+FLOAT2COORD(.1*8)+border1.x,
			INT2COORD(hashw)+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.9*8)+border1.x,
			INT2COORD(hashw)+border1.y,
			j+INT2COORD(8)+border1.x,
			INT2COORD(hashw)+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.2*8)+border1.x,
			INT2COORD(hashw)+border1.y,
			j+FLOAT2COORD(.2*8)+border1.x,
			FLOAT2COORD(hashw-(.2*8))+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.4*8)+border1.x,
			INT2COORD(hashw)+border1.y,
			j+FLOAT2COORD(.4*8)+border1.x,
			FLOAT2COORD(hashw-(.2*8))+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.6*8)+border1.x,
			INT2COORD(hashw)+border1.y,
			j+FLOAT2COORD(.6*8)+border1.x,
			FLOAT2COORD(hashw-(.2*8))+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.8*8)+border1.x,
			INT2COORD(hashw)+border1.y,
			j+FLOAT2COORD(.8*8)+border1.x,
			FLOAT2COORD(hashw-(.2*8))+border1.y);

		dc->DrawLine(j+border1.x,
			INT2COORD(hashe)+border1.y,
			j+FLOAT2COORD(.1*8)+border1.x,
			INT2COORD(hashe)+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.9*8)+border1.x,
			INT2COORD(hashe)+border1.y,
			j+INT2COORD(8)+border1.x,
			INT2COORD(hashe)+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.2*8)+border1.x,
			INT2COORD(hashe)+border1.y,
			j+FLOAT2COORD(.2*8)+border1.x,
			FLOAT2COORD(hashe+(.2*8))+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.4*8)+border1.x,
			INT2COORD(hashe)+border1.y,
			j+FLOAT2COORD(.4*8)+border1.x,
			FLOAT2COORD(hashe+(.2*8))+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.6*8)+border1.x,
			INT2COORD(hashe)+border1.y,
			j+FLOAT2COORD(.6*8)+border1.x,
			FLOAT2COORD(hashe+(.2*8))+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.8*8)+border1.x,
			INT2COORD(hashe)+border1.y,
			j+FLOAT2COORD(.8*8)+border1.x,
			FLOAT2COORD(hashe+(.2*8))+border1.y);
	}

	dc->SetFont(*yardLabelFont);
	for (i = 0; i < COORD2INT(fieldsize.x)/8+1; i++)
	{
		dc->GetTextExtent(yard_text[i+(-COORD2INT(fieldedge.x)+(MAX_YARD_LINES-1)*4)/8], &textw, &texth, &textd);
		dc->DrawText(yard_text[i+(-COORD2INT(fieldedge.x)+(MAX_YARD_LINES-1)*4)/8],
			INT2COORD(i*8) - textw/2 + border1.x,
			border1.y - texth);
		dc->DrawText(yard_text[i+(-COORD2INT(fieldedge.x)+(MAX_YARD_LINES-1)*4)/8],
			INT2COORD(i*8) - textw/2 + border1.x,
			size.y - border2.y);
	}
}


void ShowModeStandard::DrawAnim(wxDC *dc) const
{
	Coord j;
	wxPoint points[5];
	CC_coord fieldsize, fieldedge;

	fieldsize = size - border1 - border2;
	fieldedge = offset - border1;

	points[0].x = 0;
	points[0].y = 0;
	points[1].x = fieldsize.x;
	points[1].y = 0;
	points[2].x = fieldsize.x;
	points[2].y = fieldsize.y;
	points[3].x = 0;
	points[3].y = fieldsize.y;
	points[4].x = 0;
	points[4].y = 0;

// Draw vertical lines
	dc->DrawLines(5, points, border1.x, border1.y);
	for (j = INT2COORD(8); j < fieldsize.x; j+=INT2COORD(8))
	{
		dc->DrawLine(j+border1.x, border1.y,
			j+border1.x, size.y - border2.y);
	}

	for (j = 0; j < fieldsize.x; j+=INT2COORD(8))
	{
// Draw hashes
		dc->DrawLine(j+border1.x,
			INT2COORD(hashw)+border1.y,
			j+FLOAT2COORD(.1*8)+border1.x,
			INT2COORD(hashw)+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.9*8)+border1.x,
			INT2COORD(hashw)+border1.y,
			j+INT2COORD(8)+border1.x,
			INT2COORD(hashw)+border1.y);

		dc->DrawLine(j+border1.x,
			INT2COORD(hashe)+border1.y,
			j+FLOAT2COORD(.1*8)+border1.x,
			INT2COORD(hashe)+border1.y);
		dc->DrawLine(j+FLOAT2COORD(.9*8)+border1.x,
			INT2COORD(hashe)+border1.y,
			j+INT2COORD(8)+border1.x,
			INT2COORD(hashe)+border1.y);
	}

#ifdef TEXT_ON_ANIM
	unsigned short i;
	wxCoord textw, texth, textd;
	dc->SetFont(*yardLabelFont);
	for (i = 0; i < COORD2INT(fieldsize.x)/8+1; i++)
	{
		dc->GetTextExtent(yard_text[i+(-COORD2INT(fieldedge.x)+(MAX_YARD_LINES-1)*4)/8], &textw, &texth, &textd);
		dc->DrawText(yard_text[i+(-COORD2INT(fieldedge.x)+(MAX_YARD_LINES-1)*4)/8],
			INT2COORD(i*8) - textw/2 + border1.x,
			border1.y - texth);
		dc->DrawText(yard_text[i+(-COORD2INT(fieldedge.x)+(MAX_YARD_LINES-1)*4)/8],
			INT2COORD(i*8) - textw/2 + border1.x,
			size.y - border2.y);
	}
#endif
}


ShowModeSprShow::ShowModeSprShow(const wxString& nam,
CC_coord bord1, CC_coord bord2,
unsigned char which,
short stps_x, short stps_y,
short stps_w, short stps_h,
short stg_x, short stg_y,
short stg_w, short stg_h,
short fld_x, short fld_y,
short fld_w, short fld_h,
short txt_l, short txt_r,
short txt_tp, short txt_bm)
: ShowMode(nam, CC_coord(INT2COORD(stps_w),INT2COORD(stps_h)),
CC_coord(INT2COORD(-stps_x),INT2COORD(-stps_y)), bord1, bord2),
which_yards(which),
stage_x(stg_x), stage_y(stg_y), stage_w(stg_w), stage_h(stg_h),
field_x(fld_x), field_y(fld_y), field_w(fld_w), field_h(fld_h),
steps_x(stps_x), steps_y(stps_y), steps_w(stps_w), steps_h(stps_h),
text_left(txt_l), text_right(txt_r), text_top(txt_tp), text_bottom(txt_bm)
{
}


ShowModeSprShow::~ShowModeSprShow()
{
}


SHOW_TYPE ShowModeSprShow::GetType() const
{
	return SHOW_SPRINGSHOW;
}


void ShowModeSprShow::Draw(wxDC *dc) const
{
	unsigned short i;
	Coord j, k;
	wxCoord textw, texth, textd;
	CC_coord fieldsize;

	fieldsize = size - border1 - border2;

// Draw vertical lines
	for (j = 0; j <= fieldsize.x; j+=INT2COORD(8))
	{
		dc->DrawLine(j+border1.x, border1.y,
			j+border1.x, size.y - border2.y);
	}
	for (j = INT2COORD(4); j <= fieldsize.x; j+=INT2COORD(8))
	{
		for (k = 0; k < fieldsize.y; k+=INT2COORD(2))
		{
			dc->DrawLine(j+border1.x, k+border1.y,
				j+border1.x, k+INT2COORD(1)+border1.y);
		}
	}

	for (j = 0; j < fieldsize.x; j+=INT2COORD(8))
	{
// Draw horizontal lines
		for (k = INT2COORD(4); k <= fieldsize.y; k+=INT2COORD(8))
		{
			dc->DrawLine(j+border1.x, k+border1.y,
				j+INT2COORD(1)+border1.x, k+border1.y);
			dc->DrawLine(j+INT2COORD(2)+border1.x, k+border1.y,
				j+INT2COORD(3)+border1.x, k+border1.y);
			dc->DrawLine(j+INT2COORD(4)+border1.x, k+border1.y,
				j+INT2COORD(5)+border1.x, k+border1.y);
			dc->DrawLine(j+INT2COORD(6)+border1.x, k+border1.y,
				j+INT2COORD(7)+border1.x, k+border1.y);
		}
	}
	for (k = 0; k <= fieldsize.y; k+=INT2COORD(8))
	{
		dc->DrawLine(border1.x, k+border1.y,
			fieldsize.x+border1.x, k+border1.y);
	}

	dc->SetFont(*yardLabelFont);
	for (i = 0; i < COORD2INT(fieldsize.x)/8+1; i++)
	{
		dc->GetTextExtent(yard_text[i+(steps_x+(MAX_YARD_LINES-1)*4)/8], &textw, &texth, &textd);
		if (which_yards & SPR_YARD_ABOVE)
			dc->DrawText(yard_text[i+(steps_x+(MAX_YARD_LINES-1)*4)/8],
				INT2COORD(i*8) - textw/2 + border1.x,
				border1.y - texth);
		if (which_yards & SPR_YARD_BELOW)
			dc->DrawText(yard_text[i+(steps_x+(MAX_YARD_LINES-1)*4)/8],
				INT2COORD(i*8) - textw/2 + border1.x,
				size.y - border2.y);
	}
	for (i = 0; i <= COORD2INT(fieldsize.y); i+=8)
	{
		dc->GetTextExtent(spr_line_text[i/8], &textw, &texth, &textd);
		if (which_yards & SPR_YARD_LEFT)
			dc->DrawText(spr_line_text[i/8],
				border1.x - textw,
				border1.y - texth/2 + INT2COORD(i));
		if (which_yards & SPR_YARD_RIGHT)
			dc->DrawText(spr_line_text[i/8],
				fieldsize.x + border1.x,
				border1.y - texth/2 + INT2COORD(i));
	}
}


void ShowModeSprShow::DrawAnim(wxDC *dc) const
{
	Coord j;
	CC_coord fieldsize;

	fieldsize = size - border1 - border2;

// Draw vertical lines
	for (j = 0; j <= fieldsize.x; j+=INT2COORD(8))
	{
		dc->DrawLine(j+border1.x, border1.y,
			j+border1.x, size.y - border2.y);
	}

// Draw horizontal lines
	for (j = 0; j <= fieldsize.y; j+=INT2COORD(8))
	{
		dc->DrawLine(border1.x, j+border1.y,
			fieldsize.x+border1.x, j+border1.y);
	}

#ifdef TEXT_ON_ANIM
	unsigned short i;
	wxCoord textw, texth, textd;
	dc->SetFont(*yardLabelFont);
	for (i = 0; i < COORD2INT(fieldsize.x)/8+1; i++)
	{
		dc->GetTextExtent(yard_text[i+(steps_x+(MAX_YARD_LINES-1)*4)/8], &textw, &texth, &textd);
		if (which_yards & SPR_YARD_ABOVE)
			dc->DrawText(yard_text[i+(steps_x+(MAX_YARD_LINES-1)*4)/8],
				INT2COORD(i*8) - textw/2 + border1.x,
				border1.y - texth);
		if (which_yards & SPR_YARD_BELOW)
			dc->DrawText(yard_text[i+(steps_x+(MAX_YARD_LINES-1)*4)/8],
				INT2COORD(i*8) - textw/2 + border1.x,
				size.y - border2.y);
	}
	for (i = 0; i <= COORD2INT(fieldsize.y); i+=8)
	{
		dc->GetTextExtent(spr_line_text[i/8], &textw, &texth, &textd);
		if (which_yards & SPR_YARD_LEFT)
			dc->DrawText(spr_line_text[i/8],
				border1.x - textw,
				border1.y - texth/2 + INT2COORD(i));
		if (which_yards & SPR_YARD_RIGHT)
			dc->DrawText(spr_line_text[i/8],
				fieldsize.x + border1.x,
				border1.y - texth/2 + INT2COORD(i));
	}
#endif
}


ShowModeList::ShowModeList()
{}

struct DeleteObject
{
	template <typename T>
		void operator()(const T* ptr) const
	{
		delete ptr;
	}
};

ShowModeList::~ShowModeList()
{
	std::for_each(list.begin(), list.end(), DeleteObject());
}


ShowMode *ShowModeList::Find(const wxString& name) const
{
	for (CIter i = list.begin(); i != list.end(); ++i)
		if ((*i)->GetName() == name)
			return *i;
	return NULL;
}
