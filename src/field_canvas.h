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
#include <boost/shared_ptr.hpp>

class FieldView;
class FieldFrame;
class CC_shape;
class BackgroundImage;
class CC_coord;

/**
 * The cavas on which the active stuntsheet is drawn in the field frame.
 */
class FieldCanvas : public CtrlScrollCanvas
{
public:
// Basic functions
	/**
	 * Makes the canvas.
	 * @param view The view that is drawn to the canvas
	 * (should be a field view).
	 * @param frame The frame of which this canvas is a part.
	 * @param def_zoom The initial zoom factor through which to view the
	 * field.
	 */
	FieldCanvas(wxView *view, FieldFrame *frame, float def_zoom);
	/**
	 * Cleanup.
	 */
	virtual ~FieldCanvas(void);
	/**
	 * Called when the canvas should redraw its contents.
	 * @param event Unused.
	 */
	void OnPaint(wxPaintEvent& event);
	/**
	 * Erases the background of the canvas.
	 * @param event Information about erasing?
	 */
	void OnEraseBackground(wxEraseEvent& event);
	/**
	 * Called when the left mouse button is down.
	 * @param event Contains information regarding the mouse.
	 */
	virtual void OnMouseLeftDown(wxMouseEvent& event);
	/**
	 * Called when the left mouse button is released.
	 * @param event Contains information regarding the mouse.
	 */
	virtual void OnMouseLeftUp(wxMouseEvent& event);
	/**
	 * Called when the left mouse button is double-clicked.
	 * @param event Contains information about the mouse.
	 */
	virtual void OnMouseLeftDoubleClick(wxMouseEvent& event);
	/**
	 * Called when the right mouse button is down.
	 * @param event Contains information about the mouse.
	 */
	virtual void OnMouseRightDown(wxMouseEvent& event);
	/**
	 * Called when the mouse is moved.
	 * @param event Information about the mouse.
	 */
	virtual void OnMouseMove(wxMouseEvent& event);
	/**
	 * Called when the user presses down a character on the keyboard.
	 * @param event Information about the key that was pressed.
	 */
	void OnChar(wxKeyEvent& event);

// Misc show functions
	/**
	 * Finds the zoom factor that will fit the length of the field.
	 * @return The zoom factor that will fit the field.
	 */
	float ZoomToFitFactor() const;
	/**
	 * Sets the zoom factor.
	 * @param factor The new zoom factor to use.
	 */
	virtual void SetZoom(float factor);

	/**
	 * Sets the field image.
	 * @param image The new field image.
	 * @return True if successful; false otherwise.
	 */
	bool SetBackgroundImage(const wxImage& image);
	/**
	 * Sets whether or not the background image can be adjusted.
	 * @param enable True if the background image can be adjusted;
	 * False otherwise.
	 */
	void AdjustBackgroundImage(bool enable);
	/**
	 * Removes the current field image.
	 */
	void RemoveBackgroundImage();
	
	/**
	 * Returns the type of lasso that will be used next time the
	 * user begins drawing a lasso.
	 * @return The type of lasso that will be built next time a
	 * lasso is started.
	 */
	CC_DRAG_TYPES GetCurrentLasso() const;
	/**
	 * Sets the type of lasso that will be used next time the user
	 * begins drawing a lasso.
	 * @param lasso The type of lasso to build next time a lasso is
	 * started.
	 */
	void SetCurrentLasso(CC_DRAG_TYPES lasso);
	/**
	 * Returns the mode that is being used to define how points
	 * will be translated.
	 * @return The mode that is currently being used to describe
	 * how to move points.
	 */
	CC_MOVE_MODES GetCurrentMove() const;
	// implies a call to EndDrag()
	/**
	 * Sets the mode that will define how points will be translated.
	 * @param move The mode defining how points will be translated.
	 */
	void SetCurrentMove(CC_MOVE_MODES move);
	
private:
	// Variables
	/**
	 * The field frame of which this canvas is a part.
	 */
	FieldFrame *mFrame;
	/**
	 * The view that will be drawn to the canvas.
	 */
	FieldView* mView;
	/**
	 * The type of lasso that should be created next time the user
	 * creates a new lasso.
	 */
	CC_DRAG_TYPES curr_lasso;
	/**
	 * The action that should be taken when the user attempts to move a
	 * selection of points.
	 */
	CC_MOVE_MODES curr_move;

	/**
	 * Clears all shapes that are being used by the canvas, including both
	 * those that are built and those that have not been completed yet.
	 */
	void ClearShapes();

	/**
	 * Given that the mouse has just started dragging at a new
	 * start position, handle the event.
	 * @param type The drag mode which should be used to handle the
	 * drag event.
	 * @param start The point where the mouse started dragging.
	 */
	void BeginDrag(CC_DRAG_TYPES type, const CC_coord& start);
	/**
	 * Sets the shape that should handle drag events.
	 * @param type The type of drag mode that will be used.
	 * @param shape The shape that should recieve drag events.
	 */
	void AddDrag(CC_DRAG_TYPES type, boost::shared_ptr<CC_shape> shape);
	/**
	 * Given that the mouse was dragged to a new point, process the
	 * event according to the current drag mode.
	 * @param end The point where the mouse was dragged to.
	 */
	void MoveDrag(const CC_coord& end);
	/**
	 * Finalizes a drag operation, assuming that its effects have
	 * already been applied. This prepares the canvas for a new
	 * drag.
	 */
	void EndDrag();
	
	/**
	 * The type of lasso that is currently being built by the user. The type
	 * of lasso being constructed defines how the canvas should handle a mouse
	 * drag event.
	 */
	CC_DRAG_TYPES drag;
	typedef std::vector<boost::shared_ptr<CC_shape> > ShapeList;
	/**
	 * A list of all shapes, complete and incomplete, that should be drawn
	 * to the canvas.
	 */
	ShapeList shape_list;
	/**
	 * The shape that is currently being built by the user.
	 */
	boost::shared_ptr<CC_shape> curr_shape;

	/**
	 * Draws the background.
	 * @param dc The device context that will be used to draw the background.
	 */
	void PaintBackground(wxDC& dc);
	/**
	 * The current field image.
	 */
	boost::shared_ptr<BackgroundImage> mBackgroundImage;
	
	DECLARE_EVENT_TABLE()
};

#endif // _FIELD_CANVAS_H_
