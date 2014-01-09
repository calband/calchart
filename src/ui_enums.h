/*
 * ui_enums.h
 * Central place for all UI id enumerations
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

#ifndef _UI_ENUMS_H_
#define _UI_ENUMS_H_

/**
 * This enumeration keeps track of the identities that CalChart has
 * registered with wxWidgets. This comes into play when registering tools,
 * for example: when you register a tool with wxWidgets, you link a method
 * to an integer identity, and if you link an option in a toolbar to the
 * SAME identity as the method, then when you select that option from the
 * toolbar, it will call the method associated with that integer identity
 * to handle the event. 
 */
enum
{
	/**
	 * The id of the "Append" command in the file menu of the field frame.
	 */
	CALCHART__APPEND_FILE = wxID_HIGHEST,
	/**
	 * The id of the print command in the file menu.
	 * CalChart uses its own print command because the wxView print doesn't
	 * support landscape orientation for pages when printing.
	 */
	CALCHART__wxID_PRINT,
	/**
	 * The id of the print preview command in the file menu.
	 */
	CALCHART__wxID_PREVIEW,
	/**
	 * The id of the import continuity command in the file menu.
	 */
	CALCHART__IMPORT_CONT_FILE,
	/**
	 * The id of the "Print Show to PS" command in the file menu
	 * of the field frame.
	 */
	CALCHART__LEGACY_PRINT,
	/**
	 * The id of the "Print Show to EPS" command in the
	 * file menu of the field frame.
	 */
	CALCHART__LEGACY_PRINT_EPS,
	/**
	 * The id of the "Insert Sheet Before" command in the edit menu of the
	 * field frame.
	 */
	CALCHART__INSERT_BEFORE,
	/**
	 * The id of the "Insert Sheet After" command in the edit menu of the
	 * field frame.
	 */
	CALCHART__INSERT_AFTER,
	/**
	 * The id of the "Relabel Sheets" command in the edit menu of the field
	 * frame.
	 */
	CALCHART__RELABEL,
	/**
	 * The id of the "Edit Continuity" command in the edit menu of the field
	 * frame.
	 */
	CALCHART__EDIT_CONTINUITY,
	/**
	 * The id of the "Set Sheet Title" command in the edit menu of the field
	 * frame.
	 */
	CALCHART__SET_SHEET_TITLE,
	/**
	 * The id of the "Set Beats" command in the edit menu of the field frame.
	 */
	CALCHART__SET_BEATS,
	/**
	 * The id of the "Setup Number of Marchers" command in the edit menu of
	 * the field frame.
	 */
	CALCHART__SETUP,
	/**
	 * The id of the "Set Show Description" command in the edit menu of the
	 * field frame.
	 */
	CALCHART__SETDESCRIPTION,
	/**
	 * The id of the "Set Show Mode" command in the edit menu of the field
	 * frame.
	 */
	CALCHART__SETMODE,
	/**
	 * The id of the "Select Points" command in the edit menu of the field
	 * frame.
	 */
	CALCHART__POINTS,
	/**
	 * The id of the "Open Show in CalChart Viewer" command in the
	 * CalChart viewer menu of the field frame.
	 */
	CALCHART__ANIMATE,
	/**
	 * TODO - UNUSED?
	 */
	CALCHART__OMNIVIEW,
	/**
	 * TODO - UNUSED?
	 */
	CALCHART__SELECTION,

