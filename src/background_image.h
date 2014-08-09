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

#include <precomp.h>

#include <wx/dc.h>
#include <wx/event.h>
#include <memory>

class BackgroundImage
{
public:
	BackgroundImage(const wxImage& image, const wxCoord& x, const wxCoord& y);

	void OnMouseLeftDown(const wxMouseEvent& event, const wxDC& dc);
	void OnMouseLeftUp(const wxMouseEvent& event, const wxDC& dc);
	void OnMouseMove(const wxMouseEvent& event, const wxDC& dc);
	void OnPaint(wxDC& dc);

	bool DoingPictureAdjustment() { return mDoBackgroundPicAdjust; }
	void DoPictureAdjustment(bool enable) { mDoBackgroundPicAdjust = enable; }

private:
	static const long kCircleSize = 6;

	wxImage mImage;
	wxBitmap mBitmap;
	wxCoord mBitmapX, mBitmapY;
	
	bool mDoBackgroundPicAdjust;
	// what type of background adjustments could we do
	typedef enum {
		kUpperLeft = 0, kUpper, kUpperRight,
		kLeft, kMove, kRight,
		kLowerLeft, kLower, kLowerRight,
		kLast,
	} eBackgroundAdjustType;
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
	std::shared_ptr<CalculateScaleAndMove> mScaleAndMove;
};

#endif // _BACKGROUND_IMAGE_H_
