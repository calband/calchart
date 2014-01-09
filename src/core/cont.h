/*
 * cont.h
 * Classes for continuity
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

#ifndef _CONT_H_
#define _CONT_H_

#include "animatecompile.h"

/**
 * Predefined values that can be used in the Continuity Editor.
 */
enum ContDefinedValue
{
	/**
	 * The North direction.
	 */
	CC_N,
	/**
	* The Northwest direction.
	*/
	CC_NW,
	/**
	* The West direction.
	*/
	CC_W,
	/**
	* The Southwest direction.
	*/
	CC_SW,
	/**
	* The South direction.
	*/
	CC_S,
	/**
	* The Southeast direction.
	*/
	CC_SE,
	/**
	* The East direction.
	*/
	CC_E,
	/**
	* The Northeast direction.
	*/
	CC_NE,
	/**
	* The "high step" step size.
	*/
	CC_HS,
	/**
	* The "mini military" step size.
	*/
	CC_MM,
	/**
	* The "show high" step size.
	*/
	CC_SH,
	/**
	* The "jerky step" step size.
	*/
	CC_JS,
	/**
	* The "grapevine step" step size.
	*/
	CC_GV,
	/**
	* The "military step" step size.
	*/
	CC_M,
	/**
	* The "diagonal military" step size.
	*/
	CC_DM
};

/**
 * Represents some token in the Continuity editor.
 */
class ContToken
{
public:
	/**
	 * Makes a token.
	 */
	ContToken();
	/**
	 * Cleanup.
	 */
	virtual ~ContToken();
	/**
	 * The line where the token occurs in the continuity script.
	 */
	int line;
	/**
	 * The column where the token occurs in the continuity script.
	 */
	int col;
};

/** 
 * Represents a point entered into the Continuity Editor.
 */
class ContPoint: public ContToken
{
public:
	ContPoint() {}
	virtual ~ContPoint();

	/**
	 * Returns the coordinate of the point.
	 * @param anim The compiler that makes Animations from
	 * ContTokens.
	 */
	virtual CC_coord Get(AnimateCompile* anim) const;
};

/**
 * Represents the starting location for a dot on the current stuntsheet,
 * entered into the Continuity Editor.
 */
class ContStartPoint : public ContPoint
{
public:
	ContStartPoint() {}

	virtual CC_coord Get(AnimateCompile* anim) const;
};

/**
 * Represents the final location for the dot on the current stuntsheet
 * (its initial position on the next stuntsheet), entered into the
 * Continuity Editor.
 */
class ContNextPoint : public ContPoint
{
public:
	ContNextPoint() {}

	virtual CC_coord Get(AnimateCompile* anim) const;
};

/**
 * Represents a reference point (or actual location) of a dot
 * at the current beat during the current stuntsheet, entered into
 * the Continuity Editor.
 * A reference point with a reference number of zero will provide
 * the current location of the point.
 */
class ContRefPoint : public ContPoint
{
public:
	/**
	 * Makes a ref point.
	 * @param n The ref point to use for the value of this point.
	 * A value of zero will use the actual position of the point.
	 */
	ContRefPoint(unsigned n): refnum(n) {}

	virtual CC_coord Get(AnimateCompile* anim) const;
private:
	/**
	 * An integer defining which reference point to use for the
	 * value of this RefPoint.
	 */
	unsigned refnum;
};

/**
 * Represents a numeric value, or an operation which can be simplified
 * into a a numeric value, entered into the Continuity Editor.
 */
class ContValue: public ContToken
{
public:
	ContValue() {}
	virtual ~ContValue();

	/**
	 * Returns the numeric value that the object represents.
	 */
	virtual float Get(AnimateCompile* anim) const = 0;
};

/**
 * Represents a constant float value, entered into the Continuity Editor.
 */
class ContValueFloat : public ContValue
{
public:
	/**
	 * Makes a float value.
	 * @param v The value of the float.
	 */
	ContValueFloat(float v): val(v) {}

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The value of the float.
	 */
	float val;
};

/**
 * A pre-defined value in the Continuity Editor.
 */
class ContValueDefined : public ContValue
{
public:
	/**
	 * Makes the value.
	 * @param v The pre-defined value to assign to this value.
	 */
	ContValueDefined(ContDefinedValue v): val(v) {}

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The pre-defined value assigned to this value.
	 */
	ContDefinedValue val;
};

