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

extern const wxString animate_err_msgs[];

AnimateDir AnimGetDirFromVector(CC_coord& vector);
AnimateDir AnimGetDirFromAngle(float ang);

typedef CC_coord AnimatePoint;

class AnimateCommand
{
public:
	AnimateCommand(unsigned beats);
	virtual ~AnimateCommand() {}

// returns false if end of command
	virtual bool Begin(AnimatePoint& pt);
	virtual bool End(AnimatePoint& pt);
	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

// go through all beats at once
	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual AnimateDir Direction() = 0;
	virtual float RealDirection() = 0;
	virtual float MotionDirection();
	virtual void ClipBeats(unsigned beats);

	unsigned numbeats;
protected:
	unsigned beat;
};

class AnimateCommandMT : public AnimateCommand
{
public:
	AnimateCommandMT(unsigned beats, float direction);
	virtual ~AnimateCommandMT() {}

	virtual AnimateDir Direction();
	virtual float RealDirection();
protected:
	AnimateDir dir;
	float realdir;
};

class AnimateCommandMove : public AnimateCommandMT
{
public:
	AnimateCommandMove(unsigned beats, CC_coord movement);
	AnimateCommandMove(unsigned beats, CC_coord movement, float direction);
	virtual ~AnimateCommandMove() {}

	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual float MotionDirection();
	virtual void ClipBeats(unsigned beats);
private:
	CC_coord vector;
};

class AnimateCommandRotate : public AnimateCommand
{
public:
	AnimateCommandRotate(unsigned beats, CC_coord cntr, float rad,
		float ang1, float ang2, bool backwards = false);
	virtual ~AnimateCommandRotate() {}

	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual AnimateDir Direction();
	virtual float RealDirection();
	virtual void ClipBeats(unsigned beats);
private:
	CC_coord origin;
	float r, ang_start, ang_end;
	float face;
};

// An animation sheet is a collection of commands for each of the points.
class AnimateSheet
{
public:
	AnimateSheet(const std::vector<AnimatePoint>& thePoints);
	~AnimateSheet();
	void SetName(const wxString& s);

	std::vector<AnimatePoint> pts; // should probably be const
	std::vector<std::vector<boost::shared_ptr<AnimateCommand> > > commands;
	wxString name;
	unsigned numbeats;
};

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

	inline void EnableCollisions(CollisionWarning col) { check_collis = col; }
	void CheckCollisions();

	// For drawing:
	bool IsCollision(unsigned which) const;
	AnimateDir Direction(unsigned which) const;
	CC_coord Position(unsigned which) const;

	int GetNumberSheets() const;
	int GetCurrentSheet() const;
	int GetNumberBeats() const;
	int GetCurrentBeat() const;
	const wxString& GetCurrentSheetName() const;

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
	CollisionWarning check_collis;
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
