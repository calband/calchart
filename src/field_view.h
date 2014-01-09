/*
 * field_view.h
 * Header for field view
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

#ifndef _FIELD_UI_H_
#define _FIELD_UI_H_

#include "calchartdoc.h"
#include "CC_coord.h"

#include <wx/docview.h>

#include <boost/shared_ptr.hpp>

class FieldFrame;
class AnimationFrame;
class Animation;
class Matrix;

/**
 * This is a view of the CalChart doc. When drawn, it prints an overhead view
 * of the marchers on the field. You can edit the show through this view.
 */
class FieldView : public wxView
{
public:
	/**
	 * Makes the view.
	 */
    FieldView();

	/**
	 * Performs cleanup.
	 */
    ~FieldView();

	/**
	 * Called when the view is created.
	 * @param doc The document (CalChartDoc) which this view is associated
	 * with.
	 * @param flags Unused.
	 * @return True if the view was created successfully; false if the view
	 * is invalid and should be deleted.
	 */
    bool OnCreate(wxDocument *doc, long flags);

	/**
	 * Called when the view is drawn.
	 * @param dc The Device Context, which is used to draw to
	 * the portion of the screen to which this view is assigned.
	 */
    void OnDraw(wxDC *dc);

	/**
	 * Called when the view should be updated.
	 * @param sender Unused.
	 * @param hint The message sent with the update, indicating the
	 * reason for the update.
	 */
    void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);

	/**
	 * Called when the view is closed.
	 * @param deleteWindow True if the window associated with the view should
	 * be deleted; false otherwise.
	 * @return TODO
	 */
    bool OnClose(bool deleteWindow = true);

	/**
	 * Creates a setup wizard and uses it to setup a new show.
	 * @param show The new show to setup.
	 */
	void OnWizardSetup(CalChartDoc& show);

	///// Modify the show /////
	/**
	 * Moves the selected points by a vector.
	 * @param pos The amount by which to move all selected points.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoTranslatePoints(const CC_coord& pos);
	/**
	 * Transforms all selected points by a matrix.
	 * @param transmat The transformation matrix to apply to the selected
	 * points.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoTransformPoints(const Matrix& transmat);
	/**
	 * Moves the selected points into a line.
	 * @param start The first point in the line.
	 * @param second The position of the second point in the line.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoMovePointsInLine(const CC_coord& start, const CC_coord& second);
	/**
	 * Resets the reference point of each selected point that is associated with
	 * the currently selected reference group to the same position as the point
	 * itself.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoResetReferencePoint();
	/**
	 * Sets the symbol of the selected points.
	 * @param sym The new symbol to assign to the selected points.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoSetPointsSymbol(SYMBOL_TYPE sym);
	/**
	 * Sets the description of the show.
	 * @param descr The new description for the show.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoSetDescription(const wxString& descr);
	/**
	 * Sets the show mode for the current show.
	 * @param mode The name of the new show mode to use.
	 * @return True if the operation is successful; false otherwise.
	 */
	void DoSetMode(const wxString& mode);
	/**
	 * Sets information regarding the number of points in the show, how
	 * to initially arrange them, and what their labels are.
	 * @param numPoints The number of points to have in the show.
	 * @param numColumns The number of rows to initially organize the
	 * points in.
	 * @param labels The labels for the points.
	 * @return True if the operation is successful; false otherwise.
	 */
	void DoSetShowInfo(unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels);
	/**
	 * Sets the title of the current stuntsheet.
	 * @param descr The new title for the current stuntsheet.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoSetSheetTitle(const wxString& descr);
	/**
	 * Sets the duration of the current stuntsheet, in beats.
	 * @param beats The duration of the current stuntsheet, in beats.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoSetSheetBeats(unsigned short beats);
	/**
	 * Sets the position of the point label for the selected points,
	 * relative to the points.
	 * @param right True if the label should be positioned to the right
	 * of the points, false if the label should be positioned on the left.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoSetPointsLabel(bool right);
	/**
	 * Toggles the position of the point label for the selected points
	 * relative to the points between the right and left.
	 * @return True if the operation is successful; false otherwise.
	 */
	bool DoSetPointsLabelFlip();
	/**
	 * Inserts stunt sheets into the show.
	 * @param sht The sheets to add to the show.
	 * @param where TODO before or after?
	 */
	bool DoInsertSheets(const CalChartDoc::CC_sheet_container_t& sht, unsigned where);
	/**
	 * Deletes a stunt sheet.
	 * @param where The index of the stunt sheet to delete.
	 */
	bool DoDeleteSheet(unsigned where);

	///// query show attributes /////
	/**
	 * Returns the index of the point located at a position.
	 * @param pos The position to check.
	 * @return The index of the point located at a position. If no point exists
	 * at the specified location, -1 will be returned.
	 */
	int FindPoint(CC_coord pos) const;
	/**
	 * Returns the position of the point having the given index.
	 * @param which The index of the point.
	 * @return The position of the point having index [which].
	 */
	CC_coord PointPosition(int which) const;
	/**
	 * Returns the index of the active stuntsheet.
	 * @return The index of the active stuntsheet.
	 */
	unsigned GetCurrentSheetNum() const { return mShow->GetCurrentSheetNum(); }
	/**
	 * Returns the total number of stuntsheets in the show.
	 * @return The total number of stuntsheets in the show.
	 */
	unsigned short GetNumSheets() const { return mShow->GetNumSheets(); }

	/**
	 * Returns the positional offset of the field (defined by the show mode).
	 * @return The positional offset of the field.
	 */
	CC_coord GetShowFieldOffset() const;
	/**
	 * Returns the size of the field (defined by the show mode).
	 * @return The size of the field.
	 */
	CC_coord GetShowFieldSize() const;

	///// Change show attributes /////
	/**
	 * Jumps to a particular stunt sheet.
	 * @param which The index of the stunt sheet to jump to.
	 */
	void GoToSheet(size_t which);
	/**
	 * Jumps to the next stunt sheet.
	 */
	void GoToNextSheet();
	/**
	 * Jumps to the previous stunt sheet.
	 */
	void GoToPrevSheet();

	/**
	 * Sets the active reference point for the points in the show
	 * to a particular index.
	 * @param which The index of the reference point that should be active
	 * for points in the show. Zero if the actual location of the points
	 * should be active, instead of a reference point.
	 */
	void SetReferencePoint(unsigned which);

	///// Select /////
	/**
	 * Deselects all points.
	 */
	void UnselectAll() { mShow->UnselectAll(); }
	/**
	 * Selects points.
	 * @param sl The points to select.
	 */
	void AddToSelection(const SelectionList& sl);
	/**
	 * Toggles whether or not points are selected.
	 * @param sl The points to toggle between selected and unselected.
	 */
	void ToggleSelection(const SelectionList& sl);
	/**
	 * Adjusts whether or not the points within a lasso are selected.
	 * @param lasso The lasso which the points are within.
	 * @param toggleSelected True if the points in the lasso should be toggled
	 * between selected and unselected; false if the points in the lasso should
	 * be selected.
	 */
	void SelectWithLasso(const CC_lasso *lasso, bool toggleSelected);
	/**
	 * Adjusts whether or not the points within a rectangle are selected.
	 * @param c1 One corner of the rectangle.
	 * @param c2 The opposite corner of the rectangle.
	 * @param toggleSelected True if the points in the rectangle should be toggled
	 * between selected and unselected; false if the points in the rectangle should
	 * be selected.
	 */
	void SelectPointsInRect(const CC_coord& c1, const CC_coord& c2, bool toggleSelected);

	///// Drawing marcher's paths /////
	// call this when we need to generate the marcher's paths.
	/**
	 * Called when the "Draw Paths" button is pressed.
	 * @param enable True if the view should draw paths, false otherwise.
	 */
	void OnEnableDrawPaths(bool enable);

