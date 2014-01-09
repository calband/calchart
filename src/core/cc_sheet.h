/*
 * cc_sheet.h
 * Definitions for the calchart sheet classes
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

#ifndef _CC_SHEET_H_
#define _CC_SHEET_H_

#include "cc_types.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "cc_text.h"

#include <vector>
#include <set>
#include <string>

class CC_show;
class CC_coord;
class CC_continuity;
class CC_point;
typedef std::vector<CC_textline> CC_textline_list;


/**
 * A stunt sheet in a CalChart show.
 * A stunt sheet is essentially a collection of points (CC_point) and
 * instructions for marchers that are transitioning to the next
 * stunt sheet (continuities).
 */
class CC_sheet
{
public:
	/** 
	 * Makes a sheet that is associated with the provided show. The number
	 * of points on the sheet depends on the number of points in the show.
	 * @param shw The CalChart show with which this stunt sheet is associated.
	 */
	CC_sheet(CC_show *shw);

	/**
	 * Loads a stunt sheet from a the data extracted from a CalChart file
	 * (according to the file format associated with CalChart versions 3.3
	 * and earlier).
	 * @param shw The CalChart show with which this stunt sheet is associated.
	 * @param numPoints The number of points located on the sheet. This is required information
	 * to properly load the stunt sheet from the file data.
	 */
	CC_sheet(CC_show *shw, size_t numPoints, std::istream& stream, Version_3_3_and_earlier);

	/**
	 * Loads a stunt sheet from a the data extracted from a CalChart file
	 * (according to the file format associated with CalChart versions 3.4
	 * and beyond).
	 * @param shw The CalChart show with which this stunt sheet is associated.
	 * @param numPoints The number of points located on the sheet. This is required information
	 * to properly load the stunt sheet from the file data.
	 */
	CC_sheet(CC_show *shw, size_t numPoints, std::istream& stream, Current_version_and_later);

	/**
	 * Makes a stunt sheet that is associated with the given show and has the given name.
	 * The number of points on the sheet depends on the number of points in the show.
	 * @param shw The CalChart show with which this stunt sheet is associated.
	 * @param newname The name of the stunt sheet.
	 */
	CC_sheet(CC_show *shw, const std::string& newname);

	/**
	 * Performs cleanup.
	 */
	~CC_sheet();

private:
	/**
	 * Returns data that can be used to save
	 * the points to a file.
	 * @return A vector containing the data that can
	 * be used to save the points to a file.
	 */
	std::vector<uint8_t> SerializeAllPoints() const;
	/**
	 * Returns data that can be used to save the continuities associated
	 * with this sheet to a file.
	 * @return A vector containing the data that can be used to save
	 * the continuities of this sheet to a file.
	 */
	std::vector<uint8_t> SerializeContinuityData() const;
	/**
	 * Returns data that can be used to save the sheet data to
	 * a file.
	 * @return A vector containing the data that can be used
	 * to save the sheet data to a file.
	 */
	std::vector<uint8_t> SerializeSheetData() const;
public:
	/**
	 * Returns data that can be used to save the sheet to
	 * a file.
	 * @return A vector containing the data that can be used
	 * to save the sheet to a file.
	 */
	std::vector<uint8_t> SerializeSheet() const;

	// Observer functions
	/**
	 * Returns the continuity associated with a particular symbol
	 * type.
	 * @param i The target symbol type.
	 * @return The continuity associated with the target symbol.
	 */
	const CC_continuity& GetContinuityBySymbol(SYMBOL_TYPE i) const;
	/**
	 * Returns the set of points associated with a particular
	 * symbol.
	 * @param i The target symbol.
	 * @return The set of points on this stuntsheet associated with
	 * the specified symbol.
	 */
	std::set<unsigned> SelectPointsBySymbol(SYMBOL_TYPE i) const;
	/**
	 * Returns whether or not there is any continuity information
	 * for the specified symbol.
	 * @param idx The target symbol.
	 * @return True if there is continuity information associated with
	 * the target symbol; false otherwise.
	 */
	bool ContinuityInUse(SYMBOL_TYPE idx) const;

	// setting values on the stunt sheet
	// * needs to be through command only *
	/**
	 * Sets up the number of points on the sheet, the number of columns in which they
	 * should initially be organized, and the first position where new marchers should
	 * be placed if marchers need to be added to meet the new number of points on the sheet.
	 * @param num The number of marchers that should be on the sheet.
	 * @param columns The number of columns in which the points should initially be organized.
	 * @param new_march_position The first position where new marchers will be placed, if
	 * new marchers need to be added.
	 */
	void SetNumPoints(unsigned num, unsigned columns, const CC_coord& new_march_position);

	// continuity:
	// * needs to be through command only *
	/**
	 * Sets the text that describes the continuity for a particular symbol
	 * type.
	 * @param sym The symbol which the continuity describes.
	 * @param text The text describing the continuity, as would be entered
	 * into the Continuity Editor.
	 */
	void SetContinuityText(SYMBOL_TYPE sym, const std::string& text);
	
