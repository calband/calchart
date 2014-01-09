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
class AnimateCommand;
class CC_show;
class CC_sheet;

/**
 * A variable defined in a continuity that can be changed throughout
 * an animation.
 */
class AnimateVariable
{
private:
	/**
	 * The current value of the variable.
	 */
	float v;
	/**
	 * True if the value is valid; false otherwise.
	 */
	bool valid;
public:
	/**
	 * Makes the variable. It is initially invalid.
	 */
	AnimateVariable(): v(0.0), valid(false) {}
	/**
	 * Returns whether or not the variable is valid. A variable is valid
	 * if it has been initialized (that is, if its value has been set).
	 * @return True if the variable is valid; false otherwise.
	 */
	inline bool IsValid() const { return valid; }
	/**
	 * Returns the current value of the variable.
	 * @return The current value of the variable.
	 */
	inline float GetValue() const { return v; }
	/**
	 * Sets the value of the variable.
	 * @param newv The new value for the variable.
	 */
	inline void SetValue(float newv) { v = newv; valid = true; }
	/**
	 * Clears the value of the variable, making it invalid.
	 */
	inline void ClearValue() { v = 0.0; valid = false; }
};

/**
 * A collection of animation variables, and their values for each point.
 * Each unique variable can be identified by a unique index,and each variable
 * can have a different value for each point.
 */
class AnimationVariables
{
public:
	/**
	 * An error to throw if something goes wrong.
	 */
	struct AnimationVariableException {};
	/**
	 * Returns the value of a variable for a particular point. This throws an
	 * AnimationVariableException if the desired variable does not exist,
	 * or is invalid.
	 * @param varnum The identity of the variable to retrieve.
	 * @param whichPoint The point for which to retrieve the value of the
	 * variable.
	 * @return The value of the variable for a point.
	 */
	float GetVarValue(int varnum, unsigned whichPoint) const;
	/**
	 * Sets the value of a variable for a particular point.
	 * @param varnum The identity of the variable to set.
	 * @param whichPoint The point for which to set the value of the variable.
	 * @param value The value to apply to the variable.
	 */
	void SetVarValue(int varnum, unsigned whichPoint, float value);
private:
	/**
	 * A list of maps (one map for each unique variable)
	 * that map a point index to the value of a particular
	 * variable for that point.
	 */
	std::map<unsigned,AnimateVariable> mVars[NUMCONTVARS];
};

/**
 * Used to compile all continuities into a complete animation.
 */
class AnimateCompile
{
public:
// Compile a point
	// give a copy of the variable state
	AnimateCompile(const CC_show& show, AnimationVariables& variablesStates);
	~AnimateCompile();

// Compile a point
	std::vector<boost::shared_ptr<AnimateCommand> > Compile(CC_show::const_CC_sheet_iterator_t c_sheet, unsigned pt_num, SYMBOL_TYPE cont_symbol, ContProcedure* proc);
// true if successful
	bool Append(boost::shared_ptr<AnimateCommand> cmd, const ContToken *token);

public:
	inline bool Okay() { return okay; };
	void RegisterError(AnimateError err, const ContToken *token);

	float GetVarValue(int varnum, const ContToken *token);
	void SetVarValue(int varnum, float value);

	std::vector<ErrorMarker> GetErrorMarkers() const { return error_markers; }

	// helper functions to get information for building a command
	AnimatePoint GetPointPosition() const { return pt; }
	unsigned GetCurrentPoint() const { return curr_pt; }
	unsigned GetBeatsRemaining() const { return beats_rem; }
	AnimatePoint GetStartingPosition() const;
	AnimatePoint GetEndingPosition(const ContToken *token);
	AnimatePoint GetReferencePointPosition(unsigned refnum) const;
private:
	inline void SetStatus(bool s) { okay = s; };
	AnimatePoint pt;
	const CC_show& mShow;
	CC_show::const_CC_sheet_iterator_t curr_sheet;
	unsigned curr_pt;
	unsigned beats_rem;
	SYMBOL_TYPE contsymbol;
	std::vector<boost::shared_ptr<AnimateCommand> > cmds;
	std::vector<ErrorMarker> error_markers;
	AnimationVariables& vars;
	bool okay;
};

#endif // _ANIMATECOMPILE_H_