	/**
	 * The id of the tool that opens the previous stunt sheet for
	 * viewing/editing in the field frame.
	 */
	CALCHART__prev_ss,
	/**
	 * The id of the tool that opens the next stunt sheet for
	 * viewing/editing in the field frame.
	 */
	CALCHART__next_ss,
	/**
	 * The id of the tool that selects all points in a user-drawn box.
	 */
	CALCHART__box,
	/**
	 * The id of the tool that selects all points in a user-drawn polygon.
	 */
	CALCHART__poly,
	/**
	 * The id of the tool that selects all points in a user-drawn lasso.
	 */
	CALCHART__lasso,
	/**
	 * The id of the "Translate Points" tool in the field frame.
	 */
	CALCHART__move,
	/**
	 * The id of the "Move Points Into Line" tool in the field frame
	 */
	CALCHART__line,
	/**
	 * The id of the "Rotate Block" tool in the field frame.
	 */
	CALCHART__rot,
	/**
	 * The id of the "Shear Block" tool in the field frame.
	 */
	CALCHART__shear,
	/**
	 * The id of the "Reflect Block" tool in the field frame.
	 */
	CALCHART__reflect,
	/**
	 * The id of the "Resize Block" tool in the field frame./
	CALCHART__size,
	/**
	 * The id of the genius move tool.
	 */
	CALCHART__genius,
	/**
	 * The id of the tool that places the label to the left
	 * of the selected points.
	 */
	CALCHART__label_left,
	/**
	* The id of the tool that places the label to the right
	* of the selected points.
	*/
	CALCHART__label_right,
	/**
	* The id of the tool that toggles the location of the label
	* relative to the selected points between the left and right.
	*/
	CALCHART__label_flip,
	/**
	 * The id of the tool that sets the symbol of the selected points to
	 * plain open dots.
	 */
	CALCHART__setsym0,
	/**
	 * The id of the tool that sets the symbol of the selected points to
	 * solid dots.
	 */
	CALCHART__setsym1,
	/**
	 * The id of the tool that sets the symbol of the selected points to
	 * open dots with a backslash.
	 */
	CALCHART__setsym2,
	/**
	 * The id of the tool that sets the symbol of the selected points to
	 * open dots with a frontslash.
	 */
	CALCHART__setsym3,
	/**
	 * The id of the tool that sets the symbol of the selected points to
	 * open dots with an X.
	 */
	CALCHART__setsym4,
	/**
	 * The id of the tool that sets the symbol of the selected points to
	 * solid dots with a backslash.
	 */
	CALCHART__setsym5,
	/**
	 * The id of the tool that sets the symbol of the selected points to
	 * solid dots with a frontslash.
	 */
	CALCHART__setsym6,
	/**
	 * The id of the tool that sets the symbol of the selected points to
	 * solid dots with an X.
	 */
	CALCHART__setsym7,
	/**
	 * The id associated with the dropdown box that selects the zoom setting
	 * in the field frame (e.g. 50% zoom, 100% zoom, etc.).
	 */
	CALCHART__slider_zoom,
	/**
	 * The id associated with the slider that is used to select which stunt
	 * sheet is currently active in the field frame.
	 */
	CALCHART__slider_sheet_callback,
	/**
	 * The tool associated with the "Ref Group" dropdown menu in the
	 * Field Frame toolbar.
	 */
	CALCHART__refnum_callback,
	/**
	 * The id associated with the "Draw Paths" tool in the field frame.
	 */
	CALCHART__draw_paths,
	/**
	 * The id associated with the "Add Background Image" command in the Field
	 * Image menu of the Field Frame.
	 */
	CALCHART__AddBackgroundImage,
	/**
	 * The id associated with the "Adjust Background Image" command in the
	 * Field Image menu of the Field Frame.
	 */
	CALCHART__AdjustBackgroundImage,
	/**
	 * The id associated with the "Remove Background Image" command in the
	 * Field Image menu of the Field Frame.
	 */
	CALCHART__RemoveBackgroundImage,
	/**
	 * The id associated with the "Reset Ref Points" button in the Field Frame
	 * toolbar.
	 */
	CALCHART__ResetReferencePoint,

