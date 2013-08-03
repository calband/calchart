/*
 * cc_show.h
 * Definitions for the calchart show classes
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

#ifndef _CC_SHOW_H_
#define _CC_SHOW_H_

#include "cc_sheet.h"

#include <wx/wx.h>							  // For basic wx defines
#include <wx/docview.h>							  // For basic wx defines

#include <vector>
#include <set>

class CC_sheet;
class ShowMode;
class ShowUndoList;
class CC_show;
class CC_lasso;
class CalChartDoc;
class wxFFileOutputStream;

// CalChart Show
class CC_show
{
	friend CalChartDoc;
public:
	typedef std::vector<CC_sheet> CC_sheet_container_t;
	typedef CC_sheet_container_t::iterator CC_sheet_iterator_t;
	typedef CC_sheet_container_t::const_iterator const_CC_sheet_iterator_t;

	CC_show();
	~CC_show();

	// How we save and load a show:
    wxSTD ostream& SaveObject(wxSTD ostream& stream);
    wxSTD istream& LoadObject(wxSTD istream& stream);
    wxOutputStream& SaveObject(wxOutputStream& stream);
    wxInputStream& LoadObject(wxInputStream& stream);
    wxFFileOutputStream& SaveObject(wxFFileOutputStream& stream);
private:
	template <typename T>
	T& LoadObjectGeneric(T& stream);
	template <typename T>
	T& SaveObjectGeneric(T& stream);
	template <typename T>
	T& SaveObjectInternal(T& stream);

public:
	wxString ImportContinuity(const wxString& file);

public:
	const std::string& GetDescr() const;
	void SetDescr(const std::string& newdescr);

	void SetupNewShow();
	inline unsigned short GetNumSheets() const { return sheets.size(); }

	CC_sheet_iterator_t GetSheetBegin() { return sheets.begin(); }
	const_CC_sheet_iterator_t GetSheetBegin() const { return sheets.begin(); }
	CC_sheet_iterator_t GetSheetEnd() { return sheets.end(); }
	const_CC_sheet_iterator_t GetSheetEnd() const { return sheets.end(); }

	const_CC_sheet_iterator_t GetNthSheet(unsigned n) const;
	CC_sheet_iterator_t GetNthSheet(unsigned n);
	const_CC_sheet_iterator_t GetCurrentSheet() const { return GetNthSheet(mSheetNum); }
	CC_sheet_iterator_t GetCurrentSheet() { return GetNthSheet(mSheetNum); }
	unsigned GetCurrentSheetNum() const { return mSheetNum; }
	void SetCurrentSheet(unsigned n);

	CC_sheet_container_t RemoveNthSheet(unsigned sheetidx);
	void DeleteNthSheet(unsigned sheetidx);
	void InsertSheetInternal(const CC_sheet& nsheet, unsigned sheetidx);
	void InsertSheetInternal(const CC_sheet_container_t& nsheet, unsigned sheetidx);
	void InsertSheet(const CC_sheet& nsheet, unsigned sheetidx);
	inline unsigned short GetNumPoints() const { return numpoints; }
	void SetNumPoints(unsigned num, unsigned columns);
	bool RelabelSheets(unsigned sht);

	std::string GetPointLabel(unsigned i) const;
	void SetPointLabel(const std::vector<std::string>& labels) { pt_labels = labels; }
	inline const std::vector<std::string>& GetPointLabels() const { return pt_labels; }

	// how to select points:
	// Always select or unselect in groups
	bool SelectAll();
	bool UnselectAll();
	void AddToSelection(const SelectionList& sl);
	void SetSelection(const SelectionList& sl);
	void RemoveFromSelection(const SelectionList& sl);
	void ToggleSelection(const SelectionList& sl);
	void SelectWithLasso(const CC_lasso& lasso, bool toggleSelected, unsigned ref);
	inline bool IsSelected(unsigned i) const { return selectionList.count(i); }
	inline const SelectionList& GetSelectionList() const { return selectionList; }

	const ShowMode& GetMode() const { return *mode; };
	void SetMode(ShowMode* m) { mode = m; };

private:
	ShowMode *mode;

private:
	bool mOkay; // error for when we are loading shows

	std::string descr;
	unsigned short numpoints;
	CC_sheet_container_t sheets;
	std::vector<std::string> pt_labels;
	SelectionList selectionList;	  // order of selections
	unsigned mSheetNum;
};

#endif // _CC_SHOW_H_