/**
 * Represents the addition operator, entered in the Continuity Editor.
 */
class ContValueAdd : public ContValue
{
public:
	/**
	 * Creates the addition operator.
	 * @param v1 The first value in the sum.
	 * @param v2 The second value in the sum.
	 */
	ContValueAdd(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual ~ContValueAdd();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The first value in the sum.
	 */
	ContValue *val1;
	/**
	 * The second value in the sum.
	 */
	ContValue *val2;
};

/**
 * Represents the subtraction operator, entered in the Continuity Editor.
 */
class ContValueSub : public ContValue
{
public:
	/**
	 * Makes the operator.
	 * @param v1 The starting value.
	 * @param v2 The value to subtract from v1.
	 */
	ContValueSub(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual ~ContValueSub();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The starting value.
	 */
	ContValue *val1;
	/**
	 * The value to subtract from val1.
	 */
	ContValue *val2;
};

/**
 * Represents the multiplication operator, entered in the Continuity Editor.
 */
class ContValueMult : public ContValue
{
public:
	/**
	 * Makes the operator.
	 * @param v1 The first value in the product.
	 * @param v2 The second value in the product.
	 */
	ContValueMult(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual ~ContValueMult();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The first value in the product.
	 */
	ContValue *val1;
	/**
	 * The second value in the procuct.
	 */
	ContValue *val2;
};

/**
 * Represents the division operator, entered in the Continuity Editor.
 */
class ContValueDiv : public ContValue
{
public:
	/**
	 * Makes the operator.
	 * @param v1 The dividend.
	 * @param v2 The divisor.
	 */
	ContValueDiv(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual ~ContValueDiv();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The dividend.
	 */
	ContValue *val1;
	/**
	 * The divisor.
	 */
	ContValue *val2;
};

/**
 * Represents the negation operator, entered in the Continuity Editor.
 */
class ContValueNeg : public ContValue
{
public:
	/**
	 * Makes the negation operator.
	 * @param v The value to negate.
	 */
	ContValueNeg(ContValue *v) : val(v) {}
	virtual ~ContValueNeg();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The value to negate.
	 */
	ContValue *val;
};

/**
 * A value representing the number of beats remaining in the stuntsheet,
 * in the Continuity Editor.
 */
class ContValueREM : public ContValue
{
public:
	virtual float Get(AnimateCompile* anim) const;
};

/**
 * Represents a variable in the Continuity Editor.
 */
class ContValueVar : public ContValue
{
public:
	/**
	 * Creates the variable.
	 * @param num The identity associated with the variable.
	 */
	ContValueVar(unsigned num): varnum(num) {}

	virtual float Get(AnimateCompile* anim) const;

