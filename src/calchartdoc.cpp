/*
 * CalChartDoc.cpp
 * Member functions for calchart show classes
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

#include <fstream>

#include "CalChartDoc.h"

#include "cc_command.h"
#include "confgr.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "math_utils.h"
#include "cc_shapes.h"
#include "platconf.h"
#include "draw.h"
#include "cc_fileformat.h"
#include "modes.h"

#include <wx/wfstream.h>
#include <wx/textfile.h>
#include <list>


IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_modified, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_FlushAllViews, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_FinishedLoading, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CalChartDoc_setup, wxObject)

IMPLEMENT_DYNAMIC_CLASS(CalChartDoc, wxDocument);

// Create a new show
CalChartDoc::CalChartDoc() :
mShow(new CC_show()),
mMode(wxGetApp().GetModeList().front().get()),
mTimer(*this)
{
	mTimer.Start(GetConfiguration_AutosaveInterval()*1000);
}

// When a file is opened, we first check to see if there is a temporary 
// file, and if there is, prompt the user to see if they would like use
// that file instead.
bool CalChartDoc::OnOpenDocument(const wxString& filename)
{
	// first check to see if there is a recover file:
	wxString recoveryFile = TranslateNameToAutosaveName(filename);
	if (wxFileExists(recoveryFile))
	{
		// prompt the user to find out if they would like to use the recovery file
		int userchoice = wxMessageBox(
			wxT("CalChart has detected a recovery file (possibly from a previous crash).  ")
			wxT("Would you like to use the recovery file (Warning: choosing recover will ")
			wxT("destroy the original file)?"), wxT("Recovery File Detected"), wxYES_NO|wxCANCEL);
		if (userchoice == wxYES)
		{
			// move the recovery file to the filename, destroying the file and using the recovery
			wxCopyFile(recoveryFile, filename);
		}
		if (userchoice == wxNO)
		{
		}
		if (userchoice == wxCANCEL)
		{
			return false;
		}
	}
	bool success = wxDocument::OnOpenDocument(filename) && mShow;
	if (success)
	{
		// at this point the recover file is no longer useful.
		if (wxFileExists(recoveryFile))
		{
			wxRemoveFile(recoveryFile);
		}
	}
	return success;
}

// If we close a file and decide not to save the changes, don't create a recovery
// file, it may confuse the user.
bool CalChartDoc::OnCloseDocument()
{
	bool success = wxDocument::OnCloseDocument();
	// first check to see if there is a recover file:
	wxString recoveryFile = TranslateNameToAutosaveName(GetFilename());
	if (!IsModified() && wxFileExists(recoveryFile))
	{
		wxRemoveFile(recoveryFile);
	}
	return success;
}

bool CalChartDoc::OnNewDocument()
{
	bool success = wxDocument::OnNewDocument();
	if (success)
	{
		// notify the views that we are a new document.  That should prompt a wizard to set up the show
		CalChartDoc_setup show_setup;
		UpdateAllViews(NULL, &show_setup);
	}
	return success;
}

// When we save a file, the recovery file should be removed to prevent
// a false detection that the file writing failed.
bool CalChartDoc::OnSaveDocument(const wxString& filename)
{
	bool result = wxDocument::OnSaveDocument(filename);
	wxString recoveryFile = TranslateNameToAutosaveName(filename);
	if (result && wxFileExists(recoveryFile))
	{
		wxRemoveFile(recoveryFile);
	}
	return true;
}


// Destroy a show
CalChartDoc::~CalChartDoc()
{}


wxString CalChartDoc::ImportContinuity(const wxString& file)
{
	wxTextFile fp;
	fp.Open(file);
	if (!fp.IsOpened())
	{
		return wxT("Unable to open file");
	}
	std::vector<std::string> lines;
	for (size_t line = 0; line < fp.GetLineCount(); ++line)
	{
		lines.push_back(fp.GetLine(line).ToStdString());
	}

	auto result = mShow->ImportContinuity(lines);
	UpdateAllViews();
	return result;
}


template <typename T>
T& CalChartDoc::SaveObjectGeneric(T& stream)
{
	// flush out the text before we save a file.
	FlushAllTextWindows();
	return SaveObjectInternal(stream);
}

#if wxUSE_STD_IOSTREAM
wxSTD ostream& CalChartDoc::SaveObject(wxSTD ostream& stream)
{
	return SaveObjectGeneric<wxSTD ostream>(stream);
}
#else
wxOutputStream& CalChartDoc::SaveObject(wxOutputStream& stream)
{
	return SaveObjectGeneric<wxOutputStream>(stream);
}
#endif

template <typename T>
T& CalChartDoc::SaveObjectInternal(T& stream)
{
	auto data = mShow->WriteShow();
	stream.write(reinterpret_cast<const char*>(&data[0]), data.size());
	return stream;
}

template <>
wxFFileOutputStream& CalChartDoc::SaveObjectInternal<wxFFileOutputStream>(wxFFileOutputStream& stream)
{
	auto data = mShow->WriteShow();
	stream.Write(&data[0], data.size());
	return stream;
}

template <typename T>
T& CalChartDoc::LoadObjectGeneric(T& stream)
{
	try
	{
		mShow.reset(new CC_show(stream));
	}
	catch (CC_FileException& e) {
		wxString message = wxT("Error encountered:\n");
		message += e.what();
		wxMessageBox(message, wxT("Error!"));
	}
	CalChartDoc_FinishedLoading finishedLoading;
	UpdateAllViews(NULL, &finishedLoading);
	return stream;
}

#if wxUSE_STD_IOSTREAM
wxSTD istream& CalChartDoc::LoadObject(wxSTD istream& stream)
{
	return LoadObjectGeneric<wxSTD istream>(stream);
}
#else
wxInputStream& CalChartDoc::LoadObject(wxInputStream& stream)
{
	return LoadObjectGeneric<wxInputStream>(stream);
}
#endif

void CalChartDoc::FlushAllTextWindows()
{
	CalChartDoc_FlushAllViews flushMod;
	UpdateAllViews(NULL, &flushMod);
}


const std::string& CalChartDoc::GetDescr() const
{
	return mShow->GetDescr();
}


void CalChartDoc::SetDescr(const std::string& newdescr)
{
	mShow->SetDescr(newdescr);
	UpdateAllViews();
}


void CalChartDoc::Modify(bool b)
{
	wxDocument::Modify(b);
	CalChartDoc_modified showMod;
	UpdateAllViews(NULL, &showMod);
}


void CalChartDoc::AutoSaveTimer::Notify()
{
	mShow.Autosave();
}

wxString CalChartDoc::TranslateNameToAutosaveName(const wxString& name)
{
	return name + wxT("~");
}

// When the timer goes off, and if the show has a name and is modified,
// we will write the file to a version of the file that the same
// but with the extension .shw~, to indicate that there is a recovery
// file at that location.
void CalChartDoc::Autosave()
{
	if (GetFilename() != wxT("") && IsModified())
	{
		wxFFileOutputStream outputStream(TranslateNameToAutosaveName(GetFilename()));
		if (outputStream.IsOk())
		{
			SaveObjectInternal(outputStream);
		}
		if (!outputStream.IsOk())
		{
			wxMessageBox(wxT("Error creating recovery file.  Take heed, save often!"), wxT("Recovery Error"));
		}
	}
}


CalChartDoc::const_CC_sheet_iterator_t CalChartDoc::GetNthSheet(unsigned n) const
{
	return mShow->GetNthSheet(n);
}


CalChartDoc::CC_sheet_iterator_t CalChartDoc::GetNthSheet(unsigned n)
{
	return mShow->GetNthSheet(n);
}


CC_show::CC_sheet_container_t CalChartDoc::RemoveNthSheet(unsigned sheetidx)
{
	auto result = mShow->RemoveNthSheet(sheetidx);
	UpdateAllViews();
	return result;
}


void CalChartDoc::SetCurrentSheet(unsigned n)
{
	mShow->SetCurrentSheet(n);
	UpdateAllViews();
}

CalChartDoc::const_CC_sheet_iterator_t CalChartDoc::GetCurrentSheet() const { return mShow->GetCurrentSheet(); }
CalChartDoc::CC_sheet_iterator_t CalChartDoc::GetCurrentSheet() { return mShow->GetCurrentSheet(); }
unsigned short CalChartDoc::GetNumSheets() const { return mShow->GetNumSheets(); }
CalChartDoc::CC_sheet_iterator_t CalChartDoc::GetSheetBegin() { return mShow->GetSheetBegin(); }
CalChartDoc::const_CC_sheet_iterator_t CalChartDoc::GetSheetBegin() const { return mShow->GetSheetBegin(); }
CalChartDoc::CC_sheet_iterator_t CalChartDoc::GetSheetEnd() { return mShow->GetSheetEnd(); }
CalChartDoc::const_CC_sheet_iterator_t CalChartDoc::GetSheetEnd() const { return mShow->GetSheetEnd(); }


unsigned CalChartDoc::GetCurrentSheetNum() const { return mShow->GetCurrentSheetNum(); }

boost::shared_ptr<Animation> CalChartDoc::NewAnimation(NotifyStatus notifyStatus, NotifyErrorList notifyErrorList)
{
	return boost::shared_ptr<Animation>(new Animation(*mShow, notifyStatus, notifyErrorList));
}

void CalChartDoc::SetupNewShow()
{
	mShow->SetupNewShow();
	UpdateAllViews();
}


void CalChartDoc::InsertSheetInternal(const CC_sheet& sheet, unsigned sheetidx)
{
	mShow->InsertSheetInternal(sheet, sheetidx);
	UpdateAllViews();
}


void CalChartDoc::InsertSheetInternal(const CC_sheet_container_t& sheet, unsigned sheetidx)
{
	mShow->InsertSheetInternal(sheet, sheetidx);
	UpdateAllViews();
}


void CalChartDoc::InsertSheet(const CC_sheet& nsheet, unsigned sheetidx)
{
	mShow->InsertSheet(nsheet, sheetidx);
	UpdateAllViews();
}


// warning, the labels might not match up
void CalChartDoc::SetNumPoints(unsigned num, unsigned columns)
{
	mShow->SetNumPoints(num, columns, mMode->FieldOffset());
	UpdateAllViews();
}

unsigned short CalChartDoc::GetNumPoints() const { return mShow->GetNumPoints(); }

bool CalChartDoc::RelabelSheets(unsigned sht)
{
	auto result = mShow->RelabelSheets(sht);
	UpdateAllViews();
	return result;
}


std::string CalChartDoc::GetPointLabel(unsigned i) const
{
	return mShow->GetPointLabel(i);
}


void CalChartDoc::SetPointLabel(const std::vector<std::string>& labels)
{
	mShow->SetPointLabel(labels);
	UpdateAllViews();
}

const std::vector<std::string>& CalChartDoc::GetPointLabels() const { return mShow->GetPointLabels(); }

bool CalChartDoc::SelectAll()
{
	auto result = mShow->SelectAll();
	UpdateAllViews();
	return result;
}


bool CalChartDoc::UnselectAll()
{
	auto result = mShow->UnselectAll();
	UpdateAllViews();
	return result;
}


void CalChartDoc::SetSelection(const SelectionList& sl)
{
	mShow->SetSelection(sl);
	UpdateAllViews();
}


void CalChartDoc::AddToSelection(const SelectionList& sl)
{
	mShow->AddToSelection(sl);
	UpdateAllViews();
}

void CalChartDoc::RemoveFromSelection(const SelectionList& sl)
{
	mShow->RemoveFromSelection(sl);
	UpdateAllViews();
}

void CalChartDoc::ToggleSelection(const SelectionList& sl)
{
	mShow->ToggleSelection(sl);
	UpdateAllViews();
}

void CalChartDoc::SelectWithLasso(const CC_lasso& lasso, bool toggleSelected, unsigned ref)
{
	mShow->SelectWithLasso(lasso, toggleSelected, ref);
}

bool CalChartDoc::IsSelected(unsigned i) const { return mShow->IsSelected(i); }
const SelectionList& CalChartDoc::GetSelectionList() const { return mShow->GetSelectionList(); }


const ShowMode&
CalChartDoc::GetMode() const
{
	return *mMode;
}

void
CalChartDoc::SetMode(const ShowMode* m)
{
	if (!m)
	{
		throw std::runtime_error("Cannot use NULL ShowMode");
	}
	mMode = m;
	UpdateAllViews();
}

CC_show
CalChartDoc::ShowSnapShot() const
{
	return *mShow.get();
}

void
CalChartDoc::RestoreSnapShot(const CC_show& snapshot)
{
	mShow.reset(new CC_show(snapshot));
}

