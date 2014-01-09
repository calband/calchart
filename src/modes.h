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
#include <algorithm>
#include <boost/shared_ptr.hpp>

#define SPR_YARD_LEFT 8
#define SPR_YARD_RIGHT 4
#define SPR_YARD_ABOVE 2
#define SPR_YARD_BELOW 1

/**
 * The type of show being charted.
 * Mostly, different show modes are drawn differently on the field frame, as
 * to provide a background that is most suitable for editing a particular
 * kind of show.
 */
class ShowMode
{
public:
	/**
	 * The show types.
	 */
	typedef enum {
		/**
		 * A standard field show.
		 */
		SHOW_STANDARD,
		/**
		 * Springshow.
		 */
		SHOW_SPRINGSHOW
	} ShowType;
	
	/**
	 * Makes the showmode.
	 * @param name Name of the mode.
	 * @param size The size of the region where the show takes place
	 * (that is, the size of the show region).
	 * @param offset The offset of the region where the show takes place
	 * (that is, the offset of the show region).
	 * @param border1 Provides the width and height of the border in the
	 * lower left corner of the show region.
	 * @param border2 Provides the width and height of the border in the
	 * upper right corner of the show region.
	 */
	ShowMode(const wxString& name,
			 const CC_coord& size,
			 const CC_coord& offset,
			 const CC_coord& border1,
			 const CC_coord& border2);
	ShowMode(const wxString& name,
			 const CC_coord& size,
			 const CC_coord& border1,
			 const CC_coord& border2);
	virtual ~ShowMode();

	/**
	 * Returns the type of show that this show mode represents.
	 * @return The type of show represented by this show mode.
	 */
	virtual ShowType GetType() const = 0;
	/**
	 * Returns the offset of the region where the show takes place for this
	 * show mode (the offset of the show region).
	 * @return Offset of the region where the show takes place.
	 */
	inline const CC_coord& Offset() const { return mOffset; };
	/**
	 * Returns the offset of the region where the performers can actually
	 * stand for this show mode (the offset of the field region).
	 * @return Offset for the region where the dots can stand.
	 */
	inline CC_coord FieldOffset() const { return -(mOffset-mBorder1); }
	/**
	 * Returns the size of the region where the show takes place for this
	 * show mode (the offset of the show region).
	 * @return The size of the region where the show takes place.
	 */
	inline const CC_coord& Size() const { return mSize; };
	/**
	 * The size of the region where performers can actually stand for this
	 * show mode (the offset of the field region).
	 * @return The size of the region where dots can stand.
	 */
	inline CC_coord FieldSize() const { return mSize-mBorder1-mBorder2; }
	/**
	 * Returns the minimum position that a coordinate can have while remaining
	 * within the show mode's local coordinate system (which is centered about
	 * the center of the show region).
	 * @return The minimum position that a coordinate can have in this show
	 * mode.
	 */
	inline CC_coord MinPosition() const { return -mOffset; }
	/**
	 * Returns the maximum position that a coordinate can have while remaining
	 * within the show mode's local coordinate system (which is centered about
	 * the center of the show region).
	 * @return The maximum position that a coordinate can have in this show
	 * mode.
	 */
	inline CC_coord MaxPosition() const { return mSize-mOffset; }
	/**
	 * Returns the name of the show mode.
	 * @return The name of the show mode.
	 */
	inline const wxString& GetName() const { return mName; };
	/**
	 * Clips a position so that it lies within the bounds of the show
	 * region.
	 * @param pos The position to clip.
	 * @return The clipped version of the input position, which fits
	 * in the bounds of the shor region.
	 */
	CC_coord ClipPosition(const CC_coord& pos) const;

protected:
	/**
	 * The different targets to which the
	 * the show mode can be drawn.
	 */
	typedef enum
	{
		/**
		 * The field frame.
		 */
		kFieldView,
		/**
		 * The animation frame.
		 */
		kAnimation,
		/**
		 * Omniview. 
		 */
		kOmniView
	} HowToDraw;

public:
	/**
	 * Draws the show mode as it would appear in the field frame
	 * (it will be drawn to the field frame through the field view).
	 * @param dc The device context, used to draw the show mode.
	 */
	void Draw(wxDC& dc) const { DrawHelper(dc, kFieldView); }
	/**
	* Draws the show mode as it would appear in the animation frame.
	* @param dc The device context, used to draw the show mode.
	*/
	void DrawAnim(wxDC& dc) const { DrawHelper(dc, kAnimation); }
	/**
	* Draws the show mode as it would appear in omniview.
	* @param dc The device context, used to draw the show mode.
	*/
	void DrawOmni(wxDC& dc) const { DrawHelper(dc, kOmniView); }
	/**
	 * Returns the image that will be used to texture the floor
	 * of the show region in OmniView. This is responsible
	 * for defining how the field lines should be drawn.
	 * @return The image that will be used to texture the floor of
	 * the show region in OmniView.
	 */
	wxImage GetOmniLinesImage() const;

protected:
	/**
	 * Draws the show mode as it would appear in the various frames.
	 * This is called by all of the other draw methods of this class.
	 * @param dc The device context, used to draw the show mode.
	 * @param HowToDraw The style in which the show mode should be
	 * drawn (e.g. the style in which the mode is drawn in the
	 * animation frame, or the style in which the mode is drawn in
	 * the field frame.
	 */
	virtual void DrawHelper(wxDC& dc, HowToDraw howToDraw) const = 0;

