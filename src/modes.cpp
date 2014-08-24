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

#include "modes.h"
#include "confgr.h"
#include <wx/dc.h>

#include <algorithm>

ShowMode::ShowMode(const wxString& name,
				   const CC_coord& size,
				   const CC_coord& offset,
				   const CC_coord& border1,
				   const CC_coord& border2) :
mOffset(offset + border1),
mSize(size + border1 + border2),
mBorder1(border1),
mBorder2(border2),
mName(name)
{}


ShowMode::~ShowMode()
{}


CC_coord
ShowMode::ClipPosition(const CC_coord& pos) const
{
	auto min = MinPosition();
	auto max = MaxPosition();

	CC_coord clipped;
	if (pos.x < min.x) clipped.x = min.x;
	else if (pos.x > max.x) clipped.x = max.x;
	else clipped.x = pos.x;
	if (pos.y < min.y) clipped.y = min.y;
	else if (pos.y > max.y) clipped.y = max.y;
	else clipped.y = pos.y;
	return clipped;
}


wxImage
ShowMode::GetOmniLinesImage(const CalChartConfiguration& config) const
{
	auto fieldsize = FieldSize();
	wxBitmap bmp(fieldsize.x, fieldsize.y, 32);
    wxMemoryDC dc;
    dc.SelectObject(bmp);
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
	DrawMode(dc, config, ShowMode::kOmniView);
    return bmp.ConvertToImage();
}


ShowModeStandard::ShowModeStandard(const wxString& name,
								   CC_coord size,
								   CC_coord offset,
								   CC_coord border1,
								   CC_coord border2,
								   unsigned short hashw,
								   unsigned short hashe) :
ShowMode(name, size, offset, border1, border2),
mHashW(hashw),
mHashE(hashe)
{}

ShowModeStandard::~ShowModeStandard() {}

ShowMode::ShowType ShowModeStandard::GetType() const
{
	return SHOW_STANDARD;
}

void
ShowMode::DrawMode(wxDC& dc, const CalChartConfiguration& config, HowToDraw howToDraw) const
{
	switch (howToDraw)
	{
		case kFieldView:
		case kAnimation:
		case kOmniView:
			dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_FIELD_DETAIL).second);
			dc.SetTextForeground(config.Get_CalChartBrushAndPen(COLOR_FIELD_TEXT).second.GetColour());
			break;
		case kPrinting:
			dc.SetPen(*wxBLACK_PEN);
			dc.SetTextForeground(*wxBLACK);
			break;
	}
	// draw the field
	DrawHelper(dc, config, howToDraw);
}

