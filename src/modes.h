/*
 * modes.h
 * Definitions for the show mode classes
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

#ifndef _MODES_H_
#define _MODES_H_

#include "cc_coord.h"

#include <wx/wx.h>
#include <list>

enum SHOW_TYPE { SHOW_STANDARD, SHOW_SPRINGSHOW };

#define SPR_YARD_LEFT 8
#define SPR_YARD_RIGHT 4
#define SPR_YARD_ABOVE 2
#define SPR_YARD_BELOW 1

class CC_coord;

class ShowMode
{
public:
	ShowMode(const wxString& nam, const CC_coord& siz, const CC_coord& off,
		const CC_coord& bord1, const CC_coord& bord2);
	ShowMode(const wxString& nam, const CC_coord& siz, const CC_coord& bord1, const CC_coord& bord2);
	virtual ~ShowMode();

	virtual SHOW_TYPE GetType() const = 0;
	virtual void Draw(wxDC *dc) const = 0;
	virtual void DrawAnim(wxDC *dc) const = 0;
	inline const CC_coord& Offset() const { return offset; };
	inline CC_coord FieldOffset() const { return -(offset-border1); }
	inline const CC_coord& Size() const { return size; };
	inline CC_coord FieldSize() const { return size-border1-border2; }
	inline CC_coord MinPosition() const { return -offset; }
	inline CC_coord MaxPosition() const { return size-offset; }
	inline const wxString& GetName() const { return name; };
	CC_coord ClipPosition(const CC_coord& pos) const;

protected:
	CC_coord offset, size;
	CC_coord border1, border2;
private:
	wxString name;
};

class ShowModeStandard : public ShowMode
{
public:
	ShowModeStandard(const wxString& nam, CC_coord bord1, CC_coord bord2,
		unsigned short whash, unsigned short ehash);
	ShowModeStandard(const wxString& nam, CC_coord bord1, CC_coord bord2,
		CC_coord siz, CC_coord off,
		unsigned short whash, unsigned short ehash);
	virtual ~ShowModeStandard();

	virtual SHOW_TYPE GetType() const;
	virtual void Draw(wxDC *dc) const;
	virtual void DrawAnim(wxDC *dc) const;
	inline unsigned short HashW() const { return hashw; }
	inline unsigned short HashE() const { return hashe; }

private:
	unsigned short hashw, hashe;
};

class ShowModeSprShow : public ShowMode
{
public:
// Look at calchart.cfg for description of arguments
	ShowModeSprShow(const wxString& nam, CC_coord bord1, CC_coord bord2,
		unsigned char which,
		short stps_x, short stps_y,
		short stps_w, short stps_h,
		short stg_x, short stg_y,
		short stg_w, short stg_h,
		short fld_x, short fld_y,
		short fld_w, short fld_h,
		short txt_l, short txt_r,
		short txt_tp, short txt_bm);
	virtual ~ShowModeSprShow();

	virtual SHOW_TYPE GetType() const;
	virtual void Draw(wxDC *dc) const;
	virtual void DrawAnim(wxDC *dc) const;
	inline unsigned char WhichYards() const { return which_yards; }
	inline short StageX() const { return stage_x; }
	inline short StageY() const { return stage_y; }
	inline short StageW() const { return stage_w; }
	inline short StageH() const { return stage_h; }
	inline short FieldX() const { return field_x; }
	inline short FieldY() const { return field_y; }
	inline short FieldW() const { return field_w; }
	inline short FieldH() const { return field_h; }
	inline short StepsX() const { return steps_x; }
	inline short StepsY() const { return steps_y; }
	inline short StepsW() const { return steps_w; }
	inline short StepsH() const { return steps_h; }
	inline short TextLeft() const { return text_left; }
	inline short TextRight() const { return text_right; }
	inline short TextTop() const { return text_top; }
	inline short TextBottom() const { return text_bottom; }

private:
	unsigned char which_yards;
	short stage_x, stage_y, stage_w, stage_h;
	short field_x, field_y, field_w, field_h;
	short steps_x, steps_y, steps_w, steps_h;
	short text_left, text_right, text_top, text_bottom;
};

class ShowModeList
{
public:
	typedef std::list<ShowMode*> Container;
	typedef Container::iterator Iter;
	typedef Container::const_iterator CIter;
	ShowModeList();
	~ShowModeList();

	void Add(ShowMode *mode) { list.push_back(mode); }
	ShowMode *Find (const wxString& name) const;
	bool Empty() const { return list.empty(); }
	CIter Begin() const { return list.begin(); }
	Iter Begin() { return list.begin(); }
	CIter End() const { return list.end(); }
	Iter End() { return list.end(); }

private:
	Container list;
};
#endif