	/**
	 * Sets the value of the variable.
	 * @param anim The Compiler which is used to create an
	 * animation from the ContTokens.
	 * @param v The value to set the variable to.
	 */
	void Set(AnimateCompile* anim, float v);
private:
	/**
	 * The identity of the variable.
	 */
	unsigned varnum;
};

/**
 * Represents the DIR function in the Continuity editor.
 * Syntax in Continuity Editor:
 *	DIR([end point])
 * The DIR function returns the direction that the dot would have to
 * travel to reach [end point].
 */
class ContFuncDir : public ContValue
{
public:
	/**
	 * 'Applies' the function. See class description for parameter information.
	 * @param p Represents [end point].
	 */
	ContFuncDir(ContPoint *p): pnt(p) {}
	virtual ~ContFuncDir();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The destination point.
	 */
	ContPoint *pnt;
};

/**
 * Represents the DIRFROM function in the Continuity Editor.
 * Syntax in Continuity Editor:
 *	DIRFROM([start point] [end point])
 * This function returns the direction needed to travel from
 * [start point] to [end point].
 */
class ContFuncDirFrom : public ContValue
{
public:
	/**
	 * 'Applies' the function. See the class description
	 * for parameter descriptions.
	 * @param start Represents [start point].
	 * @param end Represents [end point].
	 */
	ContFuncDirFrom(ContPoint *start, ContPoint *end)
		: pnt_start(start), pnt_end(end) {}
	virtual ~ContFuncDirFrom();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The starting point for the function.
	 */
	ContPoint *pnt_start;
	/**
	 * The point to point toward from pnt_start.
	 */
	ContPoint *pnt_end;
};

/**
 * Represents the DIST function in the Continuity Editor.
 * Syntax in Continuity Editor:
 *	DIST([point])
 * This function returns the distance from the active dot to
 * [point].
 */
class ContFuncDist : public ContValue
{
public:
	/**
	 * 'Applies' the function. See the class description
	 * for parameter descriptions.
	 * @param p Represents [point].
	 */
	ContFuncDist(ContPoint *p): pnt(p) {}
	virtual ~ContFuncDist();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The point to calculate the distance to.
	 */
	ContPoint *pnt;
};

/**
 * Represents the DISTFROM function in the Continuity Editor.
 * Syntax in Continuity Editor:
 *	DISTFROM([point 1] [point 2])
 * This function returns the distance from [point 1] to
 * [point 2].
 */
class ContFuncDistFrom : public ContValue
{
public:
	/**
	 * 'Applies' the function. See the class description
	 * for parameter descriptions.
	 * @param start Represents [point 1].
	 * @param end Represents [point 2].
	 */
	ContFuncDistFrom(ContPoint *start, ContPoint *end)
		: pnt_start(start), pnt_end(end) {}
	virtual ~ContFuncDistFrom();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The starting point.
	 */
	ContPoint *pnt_start;
	/**
	 * The final point.
	 */
	ContPoint *pnt_end;
};

/**
 * Represents the EITHER function in the Continuity Editor.
 * Syntax in Continuity Editor:
 *	EITHER([direction 1] [direction 2] [reference])
 * This function returns [direction 2] if it is closer
 * to the direction between the current point and [reference],
 * and returns [direction 1] otherwise.
 */
class ContFuncEither : public ContValue
{
public:
	/**
	 * 'Applies' the function. See the class description
	 * for parameter descriptions.
	 * @param d1 Represents [direction 1].
	 * @param d2 Represents [direction 2].
	 * @param p Represents [reference].
	 */
	ContFuncEither(ContValue *d1, ContValue *d2, ContPoint *p)
		: dir1(d1), dir2(d2), pnt(p) {}
	virtual ~ContFuncEither();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The first possible direction.
	 */
	ContValue *dir1;
	/**
	 * The second possible direction.
	 */
	ContValue *dir2;
	/**
	 * The target point to turn look towards.
	 */
	ContPoint *pnt;
};

/**
* Represents the OPP function in the Continuity Editor.
* Syntax in Continuity Editor:
*	OPP([direction])
* This function returns the direction opposite to [direction].
*/
class ContFuncOpp : public ContValue
{
public:
	/**
	 * 'Applies' the function. See the class description
	 * for parameter descriptions.
	 * @param d Represents [direction].
	 */
	ContFuncOpp(ContValue *d): dir(d) {}
	virtual ~ContFuncOpp();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The original direction.
	 */
	ContValue *dir;
};

/**
* Represents the STEP function in the Continuity Editor.
* Syntax in Continuity Editor:
*	STEP([beats] [blocksize] [start point])
* This function returns the number of beats that the current
* dot should mark time at the beginning of a step-x drill
* (e.g. a step-2 drill) before moving, given that each marcher waits
* [beats] counts more than the previous marcher before moving, and that
* the distance between marchers moving after one another is [blocksize].
* The step-x drill starts at [start point].
*/
class ContFuncStep : public ContValue
{
public:
	/**
	 * 'Applies' the function. See the class description
	 * for parameter descriptions.
	 * @param beats Represents [beats].
	 * @param blocksize Represents [blocksize].
	 * @param p Represents [start point].
	 */
	ContFuncStep(ContValue *beats, ContValue *blocksize, ContPoint *p)
		: numbeats(beats), blksize(blocksize), pnt(p) {}
	virtual ~ContFuncStep();

	virtual float Get(AnimateCompile* anim) const;
private:
	/**
	 * The number of beats that each marcher waits
	 * after the previous marcher to move.
	 */
	ContValue *numbeats;
	/**
	 * The distance between marchers that move after
	 * one another in the step drill.
	 */
	ContValue *blksize;
	/**
	 * The first point to move in the step drill.
	 */
	ContPoint *pnt;
};

/**
 * Represents a procedure in the Continuity Editor.
 */
class ContProcedure: public ContToken
{
public:
	ContProcedure(): next(NULL) {}
	virtual ~ContProcedure();

