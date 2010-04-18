/* color_select_ui.cpp
 * Dialox box for selecting colors
 *
 * Modification history:
 * 3-11-10    Richard Powell              Created
 *
 */

/*
   Copyright (C) 1995-2010  Garrick Brian Meeker

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

#ifdef __GNUG__
#pragma implementation
#endif

#include "color_select_ui.h"
#include "confgr.h"
#include <wx/colordlg.h>

enum
{
	BUTTON_SELECT = 1000,
	BUTTON_RESTORE,
};

BEGIN_EVENT_TABLE(ColorSelectDialog, wxDialog)
EVT_BUTTON(BUTTON_SELECT,ColorSelectDialog::OnCmdSelectColors)
EVT_BUTTON(BUTTON_RESTORE,ColorSelectDialog::OnCmdSelectColors)
END_EVENT_TABLE()

IMPLEMENT_CLASS( ColorSelectDialog, wxDialog )

ColorSelectDialog::ColorSelectDialog()
{
	Init();
}


ColorSelectDialog::ColorSelectDialog( wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	Init();

	Create(parent, id, caption, pos, size, style);
}


ColorSelectDialog::~ColorSelectDialog()
{
}


void ColorSelectDialog::Init()
{
}


bool ColorSelectDialog::Create( wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	if (!wxDialog::Create(parent, id, caption, pos, size, style))
		return false;

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}


void ColorSelectDialog::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxBoxSizer *verticalsizer = NULL;
	verticalsizer = new wxBoxSizer( wxVERTICAL );
	nameBox = new wxComboBox(this, wxID_STATIC, wxT("Items"), wxDefaultPosition, wxDefaultSize, COLOR_NUM, ColorNames, wxCB_DROPDOWN);
	nameBox->SetSelection(0,1);
	verticalsizer->Add(nameBox);

	topsizer->Add(verticalsizer, 0, wxALL, 5 );

	wxBoxSizer *horizontalsizer = new wxBoxSizer( wxHORIZONTAL );

	wxButton *congfig = new wxButton(this, BUTTON_SELECT, wxT("&Change Color"));
	horizontalsizer->Add(congfig, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxButton *restoreDefaults = new wxButton(this, BUTTON_RESTORE, wxT("&Restore Default"));
	horizontalsizer->Add(restoreDefaults, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxButton *close = new wxButton(this, wxID_CANCEL, wxT("&Done"));
	horizontalsizer->Add(close, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	close->SetDefault();

	topsizer->Add(horizontalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

}

void ColorSelectDialog::OnCmdSelectColors(wxCommandEvent&)
{
	int selection = nameBox->GetCurrentSelection();
	wxColourData data;
	data.SetChooseFull(true);
	const wxBrush* brush = CalChartBrushes[selection];
	data.SetColour(brush->GetColour());
	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxColourData retdata = dialog.GetColourData();
		wxColour c = retdata.GetColour();
		CalChartPens[selection] = wxThePenList->FindOrCreatePen(c, 1, wxSOLID);
		CalChartBrushes[selection] = wxTheBrushList->FindOrCreateBrush(c, wxSOLID);
	} 
}