void ShowModeStandard::DrawHelper(wxDC& dc, const CalChartConfiguration& config, HowToDraw howToDraw) const
{
	wxPoint points[5];
	auto fieldsize = FieldSize();
	CC_coord border1 = mBorder1;
	CC_coord border2 = mBorder2;
	if (howToDraw == kOmniView)
	{
		border1 = border2 = CC_coord(0, 0);
	}
	
	points[0] = wxPoint(0, 0);
	points[1] = wxPoint(fieldsize.x, 0);
	points[2] = wxPoint(fieldsize.x, fieldsize.y);
	points[3] = wxPoint(0, fieldsize.y);
	points[4] = points[0];
	
	// Draw outline
	dc.DrawLines(5, points, border1.x, border1.y);
	
	// Draw vertical lines
	for (Coord j = Int2Coord(8); j < fieldsize.x; j += Int2Coord(8))
	{
		// draw solid yardlines
		points[0] = wxPoint(j, 0);
		points[1] = wxPoint(j, fieldsize.y);
		dc.DrawLines(2, points, border1.x, border1.y);
	}
	
	for (Coord j = Int2Coord(4); howToDraw == kFieldView && j < fieldsize.x; j += Int2Coord(8))
	{
		// draw mid-dotted lines
		for (Coord k = 0; k < fieldsize.y; k += Int2Coord(2))
		{
			points[0] = wxPoint(j, k);
			points[1] = wxPoint(j, k + Int2Coord(1));
			dc.DrawLines(2, points, border1.x, border1.y);
		}
	}
	
	// Draw horizontal mid-dotted lines
	for (Coord j = Int2Coord(4); howToDraw == kFieldView && j < fieldsize.y; j += Int2Coord(4))
	{
		if ((j == Int2Coord(mHashW)) || j == Int2Coord(mHashE))
			continue;
		for (Coord k = 0; k < fieldsize.x; k += Int2Coord(2))
		{
			points[0] = wxPoint(k, j);
			points[1] = wxPoint(k + Int2Coord(1), j);
			dc.DrawLines(2, points, border1.x, border1.y);
		}
	}
	
	// Draw hashes
	for (Coord j = Int2Coord(0); j < fieldsize.x; j += Int2Coord(8))
	{
		points[0] = wxPoint(j+Float2Coord(0.0*8), Int2Coord(mHashW));
		points[1] = wxPoint(j+Float2Coord(0.1*8), Int2Coord(mHashW));
		dc.DrawLines(2, points, border1.x, border1.y);
		points[0] = wxPoint(j+Float2Coord(0.9*8), Int2Coord(mHashW));
		points[1] = wxPoint(j+Float2Coord(1.0*8), Int2Coord(mHashW));
		dc.DrawLines(2, points, border1.x, border1.y);
		
		points[0] = wxPoint(j+Float2Coord(0.0*8), Int2Coord(mHashE));
		points[1] = wxPoint(j+Float2Coord(0.1*8), Int2Coord(mHashE));
		dc.DrawLines(2, points, border1.x, border1.y);
		points[0] = wxPoint(j+Float2Coord(0.9*8), Int2Coord(mHashE));
		points[1] = wxPoint(j+Float2Coord(1.0*8), Int2Coord(mHashE));
		dc.DrawLines(2, points, border1.x, border1.y);
		
		for (size_t midhash = 1; howToDraw == kFieldView && midhash < 5; ++midhash)
		{
			points[0] = wxPoint(j+Float2Coord(midhash/5.0*8), Int2Coord(mHashW));
			points[1] = wxPoint(j+Float2Coord(midhash/5.0*8), Float2Coord(mHashW-(0.2*8)));
			dc.DrawLines(2, points, border1.x, border1.y);
			
			points[0] = wxPoint(j+Float2Coord(midhash/5.0*8), Int2Coord(mHashE));
			points[1] = wxPoint(j+Float2Coord(midhash/5.0*8), Float2Coord(mHashE+(0.2*8)));
			dc.DrawLines(2, points, border1.x, border1.y);
		}
	}
	
	// Draw labels
	wxFont *yardLabelFont = wxTheFontList->FindOrCreateFont((int)Float2Coord(config.Get_YardsSize()),
															wxSWISS, wxNORMAL, wxNORMAL);
	dc.SetFont(*yardLabelFont);
	for (int i = 0; (howToDraw == kFieldView || howToDraw == kOmniView) && i < Coord2Int(fieldsize.x)/8+1; i++)
	{
		CC_coord fieldedge = mOffset - mBorder1;
		wxCoord textw, texth, textd;
		dc.GetTextExtent(config.Get_yard_text(i+(-Coord2Int(fieldedge.x)+(MAX_YARD_LINES-1)*4)/8), &textw, &texth, &textd);
		dc.DrawText(config.Get_yard_text(i+(-Coord2Int(fieldedge.x)+(MAX_YARD_LINES-1)*4)/8), Int2Coord(i*8) - textw/2 + border1.x, border1.y - texth + ((howToDraw == kOmniView) ? Int2Coord(8) : 0));
		dc.DrawText(config.Get_yard_text(i+(-Coord2Int(fieldedge.x)+(MAX_YARD_LINES-1)*4)/8), Int2Coord(i*8) - textw/2 + border1.x, border1.y + fieldsize.y - ((howToDraw == kOmniView) ? Int2Coord(8) : 0));
	}
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
: ShowMode(nam, CC_coord(Int2Coord(stps_w),Int2Coord(stps_h)),
CC_coord(Int2Coord(-stps_x),Int2Coord(-stps_y)), bord1, bord2),
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


ShowMode::ShowType ShowModeSprShow::GetType() const
{
	return SHOW_SPRINGSHOW;
}


void ShowModeSprShow::DrawHelper(wxDC& dc, const CalChartConfiguration& config, HowToDraw howToDraw) const
{
	wxPoint points[2];
	CC_coord fieldsize = mSize - mBorder1 - mBorder2;

	// Draw vertical lines
	for (Coord j = 0; j <= fieldsize.x; j+=Int2Coord(8))
	{
		// draw solid yardlines
		points[0] = wxPoint(j, 0);
		points[1] = wxPoint(j, fieldsize.y);
		dc.DrawLines(2, points, mBorder1.x, mBorder1.y);
	}

	for (Coord j = Int2Coord(4); howToDraw == kFieldView && j < fieldsize.x; j += Int2Coord(8))
	{
		// draw mid-dotted lines
		for (Coord k = 0; k < fieldsize.y; k += Int2Coord(2))
		{
			points[0] = wxPoint(j, k);
			points[1] = wxPoint(j, k + Int2Coord(1));
			dc.DrawLines(2, points, mBorder1.x, mBorder1.y);
		}
	}
	
	// Draw horizontal lines
	for (Coord j = 0; j <= fieldsize.y; j+=Int2Coord(8))
	{
		// draw solid yardlines
		points[0] = wxPoint(0, j);
		points[1] = wxPoint(fieldsize.x, j);
		dc.DrawLines(2, points, mBorder1.x, mBorder1.y);
	}
	
	// Draw horizontal mid-dotted lines
	for (Coord j = Int2Coord(4); howToDraw == kFieldView && j <= fieldsize.y; j += Int2Coord(8))
	{
		for (Coord k = 0; k < fieldsize.x; k += Int2Coord(2))
		{
			points[0] = wxPoint(k, j);
			points[1] = wxPoint(k + Int2Coord(1), j);
			dc.DrawLines(2, points, mBorder1.x, mBorder1.y);
		}
	}

	// Draw labels
	wxFont *yardLabelFont = wxTheFontList->FindOrCreateFont((int)Float2Coord(config.Get_YardsSize()),
															wxSWISS, wxNORMAL, wxNORMAL);
	dc.SetFont(*yardLabelFont);
	for (int i = 0; howToDraw == kFieldView && i < Coord2Int(fieldsize.x)/8+1; i++)
	{
		wxCoord textw, texth, textd;
		dc.GetTextExtent(config.Get_yard_text(i+(steps_x+(MAX_YARD_LINES-1)*4)/8), &textw, &texth, &textd);
		if (which_yards & SPR_YARD_ABOVE)
			dc.DrawText(config.Get_yard_text(i+(steps_x+(MAX_YARD_LINES-1)*4)/8), Int2Coord(i*8) - textw/2 + mBorder1.x, mBorder1.y - texth);
		if (which_yards & SPR_YARD_BELOW)
			dc.DrawText(config.Get_yard_text(i+(steps_x+(MAX_YARD_LINES-1)*4)/8), Int2Coord(i*8) - textw/2 + mBorder1.x, mSize.y - mBorder2.y);
	}
	for (int i = 0; howToDraw == kFieldView && i <= Coord2Int(fieldsize.y); i+=8)
	{
		wxCoord textw, texth, textd;
		dc.GetTextExtent(config.Get_spr_line_text(i/8), &textw, &texth, &textd);
		if (which_yards & SPR_YARD_LEFT)
			dc.DrawText(config.Get_spr_line_text(i/8), mBorder1.x - textw, mBorder1.y - texth/2 + Int2Coord(i));
		if (which_yards & SPR_YARD_RIGHT)
			dc.DrawText(config.Get_spr_line_text(i/8), fieldsize.x + mBorder1.x, mBorder1.y - texth/2 + Int2Coord(i));
	}
}

class FindByName
{
public:
	FindByName(wxString name) : mName(name) {}
	template <typename T>
	bool operator()(const T& a) { return mName == a->GetName(); }
private:
	wxString mName;
};

ShowMode*
ShowModeList_Find(const ShowModeList& showModes, const wxString& which)
{
	ShowModeList::const_iterator i;
	if ((i = std::find_if(showModes.begin(), showModes.end(), FindByName(which))) != showModes.end())
		return (*i).get();
	return NULL;
}

std::unique_ptr<ShowMode>
CreateShowMode(const wxString& which, std::vector<long> values)
{
	unsigned short whash = values[0];
	unsigned short ehash = values[1];
	CC_coord bord1, bord2;
	bord1.x = Int2Coord(values[2]);
	bord1.y = Int2Coord(values[3]);
	bord2.x = Int2Coord(values[4]);
	bord2.y = Int2Coord(values[5]);
	CC_coord size, offset;
	offset.x = Int2Coord(-values[6]);
	offset.y = Int2Coord(-values[7]);
	size.x = Int2Coord(values[8]);
	size.y = Int2Coord(values[9]);
	return std::unique_ptr<ShowMode>(new ShowModeStandard(which, size, offset, bord1, bord2, whash, ehash));
}

std::unique_ptr<ShowMode>
CreateSpringShowMode(const wxString& which, std::vector<long> values)
{
	unsigned char which_spr_yards = values[0];
	CC_coord bord1, bord2;
	bord1.x = Int2Coord(values[1]);
	bord1.y = Int2Coord(values[2]);
	bord2.x = Int2Coord(values[3]);
	bord2.y = Int2Coord(values[4]);
	
	short mode_steps_x = values[5];
	short mode_steps_y = values[6];
	short mode_steps_w = values[7];
	short mode_steps_h = values[8];
	short eps_stage_x = values[9];
	short eps_stage_y = values[10];
	short eps_stage_w = values[11];
	short eps_stage_h = values[12];
	short eps_field_x = values[13];
	short eps_field_y = values[14];
	short eps_field_w = values[15];
	short eps_field_h = values[16];
	short eps_text_left = values[17];
	short eps_text_right = values[18];
	short eps_text_top = values[19];
	short eps_text_bottom = values[20];
	return std::unique_ptr<ShowMode>(new ShowModeSprShow(which, bord1, bord2,
														 which_spr_yards,
														 mode_steps_x, mode_steps_y,
														 mode_steps_w, mode_steps_h,
														 eps_stage_x, eps_stage_y,
														 eps_stage_w, eps_stage_h,
														 eps_field_x, eps_field_y,
														 eps_field_w, eps_field_h,
														 eps_text_left, eps_text_right,
														 eps_text_top, eps_text_bottom));
}