	/**
	 * Adds the procedure to an animation compiler so that it can ultimately
	 * become a part of the final animation.
	 * @param anim The animation compiler.
	 */
	virtual void Compile(AnimateCompile* anim) = 0;

	/**
	 * The procedure following this one.
	 */
	ContProcedure *next;
};

/** Represents an SET procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	SET [variable] [value]
 * Sets the value of [variable] to [value].
 */
class ContProcSet : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param vr Represents [variable].
	 * @param v represents [value].
	 */
	ContProcSet(ContValueVar *vr, ContValue *v)
		: var(vr), val(v) {}
	virtual ~ContProcSet();

	virtual void Compile(AnimateCompile* anim);

private:
	/** 
	 * The number of beats to spend closed.
	 */
	ContValueVar *var;
	/**
	 * The direction to face while closed.
	 */
	ContValue *val;
};

/** Represents an BLAM procedure, entered into the Continuity Editor.
* The syntax for this procedure is:
*	BLAM
* Points will even step to their next point for the number of beats
* remaining in the stuntsheet.
*/
class ContProcBlam : public ContProcedure
{
public:
	virtual void Compile(AnimateCompile* anim);
};

/** Represents an COUNTERMARCH procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	COUNTERMARCH [point 1] [point 2] [steps] [dir 1] [dir 2] [beats]
 * Points will move first toward the opposite of [dir 2] and then in [dir 1]
 * to [point 1], where they will continue [steps] past [point 1] in [dir 1].
 * Points will then move in [dir 2], and then opposite of [dir 1] to [point 2],
 * where they will continue [steps] past [point 2] in opposite of [dir 1].
 * Points will repreate this procedure for [beats] counts.
 */
class ContProcCM : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param p1 Represents [point 1].
	 * @param p2 Represents [point 2].
	 * @param steps Represents [steps].
	 * @param d1 Represents [dir 1].
	 * @param d2 Represents [dir 2].
	 * @param beats Represents [beats].
	 */
	ContProcCM(ContPoint *p1, ContPoint *p2, ContValue *steps, ContValue *d1,
		ContValue *d2, ContValue *beats)
		: pnt1(p1), pnt2(p2), stps(steps), dir1(d1), dir2(d2), numbeats(beats) {}
	virtual ~ContProcCM();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The first point defining the shape of the countermarch.
	 */
	ContPoint *pnt1;
	/**
	 * The second point defining the shape of the countermarch.
	 */
	ContPoint *pnt2;
	/**
	 * The number of steps to march toward dir1 after
	 * leaving pnt1.
	 */
	ContValue *stps;
	/**
	 * The first direction to travel after leaving pnt1.
	 */
	ContValue *dir1;
	/**
	 * The second direction to travel after leaving pnt1.
	 */
	ContValue *dir2;
	/**
	 * The number of beats to spend executing this continuity.
	 */
	ContValue *numbeats;
};

/** Represents an DMCM procedure, entered into the Continuity Editor.
* The syntax for this procedure is:
*	DMCM [point 1] [point 2] [beats]
* A very bizarre continuity. Look it up in the CalChart Help docs for
* more info.
*/
class ContProcDMCM : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param p1 Represents [point 1].
	 * @param p2 Represents [point 2].
	 * @param beats Represents [beats].
	 */
	ContProcDMCM(ContPoint *p1, ContPoint *p2, ContValue *beats)
		: pnt1(p1), pnt2(p2), numbeats(beats) {}
	virtual ~ContProcDMCM();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * Represents [point 1] (See class description).
	 */
	ContPoint *pnt1;
	/**
	 * Represents [point 2] (See class description).
	 */
	ContPoint *pnt2;
	/**
	 * The number of beats to spend executing this continuity.
	 */
	ContValue *numbeats;
};

/** Represents an DMHS procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	DMHS [destination]
 * Points first move in a diagonal direction, and then toward either N,
 * S, E, or W to [destination].
 */
class ContProcDMHS : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param p Represents [destination].
	 */
	ContProcDMHS(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcDMHS();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The destination.
	 */
	ContPoint *pnt;
};

/** Represents an EVEN procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	EVEN [steps] [destination]
 * Points march in evenstep to [destination], taking [steps] beats.
 */
class ContProcEven : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param steps Represents [steps].
	 * @param p Represents [destination].
	 */
	ContProcEven(ContValue *steps, ContPoint *p)
		: stps(steps), pnt(p) {}
	virtual ~ContProcEven();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The number of counts to take to reach the destination.
	 */
	ContValue *stps;
	/**
	 * The destination.
	 */
	ContPoint *pnt;
};