	// points:
	/**
	 * Returns the index of a point found at a particular position (or area
	 * around it), or -1 if none is found.
	 * @param x The x coordinate to search..
	 * @param y The y coordinate to search.
	 * @param searchBound The tolerance of the search. That is, how far a point
	 * can lie away from the provided center point (x,y) and still be "found".
	 * @param ref The index of the ref point that should be used as the
	 * position of the point in deciding where each point is. Zero if
	 * the position of the point itself should be used.
	 */
	int FindPoint(Coord x, Coord y, Coord searchBound, unsigned ref = 0) const;
	/**
	 * Relabels the points on the sheet.
	 * @param table A table mapping the index that a point SHOULD have
	 * to the index that a point currently DOES have.
	 */
	void RelabelSheet(const std::vector<size_t>& table);

	/**
	 * Returns the name of the stuntsheet.
	 * @return The name of the stuntsheet.
	 */
	std::string GetName() const;
	/**
	 * Sets the name of the stuntsheet.
	 * @param newname The new name of the stuntsheet.
	 */
	void SetName(const std::string& newname);
	/**
	 * Returns the index of the stuntsheet, in string form.
	 * @return The index of the stuntsheet, in string form.
	 */
	std::string GetNumber() const;
	/**
	 * Sets the index of the stuntsheet, as a string.
	 * @param newnumber The new index for the stuntsheet, as a string.
	 */
	void SetNumber(const std::string& newnumber);

	// beats
	/**
	 * Returns the duration of the sheet, in beats.
	 * @return The duration of the sheet, in beats.
	 */
	unsigned short GetBeats() const;
	/**
	 * Sets the duration of the sheet, in beats.
	 * @param b The new duration of the sheet, in beats.
	 */
	void SetBeats(unsigned short b);
	/**
	 * Returns whether or not this sheet should appear in the animated
	 * version of the show. A sheet will appear in the animation
	 * as long as its duration is more than zero beats.
	 * @return True if this sheet will appear in the animation,
	 * false otherwise.
	 */
	bool IsInAnimation() const { return (GetBeats() != 0); }

	/**
	 * Returns the point with the specified index.
	 * @param i The index of the point.
	 * @return The point associated with the provided index.
	 */
	const CC_point& GetPoint(unsigned i) const;
	/**
	* Returns the point with the specified index.
	* @param i The index of the point.
	* @return The point associated with the provided index.
	*/
	CC_point& GetPoint(unsigned i);
	/**
	 * Returns the list of all points on the stuntsheet.
	 * @return The list of all points on the stuntsheet.
	 */
	std::vector<CC_point> GetPoints() const;
	/**
	 * Replaces the old set of points on the sheet with a new one.
	 * @param points The new list of points that will appear on the
	 * stuntsheet.
	 */
	void SetPoints(const std::vector<CC_point>& points);

	/**
	 * Returns the position of a particular ref point of a point,
	 * or the position of the point itself if the position of the
	 * zeroth ref point is requested.
	 * @param i The index of the target point.
	 * @param ref The index of the target reference point, or zero
	 * if the position of the point itself is desired.
	 * @return The position of a particular ref point of a point.
	 */
	CC_coord GetPosition(unsigned i, unsigned ref = 0) const;
	/**
	 * Sets the position of all ref points associated with a particular
	 * point (including the zeroth ref point, which is the position of
	 * the point itself).
	 * @param val The new position for the ref points.
	 * @param i The index of the point to modify.
	 */
	void SetAllPositions(const CC_coord& val, unsigned i);
	/**
	 * Sets the position of a ref point for a particular point (or
	 * the position of the point itself, if the provided ref point index
	 * is zero).
	 * @param val The new position for the ref point.
	 * @param i The index of the point to modify.
	 * @param ref The index of the target ref point. If this is zero, then
	 * the position of the point itself will be modified.
	 */
	void SetPosition(const CC_coord& val, unsigned i, unsigned ref = 0);

	// continuity that gets printed
	/**
	 * Sets the human-readable text that will be used to describe
	 * the continuities for all points on the page.
	 * @param lines The text to use as the human-readable description
	 * of the continuities on the page.
	 * @return TODO
	 */
	bool ImportPrintableContinuity(const std::vector<std::string>& lines);
	/**
	 * Returns the human-readable text describing the continuities for
	 * all points on the page.
	 * @return Human-readable text describing the continuities associated
	 * with this page.
	 */
	CC_textline_list GetPrintableContinuity() const;

private:
	/**
	 * Returns the continuity associated with a particular
	 * symbol type.
	 * @param i The symbol type to get the continuity for.
	 * @return The continuity associated with the given
	 * dot type.
	 */
	CC_continuity& GetContinuityBySymbol(SYMBOL_TYPE i);

	typedef std::vector<CC_continuity> ContContainer;
	/**
	 * A list of containing the continuities associated
	 * with each symbol type.
	 */
	ContContainer mAnimationContinuity;

	/**
	 * A human-readable description of the continuities for this
	 * stuntsheet.
	 */
	CC_textline_list mPrintableContinuity;

	/**
	 * The duration of this stunt sheet, in beats.
	 */
	unsigned short beats;

	/**
	 * A list of all points associated with this stunt sheet.
	 */
	std::vector<CC_point> pts;

	/**
	 * The name of the stunt sheet.
	 */
	std::string mName;
	/**
	 * A string representation of this sheet's index.
	 */
	std::string number;

};

#endif
