/*
 * background_image.h
 * Header for background image
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

#ifndef _BACKGROUND_IMAGE_H_
#define _BACKGROUND_IMAGE_H_

#include <wx/dc.h>
#include <wx/event.h>
#include <boost/shared_ptr.hpp>

/**
 * A field image.
 */
class BackgroundImage
{
public:
	/**
	 * Makes an image.
	 * @param image The image.
	 * @param x The x position at which to draw the image.
	 * @param y The y position at which to draw the image.
	 */
	BackgroundImage(const wxImage& image, const wxCoord& x, const wxCoord& y);

	/**
	 * Called when the left mouse button is clicked.
	 * @param event Information about the mouse.
	 * @param dc The device context to use when drawing the
	 * effects of the action.
	 */
	void OnMouseLeftDown(const wxMouseEvent& event, const wxDC& dc);
	/**
	 * Called when the left mouse button is released.
	 * @param event Information about the mouse.
	 * @param dc The device context to use when drawing the
	 * effects of the action.
	 */
	void OnMouseLeftUp(const wxMouseEvent& event, const wxDC& dc);
	/**
	 * Called when the left mouse button is moved.
	 * @param event Information about the mouse.
	 * @param dc The device context to use when drawing the
	 * effects of the action.
	 */
	void OnMouseMove(const wxMouseEvent& event, const wxDC& dc);
	/**
	 * Called when the image should be drawn.
	 * @param dc The device context that should be used to draw the image.
	 */
	void OnPaint(wxDC& dc);

	/**
	 * Returns true if the image is being adjusted.
	 * @return True if the image is being adjusted; false otherwise.
	 */
	bool DoingPictureAdjustment() { return mDoBackgroundPicAdjust; }
	/**
	 * Sets whether or not the image is being adjusted by the mouse.
	 * @param enable True if the image is being adjusted by the mouse;
	 * false otherwise.
	 */
	void DoPictureAdjustment(bool enable) { mDoBackgroundPicAdjust = enable; }

private:
	/**
	 * The size of the circles that are drawn at the corners of the image
	 * when it is being adjusted.
	 */
	static const long kCircleSize = 6;

	/**
	 * The source image.
	 */
	wxImage mImage;
	/**
	 * The bitmap to draw.
	 */
	wxBitmap mBitmap;
	/**
	 * The x origin of the bitmap.
	 */
	wxCoord mBitmapX;
	/**
	 * The y origin of the bitmap.
	 */
	wxCoord mBitmapY;
	
	/**
	 * True if the image is being adjusted by the mouse; false otherwise.
	 */
	bool mDoBackgroundPicAdjust;
	// what type of background adjustments could we do
	typedef enum {
		kUpperLeft = 0, kUpper, kUpperRight,
		kLeft, kMove, kRight,
		kLowerLeft, kLower, kLowerRight,
		kLast,
	} eBackgroundAdjustType;
	/**
	 * Records which kind of adjustment is currently being made to the image.
	 */
	eBackgroundAdjustType mBackgroundAdjustType;

	class CalculateScaleAndMove
	{
	public:
		CalculateScaleAndMove(const wxPoint& startClick, wxCoord x, wxCoord y, long width, long height, eBackgroundAdjustType adjustType);
		void operator()(long x, long y, wxCoord &topX, wxCoord &topY, wxCoord &width, wxCoord &height);
		wxPoint mStartClick;
		wxCoord mLeft, mTop;
		wxCoord mRight, mBottom;
		float mAspectRatio;
		eBackgroundAdjustType mAdjustType;
	};
	boost::shared_ptr<CalculateScaleAndMove> mScaleAndMove;
};

#endif // _BACKGROUND_IMAGE_H_