/** Represents an EWNS procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	EWNS [destination]
 * Points move first in the EW direction, and then in the NS direction
 * to [destination].
 */
class ContProcEWNS : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param p Represents [destination].
	 */
	ContProcEWNS(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcEWNS();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The destination.
	 */
	ContPoint *pnt;
};

/** Represents an FOUNTAIN procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	FOUNTAIN [direction 1] [direction 2] [destination]
 *	or:
 *	FOUNTAIN [direction 1] [direction 2] [stepsize 1]
 *		[stepsize 2] [destination]
 * Points first move in [direction 1] with a stepsize of [stepsize 1]
 * (or a stepsize of 1 if none is provided), and then in [direction 2]
 * with a stepsize of [stepsize 2] (or a stepsize of 1 if none is
 * provided) to [destination].
 */
class ContProcFountain : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param d1 Represents [direction 1].
	 * @param d2 Represents [direction 2].
	 * @param s1 Represents [stepsize 1].
	 * @param s2 Represents [stepsize 2].
	 * @param p Represents [destination].
	 */
	ContProcFountain(ContValue *d1, ContValue *d2, ContValue *s1, ContValue *s2,
		ContPoint *p)
		: dir1(d1), dir2(d2), stepsize1(s1), stepsize2(s2), pnt(p) {}
	virtual ~ContProcFountain();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The direction of travel for the first part of the procedure.
	 */
	ContValue *dir1;
	/**
	 * The direction of travel for the second part of the procedure.
	 */
	ContValue *dir2;
	/**
	 * The stepsize of the first part of the procedure.
	 */
	ContValue *stepsize1;
	/**
	 * The stepsize of the second part of the procedure.
	 */
	ContValue *stepsize2;
	/**
	 * The destination.
	 */
	ContPoint *pnt;
};

/** Represents an FM procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	FM [steps] [direction]
 * Points move in toward [direction] for [steps] beats, where the
 * size of their steps are generally that of a high step, unless
 * [direction] is diagonal, in which case, the step size will be
 * that of a diagonal step.
 */
class ContProcFM : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param steps Represents [steps].
	 * @param d Represents [direction].
	 */
	ContProcFM(ContValue *steps, ContValue *d)
		: stps(steps), dir(d) {}
	virtual ~ContProcFM();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The number of steps to march.
	 */
	ContValue *stps;
	/**
	 * The direction to march.
	 */
	ContValue *dir;
};

/** Represents an FMTO procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	FMTO [destination]
 * Takes however many steps are required to reach [destination] by moving
 * in a straight line, where the stepsize is generally a highstep size,
 * but is the size of a diagonal step if a precisely diagonal path is traversed
 * to reach [destination].
 */
class ContProcFMTO : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param p Represents [destination].
	 */
	ContProcFMTO(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcFMTO();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The destination.
	 */
	ContPoint *pnt;
};

/** Represents a GRID procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	GRID [grid scale]
 * Teleports points to a position on a grid where the spacing between
 * grid lines is [grid scale] highstep-sized steps, without taking
 * any beats.
 */
class ContProcGrid : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param g Represents [grid scale].
	 */
	ContProcGrid(ContValue *g)
		: grid(g) {}
	virtual ~ContProcGrid();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The spacing between lines of the grid, in number
	 * of highstep steps.
	 */
	ContValue *grid;
};
/** Represents a HSCM procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	HSCM [point 1] [point 2] [beats]
 * This is a really bizarre procedure. Check out the CalChart Help
 * for a description.
 */
class ContProcHSCM : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param p1 Represents [point 1].
	 * @param p2 Represents [point 2].
	 * @param beats Represents [beats].
	 */
	ContProcHSCM(ContPoint *p1, ContPoint *p2, ContValue *beats)
		: pnt1(p1), pnt2(p2), numbeats(beats) {}
	virtual ~ContProcHSCM();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * Represents [point 1] (see class description).
	 */
	ContPoint *pnt1;
	/**
	 * Represents [point 2] (see class description).
	 */
	ContPoint *pnt2;
	/**
	 * The number of beats to spend on this move.
	 */
	ContValue *numbeats;
};

