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
mOffset([=](){ return offset; }),
mSize([=](){ return size; }),
mBorder1([=](){ return border1; }),
mBorder2([=](){ return border2; }),
mName(name)
{}


ShowMode::ShowMode(const std::string& name,
			 const std::function<CC_coord()>& size,
			 const std::function<CC_coord()>& offset,
			 const std::function<CC_coord()>& border1,
			 const std::function<CC_coord()>& border2) :
mOffset(offset),
mSize(size),
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
mHashW([=](){ return hashw; }),
mHashE([=](){ return hashe; })
{}

ShowModeStandard::ShowModeStandard(const std::string& name,
					 const std::function<CC_coord()>& size,
					 const std::function<CC_coord()>& offset,
					 const std::function<CC_coord()>& border1,
					 const std::function<CC_coord()>& border2,
					 const std::function<unsigned short()>& whash,
					 const std::function<unsigned short()>& ehash) :
ShowMode(name, offset, size, border1, border2),
mHashW(whash),
mHashE(ehash)
{}

ShowModeStandard::~ShowModeStandard() {}

ShowMode::ShowType ShowModeStandard::GetType() const
{
	return SHOW_STANDARD;
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
which_yards([=](){ return which; }),
stage_x([=](){ return stg_x; }),
stage_y([=](){ return stg_y; }),
stage_w([=](){ return stg_w; }),
stage_h([=](){ return stg_h; }),
field_x([=](){ return fld_x; }),
field_y([=](){ return fld_y; }),
field_w([=](){ return fld_w; }),
field_h([=](){ return fld_h; }),
steps_x([=](){ return stps_x; }),
steps_y([=](){ return stps_y; }),
steps_w([=](){ return stps_w; }),
steps_h([=](){ return stps_h; }),
text_left([=](){ return txt_l; }),
text_right([=](){ return txt_r; }),
text_top([=](){ return txt_tp; }),
text_bottom([=](){ return txt_bm; })
{
}

ShowModeSprShow::ShowModeSprShow(const std::string& nam, const std::function<CC_coord()>& bord1, const std::function<CC_coord()>& bord2,
		const std::function<unsigned char()>& which,
		const std::function<short()>& stps_x, const std::function<short()>& stps_y,
		const std::function<short()>& stps_w, const std::function<short()>& stps_h,
		const std::function<short()>& stg_x, const std::function<short()>& stg_y,
		const std::function<short()>& stg_w, const std::function<short()>& stg_h,
		const std::function<short()>& fld_x, const std::function<short()>& fld_y,
		const std::function<short()>& fld_w, const std::function<short()>& fld_h,
		const std::function<short()>& txt_l, const std::function<short()>& txt_r,
		const std::function<short()>& txt_tp, const std::function<short()>& txt_bm)
: ShowMode(nam, [=](){ return CC_coord(Int2Coord(stps_w()),Int2Coord(stps_h())); },
[=](){ return CC_coord(Int2Coord(-stps_x()),Int2Coord(-stps_y())); }, bord1, bord2),
which_yards(which),
stage_x(stg_x),
stage_y(stg_y),
stage_w(stg_w),
stage_h(stg_h),
field_x(fld_x),
field_y(fld_y),
field_w(fld_w),
field_h(fld_h),
steps_x(stps_x),
steps_y(stps_y),
steps_w(stps_w),
steps_h(stps_h),
text_left(txt_l),
text_right(txt_r),
text_top(txt_tp),
text_bottom(txt_bm)
{
}


ShowModeSprShow::~ShowModeSprShow()
{
}


ShowMode::ShowType ShowModeSprShow::GetType() const
{
	return SHOW_SPRINGSHOW;
}


std::unique_ptr<ShowMode>
ShowModeStandard::CreateShowMode(const std::string& which, const ShowModeInfo_t& values)
{
	unsigned short whash = values[kwhash];
	unsigned short ehash = values[kehash];
	CC_coord bord1{ Int2Coord(values[kbord1_x]), Int2Coord(values[kbord1_y]) };
	CC_coord bord2{ Int2Coord(values[kbord2_x]), Int2Coord(values[kbord2_y]) };
	CC_coord size{ Int2Coord(-values[koffset_x]), Int2Coord(-values[koffset_y]) };
	CC_coord offset{ Int2Coord(values[ksize_x]), Int2Coord(values[ksize_y]) };
	return std::unique_ptr<ShowMode>(new ShowModeStandard(which, size, offset, bord1, bord2, whash, ehash));
}

std::unique_ptr<ShowMode>
ShowModeStandard::CreateShowMode(const std::string& which, const std::function<ShowModeInfo_t()>& valueGetter)
{
	auto whash = [valueGetter](){ return valueGetter()[kwhash]; };
	auto ehash = [valueGetter](){ return valueGetter()[kehash]; };
	auto bord1 = [valueGetter](){ return CC_coord{ Int2Coord(valueGetter()[kbord1_x]), Int2Coord(valueGetter()[kbord1_y]) }; };
	auto bord2 = [valueGetter](){ return CC_coord{ Int2Coord(valueGetter()[kbord2_x]), Int2Coord(valueGetter()[kbord2_y]) }; };
	auto offset = [valueGetter](){ return CC_coord{ Int2Coord(-valueGetter()[koffset_x]), Int2Coord(-valueGetter()[koffset_y]) }; };
	auto size = [valueGetter](){ return CC_coord{ Int2Coord(valueGetter()[ksize_x]), Int2Coord(valueGetter()[ksize_y]) }; };
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
	unsigned char which_spr_yards = values[kwhich_spr_yards];
	CC_coord bord1, bord2;
	bord1.x = Int2Coord(values[kbord1_x]);
	bord1.y = Int2Coord(values[kbord1_y]);
	bord2.x = Int2Coord(values[kbord2_x]);
	bord2.y = Int2Coord(values[kbord2_y]);
	
	short mode_steps_x = values[kmode_steps_x];
	short mode_steps_y = values[kmode_steps_y];
	short mode_steps_w = values[kmode_steps_w];
	short mode_steps_h = values[kmode_steps_h];
	short eps_stage_x = values[keps_stage_x];
	short eps_stage_y = values[keps_stage_y];
	short eps_stage_w = values[keps_stage_w];
	short eps_stage_h = values[keps_stage_h];
	short eps_field_x = values[keps_field_x];
	short eps_field_y = values[keps_field_y];
	short eps_field_w = values[keps_field_w];
	short eps_field_h = values[keps_field_h];
	short eps_text_left = values[keps_text_left];
	short eps_text_right = values[keps_text_right];
	short eps_text_top = values[keps_text_top];
	short eps_text_bottom = values[keps_text_bottom];
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

std::unique_ptr<ShowMode>
ShowModeSprShow::CreateSpringShowMode(const std::string& which, const std::function<SpringShowModeInfo_t()>& valueGetter)
{
	auto which_spr_yards = [valueGetter](){ return valueGetter()[kwhich_spr_yards]; };
	auto bord1 = [valueGetter]() { return CC_coord{ Int2Coord(valueGetter()[kbord1_x]), Int2Coord(valueGetter()[kbord1_y]) }; };
	auto bord2 = [valueGetter]() { return CC_coord{ Int2Coord(valueGetter()[kbord2_x]), Int2Coord(valueGetter()[kbord2_y]) }; };
	
	auto mode_steps_x = [valueGetter]() { return valueGetter()[kmode_steps_x]; };
	auto mode_steps_y = [valueGetter]() { return valueGetter()[kmode_steps_y]; };
	auto mode_steps_w = [valueGetter]() { return valueGetter()[kmode_steps_w]; };
	auto mode_steps_h = [valueGetter]() { return valueGetter()[kmode_steps_h]; };
	auto eps_stage_x = [valueGetter]() { return valueGetter()[keps_stage_x]; };
	auto eps_stage_y = [valueGetter]() { return valueGetter()[keps_stage_y]; };
	auto eps_stage_w = [valueGetter]() { return valueGetter()[keps_stage_w]; };
	auto eps_stage_h = [valueGetter]() { return valueGetter()[keps_stage_h]; };
	auto eps_field_x = [valueGetter]() { return valueGetter()[keps_field_x]; };
	auto eps_field_y = [valueGetter]() { return valueGetter()[keps_field_y]; };
	auto eps_field_w = [valueGetter]() { return valueGetter()[keps_field_w]; };
	auto eps_field_h = [valueGetter]() { return valueGetter()[keps_field_h]; };
	auto eps_text_left = [valueGetter]() { return valueGetter()[keps_text_left]; };
	auto eps_text_right = [valueGetter]() { return valueGetter()[keps_text_right]; };
	auto eps_text_top = [valueGetter]() { return valueGetter()[keps_text_top]; };
	auto eps_text_bottom = [valueGetter]() { return valueGetter()[keps_text_bottom]; };
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

