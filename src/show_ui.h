/*
 * show_ui.h
 * Classes for interacting with shows
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

#ifndef _SHOW_UI_H_
#define _SHOW_UI_H_

#include "cc_show.h"
#include <wx/wizard.h>
#include <wx/docview.h>

#include <vector>

class PointPickerView : public wxView
{
public:
	PointPickerView();
	~PointPickerView();
    virtual void OnDraw(wxDC *dc);
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
};

class PointPicker : public wxDialog
{
public:
	PointPicker();
	PointPicker(CC_show *shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Select Points"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~PointPicker();

	void Update();

private:
	CC_show *show;
	PointPickerView *mView;
	wxListBox *mList;
	std::vector<wxString> mCachedLabels;
	CC_show::SelectionList mCachedSelection;

	void Init();

	bool Create(CC_show *shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Select Points"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();


	void PointPickerAll(wxCommandEvent&);
	void PointPickerNone(wxCommandEvent&);
	void PointPickerSelect(wxCommandEvent&);

	DECLARE_EVENT_TABLE()
};

class ShowInfoReq : public wxDialog
{
	DECLARE_CLASS( ShowInfoReq )
	DECLARE_EVENT_TABLE()

public:
	ShowInfoReq();
	ShowInfoReq(CC_show *shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Show Info"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~ShowInfoReq( );

	void Init();

	bool Create(CC_show *shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Show Info"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

    virtual bool Validate();
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

	// The data this dialog sets for the user
private:
	unsigned mNumberPoints;
	unsigned mNumberColumns;
	std::vector<wxString> mLabels;

public:
	unsigned GetNumberPoints() const { return mNumberPoints; }
	unsigned GetNumberColumns() const { return mNumberColumns; }
	std::vector<wxString> GetLabels() { return mLabels; }

private:
	CC_show *mShow;
	void OnReset(wxCommandEvent&);
};

class ShowInfoReqWizard : public wxWizardPageSimple
{
	DECLARE_CLASS( ShowInfoReqWizard )
public:
	ShowInfoReqWizard(wxWizard *parent);

	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();
	virtual bool Validate();

	// The data this dialog sets for the user
private:
	bool mTransferDataToWindowFirstTime;
	unsigned mNumberPoints;
	unsigned mNumberColumns;
	std::vector<wxString> mLabels;

public:
	unsigned GetNumberPoints() const { return mNumberPoints; }
	unsigned GetNumberColumns() const { return mNumberColumns; }
	std::vector<wxString> GetLabels() { return mLabels; }
};

#endif
