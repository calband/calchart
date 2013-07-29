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

#include <wx/wx.h>
#include <vector>
#include <set>
#include <iosfwd>

class CC_show;
class CC_coord;
class Matrix;
typedef std::vector<CC_textline> CC_textline_list;

class CC_sheet
{
public:
	CC_sheet(CC_show *shw);
	CC_sheet(CC_show *shw, const std::string& newname);
	~CC_sheet();

	// setting values on the stunt sheet
	// * needs to be through command only *
	void SetNumPoints(unsigned num, unsigned columns);

	// continuity:
	std::set<unsigned> SelectPointsOfContinuity(unsigned i) const;
	const CC_continuity& GetNthContinuity(unsigned i) const;
	// * needs to be through command only *
	void SetNthContinuity(const wxString& text, unsigned i);
	// * needs to be through command only *
	CC_continuity RemoveNthContinuity(unsigned i);
	// * needs to be through command only *
	void InsertContinuity(const CC_continuity& newcont, unsigned i);
	// * needs to be through command only *
	void AppendContinuity(const CC_continuity& newcont);
	// * needs to be through command only *
	unsigned NextUnusedContinuityNum();
// creates if doesn't exist
	// * needs to be through command only * //
	const CC_continuity& GetStandardContinuity(SYMBOL_TYPE sym);
// return 0 if not found else index+1
	unsigned FindContinuityByName(const wxString& name) const;
	bool ContinuityInUse(unsigned idx) const;
	
	// points:
	int FindPoint(Coord x, Coord y, unsigned ref = 0) const;
	void RelabelSheet(unsigned *table);

	std::string GetName() const;
	void SetName(const std::string& newname);
	std::string GetNumber() const;
	void SetNumber(const std::string& newnumber);
	unsigned short GetBeats() const;

	// * needs to be through command only * //
	void SetBeats(unsigned short b);
	inline bool IsInAnimation() const { return (beats != 0); }

	const CC_point& GetPoint(unsigned i) const { return pts[i]; }
	CC_point& GetPoint(unsigned i) { return pts[i]; }
	std::vector<CC_point> GetPoints() const { return pts; }
	void SetPoints(const std::vector<CC_point>& points) { pts = points; }

	CC_coord GetPosition(unsigned i, unsigned ref = 0) const;
	void SetAllPositions(const CC_coord& val, unsigned i);
	void SetPosition(const CC_coord& val, unsigned i, unsigned ref = 0);

	CC_textline_list continuity;
	typedef std::vector<CC_continuity> ContContainer;
	ContContainer animcont;
	
private:
	CC_show *show;
	unsigned short beats;
	std::vector<CC_point> pts;
	std::string name;
	std::string number;

};


#endif
