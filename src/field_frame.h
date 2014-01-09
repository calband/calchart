/*
 * field_frame.h
 * Frame for the field window
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

#ifndef _FIELD_FRAME_H_
#define _FIELD_FRAME_H_

#include "cc_types.h"

#include <wx/docview.h>

class CC_coord;
class wxComboBox;
class wxSlider;
class CC_show;

class FieldCanvas;
class FieldView;
class CalChartDoc;

/**
 * The main show editing frame.
 * This is the window that opens every time you open a saved/new show.
 */
class FieldFrame : public wxDocChildFrame
{
	typedef wxDocChildFrame super;
public:
	/**
	 * Makes the frame.
	 * @param doc The show document that this frame displays and edits.
	 * @param view The view of the document that this frame is displaying.
	 * @param frame The parent frame for this frame. If that frame is closed,
	 * this frame will be closed as well.
	 * @param pos The initial position of the frame.
	 * @param size the initial size of the frame.
	 */
	FieldFrame(wxDocument* doc, wxView* view, wxDocParentFrame *frame, const wxPoint& pos, const wxSize& size);
	/**
	 * Cleanup.
	 */
	~FieldFrame();

	/**
	 * Called when the "Append" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdAppend(wxCommandEvent& event);
	/**
	 * Called when the "Import Continuity" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdImportCont(wxCommandEvent& event);
	/**
	 * Called when the "Save" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdSave(wxCommandEvent& event);
	/**
	 * Called when the "Save As" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdSaveAs(wxCommandEvent& event);
	/**
	 * Called when the "Print" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdPrint(wxCommandEvent& event);
	/**
	 * Called when the "Print Preview" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdPrintPreview(wxCommandEvent& event);
	/**
	 * Called when the "Print to PS" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdLegacyPrint(wxCommandEvent& event);
	/**
	 * Called when the "Print to EPS" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdLegacyPrintEPS(wxCommandEvent& event);
	/**
	 * Called when the "Page Setup" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdPageSetup(wxCommandEvent& event);
	/**
	 * Called when the "Preferences" command is selected from the file menu.
	 * @param event Unused.
	 */
	void OnCmdPreferences(wxCommandEvent& event);
	/**
	  * Called when the "Close Window" command is selected from the file menu.
	  * @param event Unused.
	  */
	void OnCmdClose(wxCommandEvent& event);
	/**
	 * Called when the "Redo" command is selected from the edit menu.
	 * @param event Unused.
	 */
	void OnCmdRedo(wxCommandEvent& event);
	/**
	 * Called when the "Insert Sheet Before" command is selected from the
	 * edit menu.
	 * @param event Unused.
	 */
	void OnCmdInsertBefore(wxCommandEvent& event);
	/**
	 * Called when the "Insert Sheet After" command is selected from the
	 * edit menu.
	 * @param event Unused.
	 */
	void OnCmdInsertAfter(wxCommandEvent& event);
	/**
	 * Called when the "Delete Sheet" command is selected from the edit menu.
	 * @param event Unused.
	 */
	void OnCmdDelete(wxCommandEvent& event);
	/**
	 * Called when the "Relabel Sheets" command is selected from the edit menu.
	 * @param event Unused.
	 */
	void OnCmdRelabel(wxCommandEvent& event);
	/**
	 * Called when the "Edit Continuity" command is selected from the edit menu.
	 * @param event Unused.
	 */
	void OnCmdEditCont(wxCommandEvent& event);
	/**
	 * Called when the "Set Sheet Title" command is selected from the edit menu.
	 * @param event Unused.
	 */
	void OnCmdSetSheetTitle(wxCommandEvent& event);
	/**
	 * Called when the "Set Beats" command is selected from the edit menu.
	 * @param event Unused.
	 */
	void OnCmdSetBeats(wxCommandEvent& event);
	/**
	 * Called when the "Set Up Marchers" command is selected from the edit menu.
	 * @param event Unused.
	 */
	void OnCmdSetup(wxCommandEvent& event);
	/**
	 * Called when the "Set Show Description" command is selected from the
	 * edit menu.
	 * @param event Unused.
	*/
	void OnCmdSetDescription(wxCommandEvent& event);
	/**
	 * Called when the "Set Show Mode" command is selected from the edit menu.
	 * @param event Unused.
	 */
	void OnCmdSetMode(wxCommandEvent& event);
	/**
	 * Called when the "Point Selections" command is selected from the
	 * edit menu.
	 * @param event Unused.
	*/
	void OnCmdPoints(wxCommandEvent& event);
	/**
	 * Called when the "Open in Viewer" command is selected from the 
	 * CalChart Viewer menu.
	 * @param event Unused.
	 */
	void OnCmdAnimate(wxCommandEvent& event);
	/**
	 * Called when the "About CalChart" command is selected from the help menu.
	 * @param event Unused.
	 */
	void OnCmdAbout(wxCommandEvent& event);
	/**
	 * Called when the "Help on CalChart" command is selected from the
	 * help menu.
	 * @param event Unused.
	 */
	void OnCmdHelp(wxCommandEvent& event);

