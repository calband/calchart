/*
 * field_canvas.h
 * Canvas for the Field window
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#ifndef _FIELD_CANVAS_H_
#define _FIELD_CANVAS_H_

#include "basic_ui.h"
#include "cc_types.h"

#include <wx/docview.h>

#include <vector>
#include <memory>

class FieldView;
class FieldFrame;
class CC_shape;
class BackgroundImage;
class CC_coord;

// Field Canvas controls how to paint and the first line control of user input
class FieldCanvas : public CtrlScrollCanvas
{
public:
// Basic functions
	FieldCanvas(wxView *view, FieldFrame *frame, float def_zoom);
	virtual ~FieldCanvas(void);
	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	virtual void OnMouseLeftDown(wxMouseEvent& event);
	virtual void OnMouseLeftUp(wxMouseEvent& event);
	virtual void OnMouseLeftDoubleClick(wxMouseEvent& event);
	virtual void OnMouseRightDown(wxMouseEvent& event);
	virtual void OnMouseMove(wxMouseEvent& event);
	void OnChar(wxKeyEvent& event);

// Misc show functions
	float ZoomToFitFactor() const;
	virtual void SetZoom(float factor);

	// return true on success
	bool SetBackgroundImage(const wxImage& image);
	void AdjustBackgroundImage(bool enable);
	void RemoveBackgroundImage();
	
	CC_DRAG_TYPES GetCurrentLasso() const;
	void SetCurrentLasso(CC_DRAG_TYPES lasso);
	CC_MOVE_MODES GetCurrentMove() const;
	// implies a call to EndDrag()
	void SetCurrentMove(CC_MOVE_MODES move);
	
private:
	// Variables
	FieldFrame *mFrame;
	FieldView* mView;
	CC_DRAG_TYPES curr_lasso;
	CC_MOVE_MODES curr_move;

	void ClearShapes();

	void BeginDrag(CC_DRAG_TYPES type, const CC_coord& start);
	void AddDrag(CC_DRAG_TYPES type, std::unique_ptr<CC_shape> shape);
	void MoveDrag(const CC_coord& end);
	void EndDrag();
	
	CC_DRAG_TYPES drag;
	typedef std::vector<std::shared_ptr<CC_shape>> ShapeList;
	ShapeList shape_list;
	std::shared_ptr<CC_shape> curr_shape;

	// Background Picture
	void PaintBackground(wxDC& dc);
	std::unique_ptr<BackgroundImage> mBackgroundImage;
	
	DECLARE_EVENT_TABLE()
};

#endif // _FIELD_CANVAS_H_