	/**
	 * The id of the "Reanimate" command in the Animate menu of the Animation
	 * Frame.
	 */
	CALCHART__anim_reanimate,
	/**
	 * The id of the "Select Collisions" command in the Animate menu
	 * of the Animation Frame.
	 */
	CALCHART__anim_select_coll,
	/**
	 * The id of the button in the Animation Frame toolbar that is used
	 * to stop animating the show.
	 */
	CALCHART__anim_stop,
	/**
	 * The id of the button in the Animation Frame toolbar that is used to
	 * start animating the show.
	 */
	CALCHART__anim_play,
	/**
	 * The id associated with the button in the Animation Frame toolbar that
	 * is used to jump to the previous beat.
	 */
	CALCHART__anim_prev_beat,
	/**
	* The id associated with the button in the Animation Frame toolbar that
	 * is used to jump to the next stuntsheet.
	 */
	CALCHART__anim_next_beat,
	/**
	 * The id associated with a timer that sends notifies the Animation Frame
	 * about when the Animation Frame should jump to the next beat while
	 * animating.
	 * /
	CALCHART__anim_next_beat_timer,
	/**
	 * The id associated with the button in the Animation Frame toolbar that
	 * is used to jump to the previous stuntsheet.
	 */
	CALCHART__anim_prev_sheet,
	/**
	 * The id associated with the button in the Animation Frame toolbar that
	 * is used to jump to the next stuntsheet.
	 */
	CALCHART__anim_next_sheet,
	/**
	 * The id associated with the dropdown menu in the Animation Frame toolbar
	 * that is used to select how the viewer will respond to collisions between
	 * dots while animating.
	 */
	CALCHART__anim_collisions,
	/**
	 * the id associated with the errors dropdown menu in the Animation Frame
	 * Toolbar.
	 */
	CALCHART__anim_errors,
	/**
	 * The id associated with the spinner in the Animation Frame toolbar that is
	 * used to select the tempo of the animation.
	 */
	CALCHART__anim_tempo,
	/**
	 * The id associated with the slider in the Animation Frame toolbar that is
	 * used to browse through the stuntsheets of the show.
	 */
	CALCHART__anim_gotosheet,
	/**
	 * The id associated with the slider in the Animation Frame toolbar that is
	 * used to browse through the beats of the active stuntsheet.
	 */
	CALCHART__anim_gotobeat,
	
	/**
	 * The id of the "Follow Marcher" command in the OmniView menu of the
	 * Animation Frame.
	 */
	CALCHART__FollowMarcher,
	/**
	 * TODO - Unused?
	 */
	CALCHART__SaveCameraAngle,
	/**
	 * TODO - Unused?
	 */
	CALCHART__GoToCameraAngle,
	/**
	 * The id of the "Show Controls" command in the OmniView menu of the
	 * animation frame.
	 */
	CALCHART__ShowKeyboardControls,
	/**
	 * The id of the "Toggle Crowd" command in the OmniView menu of the
	 * Animation Frame.
	 */
	CALCHART__ToggleCrowd,
	/**
	 * The id of the "Toggle Marching" command in the OmniView menu of the
	 * Animation Frame.
	 */
	CALCHART__ToggleMarching,
	/**
	 * The id of the "Toggle Show Selected" command in the OmniView menu of
	 * the Animation Frame.
	 */
	CALCHART__ToggleShowOnlySelected,
	/**
	 * TODO - UNUSED?
	 */
	CALCHART__ToggleWhichCanvas,
	/**
	 * The id associated with the "Split Horizontally" command in the split
	 * menu of the viewer.
	 */
	CALCHART__SplitViewHorizontal,
	/**
	 * The id associated with the "Split Vertically" command in the split menu
	 * of the viewer.
	 */
	CALCHART__SplitViewVertical,
	/**
	 * The id associated with the "Unsplit" command in the split menu of the
	 * viewer.
	 */
	CALCHART__SplitViewUnsplit,
	/**
	 * The id associated with the "Swap Views" command in the split menu of
	 * the viewer.
	 */
	CALCHART__SplitViewSwapAnimateAndOmni,
};

#endif // _UI_ENUMS_H_
