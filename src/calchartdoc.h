/*
 * CalChartDoc.h
 * Definitions for the wxDoc for calchart shows
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

#ifndef _CALCHARTDOC_H_
#define _CALCHARTDOC_H_

#include "animate.h"

#include <wx/wx.h>							  // For basic wx defines
#include <wx/docview.h>							  // For basic wx defines

#include <boost/shared_ptr.hpp>
#include <vector>
#include <set>

class CC_sheet;
class ShowMode;
class ShowUndoList;
class CC_show;
class CC_lasso;
class Animation;

// The CalChartDoc_modified class is used for indicating to views if the doc has been modified
// some views behave differently if the show has been modified
class CalChartDoc_modified : public wxObject
{
DECLARE_DYNAMIC_CLASS(CalChartDoc_modified)
};

// The CalChartDoc_modified class is used for indicating to views to save any text
class CalChartDoc_FlushAllViews : public wxObject
{
DECLARE_DYNAMIC_CLASS(CalChartDoc_FlushAllViews)
};

// The CalChartDoc_FinishedLoading class is used for indicating to views that a new file has been loaded
class CalChartDoc_FinishedLoading : public wxObject
{
	DECLARE_DYNAMIC_CLASS(CalChartDoc_FinishedLoading)
};

// The CalChartDoc_setup class is used for indicating to views to set up a new show
class CalChartDoc_setup : public wxObject
{
DECLARE_DYNAMIC_CLASS(CalChartDoc_setup)
};



// CalChart Document.
// This holds the CC_show, the core part of CalChart.
class CalChartDoc : public wxDocument
{
	DECLARE_DYNAMIC_CLASS(CalChartDoc)
public:
	typedef std::vector<CC_sheet> CC_sheet_container_t;
	typedef CC_sheet_container_t::iterator CC_sheet_iterator_t;
	typedef CC_sheet_container_t::const_iterator const_CC_sheet_iterator_t;

	CalChartDoc();
	virtual ~CalChartDoc();

	// Override the wxDocument functions:
	// Need to override OnOpenDoc so we can report errors, handle recovery file
	virtual bool OnOpenDocument(const wxString& filename);
	// Need to override OnOpenDoc so we can report errors, handle recovery file
	virtual bool OnCloseDocument();
	// Need to override OnNewDoc so we can start the setup wizard
	virtual bool OnNewDocument();
	// Need to override OnSaveDoc so we can handle recovery files
	virtual bool OnSaveDocument(const wxString& filename);
	// Update the views that the doc been modified
	virtual void Modify(bool b);

	// How we save and load a show:
#if wxUSE_STD_IOSTREAM
    virtual wxSTD ostream& SaveObject(wxSTD ostream& stream);
    virtual wxSTD istream& LoadObject(wxSTD istream& stream);
#else
    virtual wxOutputStream& SaveObject(wxOutputStream& stream);
    virtual wxInputStream& LoadObject(wxInputStream& stream);
#endif
private:
	template <typename T>
	T& LoadObjectGeneric(T& stream);
	template <typename T>
	T& SaveObjectGeneric(T& stream);
	template <typename T>
	T& SaveObjectInternal(T& stream);

public:
	bool ImportPrintableContinuity(const std::vector<std::string>& lines);
	bool SetPrintableContinuity(unsigned current_sheet, const std::string& number, const std::string& lines);

	void FlushAllTextWindows();
	
	boost::shared_ptr<Animation> NewAnimation(NotifyStatus notifyStatus, NotifyErrorList notifyErrorList);
	void SetupNewShow();
	
	const std::string& GetDescr() const;
	void SetDescr(const std::string& newdescr);

	unsigned short GetNumSheets() const;

	CC_sheet_iterator_t GetSheetBegin();
	const_CC_sheet_iterator_t GetSheetBegin() const;
	CC_sheet_iterator_t GetSheetEnd();
	const_CC_sheet_iterator_t GetSheetEnd() const;

	const_CC_sheet_iterator_t GetNthSheet(unsigned n) const;
	CC_sheet_iterator_t GetNthSheet(unsigned n);
	const_CC_sheet_iterator_t GetCurrentSheet() const;
	CC_sheet_iterator_t GetCurrentSheet();
	unsigned GetCurrentSheetNum() const;
	void SetCurrentSheet(unsigned n);

	CC_sheet_container_t RemoveNthSheet(unsigned sheetidx);
	void DeleteNthSheet(unsigned sheetidx);
	void InsertSheetInternal(const CC_sheet& nsheet, unsigned sheetidx);
	void InsertSheetInternal(const CC_sheet_container_t& nsheet, unsigned sheetidx);
	void InsertSheet(const CC_sheet& nsheet, unsigned sheetidx);
	unsigned short GetNumPoints() const;
	void SetNumPoints(unsigned num, unsigned columns);
	bool RelabelSheets(unsigned sht);

	std::string GetPointLabel(unsigned i) const;
	void SetPointLabel(const std::vector<std::string>& labels);
	const std::vector<std::string>& GetPointLabels() const;

	// how to select points:
	// Always select or unselect in groups
	bool SelectAll();
	bool UnselectAll();
	void AddToSelection(const SelectionList& sl);
	void SetSelection(const SelectionList& sl);
	void RemoveFromSelection(const SelectionList& sl);
	void ToggleSelection(const SelectionList& sl);
	void SelectWithLasso(const CC_lasso& lasso, bool toggleSelected, unsigned ref);
	bool IsSelected(unsigned i) const;
	const SelectionList& GetSelectionList() const;

	const ShowMode& GetMode() const;
	void SetMode(const ShowMode* m);

private:
	// Autosaving:
	// goal is to allow the user to have a recoverable file.
	// 
	// When the timer goes off, and if the show is modified,
	// we will write the file to a version of the file that the same
	// but with the extension .shw~, to indicate that there is a recovery
	// file at that location.
	// When a file is opened, we first check to see if there is a temporary 
	// file, and if there is, prompt the user to see if they would like use
	// that file instead.
	// When we save a file, the recovery file should be removed to prevent
	// a false detection that the file writing failed.
	static wxString TranslateNameToAutosaveName(const wxString& name);
	void Autosave();
	
	class AutoSaveTimer: public wxTimer
	{
	public:
		AutoSaveTimer(CalChartDoc& show) : mShow(show) {}
		void Notify();
	private:
		CalChartDoc& mShow;
	};

	friend class BasicCalChartCommand;
	CC_show ShowSnapShot() const;
	void RestoreSnapShot(const CC_show& snapshot);
	
	std::unique_ptr<CC_show> mShow;
	const ShowMode* mMode;
	AutoSaveTimer mTimer;
};

#endif // _CALCHARTDOC_H_