private:
#if defined(BUILD_FOR_VIEWER) && (BUILD_FOR_VIEWER != 0)
	AnimationFrame *mFrame;
#else
	/**
	 * The field frame to which the this view is attached.
	 */
	FieldFrame *mFrame;
#endif
	
	/**
	 * Draws the paths of the current selection of points on the
	 * given stunt sheet.
	 * @param dc The device context that is used to draw the paths.
	 * @param sheet The stuntsheet do draw paths for.
	 */
	void DrawPaths(wxDC& dc, const CC_sheet& sheet);
	/**
	 * Solves for the paths of all points.
	 * This makes a new Animation object that handles
	 * the paths for the points.
	 */
	void GeneratePaths();
	/**
	 * The animation of the show. This tracks how points
	 * transition between stuntsheets, and thus can be used
	 * in drawing paths.
	 */
	boost::shared_ptr<Animation> mAnimation;
	/**
	 * True if the view should draw paths; false otherwise.
	 */
	bool mDrawPaths;
	
private:
	/**
	 * The document which this view is associated with.
	 */
	CalChartDoc* mShow;
	/**
	 * The index of the active reference point for the dots
	 * in the show.
	 */
	unsigned mCurrentReferencePoint;

    DECLARE_DYNAMIC_CLASS(FieldView)
};

#endif
