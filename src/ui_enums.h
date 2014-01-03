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
	CALCHART__APPEND_FILE = wxID_HIGHEST,
	// the wxView print is wrong, doesn't do landscape.  rolling our own
	CALCHART__wxID_PRINT,
	CALCHART__wxID_PREVIEW,
	CALCHART__IMPORT_CONT_FILE,
	CALCHART__LEGACY_PRINT,
	CALCHART__LEGACY_PRINT_EPS,
	CALCHART__INSERT_BEFORE,
	CALCHART__INSERT_AFTER,
	CALCHART__RELABEL,
	CALCHART__EDIT_CONTINUITY,
	CALCHART__SET_SHEET_TITLE,
	CALCHART__SET_BEATS,
	CALCHART__SETUP,
	CALCHART__SETDESCRIPTION,
	CALCHART__SETMODE,
	CALCHART__POINTS,
	CALCHART__ANIMATE,
	CALCHART__OMNIVIEW,
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
	CALCHART__move,
	CALCHART__line,
	CALCHART__rot,
	CALCHART__shear,
	CALCHART__reflect,
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
	CALCHART__slider_zoom,
	CALCHART__slider_sheet_callback,
	CALCHART__refnum_callback,
	CALCHART__draw_paths,
	CALCHART__AddBackgroundImage,
	CALCHART__AdjustBackgroundImage,
	CALCHART__RemoveBackgroundImage,
	CALCHART__ResetReferencePoint,

	CALCHART__anim_reanimate,
	CALCHART__anim_select_coll,
	CALCHART__anim_stop,
	CALCHART__anim_play,
	CALCHART__anim_prev_beat,
	CALCHART__anim_next_beat,
	CALCHART__anim_next_beat_timer,
	CALCHART__anim_prev_sheet,
	CALCHART__anim_next_sheet,
	CALCHART__anim_collisions,
	CALCHART__anim_errors,
	CALCHART__anim_tempo,
	CALCHART__anim_gotosheet,
	CALCHART__anim_gotobeat,
	
	/**
	 * The id of the tool that is used to tell the CalChart viewer to follow
	 * a marcher in omniview.
	 */
	CALCHART__FollowMarcher,
	CALCHART__SaveCameraAngle,
	CALCHART__GoToCameraAngle,
	CALCHART__ShowKeyboardControls,
	CALCHART__ToggleCrowd,
	CALCHART__ToggleMarching,
	CALCHART__ToggleShowOnlySelected,
	CALCHART__ToggleWhichCanvas,
	CALCHART__SplitViewHorizontal,
	CALCHART__SplitViewVertical,
	CALCHART__SplitViewUnsplit,
	CALCHART__SplitViewSwapAnimateAndOmni,
};

#endif // _UI_ENUMS_H_
