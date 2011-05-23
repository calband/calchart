/*
 * color_select_ui.cpp
 * Dialox box for selecting colors
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

#include "color_select_ui.h"
#include "confgr.h"
#include <wx/colordlg.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/listbook.h>

enum
{
	BUTTON_SELECT = 1000,
	BUTTON_RESTORE,
	BUTTON_RESTORE_ALL,
	SPIN_WIDTH,
	NEW_COLOR_CHOICE
};

BEGIN_EVENT_TABLE(ColorSelectDialog, wxPanel)
EVT_BUTTON(BUTTON_SELECT,ColorSelectDialog::OnCmdSelectColors)
EVT_BUTTON(BUTTON_RESTORE,ColorSelectDialog::OnCmdResetColors)
EVT_BUTTON(BUTTON_RESTORE_ALL,ColorSelectDialog::OnCmdResetAll)
EVT_SPINCTRL(SPIN_WIDTH,ColorSelectDialog::OnCmdSelectWidth)
EVT_COMBOBOX(NEW_COLOR_CHOICE,ColorSelectDialog::OnCmdChooseNewColor)
END_EVENT_TABLE()

IMPLEMENT_CLASS( ColorSelectDialog, wxPanel )

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
	if (!wxPanel::Create(parent, id, pos, size, style, caption))
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


typedef enum
{
	RESET = 1000,
	HEADFONT,
	MAINFONT,
	NUMBERFONT,
	CONTFONT,
	BOLDFONT,
	ITALFONT,
	BOLDITALFONT,
	HEADERSIZE,
	YARDSSIZE,
	TEXTSIZE,
	DOTRATIO,
	NUMRATIO,
	PLINERATIO,
	SLINERATIO,
	CONTRATIO
} SetUpPrintingValues_IDs;


BEGIN_EVENT_TABLE(SetUpPrintingValues, wxPanel)
EVT_BUTTON(wxID_RESET,SetUpPrintingValues::OnCmdReset)
END_EVENT_TABLE()

IMPLEMENT_CLASS( SetUpPrintingValues, wxPanel )

SetUpPrintingValues::SetUpPrintingValues()
{
	Init();
}

SetUpPrintingValues::SetUpPrintingValues( wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	Init();

	Create(parent, id, caption, pos, size, style);
}

SetUpPrintingValues::~SetUpPrintingValues()
{
}

void SetUpPrintingValues::Init()
{
}

bool SetUpPrintingValues::Create( wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	if (!wxPanel::Create(parent, id, pos, size, style, caption))
		return false;

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}

static void AddTextboxWithCaption(wxWindow* parent, wxBoxSizer* verticalsizer, int id, const wxString& caption)
{
	wxBoxSizer* textsizer = new wxBoxSizer( wxVERTICAL );
	textsizer->Add(new wxStaticText(parent, wxID_STATIC, caption, wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	textsizer->Add(new wxTextCtrl(parent, id), 0, wxGROW|wxALL, 5 );
	verticalsizer->Add(textsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
}

void SetUpPrintingValues::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	// do the first column:
	wxBoxSizer *horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	topsizer->Add(horizontalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer *verticalsizer = new wxBoxSizer( wxVERTICAL );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	AddTextboxWithCaption(this, verticalsizer, HEADFONT, wxT("Header Font:"));
	AddTextboxWithCaption(this, verticalsizer, MAINFONT, wxT("Main Font:"));
	AddTextboxWithCaption(this, verticalsizer, NUMBERFONT, wxT("Number Font:"));
	AddTextboxWithCaption(this, verticalsizer, CONTFONT, wxT("Continuity Font:"));
	AddTextboxWithCaption(this, verticalsizer, BOLDFONT, wxT("Bold Font:"));
	AddTextboxWithCaption(this, verticalsizer, ITALFONT, wxT("Italic Font:"));
	AddTextboxWithCaption(this, verticalsizer, BOLDITALFONT, wxT("Bold Italic Font:"));

	// the next column
	verticalsizer = new wxBoxSizer( wxVERTICAL );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	AddTextboxWithCaption(this, verticalsizer, HEADERSIZE, wxT("Header Size:"));
	AddTextboxWithCaption(this, verticalsizer, YARDSSIZE, wxT("Yards Side:"));
	AddTextboxWithCaption(this, verticalsizer, TEXTSIZE, wxT("Text Side:"));
	AddTextboxWithCaption(this, verticalsizer, DOTRATIO, wxT("Dot Ratio:"));
	AddTextboxWithCaption(this, verticalsizer, NUMRATIO, wxT("Num Ratio:"));
	AddTextboxWithCaption(this, verticalsizer, PLINERATIO, wxT("P-Line Ratio:"));
	AddTextboxWithCaption(this, verticalsizer, SLINERATIO, wxT("S-Line Ratio:"));
	AddTextboxWithCaption(this, verticalsizer, CONTRATIO, wxT("Continuity Ratio:"));

	// put a line between the controls and the buttons
	topsizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW|wxALL, 5);

	// the buttons on the bottom
	wxBoxSizer *okCancelBox = new wxBoxSizer( wxHORIZONTAL );
	topsizer->Add(okCancelBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	okCancelBox->Add(new wxButton(this, wxID_OK), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	okCancelBox->Add(new wxButton(this, wxID_CANCEL), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	okCancelBox->Add(new wxButton(this, wxID_RESET, wxT("&Reset")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

}

bool SetUpPrintingValues::TransferDataToWindow()
{
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(HEADFONT);
	text->SetValue(GetConfiguration_HeadFont());
	text = (wxTextCtrl*) FindWindow(MAINFONT);
	text->SetValue(GetConfiguration_MainFont());
	text = (wxTextCtrl*) FindWindow(NUMBERFONT);
	text->SetValue(GetConfiguration_NumberFont());
	text = (wxTextCtrl*) FindWindow(CONTFONT);
	text->SetValue(GetConfiguration_ContFont());
	text = (wxTextCtrl*) FindWindow(BOLDFONT);
	text->SetValue(GetConfiguration_BoldFont());
	text = (wxTextCtrl*) FindWindow(ITALFONT);
	text->SetValue(GetConfiguration_ItalFont());
	text = (wxTextCtrl*) FindWindow(BOLDITALFONT);
	text->SetValue(GetConfiguration_BoldItalFont());
	text = (wxTextCtrl*) FindWindow(HEADERSIZE);
	wxString buf;
	buf.Printf(wxT("%.2f"), GetConfiguration_HeaderSize());
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(YARDSSIZE);
	buf.Printf(wxT("%.2f"), GetConfiguration_YardsSize());
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(TEXTSIZE);
	buf.Printf(wxT("%.2f"), GetConfiguration_TextSize());
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(DOTRATIO);
	buf.Printf(wxT("%.2f"), GetConfiguration_DotRatio());
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(NUMRATIO);
	buf.Printf(wxT("%.2f"), GetConfiguration_NumRatio());
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(PLINERATIO);
	buf.Printf(wxT("%.2f"), GetConfiguration_PLineRatio());
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(SLINERATIO);
	buf.Printf(wxT("%.2f"), GetConfiguration_SLineRatio());
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(CONTRATIO);
	buf.Printf(wxT("%.2f"), GetConfiguration_ContRatio());
	text->SetValue(buf);
	return true;
}

bool SetUpPrintingValues::TransferDataFromWindow()
{
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(HEADFONT);
	SetConfiguration_HeadFont(text->GetValue());
	text = (wxTextCtrl*) FindWindow(MAINFONT);
	SetConfiguration_MainFont(text->GetValue());
	text = (wxTextCtrl*) FindWindow(NUMBERFONT);
	SetConfiguration_NumberFont(text->GetValue());
	text = (wxTextCtrl*) FindWindow(CONTFONT);
	SetConfiguration_ContFont(text->GetValue());
	text = (wxTextCtrl*) FindWindow(BOLDFONT);
	SetConfiguration_BoldFont(text->GetValue());
	text = (wxTextCtrl*) FindWindow(ITALFONT);
	SetConfiguration_ItalFont(text->GetValue());
	text = (wxTextCtrl*) FindWindow(BOLDITALFONT);
	SetConfiguration_BoldItalFont(text->GetValue());
	text = (wxTextCtrl*) FindWindow(HEADERSIZE);
	double dval;
	text->GetValue().ToDouble(&dval);
	SetConfiguration_HeaderSize(dval);
	text = (wxTextCtrl*) FindWindow(YARDSSIZE);
	text->GetValue().ToDouble(&dval);
	SetConfiguration_YardsSize(dval);
	text = (wxTextCtrl*) FindWindow(TEXTSIZE);
	text->GetValue().ToDouble(&dval);
	SetConfiguration_TextSize(dval);
	text = (wxTextCtrl*) FindWindow(DOTRATIO);
	text->GetValue().ToDouble(&dval);
	SetConfiguration_DotRatio(dval);
	text = (wxTextCtrl*) FindWindow(NUMRATIO);
	text->GetValue().ToDouble(&dval);
	SetConfiguration_NumRatio(dval);
	text = (wxTextCtrl*) FindWindow(PLINERATIO);
	text->GetValue().ToDouble(&dval);
	SetConfiguration_PLineRatio(dval);
	text = (wxTextCtrl*) FindWindow(SLINERATIO);
	text->GetValue().ToDouble(&dval);
	SetConfiguration_SLineRatio(dval);
	text = (wxTextCtrl*) FindWindow(CONTRATIO);
	text->GetValue().ToDouble(&dval);
	SetConfiguration_ContRatio(dval);
	return true;
}

void SetUpPrintingValues::OnCmdReset(wxCommandEvent&)
{
	ClearConfiguration_HeadFont();
	ClearConfiguration_MainFont();
	ClearConfiguration_NumberFont();
	ClearConfiguration_ContFont();
	ClearConfiguration_BoldFont();
	ClearConfiguration_ItalFont();
	ClearConfiguration_BoldItalFont();
	ClearConfiguration_HeaderSize();
	ClearConfiguration_YardsSize();
	ClearConfiguration_TextSize();
	ClearConfiguration_DotRatio();
	ClearConfiguration_NumRatio();
	ClearConfiguration_PLineRatio();
	ClearConfiguration_SLineRatio();
	ClearConfiguration_ContRatio();
	TransferDataToWindow();
}


class SetUpModes : public wxPanel
{
	DECLARE_CLASS( SetUpModes )
	DECLARE_EVENT_TABLE()

public:
	SetUpModes( )
	{
		Init();
	}
	SetUpModes( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Setup Modes"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU )
	{
		Init();
		Create(parent, id, caption, pos, size, style);
	}
	~SetUpModes( ) {}

	void Init() {}

	bool Create(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Setup Modes"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

	// use these to get and set default values
	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();
private:
	void OnCmdReset(wxCommandEvent&);
};

enum
{
	MODE_CHOICE = 1000,
	WESTHASH,
	EASTHASH,
	BORDER_LEFT,
	BORDER_TOP,
	BORDER_RIGHT,
	BORDER_BOTTOM,
	OFFSET_X,
	OFFSET_Y,
	SIZE_X,
	SIZE_Y
};

BEGIN_EVENT_TABLE(SetUpModes, wxPanel)
END_EVENT_TABLE()

IMPLEMENT_CLASS( SetUpModes, wxPanel )

bool SetUpModes::Create(wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	if (!wxPanel::Create(parent, id, pos, size, style, caption))
		return false;

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}

void SetUpModes::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxChoice* modes = new wxChoice(this, MODE_CHOICE, wxDefaultPosition, wxDefaultSize, SHOWMODE_NUM, kShowModeStrings);
	topsizer->Add(modes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);

	AddTextboxWithCaption(this, sizer1, WESTHASH, wxT("West Hash"));
	AddTextboxWithCaption(this, sizer1, EASTHASH, wxT("East Hash"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	AddTextboxWithCaption(this, sizer1, BORDER_LEFT, wxT("Left Border"));
	AddTextboxWithCaption(this, sizer1, BORDER_TOP, wxT("Top Border"));
	AddTextboxWithCaption(this, sizer1, BORDER_RIGHT, wxT("Right Border"));
	AddTextboxWithCaption(this, sizer1, BORDER_BOTTOM, wxT("Bottom Border"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	AddTextboxWithCaption(this, sizer1, OFFSET_X, wxT("Offset X"));
	AddTextboxWithCaption(this, sizer1, OFFSET_Y, wxT("Offset Y"));
	AddTextboxWithCaption(this, sizer1, SIZE_X, wxT("Size X"));
	AddTextboxWithCaption(this, sizer1, SIZE_Y, wxT("Size Y"));

	TransferDataToWindow();
}

bool SetUpModes::TransferDataToWindow()
{
	wxChoice* modes = (wxChoice*) FindWindow(MODE_CHOICE);
	long values[10];
	GetConfigurationShowMode(modes->GetSelection(), values);

	for (size_t i = 0; i < 10; ++i)
	{
		wxString buf;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(WESTHASH + i);
		buf.Printf(wxT("%d"), values[i]);
		text->SetValue(buf);
	}
	return true;
}

bool SetUpModes::TransferDataFromWindow()
{
	wxChoice* modes = (wxChoice*) FindWindow(MODE_CHOICE);
	long values[10];

	for (size_t i = 0; i < 10; ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(WESTHASH + i);
		text->GetValue().ToLong(&val);
		values[i] = val;
	}

	SetConfigurationShowMode(modes->GetSelection(), values);
	return true;
}

class SetUpSpringModes : public wxPanel
{
	DECLARE_CLASS( SetUpSpringModes )
	DECLARE_EVENT_TABLE()

public:
	SetUpSpringModes( )
	{
		Init();
	}
	SetUpSpringModes( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Setup Modes"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU )
	{
		Init();
		Create(parent, id, caption, pos, size, style);
	}
	~SetUpSpringModes( ) {}

	void Init() {}

	bool Create(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Setup Modes"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

	// use these to get and set default values
	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();
private:
	void OnCmdReset(wxCommandEvent&);
};

enum
{
	SPRING_MODE_CHOICE = 1000,
	DISPLAY_YARDLINE_BELOW,
	DISPLAY_YARDLINE_ABOVE,
	DISPLAY_YARDLINE_RIGHT,
	DISPLAY_YARDLINE_LEFT,
	SPRING_BORDER_LEFT,
	SPRING_BORDER_TOP,
	SPRING_BORDER_RIGHT,
	SPRING_BORDER_BOTTOM,
	MODE_STEPS_X,
	MODE_STEPS_Y,
	MODE_STEPS_W,
	MODE_STEPS_H,
	EPS_STAGE_X,
	EPS_STAGE_Y,
	EPS_STAGE_W,
	EPS_STAGE_H,
	EPS_FIELD_X,
	EPS_FIELD_Y,
	EPS_FIELD_W,
	EPS_FIELD_H,
	EPS_TEXT_X,
	EPS_TEXT_Y,
	EPS_TEXT_W,
	EPS_TEXT_H,
};

BEGIN_EVENT_TABLE(SetUpSpringModes, wxPanel)
END_EVENT_TABLE()

IMPLEMENT_CLASS( SetUpSpringModes, wxPanel )

bool SetUpSpringModes::Create(wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	if (!wxPanel::Create(parent, id, pos, size, style, caption))
		return false;

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}

void SetUpSpringModes::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxChoice* modes = new wxChoice(this, SPRING_MODE_CHOICE, wxDefaultPosition, wxDefaultSize, SPRINGSHOWMODE_NUM, kSpringShowModeStrings);
	topsizer->Add(modes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_LEFT, wxT("Display Yardline Left")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_RIGHT, wxT("Display Yardline Right")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_ABOVE, wxT("Display Yardline Above")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_BELOW, wxT("Display Yardline Below")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	AddTextboxWithCaption(this, sizer1, SPRING_BORDER_LEFT, wxT("Left Border"));
	AddTextboxWithCaption(this, sizer1, SPRING_BORDER_TOP, wxT("Top Border"));
	AddTextboxWithCaption(this, sizer1, SPRING_BORDER_RIGHT, wxT("Right Border"));
	AddTextboxWithCaption(this, sizer1, SPRING_BORDER_BOTTOM, wxT("Bottom Border"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	AddTextboxWithCaption(this, sizer1, MODE_STEPS_X, wxT("Mode Offset X"));
	AddTextboxWithCaption(this, sizer1, MODE_STEPS_Y, wxT("Mode Offset Y"));
	AddTextboxWithCaption(this, sizer1, MODE_STEPS_W, wxT("Mode Offset W"));
	AddTextboxWithCaption(this, sizer1, MODE_STEPS_H, wxT("Mode Offset H"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	AddTextboxWithCaption(this, sizer1, EPS_STAGE_X, wxT("EPS Stage X"));
	AddTextboxWithCaption(this, sizer1, EPS_STAGE_Y, wxT("EPS Stage Y"));
	AddTextboxWithCaption(this, sizer1, EPS_STAGE_W, wxT("EPS Stage W"));
	AddTextboxWithCaption(this, sizer1, EPS_STAGE_H, wxT("EPS Stage H"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	AddTextboxWithCaption(this, sizer1, EPS_FIELD_X, wxT("EPS Field X"));
	AddTextboxWithCaption(this, sizer1, EPS_FIELD_Y, wxT("EPS Field Y"));
	AddTextboxWithCaption(this, sizer1, EPS_FIELD_W, wxT("EPS Field W"));
	AddTextboxWithCaption(this, sizer1, EPS_FIELD_H, wxT("EPS Field H"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	AddTextboxWithCaption(this, sizer1, EPS_TEXT_X, wxT("EPS Text X"));
	AddTextboxWithCaption(this, sizer1, EPS_TEXT_Y, wxT("EPS Text Y"));
	AddTextboxWithCaption(this, sizer1, EPS_TEXT_W, wxT("EPS Text W"));
	AddTextboxWithCaption(this, sizer1, EPS_TEXT_H, wxT("EPS Text H"));

	TransferDataToWindow();
}

bool SetUpSpringModes::TransferDataToWindow()
{
	wxChoice* modes = (wxChoice*) FindWindow(SPRING_MODE_CHOICE);
	long values[21];
	GetConfigurationSpringShowMode(modes->GetSelection(), values);

	for (size_t i = 0; i < 4; ++i)
	{
		wxCheckBox* checkbox = (wxCheckBox*) FindWindow(DISPLAY_YARDLINE_BELOW + i);
		checkbox->SetValue(values[0] & (1<<i));
	}
	for (size_t i = 0; i < 20; ++i)
	{
		wxString buf;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_BORDER_LEFT + i);
		buf.Printf(wxT("%d"), values[i+1]);
		text->SetValue(buf);
	}
	return true;
}

bool SetUpSpringModes::TransferDataFromWindow()
{
	wxChoice* modes = (wxChoice*) FindWindow(SPRING_MODE_CHOICE);
	long values[21];

	values[0] = 0;
	for (size_t i = 0; i < 4; ++i)
	{
		wxCheckBox* checkbox = (wxCheckBox*) FindWindow(DISPLAY_YARDLINE_BELOW + i);
		values[0] |= checkbox->IsChecked() ? (1<<i) : 0;
	}
	for (size_t i = 0; i < 20; ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_BORDER_LEFT + i);
		text->GetValue().ToLong(&val);
		values[i] = val;
	}

	SetConfigurationSpringShowMode(modes->GetSelection(), values);
	return true;
}

class YardlineSetup : public wxPanel
{
	DECLARE_CLASS( YardlineSetup )
	DECLARE_EVENT_TABLE()

public:
	YardlineSetup( )
	{
		Init();
	}
	YardlineSetup( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Yardline Setup"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU )
	{
		Init();
		Create(parent, id, caption, pos, size, style);
	}
	~YardlineSetup( ) {}

	void Init() {}

	bool Create(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Yardline Setup"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

	// use these to get and set default values
	virtual bool TransferDataToWindow();
private:
	void OnCmdReset(wxCommandEvent&);
};

enum
{
	SHOW_LINE_MARKING = 1000,
	SHOW_LINE_VALUE,
	SPRING_SHOW_LINE_MARKING,
	SPRING_SHOW_LINE_VALUE,
};

BEGIN_EVENT_TABLE(YardlineSetup, wxPanel)
END_EVENT_TABLE()

IMPLEMENT_CLASS( YardlineSetup, wxPanel )

bool YardlineSetup::Create(wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	if (!wxPanel::Create(parent, id, pos, size, style, caption))
		return false;

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}

void YardlineSetup::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );


	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);

	wxChoice* modes = new wxChoice(this, SHOW_LINE_MARKING, wxDefaultPosition, wxDefaultSize, MAX_YARD_LINES, yard_text_index);
	sizer1->Add(modes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	sizer1->Add(new wxTextCtrl(this, SHOW_LINE_VALUE), 0, wxGROW|wxALL, 5 );

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	modes = new wxChoice(this, SPRING_SHOW_LINE_MARKING, wxDefaultPosition, wxDefaultSize, MAX_SPR_LINES, spr_line_text_index);
	sizer1->Add(modes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	sizer1->Add(new wxTextCtrl(this, SPRING_SHOW_LINE_VALUE), 0, wxGROW|wxALL, 5 );

	TransferDataToWindow();
}

bool YardlineSetup::TransferDataToWindow()
{
	wxChoice* modes = (wxChoice*) FindWindow(SHOW_LINE_MARKING);
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(SHOW_LINE_VALUE);
	text->SetValue(yard_text[modes->GetSelection()]);
	modes = (wxChoice*) FindWindow(SPRING_SHOW_LINE_MARKING);
	text = (wxTextCtrl*) FindWindow(SPRING_SHOW_LINE_VALUE);
	text->SetValue(spr_line_text[modes->GetSelection()]);
	return true;
}


class GeneralSetup : public wxPanel
{
	DECLARE_CLASS( GeneralSetup )
	DECLARE_EVENT_TABLE()

public:
	GeneralSetup( )
	{
		Init();
	}
	GeneralSetup( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("General Setup"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU )
	{
		Init();
		Create(parent, id, caption, pos, size, style);
	}
	~GeneralSetup( ) {}

	void Init() {}

	bool Create(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("General Setup"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

	// use these to get and set default values
	virtual bool TransferDataToWindow();
private:
	void OnCmdReset(wxCommandEvent&);
};

enum
{
	AUTOSAVE_DIR = 1000,
	AUTOSAVE_INTERVAL
};

BEGIN_EVENT_TABLE(GeneralSetup, wxPanel)
END_EVENT_TABLE()

IMPLEMENT_CLASS( GeneralSetup, wxPanel )

bool GeneralSetup::Create(wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	if (!wxPanel::Create(parent, id, pos, size, style, caption))
		return false;

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}

void GeneralSetup::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxBoxSizer *sizer1 = new wxBoxSizer(wxVERTICAL);
	topsizer->Add(sizer1);

	AddTextboxWithCaption(this, sizer1, AUTOSAVE_DIR, wxT("Autosave Directory"));
	AddTextboxWithCaption(this, sizer1, AUTOSAVE_INTERVAL, wxT("Autosave Interval"));

	TransferDataToWindow();
}

bool GeneralSetup::TransferDataToWindow()
{
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(AUTOSAVE_DIR);
	text->SetValue(GetConfiguration_AutosaveDir());
	text = (wxTextCtrl*) FindWindow(AUTOSAVE_INTERVAL);
	wxString buf;
	buf.Printf(wxT("%d"), GetConfiguration_AutosaveInterval());
	text->SetValue(buf);
	return true;
}


BEGIN_EVENT_TABLE(CalChartPreferences, wxDialog)
END_EVENT_TABLE()

IMPLEMENT_CLASS( CalChartPreferences, wxDialog )

CalChartPreferences::CalChartPreferences()
{
	Init();
}

CalChartPreferences::CalChartPreferences( wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	Init();

	Create(parent, id, caption, pos, size, style);
}

CalChartPreferences::~CalChartPreferences()
{
}

void CalChartPreferences::Init()
{
}

bool CalChartPreferences::Create( wxWindow *parent,
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

void CalChartPreferences::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxNotebook* notebook = new wxNotebook(this, wxID_ANY);
	topsizer->Add(notebook, 0, wxALL, 5 );

	wxPanel* window0 = new GeneralSetup(notebook, wxID_ANY);
	notebook->AddPage(window0, wxT("General Preferences"));
	wxPanel* window1 = new ColorSelectDialog(notebook, wxID_ANY);
	notebook->AddPage(window1, wxT("Color Preferences"));
	wxPanel* window2 = new SetUpPrintingValues(notebook, wxID_ANY);
	notebook->AddPage(window2, wxT("PS Printing"));
	wxPanel* window3 = new SetUpModes(notebook, wxID_ANY);
	notebook->AddPage(window3, wxT("Mode Setup"));
	wxPanel* window4 = new SetUpSpringModes(notebook, wxID_ANY);
	notebook->AddPage(window4, wxT("Springshow Mode Setup"));
	wxPanel* window5 = new YardlineSetup(notebook, wxID_ANY);
	notebook->AddPage(window5, wxT("Yardline Setup"));
	
}
