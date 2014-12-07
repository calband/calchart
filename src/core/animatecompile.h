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
#include <memory>

class ContProcedure;
class ContToken;
class AnimateCommand;
class CC_show;
class CC_sheet;

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


class AnimationVariables
{
public:
	struct AnimationVariableException {};
	float GetVarValue(int varnum, unsigned whichPoint) const; // may throw AnimationVariableException if variable doesn't exist
	void SetVarValue(int varnum, unsigned whichPoint, float value);
private:
	std::map<unsigned,AnimateVariable> mVars[NUMCONTVARS];
};

class AnimateCompile
{
public:
// Compile a point
	// give a copy of the variable state
	AnimateCompile(const CC_show& show, CC_show::const_CC_sheet_iterator_t c_sheet, unsigned pt_num, SYMBOL_TYPE cont_symbol, AnimationVariables& variablesStates, std::map<AnimateError, ErrorMarker>& error_markers);
	~AnimateCompile();

// Compile a point
	std::vector<std::shared_ptr<AnimateCommand> > Compile(ContProcedure* proc);
// true if successful
	bool Append(std::shared_ptr<AnimateCommand> cmd, const ContToken *token = nullptr);
	bool Append(std::shared_ptr<AnimateCommand> cmd, int line, int column);

public:
	void RegisterError(AnimateError err, const ContToken *token);
	void RegisterError(AnimateError err, int line, int col);
	static void RegisterError(std::map<AnimateError, ErrorMarker>& error_markers, SYMBOL_TYPE contsymbol, unsigned curr_pt, AnimateError err);
	static void RegisterError(std::map<AnimateError, ErrorMarker>& error_markers, SYMBOL_TYPE contsymbol, unsigned curr_pt, AnimateError err, int line, int col);

	float GetVarValue(int varnum, const ContToken *token = nullptr);
	float GetVarValue(int varnum, int line, int column);
	void SetVarValue(int varnum, float value);

	// helper functions to get information for building a command
	AnimatePoint GetPointPosition() const { return pt; }
	unsigned GetCurrentPoint() const { return curr_pt; }
	unsigned GetBeatsRemaining() const { return beats_rem; }
	AnimatePoint GetStartingPosition() const;
	AnimatePoint GetEndingPosition(const ContToken *token = nullptr);
	AnimatePoint GetReferencePointPosition(unsigned refnum) const;
private:
	AnimatePoint pt;
	const CC_show& mShow;
	const CC_show::const_CC_sheet_iterator_t curr_sheet;
	const unsigned curr_pt;
	unsigned beats_rem;
	const SYMBOL_TYPE contsymbol;
	std::vector<std::shared_ptr<AnimateCommand> > cmds;
	std::map<AnimateError, ErrorMarker>& error_markers;
	AnimationVariables& vars;
};

#endif // _ANIMATECOMPILE_H_