	/**
	 * The offset of the show region.
	 */
	CC_coord mOffset;
	/**
	 * The size of the show region.
	 */
	CC_coord mSize;
	/**
	 * The width and height of the lower left corner of the show region.
	 */
	CC_coord mBorder1;
	/**
	 * The width and height of the upper right corner of the show region.
	 */
	CC_coord mBorder2;
private:
	/**
	 * The name of the show mode.
	 */
	wxString mName;
};

/**
 * A standard show mode (represents a show that is marched on something that
 * can be represented by a football field).
 */
class ShowModeStandard : public ShowMode
{
public:
	/**
	 * Makes the show mode.
	 * @param name Name of the mode.
	 * @param size The size of the region where the show takes place
	 * (that is, the size of the show region).
	 * @param offset The offset of the region where the show takes place
	 * (that is, the offset of the show region).
	 * @param border1 Provides the width and height of the border in the
	 * lower left corner of the show region.
	 * @param border2 Provides the width and height of the border in the
	 * upper right corner of the show region.
	 * @param whash The y coordinate of the west hash of the field.
	 * @param ehash The y coordinate of the east hash of the field. 
	 */
	ShowModeStandard(const wxString& name,
					 CC_coord size,
					 CC_coord offset,
					 CC_coord border1,
					 CC_coord border2,
					 unsigned short whash,
					 unsigned short ehash);
	/**
	 * Cleanup.
	 */
	virtual ~ShowModeStandard();

	virtual ShowType GetType() const;
	inline unsigned short HashW() const { return mHashW; }
	inline unsigned short HashE() const { return mHashE; }

protected:
	virtual void DrawHelper(wxDC& dc, HowToDraw howToDraw) const;

private:
	/**
	 * The y coordinate of the west hash of the field.
	 */
	unsigned short mHashW;
	/**
	 * The y coordinate of the east hash of the field.
	 */
	unsigned short mHashE;
};

/**
 * A springshow show mode (represents a show that is marched on something that
 * can be represented by a stage).
 * TODO - Finish documentation
 */
class ShowModeSprShow : public ShowMode
{
public:
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
	virtual void DrawHelper(wxDC& dc, HowToDraw howToDraw) const;
	
private:
	unsigned char which_yards;
	short stage_x, stage_y, stage_w, stage_h;
	short field_x, field_y, field_w, field_h;
	short steps_x, steps_y, steps_w, steps_h;
	short text_left, text_right, text_top, text_bottom;
};

typedef std::list<boost::shared_ptr<ShowMode> > ShowModeList;

ShowMode*
ShowModeList_Find(const ShowModeList& showModes, const wxString& which);

#endif