/**
 * Represents a HSDM procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	HSDM [destination]
 * Points will first march in the N, S, E, or W direction, and then will
 * complete a path to [destination] with a diagonal march.
 */
class ContProcHSDM : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param p Represents [destination].
	 */
	ContProcHSDM(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcHSDM();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The destination.
	 */
	ContPoint *pnt;
};

/**
 * Represents essentially a teleportation procedure, entered
 * into the Continuity Editor.
 * The syntax for this procedure is:
 *	MAGIC [destination]
 * Teleports to [destination] without using any beats at all.
 */
class ContProcMagic : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param p Represents [destination].
	 */
	ContProcMagic(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcMagic();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The destination to teleport to.
	 */
	ContPoint *pnt;
};

/**
 * Represents a march procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	MARCH [stepsize] [steps] [direction]
 *	or
 *	MARCH [stepsize] [steps] [direction] [face direction]
 * This will cause points to move towards [direction] for [steps] beats,
 * with each step of size [stepsize]. The points will face in the direction
 * of movement, or in [face direction], if provided.
 */
class ContProcMarch : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param stepsize Represents [stepsize].
	 * @param steps Represents [steps].
	 * @param d Represents [direction].
	 * @param face Represents [face direction].
	 */
	ContProcMarch(ContValue *stepsize, ContValue *steps, ContValue *d, ContValue *face)
		: stpsize(stepsize), stps(steps), dir(d), facedir(face) {}
	virtual ~ContProcMarch();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The stepsize of the move.
	 */
	ContValue *stpsize;
	/**
	 * the number of steps to move.
	 */
	ContValue *stps;
	/**
	 * The direction in which to move.
	 */
	ContValue *dir;
	/**
	 * The direction to face while moving.
	 */
	ContValue *facedir;
};

/**
 * Represents a marktime procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	MT [beats] [direction]
 * This will cause beats to mark time for [beats] beats towards [direction].
 */
class ContProcMT : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param beats Represents [beats].
	 * @param d Represents [direction].
	 */
	ContProcMT(ContValue *beats, ContValue *d)
		: numbeats(beats), dir(d) {}
	virtual ~ContProcMT();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The number of beats to spend marking time.
	 */
	ContValue *numbeats;
	/**
	 * The direction to face while marking time.
	 */
	ContValue *dir;
};

/**
 * Represents a marktime to the end of the stuntsheet procedure, entered
 * into the Continuity Editor.
 * The syntax for this procedure is:
 *	MTRM [direction]
 * This will cause points to mark time until the end of the stuntsheet, facing
 * [direction].
 */
class ContProcMTRM : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param d Represents [direction].
	 */
	ContProcMTRM(ContValue *d)
		: dir(d) {}
	virtual ~ContProcMTRM();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The direction to face while marking time.
	 */
	ContValue *dir;
};

/**
 * Represents a NSEW march procedure, entered into the Continuity Editor.
 * The syntax for this procedure is:
 *	NSEW [destination]
 * This will cause points to move first in the NS direction, and then in
 * the EW direction to [destination]
 */
class ContProcNSEW : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param p Represents [destination].
	 */
	ContProcNSEW(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcNSEW();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The destination.
	 */
	ContPoint *pnt;
};

/**
 * Represents a rotate procedure, entered into the Continuity Editor.
 * In the continuity editor, the syntax for this procedure is:
 *	ROTATE [angle] [steps] [pivot point]
 * This will cause points to move [angle] degrees counterclockwise
 * around [pivot point] in [steps] beats.
 */
class ContProcRotate : public ContProcedure
{
public:
	/**
	 * Creates the procedure. See class description for the meanings
	 * of each parameter.
	 * @param angle Represents [angle].
	 * @param steps Represents [steps].
	 * @param p Represents [pivot point].
	 */
	ContProcRotate(ContValue *angle, ContValue *steps, ContPoint *p)
		: ang(angle), stps(steps), pnt(p) {}
	virtual ~ContProcRotate();

	virtual void Compile(AnimateCompile* anim);

private:
	/**
	 * The angle, in degrees, around the pivot point which the points
	 * should rotate (in the counter-clockwise direction).
	 */
	ContValue *ang;
	/**
	 * The numer of steps that should be taken to complete the rotation.
	 */
	ContValue *stps;
	/**
	 * The pivot point.
	 */
	ContPoint *pnt;
};
#endif
