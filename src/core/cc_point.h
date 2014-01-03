/*
 * cc_point.h
 * Definitions for the point classes
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

#ifndef _CC_POINT_H_
#define _CC_POINT_H_

#include "cc_types.h"
#include "cc_coord.h"

#include <bitset>
#include <vector>

// Perhaps this should be put in namespace?


/** 
 * A CalChart point.
 * This class represents a point in a a CalChart stunt sheet. A show is
 * a collection of stunt sheets, and a stunt sheet is a collection of points.
 * A point does NOT represent a marcher - it represents a location FOR a marcher.
 * Thus, the point does not keep track of its own label - the CalChart show does
 * that instead. Another consequence of the separation between marcher and point
 * is that the point does not keep track of how it changes between stunt sheets,
 * because a point does not move. It's just a location - only MARCHERS move, and
 * they move BETWEEN POINTS as they transition from stunt sheet to stunt sheet.
 */
class CC_point
{
public:
	
	/** 
	 * The number of reference points associated with each point.
	 * This must be at least 1, since the first ref point is the position of
	 * the dot.
	 */
	static const unsigned kNumRefPoints = 3;

	/**
	 * Makes a plain dot, with a null position and uninitialized reference
	 * points. Take caution when using this constructor, because many of the
	 * member variables will be null for the new point.
	 */
	CC_point();

	/**
	 * Makes a plain dot located at the position provided by the parameters.
	 * The reference points for the point will also all be set to that same
	 * position.
	 * @param pos The location of the new point, which will also be used as
	 * the position for each of its reference points.
	 */
	CC_point(const CC_coord& pos);

	/**
	 * Used to load a point from a file. A file contains serialized data
	 * that describes a point, and this constructor uses that data to
	 * construct a point that is identical to the one that was saved.
	 * @param serialized_data The serialized data which would represent
	 * this point in a file. This should be in the same format as the
	 * serialized data returned by the Serialize() method.
	 */
	CC_point(const std::vector<uint8_t>& serialized_data);

	/**
	 * Generates a serialized version of this point to be saved in a
	 * file.
	 * @return The serialized data that represents this point in a
	 * CalChart file. The format is as follows:
	 *    TODO FORMAT OF VECTOR
	 */
	std::vector<uint8_t> Serialize() const;

	/**
	 * Returns true if the label on the dot is flipped.
	 * @return True if the label on the dot is flipped; false
	 * otherwise.
	 */
	bool GetFlip() const;

	/** 
	 * Indicates which side of the dot that the label should be
	 * located in the viewer (that is, indicates whether or not
	 * the label of this dot is flipped).
	 * @param val True if the label should be flipped; false
	 * otherwise.
	 */
	void Flip(bool val = true);

	/**
	 * Toggles whether or not the label of the dot is flipped
	 * in the viewer.
	 */
	void FlipToggle();

	/**
	 * Returns the symbol associated with this dot (e.g. solid, slash,
	 * plain, etc.).
	 * @return The symbol type associated with this dot.
	 */
	SYMBOL_TYPE GetSymbol() const;

	/**
	 * Sets the symbol associated with this dot.
	 * @param The new symbol for this dot.
	 */
	void SetSymbol(SYMBOL_TYPE sym);

	/**
	 * Returns the position of the point or one of its reference points. 
	 * @param ref The index of the reference point whose position will
	 * be returned, or 0 to get the position of the point itself.
	 * Values in the inclusive range [1, kNumRefPoints] will retrieve
	 * the positions of the object's ref points. If no parameter is
	 * supplied, the default parameter will cause the method to provide
	 * the position of the point itself.
	 * @return The position of the point if 0 was passed as the
	 * parameter (ref), or the position of a reference point if a value
	 * in the range [1, kNumRefPoints] was passed as the parameter (ref).
	 */
	CC_coord GetPos(unsigned ref = 0) const;

	/**
	 * Sets the position of the point or one of its reference points.
	 * @param c The new position of the point or one of its reference
	 * points.
	 * @param ref Indicates which reference point, if any, to set the
	 * position of. A value of zero indicates that the new position
	 * should be applied to the point itself, and a value in the
	 * range of [1, kNumRefPoints] indicates that the new position
	 * should be applied to one of the reference points.
	 */
	void SetPos(const CC_coord& c, unsigned ref = 0);

private:
	/**
	 * An enumeration that provides mappings between the properties of a point
	 * that can be representated by a bit and the indices where those
	 * properties are stored in mFlags. For example, to retrieve whether or
	 * not the label of the point is flipped, one would use:
	 *     mFlags.test(kPointLabelFlipped)
	 */
	enum {
		/**
		 * The index in mFlags of the bit representing whether or
		 * not the label on the point is flipped.
		 */
		kPointLabelFlipped,

		/**
		 * The number of bits stored in mFlags.
		 */
		kTotalBits
	};
	
	/** 
	 * Used to efficiently store any data associated with the point that
	 * can be represented by a bit, such as whether or not the label on
	 * the point is flipped.
	 */
	std::bitset<kTotalBits> mFlags;

	// by having both a sym type and cont index, we can have several
	// points share the same symbol but have different continuities.
	/**
	 * The symbol associated with this point (e.g. plain open, solid slash, etc.).
	 */
	SYMBOL_TYPE mSym;

	/** 
	 * The position of the point itself.
	 */
	CC_coord mPos;

	/** 
	 * The reference points associated with this object.
	 */
	CC_coord mRef[kNumRefPoints];

	/** 
	 * This friendship is used for testing.
	 */
	friend struct CC_point_values;

	/**
	 * This friendship is used for testing.
	 */
	friend bool Check_CC_point(const CC_point&, const struct CC_point_values&);
};

void CC_point_UnitTests();

#endif // _CC_POINT_H_
