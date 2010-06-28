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
	BUTTON_RESTORE_ALL,
	SPIN_WIDTH,
	NEW_COLOR_CHOICE
};

BEGIN_EVENT_TABLE(ColorSelectDialog, wxDialog)
EVT_BUTTON(BUTTON_SELECT,ColorSelectDialog::OnCmdSelectColors)
EVT_BUTTON(BUTTON_RESTORE,ColorSelectDialog::OnCmdResetColors)
EVT_BUTTON(BUTTON_RESTORE_ALL,ColorSelectDialog::OnCmdResetAll)
EVT_SPINCTRL(SPIN_WIDTH,ColorSelectDialog::OnCmdSelectWidth)
EVT_COMBOBOX(NEW_COLOR_CHOICE,ColorSelectDialog::OnCmdChooseNewColor)
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
	wxBoxSizer *horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	nameBox = new wxBitmapComboBox(this, NEW_COLOR_CHOICE, ColorNames[0], wxDefaultPosition, wxDefaultSize, COLOR_NUM, ColorNames, wxCB_READONLY|wxCB_DROPDOWN);	
	horizontalsizer->Add(nameBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	for (int i = 0; i < COLOR_NUM; ++i)
	{
		wxBitmap temp_bitmap(16, 16);
		wxMemoryDC temp_dc;
		temp_dc.SelectObject(temp_bitmap);
		temp_dc.SetBackground(*CalChartBrushes[i]);
		temp_dc.Clear();
		nameBox->SetItemBitmap(i, temp_bitmap);
	}
	nameBox->SetSelection(0);

	spin = new wxSpinCtrl(this, SPIN_WIDTH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, CalChartPens[nameBox->GetSelection()]->GetWidth());
	spin->SetValue(CalChartPens[nameBox->GetSelection()]->GetWidth());
	horizontalsizer->Add(spin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	verticalsizer->Add(horizontalsizer );

	topsizer->Add(verticalsizer, 0, wxALL, 5 );

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );

	horizontalsizer->Add(new wxButton(this, BUTTON_SELECT, wxT("&Change Color")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	horizontalsizer->Add(new wxButton(this, BUTTON_RESTORE, wxT("&Reset Color")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	horizontalsizer->Add(new wxButton(this, BUTTON_RESTORE_ALL, wxT("&Reset All")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxButton *close = new wxButton(this, wxID_CANCEL, wxT("&Done"));
	horizontalsizer->Add(close, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	close->SetDefault();

	topsizer->Add(horizontalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
}

void ColorSelectDialog::SetColor(int selection, int width, const wxColour& color)
{
	CalChartPens[selection] = wxThePenList->FindOrCreatePen(color, width, wxSOLID);
	CalChartBrushes[selection] = wxTheBrushList->FindOrCreateBrush(color, wxSOLID);

	// update the namebox list
	{
		wxBitmap test_bitmap(16, 16);
		wxMemoryDC temp_dc;
		temp_dc.SelectObject(test_bitmap);
		temp_dc.SetBackground(*CalChartBrushes[selection]);
		temp_dc.Clear();
		nameBox->SetItemBitmap(selection, test_bitmap);
	}
}

void ColorSelectDialog::OnCmdSelectColors(wxCommandEvent&)
{
	int selection = nameBox->GetSelection();
	wxColourData data;
	data.SetChooseFull(true);
	const wxBrush* brush = CalChartBrushes[selection];
	data.SetColour(brush->GetColour());
	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxColourData retdata = dialog.GetColourData();
		wxColour c = retdata.GetColour();
		SetColor(selection, CalChartPens[selection]->GetWidth(), c);
		SetConfigColor(selection);
	}
}

void ColorSelectDialog::OnCmdSelectWidth(wxSpinEvent& e)
{
	int selection = nameBox->GetSelection();
	SetColor(selection, e.GetPosition(), CalChartPens[selection]->GetColour());
	SetConfigColor(selection);
}

void ColorSelectDialog::OnCmdResetColors(wxCommandEvent&)
{
	int selection = nameBox->GetSelection();
	SetColor(selection, DefaultPenWidth[selection], DefaultColors[selection]);
	ClearConfigColor(selection);
}

void ColorSelectDialog::OnCmdResetAll(wxCommandEvent&)
{
	for (int i = 0; i < COLOR_NUM; ++i)
	{
		SetColor(i, DefaultPenWidth[i], DefaultColors[i]);
		ClearConfigColor(i);
	}
}

void ColorSelectDialog::OnCmdChooseNewColor(wxCommandEvent&)
{
	spin->SetValue(CalChartPens[nameBox->GetSelection()]->GetWidth());
}

