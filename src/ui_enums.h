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

	CALCHART__prev_ss,
	CALCHART__next_ss,
	CALCHART__box,
	CALCHART__poly,
	CALCHART__lasso,
	CALCHART__move,
	CALCHART__line,
	CALCHART__rot,
	CALCHART__shear,
	CALCHART__reflect,
	CALCHART__size,
	CALCHART__genius,
	CALCHART__label_left,
	CALCHART__label_right,
	CALCHART__label_flip,
	CALCHART__setsym0,
	CALCHART__setsym1,
	CALCHART__setsym2,
	CALCHART__setsym3,
	CALCHART__setsym4,
	CALCHART__setsym5,
	CALCHART__setsym6,
	CALCHART__setsym7,
	CALCHART__slider_zoom,
	CALCHART__slider_sheet_callback,
	CALCHART__refnum_callback,
	CALCHART__draw_paths,
	CALCHART__AddBackgroundImage,
	CALCHART__AdjustBackgroundImage,
	CALCHART__RemoveBackgroundImage,

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
	CALCHART__anim_tempo,
	CALCHART__anim_gotosheet,
	CALCHART__anim_gotobeat,
	
	CALCHART__FollowMarcher,
	CALCHART__SaveCameraAngle,
	CALCHART__GoToCameraAngle,
	CALCHART__ShowKeyboardControls,
	CALCHART__ToggleCrowd,
	CALCHART__ToggleMarching,
	CALCHART__ToggleShowOnlySelected,
};

#endif // _UI_ENUMS_H_