	/**
	 * Called when the button that jumps to the previous stuntsheet is pressed.
	 * @param event Unused.
	 */
	void OnCmd_prev_ss(wxCommandEvent& event);
	/**
	 * Called when the button that jumps to the next stuntsheet is pressed.
	 * @param event Unused.
	 */
	void OnCmd_next_ss(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the tool that
	 * selects points within the drawn rectangle.
	 * @param event Unused.
	 */
	void OnCmd_box(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the tool that
	 * selects points within the drawn polygon.
	 * @param event Unused.
	 */
	void OnCmd_poly(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the tool that
	 * selects points within the drawn lasso.
	 * @param event Unused.
	 */
	void OnCmd_lasso(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the translate
	 * points tool.
	 * @param event Unused.
	 */
	void OnCmd_move(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the tool that
	 * moves points into a line.
	 * @param event Unused.
	 */
	void OnCmd_line(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the rotate
	 * block tool.
	 * @param event Unused.
	 */
	void OnCmd_rot(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the shear
	 * block tool.
	 * @param event Unused.
	 */
	void OnCmd_shear(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the reflect
	 * block tool.
	 * @param event Unused.
	 */
	void OnCmd_reflect(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the resize
	 * block tool.
	 * @param event Unused.
	 */
	void OnCmd_size(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that activates the genius
	 * move tool.
	 * @param event Unused.
	 */
	void OnCmd_genius(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that places the dot labels
	 * to the left of the dots.
	 * @param event Unused.
	 */
	void OnCmd_label_left(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that places the dot labels
	 * to the right of the dots.
	 * @param event Unused.
	 */
	void OnCmd_label_right(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that toggles whether the
	 * dot labels are placed to the right or left of their dots.
	 * @param event Unused.
	 */
	void OnCmd_label_flip(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that sets the symbol of the
	 * selected dots to plain open dots.
	 * @param event Unused.
	 */
	void OnCmd_setsym0(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that sets the symbol of the
	 * selected dots to solid dots.
	 * @param event Unused.
	 */
	void OnCmd_setsym1(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that sets the symbol of the
	 * selected dots to open backslash dots.
	 * @param event Unused.
	 */
	void OnCmd_setsym2(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that sets the symbol of the
	 * selected dots to open frontslash dots.
	 * @param event Unused.
	 */
	void OnCmd_setsym3(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that sets the symbol of the
	 * selected dots to open X dots.
	 * @param event Unused.
	 */
	void OnCmd_setsym4(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that sets the symbol of the
	 * selected dots to solid backslash dots.
	 * @param event Unused.
	 */
	void OnCmd_setsym5(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that sets the symbol of the
	 * selected dots to solid frontslash dots.
	 * @param event Unused.
	 */
	void OnCmd_setsym6(wxCommandEvent& event);
	/**
	 * Called when the user presses the button that sets the symbol of the
	 * selected dots to solid X dots.
	 * @param event Unused.
	 */
	void OnCmd_setsym7(wxCommandEvent& event);
	/**
	 * Called when the user presses a character on the keyboard.
	 * @param event Tracks which key was pressed.
	 */
	void OnChar(wxKeyEvent& event);

	/**
	 * Called when the "Add Background Image" command is selected from the
	 * field image menu.
	 * @param event Unused.
	 */
	void OnCmd_AddBackgroundImage(wxCommandEvent& event);
	/**
	 * Called when the "Adjust Background Image" command is selected from the
	 * field image menu.
	 * @param event Unused.
	 */
	void OnCmd_AdjustBackgroundImage(wxCommandEvent& event);
	/**
	 * Called when the "Remove Background Image" command is selected from the
	 * field image menu.
	 * @param event Unused.
	 */
	void OnCmd_RemoveBackgroundImage(wxCommandEvent& event);

	/**
	 * Called when the "Reset Reference Point" command is selected from
	 * the edit menu.
	 * @param event Unused
	 */
	void OnCmd_ResetReferencePoint(wxCommandEvent& event);

	/**
	 * Called when the frame is resized.
	 * @param event Contains information about the resizing that was applied
	 * to the frame.
	 */
	void OnSize(wxSizeEvent& event);

	/**
	 * Opens up a file browser that can be used to select a CalChart show to
	 * append to the end of the show being edited.
	 */
	void AppendShow();
	/**
	 * Opens up a file browser that can be used to select a continuity file
	 * to import.
	 * The continuity file is used to load printable continuities for
	 * each stuntsheet.
	 */
	void ImportContFile();

	/**
	 * Adjusts given coordinate so that it is on the grid that is currently
	 * being used by the Field Frame.
	 * @param c The coordinate to snap to the grid.
	 */
	void SnapToGrid(CC_coord& c);
	void UpdatePanel();

	/**
	 * Sets the mode that is being used to select points
	 * on the stunt sheet.
	 * @param type The mode to use for selecting/deselcting
	 * points on the stunt sheet.
	 */
	void SetCurrentLasso(CC_DRAG_TYPES type);
	/**
	 * Sets the mode that is being used to manipulate the
	 * positions of the dots on the stunt sheet.
	 * @param type The mode to use for manipulating the
	 * positions of the dots on the stunt sheet.
	 */
	void SetCurrentMove(CC_MOVE_MODES type);
	/**
	 * Called when the user changes the zoom setting
	 * through the zoom drop down menu.
	 */
	void zoom_callback(wxCommandEvent &);
	/**
	 * Called when the user types a custom zoom factor
	 * into the zoom drop down menu.
	 */
	void zoom_callback_textenter(wxCommandEvent &);
	/**
	 * Called when the user changes the active stuntsheet
	 * through the stunt sheet slider.
	 */
	void slider_sheet_callback(wxScrollEvent &);
	/**
	 * Called when the user changes the active refpoint index
	 * by using the Ref Group drop down menu.
	 */
	void refnum_callback(wxCommandEvent &);
	/**
	 * Called when the user enables/disables the "Draw Paths"
	 * tool.
	 */
	void OnEnableDrawPaths(wxCommandEvent &);

	/**
	 * Creates a dialog through which the user can set up
	 * the number of points in the show, their labels, and
	 * the number of of columns that they are initially
	 * arranged into.
	 */
	void Setup();
	/**
	 * Creates a dialog through which the user can change
	 * the show description.
	 */
	void SetDescription();
	/**
	 * Creates a dialog through which the user can change
	 * the show mode.
	 */
	void SetMode();

	/**
	 * Returns the canvas to which the field view is drawn.
	 * @return The canvas to which the field view is drawn.
	 */
	const FieldCanvas * GetCanvas() const { return mCanvas; }
	/**
	 * Returns the canvas to which the field view is drawn.
	 * @return The canvas to which the field view is drawn.
	 */
	FieldCanvas * GetCanvas() { return mCanvas; }

	/**
	 * Returns the field view which this frame is displaying and editing
	 * the CalChartDoc through.
	 * @return The field view associated with this frame.
	 */
	const FieldView * GetFieldView() const;
	/**
	 * Returns the field view which this frame is displaying and editing
	 * the CalChartDoc through.
	 * @return The field view associated with this frame.
	 */
	FieldView * GetFieldView();

	/**
	 * Returns the CalChartDoc which is being viewed and edited through
	 * this frame.
	 * @return The CalChartDoc which is viewed/edited with this frame.
	 */
	const CalChartDoc * GetShow() const;
	/**
	 * Returns the CalChartDoc which is being viewed and edited through
	 * this frame.
	 * @return The CalChartDoc which is viewed/edited with this frame.
	 */
	CalChartDoc * GetShow();
	
	/**
	 * The drop-down menu which allows the user to define the spacing
	 * of the grid to which points are locked when they are moved.
	 */
	wxChoice *mGridChoice;
	/**
	 * The drop-down menu which allows the user to choose which reference
	 * point to view for each of the points.
	 */
	wxChoice *mRefChoice;
	/**
	 * The drop-down menu which allows the user to select how closely
	 * to zoom on the field.
	 */
	wxComboBox *mZoomBox;
	/**
	 * The slider used to select which stuntsheet to view and edit.
	 */
	wxSlider *mSheetSlider;

	/**
	 * The canvas to which the field view is drawn.
	 */
	FieldCanvas *mCanvas;
	
	/**
	 * The animation frame which is opened when the CalChart Viewer is
	 * opened from this frame.
	 */
	wxWindow *mAnimationFrame;
	/**
	 * Disconnects this frame from its animation frame.
	 */
	void ClearAnimationFrame();
	
	DECLARE_EVENT_TABLE()
};

#endif
