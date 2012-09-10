/*
 * animate.h
 * Classes for animating shows
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

#ifndef _ANIMATE_H_
#define _ANIMATE_H_

#include "cc_coord.h"
#include "cc_show.h"
#include <wx/string.h>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <map>
#include <set>
#include <vector>

// Number of variables in continuity language (A B C D X Y Z DOF DOH)
enum
{
	CONTVAR_A,
	CONTVAR_B,
	CONTVAR_C,
	CONTVAR_D,
	CONTVAR_X,
	CONTVAR_Y,
	CONTVAR_Z,
	CONTVAR_DOF,
	CONTVAR_DOH,
	NUMCONTVARS
};

enum AnimateDir
{
	ANIMDIR_N, ANIMDIR_NE, ANIMDIR_E, ANIMDIR_SE,
	ANIMDIR_S, ANIMDIR_SW, ANIMDIR_W, ANIMDIR_NW
};

enum AnimateError
{
	ANIMERR_OUTOFTIME,
	ANIMERR_EXTRATIME,
	ANIMERR_WRONGPLACE,
	ANIMERR_INVALID_CM,
	ANIMERR_INVALID_FNTN,
	ANIMERR_DIVISION_ZERO,
	ANIMERR_UNDEFINED,
	ANIMERR_SYNTAX,
	ANIMERR_NONINT,
	ANIMERR_NEGINT,
	NUM_ANIMERR
};

enum MarchingStyle
{
	STYLE_HighStep,
	STYLE_Military,
	STYLE_ShowHigh,
	STYLE_GrapeVine,
	STYLE_JerkyStep,
	STYLE_Close
};

extern const wxString animate_err_msgs[];

AnimateDir AnimGetDirFromAngle(float ang);

typedef CC_coord AnimatePoint;
class AnimateCommand;
class AnimateSheet;

enum CollisionWarning
{
	COLLISION_NONE,
	COLLISION_SHOW,
	COLLISION_BEEP
};

class ErrorMarker
{
public:
	std::set<unsigned> pntgroup;			  // which points have this error
	unsigned contnum;						  // which continuity
	int line, col;							  // where
	ErrorMarker(): contnum(0), line(-1), col(-1) {}
	~ErrorMarker() {}
	void Reset()
	{
		pntgroup.clear();
		contnum = 0;
		line = col = -1;
	}
};

typedef boost::function<void (const wxString& notice)> NotifyStatus;
typedef boost::function<bool (const ErrorMarker error_markers[NUM_ANIMERR], unsigned sheetnum, const wxString& message)> NotifyErrorList;

class Animation
{
public:
	Animation(CC_show *show, NotifyStatus notifyStatus, NotifyErrorList notifyErrorList);
	~Animation();

// Returns true if changes made
	bool PrevSheet();
	bool NextSheet();

	void GotoBeat(unsigned i);

	bool PrevBeat();
	bool NextBeat();

	void GotoSheet(unsigned i);

	typedef void (*CollisionAction_t)();
	// set collision action returns the previous collision action
	CollisionAction_t SetCollisionAction(CollisionAction_t col) { CollisionAction_t oldaction = mCollisionAction; mCollisionAction = col; return oldaction; }

	// For drawing:
	bool IsCollision(unsigned which) const;
	AnimateDir Direction(unsigned which) const;
	float RealDirection(unsigned which) const;
	CC_coord Position(unsigned which) const;

	int GetNumberSheets() const;
	int GetCurrentSheet() const;
	int GetNumberBeats() const;
	int GetCurrentBeat() const;
	wxString GetCurrentSheetName() const;

	void DrawPath(wxDC& dc, int whichPoint, const CC_coord& offset) const;

private:
	const unsigned numpts;
	std::vector<AnimatePoint> pts;
	std::vector<std::vector<boost::shared_ptr<AnimateCommand> >::iterator > curr_cmds; // pointer to the current command
	std::set<int> mCollisions;
	unsigned curr_sheetnum;
	unsigned curr_beat;

	void BeginCmd(unsigned i);
	void EndCmd(unsigned i);

	void RefreshSheet();

	std::vector<AnimateSheet> sheets;
	
	void CheckCollisions();
	CollisionAction_t mCollisionAction;
};

class AnimateVariable
{
private:
	float v;
	bool valid;
public:
	AnimateVariable(): v(0.0), valid(false) {}
	inline bool IsValid() const { return valid; }
	inline float GetValue() const { return v; }
	inline void SetValue(float newv) { v = newv; valid = true; }
	inline void ClearValue() { v = 0.0; valid = false; }
};

class ContProcedure;
class ContToken;
class AnimateCompile
{
public:
// Compile a point
	AnimateCompile(CC_show *show);
	~AnimateCompile();

// Compile a point
	void Compile(CC_show::const_CC_sheet_iterator_t c_sheet, unsigned pt_num, unsigned cont_num, ContProcedure* proc);
// true if successful
	bool Append(boost::shared_ptr<AnimateCommand> cmd, const ContToken *token);

public:
	inline bool Okay() { return okay; };
	inline void SetStatus(bool s) { okay = s; };
	void RegisterError(AnimateError err, const ContToken *token);
	void FreeErrorMarkers();

	float GetVarValue(int varnum, const ContToken *token);
	void SetVarValue(int varnum, float value);

	AnimatePoint pt;
	std::vector<boost::shared_ptr<AnimateCommand> > cmds;
	CC_show *mShow;
	CC_show::const_CC_sheet_iterator_t curr_sheet;
	unsigned curr_pt;
	unsigned beats_rem;
	ErrorMarker error_markers[NUM_ANIMERR];
private:
	unsigned contnum;
	std::map<unsigned,AnimateVariable> vars[NUMCONTVARS];
	bool okay;
};
#endif
