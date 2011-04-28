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

#include <wx/wx.h>							  // For basic wx defines
#include <wx/docview.h>							  // For basic wx defines

#include <vector>
#include <set>

class CC_sheet;
class ShowMode;
class ShowUndoList;

class CC_show : public wxDocument
{
	DECLARE_DYNAMIC_CLASS(CC_show)
public:
	CC_show();
	CC_show(unsigned npoints);
	~CC_show();

	virtual bool AddView(wxView *view) { return wxDocument::AddView(view); }
	virtual bool RemoveView(wxView *view) { return wxDocument::RemoveView(view); }

	// Need to override OnOpenDoc so we can report errors
	virtual bool OnOpenDocument(const wxString& filename) { return wxDocument::OnOpenDocument(filename) && Ok(); }

	virtual wxOutputStream& SaveObject(wxOutputStream& stream);
	virtual wxInputStream& LoadObject(wxInputStream& stream);

	wxString ImportContinuity(const wxString& file);

	int Print(FILE *fp, bool eps = false, bool overview = false,
		unsigned curr_ss = 0, int min_yards = 50) const;

	inline const wxString& GetError() const { return error; }
	inline bool Ok() const { return okay; }

	void Append(CC_show *shw);
	void Append(CC_sheet *newsheets);
	wxString Autosave();
	void ClearAutosave() const;
	void FlushAllTextWindows() const;

public:
	const wxString& GetDescr() const;
	void SetDescr(const wxString& newdescr);

	virtual void Modify(bool b);

	inline unsigned short GetNumSheets() const { return numsheets; }
	inline CC_sheet *GetSheet() const { return sheets; }

	const CC_sheet *GetNthSheet(unsigned n) const;
	CC_sheet *GetNthSheet(unsigned n);
	const CC_sheet *GetCurrentSheet() const { return GetNthSheet(mSheetNum); }
	CC_sheet *GetCurrentSheet() { return GetNthSheet(mSheetNum); }
	unsigned GetCurrentSheetNum() const { return mSheetNum; }
	void SetCurrentSheet(unsigned n) { mSheetNum = n; }

	unsigned GetSheetPos(const CC_sheet *sheet) const;
	CC_sheet *RemoveNthSheet(unsigned sheetidx);
	CC_sheet *RemoveLastSheets(unsigned numtoremain);
	void DeleteNthSheet(unsigned sheetidx);
	void UserDeleteSheet(unsigned sheetidx);
	void InsertSheetInternal(CC_sheet *nsheet, unsigned sheetidx);
	void InsertSheet(CC_sheet *nsheet, unsigned sheetidx);
	void UserInsertSheet(CC_sheet *sht, unsigned sheetidx);
	inline unsigned short GetNumPoints() const { return numpoints; }
	void SetNumPoints(unsigned num, unsigned columns);
	void SetNumPointsInternal(unsigned num);  //Only for creating show class
	bool RelabelSheets(unsigned sht);

	inline const wxString& GetPointLabel(unsigned i) const { return pt_labels[i]; }
	inline wxString& GetPointLabel(unsigned i) { return pt_labels[i]; }
	inline const wxString* GetPointLabels() const { return &pt_labels[0]; }
	inline wxString* GetPointLabels() { return &pt_labels[0]; }
	inline bool GetBoolLandscape() const { return print_landscape; }
	inline bool GetBoolDoCont() const { return print_do_cont; }
	inline bool GetBoolDoContSheet() const { return print_do_cont_sheet; }
	inline void SetBoolLandscape(bool v) { print_landscape = v; }
	inline void SetBoolDoCont(bool v) { print_do_cont = v; }
	inline void SetBoolDoContSheet(bool v) { print_do_cont_sheet = v; }

	bool UnselectAll();
	inline bool IsSelected(unsigned i) const { return selectionList.count(i); }
	void Select(unsigned i, bool val = true);
	inline void SelectToggle(unsigned i)
	{
		Select(i, !IsSelected(i));
	}
	typedef std::set<unsigned> SelectionList;
	inline const SelectionList& GetSelectionList() const { return selectionList; }
	const ShowMode& GetMode() const { return *mode; };
	void SetMode(ShowMode* m) { mode = m; };

	ShowUndoList *undolist;
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
	unsigned short numsheets;
	CC_sheet *sheets;
	std::vector<wxString> pt_labels;
	SelectionList selectionList;	  // order of selections
	bool print_landscape;
	bool print_do_cont;
	bool print_do_cont_sheet;
	unsigned mSheetNum;
};

#endif // _CC_SHOW_H_
