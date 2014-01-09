/*
 * cc_continuity.h
 * Definitions for the continuity classes
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

#ifndef _CC_CONTINUITY_H_
#define _CC_CONTINUITY_H_

#include <string>

/**
 * Represents a CalChart continuity (the set of instructions for points
 * transitioning between two stuntsheets).
 * The continuity has text associated with it that defines the continuity,
 * and that text is entered through the continuity editor.
 * Continuities are also associated with a particular symbol type, but that
 * information is not local to the continuity itself. The CalChart sheets
 * keep track of which continuity object to associate with each symbol type -
 * that way, a single continuity object can be used for multiple symbol types,
 * because the continuity itself is separated from the dot type that it is
 * associated with.
 */
class CC_continuity
{
public:
	/**
	 * Makes the continuity.
	 */
	CC_continuity();
	/**
	 * Cleanup.
	 */
	~CC_continuity();

	/**
	 * Sets the text of the continuity, which should define the instructions
	 * for the dots which adhere to this continuity.
	 * @param s The text for the continuity.
	 */
	void SetText(const std::string& s);
	/**
	 * Adds extra text onto the end of the text for this continuity.
	 * @param s The text to add on to the end of the text for the continuity.
	 */
	void AppendText(const std::string& s);
	/**
	 * Returns the text associated with this continuity.
	 * @return The text associated with this continuity.
	 */
	const std::string& GetText() const;

private:
	/**
	 * The text associated with this continuity. This should essentially be a
	 * script that describes how points adhering to this continuity should
	 * behave.
	 */
	std::string text;

	/**
	 * Friended to give tests full access to the continuity.
	 */
	friend bool Check_CC_continuity(const CC_continuity&, const struct CC_continuity_values&);
	/**
	* Friended to give tests full access to the continuity.
	*/
	friend void CC_continuity_UnitTests();
};

bool Check_CC_continuity(const CC_continuity&, const struct CC_continuity_values&);

void CC_continuity_UnitTests();

#endif
