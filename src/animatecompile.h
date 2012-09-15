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

#ifndef _ANIMATECOMPILE_H_
#define _ANIMATECOMPILE_H_

#include "animate_types.h"
#include "animate.h"
#include <vector>
#include <boost/shared_ptr.hpp>

class ContProcedure;
class ContToken;
class AnimateVariable;
class AnimateCommand;
class CC_show;
class CC_sheet;

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

#endif // _ANIMATECOMPILE_H_
