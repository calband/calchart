/* cc_sheet.h
 * Definitions for the sheet classes
 *
 */

/*
   Copyright (C) 1995-2010  Richard Powell

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

#ifdef __GNUG__
#pragma interface
#endif

#include "cc_types.h"
#include "cc_continuity.h"
#include "cc_point.h"

#include <wx/wx.h>
#include <vector>

class CC_show;
class CC_coord;
class Matrix;
class CC_textline;
struct cc_oldpoint;

typedef std::vector<CC_textline> CC_textline_list;

class CC_sheet
{
public:
	CC_sheet(CC_show *shw);
	CC_sheet(CC_show *shw, const wxString& newname);
	CC_sheet(CC_sheet *sht);
	~CC_sheet();

	unsigned GetNumSelectedPoints() const;
	int FindPoint(Coord x, Coord y, unsigned ref = 0) const;
	bool SelectContinuity(unsigned i) const;
	void SetNumPoints(unsigned num, unsigned columns);
	void RelabelSheet(unsigned *table);

	const CC_continuity& GetNthContinuity(unsigned i) const;
	CC_continuity& GetNthContinuity(unsigned i);
	void SetNthContinuity(const wxString& text, unsigned i);
	CC_continuity RemoveNthContinuity(unsigned i);
	void InsertContinuity(const CC_continuity& newcont, unsigned i);
	void AppendContinuity(const CC_continuity& newcont);
	unsigned NextUnusedContinuityNum();
// creates if doesn't exist
	const CC_continuity& GetStandardContinuity(SYMBOL_TYPE sym);
// return 0 if not found else index+1
	unsigned FindContinuityByName(const wxString& name) const;
	bool ContinuityInUse(unsigned idx) const;

	const wxString& GetName() const;
	void SetName(const wxString& newname);
	inline const wxString& GetNumber() const { return number; }
	inline void SetNumber(const wxString& newnumber) { number = newnumber; }
	unsigned short GetBeats() const;
	void SetBeats(unsigned short b);
	inline bool IsInAnimation() const { return (beats != 0); }

	inline const CC_point& GetPoint(unsigned i) const { return pts[i]; }
	inline CC_point& GetPoint(unsigned i) { return pts[i]; }
	std::vector<CC_point> GetPoints() const { return pts; }
	void SetPoints(const std::vector<CC_point>& points) { pts = points; }
	void SetPoint(const cc_oldpoint& val, unsigned i);

	const CC_coord& GetPosition(unsigned i, unsigned ref = 0) const;
	void SetAllPositions(const CC_coord& val, unsigned i);
	void SetPosition(const CC_coord& val, unsigned i, unsigned ref = 0);
	void SetPositionQuick(const CC_coord& val, unsigned i, unsigned ref = 0);

	CC_textline_list continuity;
	typedef std::vector<CC_continuity> ContContainer;
	ContContainer animcont;
	CC_show *show;
	bool picked;							  /* for requestors like printing */
private:
	unsigned short beats;
	std::vector<CC_point> pts;
	wxString name;
	wxString number;

friend void Draw(wxDC *dc, const CC_sheet& sheet, unsigned ref, bool primary, bool drawall, int point);
friend void DrawForPrinting(wxDC *dc, const CC_sheet& sheet, unsigned ref, bool landscape);
friend void DrawCont(wxDC& dc, const CC_sheet& sheet, wxCoord yStart, bool landscape);

friend const wxChar *PrintStandard(FILE *fp, const CC_sheet& sheet);
friend const wxChar *PrintSpringshow(FILE *fp, const CC_sheet& sheet);
friend const wxChar *PrintOverview(FILE *fp, const CC_sheet& sheet);
friend const wxChar *PrintCont(FILE *fp, const CC_sheet& sheet);

};

#endif
