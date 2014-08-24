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
#include "confgr.h"

#include <wx/wx.h>
#include <list>
#include <algorithm>
#include <memory>

#define SPR_YARD_LEFT 8
#define SPR_YARD_RIGHT 4
#define SPR_YARD_ABOVE 2
#define SPR_YARD_BELOW 1

class ShowMode
{
public:
	typedef enum { SHOW_STANDARD, SHOW_SPRINGSHOW } ShowType;

	// to find a specific Show:
	static std::unique_ptr<ShowMode> GetMode(const wxString& which);
	virtual ~ShowMode();

public:

	virtual ShowType GetType() const = 0;
	inline const CC_coord& Offset() const { return mOffset; };
	inline CC_coord FieldOffset() const { return -(mOffset-mBorder1); }
	inline const CC_coord& Size() const { return mSize; };
	inline CC_coord FieldSize() const { return mSize-mBorder1-mBorder2; }
	inline CC_coord MinPosition() const { return -mOffset; }
	inline CC_coord MaxPosition() const { return mSize-mOffset; }
	inline const wxString& GetName() const { return mName; };
	CC_coord ClipPosition(const CC_coord& pos) const;

public:
	typedef enum
	{
		kFieldView,
		kAnimation,
		kPrinting,
		kOmniView
	} HowToDraw;

//public:
//	void Draw(wxDC& dc, const CalChartConfiguration& config) const { DrawHelper(dc, config, kFieldView); }
//	void DrawAnim(wxDC& dc, const CalChartConfiguration& config) const { DrawHelper(dc, config, kAnimation); }
//	void DrawOmni(wxDC& dc, const CalChartConfiguration& config) const { DrawHelper(dc, config, kOmniView); }
	void DrawMode(wxDC& dc, const CalChartConfiguration& config, HowToDraw howToDraw) const;

	wxImage GetOmniLinesImage(const CalChartConfiguration& config) const;

protected:
	// Users shouldn't create show modes, it should be done through derived classes
	ShowMode(const wxString& name,
			 const CC_coord& size,
			 const CC_coord& offset,
			 const CC_coord& border1,
			 const CC_coord& border2);
	virtual void DrawHelper(wxDC& dc, const CalChartConfiguration& config, HowToDraw howToDraw) const = 0;

	const CC_coord mOffset;
	const CC_coord mSize;
	const CC_coord mBorder1;
	const CC_coord mBorder2;
private:
	wxString mName;
};

class ShowModeStandard : public ShowMode
{
public:
	static std::unique_ptr<ShowMode> CreateShowMode(const wxString& which, std::vector<long> values);
	static std::unique_ptr<ShowMode> CreateShowMode(const wxString& name,
													CC_coord size,
													CC_coord offset,
													CC_coord border1,
													CC_coord border2,
													unsigned short whash,
													unsigned short ehash);

private:
	ShowModeStandard(const wxString& name,
					 CC_coord size,
					 CC_coord offset,
					 CC_coord border1,
					 CC_coord border2,
					 unsigned short whash,
					 unsigned short ehash);
public:
	virtual ~ShowModeStandard();

	virtual ShowType GetType() const;
	inline unsigned short HashW() const { return mHashW; }
	inline unsigned short HashE() const { return mHashE; }

protected:
	virtual void DrawHelper(wxDC& dc, const CalChartConfiguration& config, HowToDraw howToDraw) const;

private:
	const unsigned short mHashW, mHashE;
};

class ShowModeSprShow : public ShowMode
{
public:
	static std::unique_ptr<ShowMode> CreateSpringShowMode(const wxString& which, std::vector<long> values);

private:
// Look at calchart.cfg for description of arguments
	ShowModeSprShow(const wxString& name, CC_coord border1, CC_coord border2,
		unsigned char which,
		short stps_x, short stps_y,
		short stps_w, short stps_h,
		short stg_x, short stg_y,
		short stg_w, short stg_h,
		short fld_x, short fld_y,
		short fld_w, short fld_h,
		short txt_l, short txt_r,
		short txt_tp, short txt_bm);
public:
	virtual ~ShowModeSprShow();

	virtual ShowType GetType() const;
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

protected:
	virtual void DrawHelper(wxDC& dc, const CalChartConfiguration& config, HowToDraw howToDraw) const;
	
private:
	unsigned char which_yards;
	short stage_x, stage_y, stage_w, stage_h;
	short field_x, field_y, field_w, field_h;
	short steps_x, steps_y, steps_w, steps_h;
	short text_left, text_right, text_top, text_bottom;
};

#endif
