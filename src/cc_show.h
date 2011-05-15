/* show.h
 * Definitions for the show classes
 *
 * Modification history:
 * 1-2-95     Garrick Meeker              Created from previous CalPrint
 * 4-16-95    Garrick Meeker              Converted to C++
 * 12-19-10    Richard Powell             Broken into own file
 *
 */

/*
   Copyright (C) 1994-2008  Garrick Brian Meeker

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

#ifdef __GNUG__
#pragma interface
#endif

#include "cc_sheet.h"

#include <wx/wx.h>							  // For basic wx defines
#include <wx/docview.h>							  // For basic wx defines

#include <vector>
#include <set>

class CC_sheet;
class ShowMode;
class ShowUndoList;

// The CC_show_modified class is used for indicating to views if the show has been modified
// some views behave differently if the show has been modified
class CC_show_modified : public wxObject
{
DECLARE_DYNAMIC_CLASS(CC_show_modified)
};

// The CC_show_modified class is used for indicating to views to save any text
class CC_show_FlushAllViews : public wxObject
{
DECLARE_DYNAMIC_CLASS(CC_show_FlushAllViews)
};

// The CC_show_modified class is used for indicating to views to save any text
class CC_show_AllViewsGoToCont : public wxObject
{
DECLARE_DYNAMIC_CLASS(CC_show_AllViewsGoToCont)
public:
	CC_show_AllViewsGoToCont(unsigned contnum = 0, int line = 0, int col = -1) : mContnum(contnum), mLine(line), mCol(col) {}
	unsigned mContnum;
	int mLine;
	int mCol;
};

// The CC_show_setup class is used for indicating to views to set up a new show
class CC_show_setup : public wxObject
{
DECLARE_DYNAMIC_CLASS(CC_show_setup)
};

class CC_show : public wxDocument
{
	DECLARE_DYNAMIC_CLASS(CC_show)
public:

	typedef std::vector<CC_sheet> CC_sheet_container_t;
	typedef CC_sheet_container_t::iterator CC_sheet_iterator_t;
	typedef CC_sheet_container_t::const_iterator const_CC_sheet_iterator_t;

	CC_show();
	virtual ~CC_show();

	// Need to override OnOpenDoc so we can report errors
	virtual bool OnOpenDocument(const wxString& filename) { return wxDocument::OnOpenDocument(filename) && Ok(); }
	virtual bool OnNewDocument();

	virtual wxOutputStream& SaveObject(wxOutputStream& stream);
	virtual wxInputStream& LoadObject(wxInputStream& stream);

	wxString ImportContinuity(const wxString& file);

	int Print(FILE *fp, bool eps = false, bool overview = false,
		unsigned curr_ss = 0, int min_yards = 50) const;

	inline const wxString& GetError() const { return error; }
	inline bool Ok() const { return okay; }

	wxString Autosave();
	void ClearAutosave() const;
	void FlushAllTextWindows();

public:
	const wxString& GetDescr() const;
	void SetDescr(const wxString& newdescr);

	virtual void Modify(bool b);

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

	wxString GetPointLabel(unsigned i) const;
	void SetPointLabel(const std::vector<wxString>& labels) { pt_labels = labels; }
	inline const std::vector<wxString>& GetPointLabels() const { return pt_labels; }
	inline bool GetBoolLandscape() const { return print_landscape; }
	inline bool GetBoolDoCont() const { return print_do_cont; }
	inline bool GetBoolDoContSheet() const { return print_do_cont_sheet; }
	inline void SetBoolLandscape(bool v) { print_landscape = v; }
	inline void SetBoolDoCont(bool v) { print_do_cont = v; }
	inline void SetBoolDoContSheet(bool v) { print_do_cont_sheet = v; }

	// how to select points:
	// Always select or unselect in groups
	typedef std::set<unsigned> SelectionList;
	bool SelectAll();
	bool UnselectAll();
	void AddToSelection(const SelectionList& sl);
	void SetSelection(const SelectionList& sl);
	void RemoveFromSelection(const SelectionList& sl);
	void ToggleSelection(const SelectionList& sl);
	inline bool IsSelected(unsigned i) const { return selectionList.count(i); }
	inline const SelectionList& GetSelectionList() const { return selectionList; }

	const ShowMode& GetMode() const { return *mode; };
	void SetMode(ShowMode* m) { mode = m; };

	void AllViewGoToCont(unsigned contnum, int line, int col);
private:
	ShowMode *mode;

private:
	
	void PrintSheets(FILE *fp) const;		  // called by Print()
	void SetAutosaveName(const wxString& realname);

	void AddError(const wxString& str)
	{
		error += str + wxT("\n");
		okay = false;
	}

	mutable wxString error;
	bool okay;

	wxString autosave_name;
	wxString descr;
	unsigned short numpoints;
	CC_sheet_container_t sheets;
	std::vector<wxString> pt_labels;
	SelectionList selectionList;	  // order of selections
	bool print_landscape;
	bool print_do_cont;
	bool print_do_cont_sheet;
	unsigned mSheetNum;
};

#endif // _CC_SHOW_H_
