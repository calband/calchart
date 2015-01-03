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

#include <algorithm>

ShowMode::ShowMode(const std::string& name,
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


ShowModeStandard::ShowModeStandard(const std::string& name,
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
	return ShowType::STANDARD;
}

ShowModeSprShow::ShowModeSprShow(const std::string& nam,
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
	return ShowType::SPRINGSHOW;
}


std::unique_ptr<ShowMode>
ShowModeStandard::CreateShowMode(const std::string& which, const ShowModeInfo_t& values)
{
	unsigned short whash = values[toUType(ArrayValues::kwhash)];
	unsigned short ehash = values[toUType(ArrayValues::kehash)];
	CC_coord bord1, bord2;
	bord1.x = Int2Coord(values[toUType(ArrayValues::kbord1_x)]);
	bord1.y = Int2Coord(values[toUType(ArrayValues::kbord1_y)]);
	bord2.x = Int2Coord(values[toUType(ArrayValues::kbord2_x)]);
	bord2.y = Int2Coord(values[toUType(ArrayValues::kbord2_y)]);
	CC_coord size, offset;
	offset.x = Int2Coord(-values[toUType(ArrayValues::koffset_x)]);
	offset.y = Int2Coord(-values[toUType(ArrayValues::koffset_y)]);
	size.x = Int2Coord(values[toUType(ArrayValues::ksize_x)]);
	size.y = Int2Coord(values[toUType(ArrayValues::ksize_y)]);
	return std::unique_ptr<ShowMode>(new ShowModeStandard(which, size, offset, bord1, bord2, whash, ehash));
}

std::unique_ptr<ShowMode>
ShowModeStandard::CreateShowMode(const std::string& name,
								 CC_coord size,
								 CC_coord offset,
								 CC_coord border1,
								 CC_coord border2,
								 unsigned short whash,
								 unsigned short ehash)
{
	return std::unique_ptr<ShowMode>(new ShowModeStandard(name, size, offset, border1, border2, whash, ehash));
}

std::unique_ptr<ShowMode>
ShowModeSprShow::CreateSpringShowMode(const std::string& which, const SpringShowModeInfo_t& values)
{
	unsigned char which_spr_yards = values[toUType(ArrayValues::kwhich_spr_yards)];
	CC_coord bord1, bord2;
	bord1.x = Int2Coord(values[toUType(ArrayValues::kbord1_x)]);
	bord1.y = Int2Coord(values[toUType(ArrayValues::kbord1_y)]);
	bord2.x = Int2Coord(values[toUType(ArrayValues::kbord2_x)]);
	bord2.y = Int2Coord(values[toUType(ArrayValues::kbord2_y)]);
	
	short mode_steps_x = values[toUType(ArrayValues::kmode_steps_x)];
	short mode_steps_y = values[toUType(ArrayValues::kmode_steps_y)];
	short mode_steps_w = values[toUType(ArrayValues::kmode_steps_w)];
	short mode_steps_h = values[toUType(ArrayValues::kmode_steps_h)];
	short eps_stage_x = values[toUType(ArrayValues::keps_stage_x)];
	short eps_stage_y = values[toUType(ArrayValues::keps_stage_y)];
	short eps_stage_w = values[toUType(ArrayValues::keps_stage_w)];
	short eps_stage_h = values[toUType(ArrayValues::keps_stage_h)];
	short eps_field_x = values[toUType(ArrayValues::keps_field_x)];
	short eps_field_y = values[toUType(ArrayValues::keps_field_y)];
	short eps_field_w = values[toUType(ArrayValues::keps_field_w)];
	short eps_field_h = values[toUType(ArrayValues::keps_field_h)];
	short eps_text_left = values[toUType(ArrayValues::keps_text_left)];
	short eps_text_right = values[toUType(ArrayValues::keps_text_right)];
	short eps_text_top = values[toUType(ArrayValues::keps_text_top)];
	short eps_text_bottom = values[toUType(ArrayValues::keps_text_bottom)];
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

