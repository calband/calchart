/*
 * cc_preferences_ui.cpp
 * Dialox box for preferences
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

#include "cc_preferences_ui.h"
#include "confgr.h"
#include <wx/colordlg.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/listbook.h>

// how the preferences work:
// preference dialog will read system data on startup, cache a copy and then 
// when the user presses apply, they will be written back to the system.
// first page will be general settings:
//   Auto save behavior: file location, time
//   Color preferences
// second page is PS printing settings
// 3rd page is Show mode setup
//
// organized into pages.  Each page is responsible for reading out
// on TransferDataToWindow, caching the values locally, and
// setting them to the system on TransferDataFromWindow

// convience sizers to change the view behavior in all at once.
static wxSizerFlags sBasicSizerFlags;
static wxSizerFlags sLeftBasicSizerFlags;
static wxSizerFlags sExpandSizerFlags;

static void AddTextboxWithCaption(wxWindow* parent, wxBoxSizer* verticalsizer, int id, const wxString& caption)
{
	wxBoxSizer* textsizer = new wxBoxSizer( wxVERTICAL );
	textsizer->Add(new wxStaticText(parent, wxID_STATIC, caption, wxDefaultPosition, wxDefaultSize, 0), sLeftBasicSizerFlags);
	textsizer->Add(new wxTextCtrl(parent, id), sBasicSizerFlags );
	verticalsizer->Add(textsizer, sBasicSizerFlags);
}

// the basic class panel we use for all the pages:
class PreferencePage : public wxPanel
{
	DECLARE_ABSTRACT_CLASS( GeneralSetup )
public:
	PreferencePage()
	{
		Init();
	}
	virtual ~PreferencePage( ) {}
	virtual void Init() {}
	virtual bool Create(wxWindow *parent,
		wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style)
	{
		if (!wxPanel::Create(parent, id, pos, size, style, caption))
			return false;
		CreateControls();
		GetSizer()->Fit(this);
		GetSizer()->SetSizeHints(this);
		Center();
		return true;
	}

	virtual void CreateControls() = 0;

	// use these to get and set default values
	virtual bool TransferDataToWindow() = 0;
	virtual bool TransferDataFromWindow() = 0;
	virtual bool ClearValuesToDefault() = 0;

};

IMPLEMENT_ABSTRACT_CLASS( PreferencePage, wxPanel )


//////// General setup ////////
// handling autosave and colors
////////

class GeneralSetup : public PreferencePage
{
	DECLARE_CLASS( GeneralSetup )
	DECLARE_EVENT_TABLE()

public:
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
	virtual ~GeneralSetup( ) {}

	virtual void Init();
	virtual void CreateControls();

	// use these to get and set default values
	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();
	virtual bool ClearValuesToDefault();

private:
	void OnCmdSelectColors(wxCommandEvent&);
	void OnCmdSelectWidth(wxSpinEvent&);
	void OnCmdResetColors(wxCommandEvent&);
	void OnCmdResetAll(wxCommandEvent&);
	void OnCmdChooseNewColor(wxCommandEvent&);

	void SetColor(int selection, int width, const wxColour& color);
	wxBitmapComboBox* nameBox;
	wxSpinCtrl* spin;

	const wxPen *mCalChartPens[COLOR_NUM];
	const wxBrush *mCalChartBrushes[COLOR_NUM];

	wxString mAutoSave_Dir;
	wxString mAutoSave_Interval;
};

enum
{
	AUTOSAVE_DIR = 1000,
	AUTOSAVE_INTERVAL,
	BUTTON_SELECT,
	BUTTON_RESTORE,
	SPIN_WIDTH,
	NEW_COLOR_CHOICE
};

BEGIN_EVENT_TABLE(GeneralSetup, PreferencePage)
EVT_BUTTON(BUTTON_SELECT,GeneralSetup::OnCmdSelectColors)
EVT_BUTTON(BUTTON_RESTORE,GeneralSetup::OnCmdResetColors)
EVT_SPINCTRL(SPIN_WIDTH,GeneralSetup::OnCmdSelectWidth)
EVT_COMBOBOX(NEW_COLOR_CHOICE,GeneralSetup::OnCmdChooseNewColor)
END_EVENT_TABLE()

IMPLEMENT_CLASS( GeneralSetup, PreferencePage )

void GeneralSetup::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxStaticBoxSizer* boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Autosave settings")), wxVERTICAL);
	topsizer->Add(boxsizer);

	wxBoxSizer *sizer1 = new wxBoxSizer(wxVERTICAL);
	boxsizer->Add(sizer1, sLeftBasicSizerFlags);

	AddTextboxWithCaption(this, sizer1, AUTOSAVE_DIR, wxT("Autosave Directory"));
	AddTextboxWithCaption(this, sizer1, AUTOSAVE_INTERVAL, wxT("Autosave Interval"));

	boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Color settings")), wxVERTICAL);
	topsizer->Add(boxsizer);

	wxBoxSizer *horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	nameBox = new wxBitmapComboBox(this, NEW_COLOR_CHOICE, ColorNames[0], wxDefaultPosition, wxDefaultSize, COLOR_NUM, ColorNames, wxCB_READONLY|wxCB_DROPDOWN);	
	horizontalsizer->Add(nameBox, sBasicSizerFlags );
	
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

	spin = new wxSpinCtrl(this, SPIN_WIDTH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mCalChartPens[nameBox->GetSelection()]->GetWidth());
	spin->SetValue(mCalChartPens[nameBox->GetSelection()]->GetWidth());
	horizontalsizer->Add(spin, sBasicSizerFlags );
	boxsizer->Add(horizontalsizer, sLeftBasicSizerFlags );

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );

	horizontalsizer->Add(new wxButton(this, BUTTON_SELECT, wxT("&Change Color")), sBasicSizerFlags );
	horizontalsizer->Add(new wxButton(this, BUTTON_RESTORE, wxT("&Reset Color")), sBasicSizerFlags );

	boxsizer->Add(horizontalsizer, sBasicSizerFlags );

	TransferDataToWindow();
}

void GeneralSetup::Init()
{
	// first read out the defaults:
	for (size_t i = 0; i < COLOR_NUM; ++i)
	{
		mCalChartPens[i] = CalChartPens[i];
		mCalChartBrushes[i] = CalChartBrushes[i];
	}

	mAutoSave_Dir = GetConfiguration_AutosaveDir();
	mAutoSave_Interval.Printf(wxT("%d"), GetConfiguration_AutosaveInterval());
}

bool GeneralSetup::TransferDataToWindow()
{
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(AUTOSAVE_DIR);
	text->SetValue(mAutoSave_Dir);
	text = (wxTextCtrl*) FindWindow(AUTOSAVE_INTERVAL);
	text->SetValue(mAutoSave_Interval);
	return true;
}

bool GeneralSetup::TransferDataFromWindow()
{
	// read out the values from the window
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(AUTOSAVE_DIR);
	mAutoSave_Dir = text->GetValue();
	text = (wxTextCtrl*) FindWindow(AUTOSAVE_INTERVAL);
	mAutoSave_Interval = text->GetValue();

	// write out the values defaults:
	for (size_t i = 0; i < COLOR_NUM; ++i)
	{
		if (CalChartPens[i] != mCalChartPens[i] && CalChartBrushes[i] != mCalChartBrushes[i])
		{
			CalChartPens[i] = mCalChartPens[i];
			CalChartBrushes[i] = mCalChartBrushes[i];
			SetConfigColor(i);
		}
	}
	SetConfiguration_AutosaveDir(mAutoSave_Dir);
	long val;
	mAutoSave_Interval.ToLong(&val);
	SetConfiguration_AutosaveInterval(val);
	return true;
}

bool GeneralSetup::ClearValuesToDefault()
{
	for (int i = 0; i < COLOR_NUM; ++i)
	{
		SetColor(i, DefaultPenWidth[i], DefaultColors[i]);
		ClearConfigColor(i);
	}
	ClearConfiguration_AutosaveDir();
	ClearConfiguration_AutosaveInterval();
	Init();
	TransferDataToWindow();
	return true;
}

void GeneralSetup::SetColor(int selection, int width, const wxColour& color)
{
	mCalChartPens[selection] = wxThePenList->FindOrCreatePen(color, width, wxSOLID);
	mCalChartBrushes[selection] = wxTheBrushList->FindOrCreateBrush(color, wxSOLID);

	// update the namebox list
	{
		wxBitmap test_bitmap(16, 16);
		wxMemoryDC temp_dc;
		temp_dc.SelectObject(test_bitmap);
		temp_dc.SetBackground(*mCalChartBrushes[selection]);
		temp_dc.Clear();
		nameBox->SetItemBitmap(selection, test_bitmap);
	}
}

void GeneralSetup::OnCmdSelectColors(wxCommandEvent&)
{
	int selection = nameBox->GetSelection();
	wxColourData data;
	data.SetChooseFull(true);
	const wxBrush* brush = mCalChartBrushes[selection];
	data.SetColour(brush->GetColour());
	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxColourData retdata = dialog.GetColourData();
		wxColour c = retdata.GetColour();
		SetColor(selection, mCalChartPens[selection]->GetWidth(), c);
	}
}

void GeneralSetup::OnCmdSelectWidth(wxSpinEvent& e)
{
	int selection = nameBox->GetSelection();
	SetColor(selection, e.GetPosition(), mCalChartPens[selection]->GetColour());
}

void GeneralSetup::OnCmdResetColors(wxCommandEvent&)
{
	int selection = nameBox->GetSelection();
	SetColor(selection, DefaultPenWidth[selection], DefaultColors[selection]);
	ClearConfigColor(selection);
}

void GeneralSetup::OnCmdChooseNewColor(wxCommandEvent&)
{
	spin->SetValue(mCalChartPens[nameBox->GetSelection()]->GetWidth());
}


//////// General setup ////////
// setup pringing values and colors
////////

class PSPrintingSetUp : public PreferencePage
{
	DECLARE_CLASS( PSPrintingSetUp )
	DECLARE_EVENT_TABLE()

public:
	PSPrintingSetUp( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Printing Values"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU )
	{
		Init();
		Create(parent, id, caption, pos, size, style);
	}
	virtual ~PSPrintingSetUp( ) {}

	virtual void Init();
	virtual void CreateControls();

	// use these to get and set default values
	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();
	virtual bool ClearValuesToDefault();
private:
	wxString mFontNames[7];
	double mPrintValues[8];
};


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
} PSPrintingSetUp_IDs;


BEGIN_EVENT_TABLE(PSPrintingSetUp, PreferencePage)
END_EVENT_TABLE()

IMPLEMENT_CLASS( PSPrintingSetUp, PreferencePage )

void PSPrintingSetUp::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxBoxSizer *horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	topsizer->Add(horizontalsizer, sLeftBasicSizerFlags );

	AddTextboxWithCaption(this, horizontalsizer, HEADFONT, wxT("Header Font:"));
	AddTextboxWithCaption(this, horizontalsizer, MAINFONT, wxT("Main Font:"));
	AddTextboxWithCaption(this, horizontalsizer, NUMBERFONT, wxT("Number Font:"));
	AddTextboxWithCaption(this, horizontalsizer, CONTFONT, wxT("Continuity Font:"));

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	topsizer->Add(horizontalsizer, sLeftBasicSizerFlags );

	AddTextboxWithCaption(this, horizontalsizer, BOLDFONT, wxT("Bold Font:"));
	AddTextboxWithCaption(this, horizontalsizer, ITALFONT, wxT("Italic Font:"));
	AddTextboxWithCaption(this, horizontalsizer, BOLDITALFONT, wxT("Bold Italic Font:"));

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	topsizer->Add(horizontalsizer, sLeftBasicSizerFlags );

	AddTextboxWithCaption(this, horizontalsizer, HEADERSIZE, wxT("Header Size:"));
	AddTextboxWithCaption(this, horizontalsizer, YARDSSIZE, wxT("Yards Side:"));
	AddTextboxWithCaption(this, horizontalsizer, TEXTSIZE, wxT("Text Side:"));

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	topsizer->Add(horizontalsizer, sLeftBasicSizerFlags );

	AddTextboxWithCaption(this, horizontalsizer, DOTRATIO, wxT("Dot Ratio:"));
	AddTextboxWithCaption(this, horizontalsizer, NUMRATIO, wxT("Num Ratio:"));
	AddTextboxWithCaption(this, horizontalsizer, PLINERATIO, wxT("P-Line Ratio:"));
	AddTextboxWithCaption(this, horizontalsizer, SLINERATIO, wxT("S-Line Ratio:"));
	AddTextboxWithCaption(this, horizontalsizer, CONTRATIO, wxT("Continuity Ratio:"));

	TransferDataToWindow();
}

void PSPrintingSetUp::Init()
{
	mFontNames[0] = GetConfiguration_HeadFont();
	mFontNames[1] = GetConfiguration_MainFont();
	mFontNames[2] = GetConfiguration_NumberFont();
	mFontNames[3] = GetConfiguration_ContFont();
	mFontNames[4] = GetConfiguration_BoldFont();
	mFontNames[5] = GetConfiguration_ItalFont();
	mFontNames[6] = GetConfiguration_BoldItalFont();
	mPrintValues[0] = GetConfiguration_HeaderSize();
	mPrintValues[1] = GetConfiguration_YardsSize();
	mPrintValues[2] = GetConfiguration_TextSize();
	mPrintValues[3] = GetConfiguration_DotRatio();
	mPrintValues[4] = GetConfiguration_NumRatio();
	mPrintValues[5] = GetConfiguration_PLineRatio();
	mPrintValues[6] = GetConfiguration_SLineRatio();
	mPrintValues[7] = GetConfiguration_ContRatio();
}

bool PSPrintingSetUp::TransferDataToWindow()
{
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(HEADFONT);
	text->SetValue(mFontNames[0]);
	text = (wxTextCtrl*) FindWindow(MAINFONT);
	text->SetValue(mFontNames[1]);
	text = (wxTextCtrl*) FindWindow(NUMBERFONT);
	text->SetValue(mFontNames[2]);
	text = (wxTextCtrl*) FindWindow(CONTFONT);
	text->SetValue(mFontNames[3]);
	text = (wxTextCtrl*) FindWindow(BOLDFONT);
	text->SetValue(mFontNames[4]);
	text = (wxTextCtrl*) FindWindow(ITALFONT);
	text->SetValue(mFontNames[5]);
	text = (wxTextCtrl*) FindWindow(BOLDITALFONT);
	text->SetValue(mFontNames[6]);
	text = (wxTextCtrl*) FindWindow(HEADERSIZE);
	wxString buf;
	buf.Printf(wxT("%.2f"), mPrintValues[0]);
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(YARDSSIZE);
	buf.Printf(wxT("%.2f"), mPrintValues[1]);
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(TEXTSIZE);
	buf.Printf(wxT("%.2f"), mPrintValues[2]);
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(DOTRATIO);
	buf.Printf(wxT("%.2f"), mPrintValues[3]);
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(NUMRATIO);
	buf.Printf(wxT("%.2f"), mPrintValues[4]);
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(PLINERATIO);
	buf.Printf(wxT("%.2f"), mPrintValues[5]);
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(SLINERATIO);
	buf.Printf(wxT("%.2f"), mPrintValues[6]);
	text->SetValue(buf);
	text = (wxTextCtrl*) FindWindow(CONTRATIO);
	buf.Printf(wxT("%.2f"), mPrintValues[7]);
	text->SetValue(buf);
	return true;
}

bool PSPrintingSetUp::TransferDataFromWindow()
{
	// read out the values from the window
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(HEADFONT);
	mFontNames[0] = text->GetValue();
	text = (wxTextCtrl*) FindWindow(MAINFONT);
	mFontNames[1] = text->GetValue();
	text = (wxTextCtrl*) FindWindow(NUMBERFONT);
	mFontNames[2] = text->GetValue();
	text = (wxTextCtrl*) FindWindow(CONTFONT);
	mFontNames[3] = text->GetValue();
	text = (wxTextCtrl*) FindWindow(BOLDFONT);
	mFontNames[4] = text->GetValue();
	text = (wxTextCtrl*) FindWindow(ITALFONT);
	mFontNames[5] = text->GetValue();
	text = (wxTextCtrl*) FindWindow(BOLDITALFONT);
	mFontNames[6] = text->GetValue();
	text = (wxTextCtrl*) FindWindow(HEADERSIZE);
	text->GetValue().ToDouble(&mPrintValues[0]);
	text = (wxTextCtrl*) FindWindow(YARDSSIZE);
	text->GetValue().ToDouble(&mPrintValues[1]);
	text = (wxTextCtrl*) FindWindow(TEXTSIZE);
	text->GetValue().ToDouble(&mPrintValues[2]);
	text = (wxTextCtrl*) FindWindow(DOTRATIO);
	text->GetValue().ToDouble(&mPrintValues[3]);
	text = (wxTextCtrl*) FindWindow(NUMRATIO);
	text->GetValue().ToDouble(&mPrintValues[4]);
	text = (wxTextCtrl*) FindWindow(PLINERATIO);
	text->GetValue().ToDouble(&mPrintValues[5]);
	text = (wxTextCtrl*) FindWindow(SLINERATIO);
	text->GetValue().ToDouble(&mPrintValues[6]);
	text = (wxTextCtrl*) FindWindow(CONTRATIO);
	text->GetValue().ToDouble(&mPrintValues[7]);

	// write out the values defaults:
	SetConfiguration_HeadFont(mFontNames[0]);
	SetConfiguration_MainFont(mFontNames[1]);
	SetConfiguration_NumberFont(mFontNames[2]);
	SetConfiguration_ContFont(mFontNames[3]);
	SetConfiguration_BoldFont(mFontNames[4]);
	SetConfiguration_ItalFont(mFontNames[5]);
	SetConfiguration_BoldItalFont(mFontNames[6]);
	SetConfiguration_HeaderSize(mPrintValues[0]);
	SetConfiguration_YardsSize(mPrintValues[1]);
	SetConfiguration_TextSize(mPrintValues[2]);
	SetConfiguration_DotRatio(mPrintValues[3]);
	SetConfiguration_NumRatio(mPrintValues[4]);
	SetConfiguration_PLineRatio(mPrintValues[5]);
	SetConfiguration_SLineRatio(mPrintValues[6]);
	SetConfiguration_ContRatio(mPrintValues[7]);
	return true;
}

bool PSPrintingSetUp::ClearValuesToDefault()
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
	Init();
	return TransferDataToWindow();
}


//////// Show Mode setup ////////
// setup pringing values and colors
////////

class ShowModeSetup : public PreferencePage
{
	DECLARE_CLASS( ShowModeSetup )
	DECLARE_EVENT_TABLE()

public:
	ShowModeSetup( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Setup Modes"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU )
	{
		Init();
		Create(parent, id, caption, pos, size, style);
	}
	~ShowModeSetup( ) {}

	virtual void Init();
	virtual void CreateControls();

	// use these to get and set default values
	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();
	virtual bool ClearValuesToDefault();
private:
	void OnCmdLineText(wxCommandEvent&);
	void OnCmdChoice(wxCommandEvent&);
	long mShowModeValues[SHOWMODE_NUM][kShowModeValues];
	wxString mYardText[MAX_YARD_LINES];
	int mWhichMode;
	int mWhichYardLine;
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
	SIZE_Y,
	SHOW_LINE_MARKING,
	SHOW_LINE_VALUE,
};

BEGIN_EVENT_TABLE(ShowModeSetup, PreferencePage)
EVT_CHOICE(MODE_CHOICE, ShowModeSetup::OnCmdChoice)
EVT_CHOICE(SHOW_LINE_MARKING, ShowModeSetup::OnCmdChoice)
END_EVENT_TABLE()

IMPLEMENT_CLASS( ShowModeSetup, PreferencePage )

void ShowModeSetup::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxChoice* modes = new wxChoice(this, MODE_CHOICE, wxDefaultPosition, wxDefaultSize, SHOWMODE_NUM, kShowModeStrings);
	topsizer->Add(modes, sLeftBasicSizerFlags );

	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);

	AddTextboxWithCaption(this, sizer1, WESTHASH, wxT("West Hash"));
	AddTextboxWithCaption(this, sizer1, EASTHASH, wxT("East Hash"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	AddTextboxWithCaption(this, sizer1, BORDER_LEFT, wxT("Left Border"));
	AddTextboxWithCaption(this, sizer1, BORDER_TOP, wxT("Top Border"));
	AddTextboxWithCaption(this, sizer1, BORDER_RIGHT, wxT("Right Border"));
	AddTextboxWithCaption(this, sizer1, BORDER_BOTTOM, wxT("Bottom Border"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	AddTextboxWithCaption(this, sizer1, OFFSET_X, wxT("Offset X"));
	AddTextboxWithCaption(this, sizer1, OFFSET_Y, wxT("Offset Y"));
	AddTextboxWithCaption(this, sizer1, SIZE_X, wxT("Size X"));
	AddTextboxWithCaption(this, sizer1, SIZE_Y, wxT("Size Y"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	wxBoxSizer* textsizer = new wxBoxSizer( wxHORIZONTAL );
	sizer1->Add(textsizer, sBasicSizerFlags );
	textsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Adjust yardline marker"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	textsizer->Add(new wxChoice(this, SHOW_LINE_MARKING, wxDefaultPosition, wxDefaultSize, MAX_YARD_LINES, yard_text_index));
	textsizer->Add(new wxTextCtrl(this, SHOW_LINE_VALUE), sBasicSizerFlags );

	TransferDataToWindow();
}

void ShowModeSetup::Init()
{
	mWhichMode = 0;
	mWhichYardLine = 0;
	for (size_t i = 0; i < SHOWMODE_NUM; ++i)
	{
		GetConfigurationShowMode(i, mShowModeValues[i]);
	}
	for (size_t i = 0; i < MAX_YARD_LINES; ++i)
	{
		mYardText[i] = yard_text[i];
	}
}

bool ShowModeSetup::TransferDataToWindow()
{
	// standard show
	for (size_t i = 0; i < kShowModeValues; ++i)
	{
		wxString buf;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(WESTHASH + i);
		buf.Printf(wxT("%d"), mShowModeValues[mWhichMode][i]);
		text->SetValue(buf);
	}

	wxTextCtrl* text = (wxTextCtrl*) FindWindow(SHOW_LINE_VALUE);
	text->SetValue(mYardText[mWhichYardLine]);
	return true;
}

bool ShowModeSetup::TransferDataFromWindow()
{
	// read out the values from the window
	// standard show
	for (size_t i = 0; i < kShowModeValues; ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(WESTHASH + i);
		text->GetValue().ToLong(&val);
		mShowModeValues[mWhichMode][i] = val;
	}
	// write out the values defaults:
	for (size_t i = 0; i < SHOWMODE_NUM; ++i)
	{
		SetConfigurationShowMode(i, mShowModeValues[i]);
	}

	for (size_t i = 0; i < MAX_YARD_LINES; ++i)
	{
		yard_text[i] = mYardText[i];
	}
	SetConfigShowYardline();

	return true;
}

bool ShowModeSetup::ClearValuesToDefault()
{
	for (size_t i = 0; i < SHOWMODE_NUM; ++i)
	{
		ClearConfigurationShowMode(i);
	}
	ClearConfigShowYardline();
	Init();
	return TransferDataToWindow();
}

void ShowModeSetup::OnCmdChoice(wxCommandEvent&)
{
	// save off all the old values:
	for (size_t i = 0; i < kShowModeValues; ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(WESTHASH + i);
		text->GetValue().ToLong(&val);
		mShowModeValues[mWhichMode][i] = val;
	}
	wxChoice* modes = (wxChoice*) FindWindow(MODE_CHOICE);
	mWhichMode = modes->GetSelection();

	wxTextCtrl* text = (wxTextCtrl*) FindWindow(SHOW_LINE_VALUE);
	mYardText[mWhichYardLine] = text->GetValue();
	modes = (wxChoice*) FindWindow(SHOW_LINE_MARKING);
	mWhichYardLine = modes->GetSelection();
	TransferDataToWindow();
}

//////// Spring Show Mode setup ////////
// setup pringing values and colors
////////

class SpringShowModeSetup : public PreferencePage
{
	DECLARE_CLASS( SpringShowModeSetup )
	DECLARE_EVENT_TABLE()

public:
	SpringShowModeSetup( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Setup Modes"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU )
	{
		Init();
		Create(parent, id, caption, pos, size, style);
	}
	~SpringShowModeSetup( ) {}

	virtual void Init();
	virtual void CreateControls();

	// use these to get and set default values
	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();
	virtual bool ClearValuesToDefault();
private:
	void OnCmdChoice(wxCommandEvent&);
	long mSpringShowModeValues[SPRINGSHOWMODE_NUM][kSpringShowModeValues];
	wxString mYardText[MAX_SPR_LINES];
	int mWhichMode;
	int mWhichYardLine;
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
	SPRING_SHOW_LINE_MARKING,
	SPRING_SHOW_LINE_VALUE,
};

BEGIN_EVENT_TABLE(SpringShowModeSetup, PreferencePage)
EVT_CHOICE(SPRING_MODE_CHOICE, SpringShowModeSetup::OnCmdChoice)
EVT_CHOICE(SPRING_SHOW_LINE_MARKING, SpringShowModeSetup::OnCmdChoice)
END_EVENT_TABLE()

IMPLEMENT_CLASS( SpringShowModeSetup, PreferencePage )

void SpringShowModeSetup::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxChoice* modes = new wxChoice(this, SPRING_MODE_CHOICE, wxDefaultPosition, wxDefaultSize, SPRINGSHOWMODE_NUM, kSpringShowModeStrings);
	topsizer->Add(modes, sLeftBasicSizerFlags );

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1);
	sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_LEFT, wxT("Display Yardline Left")), sBasicSizerFlags );
	sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_RIGHT, wxT("Display Yardline Right")), sBasicSizerFlags );
	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_ABOVE, wxT("Display Yardline Above")), sBasicSizerFlags );
	sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_BELOW, wxT("Display Yardline Below")), sBasicSizerFlags );

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	AddTextboxWithCaption(this, sizer1, SPRING_BORDER_LEFT, wxT("Left Border"));
	AddTextboxWithCaption(this, sizer1, SPRING_BORDER_TOP, wxT("Top Border"));
	AddTextboxWithCaption(this, sizer1, SPRING_BORDER_RIGHT, wxT("Right Border"));
	AddTextboxWithCaption(this, sizer1, SPRING_BORDER_BOTTOM, wxT("Bottom Border"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	AddTextboxWithCaption(this, sizer1, MODE_STEPS_X, wxT("Mode Offset X"));
	AddTextboxWithCaption(this, sizer1, MODE_STEPS_Y, wxT("Mode Offset Y"));
	AddTextboxWithCaption(this, sizer1, MODE_STEPS_W, wxT("Mode Offset W"));
	AddTextboxWithCaption(this, sizer1, MODE_STEPS_H, wxT("Mode Offset H"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	AddTextboxWithCaption(this, sizer1, EPS_STAGE_X, wxT("EPS Stage X"));
	AddTextboxWithCaption(this, sizer1, EPS_STAGE_Y, wxT("EPS Stage Y"));
	AddTextboxWithCaption(this, sizer1, EPS_STAGE_W, wxT("EPS Stage W"));
	AddTextboxWithCaption(this, sizer1, EPS_STAGE_H, wxT("EPS Stage H"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	AddTextboxWithCaption(this, sizer1, EPS_FIELD_X, wxT("EPS Field X"));
	AddTextboxWithCaption(this, sizer1, EPS_FIELD_Y, wxT("EPS Field Y"));
	AddTextboxWithCaption(this, sizer1, EPS_FIELD_W, wxT("EPS Field W"));
	AddTextboxWithCaption(this, sizer1, EPS_FIELD_H, wxT("EPS Field H"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	AddTextboxWithCaption(this, sizer1, EPS_TEXT_X, wxT("EPS Text X"));
	AddTextboxWithCaption(this, sizer1, EPS_TEXT_Y, wxT("EPS Text Y"));
	AddTextboxWithCaption(this, sizer1, EPS_TEXT_W, wxT("EPS Text W"));
	AddTextboxWithCaption(this, sizer1, EPS_TEXT_H, wxT("EPS Text H"));

	sizer1 = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer1, sLeftBasicSizerFlags);
	wxBoxSizer* textsizer = new wxBoxSizer( wxHORIZONTAL );
	sizer1->Add(textsizer, sBasicSizerFlags );
	textsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Adjust yardline marker"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	textsizer->Add(new wxChoice(this, SPRING_SHOW_LINE_MARKING, wxDefaultPosition, wxDefaultSize, MAX_SPR_LINES, spr_line_text_index));
	textsizer->Add(new wxTextCtrl(this, SPRING_SHOW_LINE_VALUE), sBasicSizerFlags );

	TransferDataToWindow();
}

void SpringShowModeSetup::Init()
{
	mWhichMode = 0;
	mWhichYardLine = 0;
	for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i)
	{
		GetConfigurationSpringShowMode(i, mSpringShowModeValues[i]);
	}
	for (size_t i = 0; i < MAX_SPR_LINES; ++i)
	{
		mYardText[i] = spr_line_text[i];
	}
}

bool SpringShowModeSetup::TransferDataToWindow()
{
	// standard show
	for (size_t i = 0; i < 4; ++i)
	{
		wxCheckBox* checkbox = (wxCheckBox*) FindWindow(DISPLAY_YARDLINE_BELOW + i);
		checkbox->SetValue(mSpringShowModeValues[mWhichMode][0] & (1<<i));
	}
	for (size_t i = 1; i < kSpringShowModeValues; ++i)
	{
		wxString buf;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_BORDER_LEFT + i - 1);
		buf.Printf(wxT("%d"), mSpringShowModeValues[mWhichMode][i]);
		text->SetValue(buf);
	}

	wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_SHOW_LINE_VALUE);
	text->SetValue(spr_line_text[mWhichYardLine]);
	return true;
}

bool SpringShowModeSetup::TransferDataFromWindow()
{
	// read out the values from the window
	// spring show
	wxChoice* modes = (wxChoice*) FindWindow(SPRING_MODE_CHOICE);

	mSpringShowModeValues[mWhichMode][0] = 0;
	for (size_t i = 0; i < 4; ++i)
	{
		wxCheckBox* checkbox = (wxCheckBox*) FindWindow(DISPLAY_YARDLINE_BELOW + i);
		mSpringShowModeValues[mWhichMode][0] |= checkbox->IsChecked() ? (1<<i) : 0;
	}
	for (size_t i = 1; i < kSpringShowModeValues; ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_BORDER_LEFT + i - 1);
		text->GetValue().ToLong(&val);
		mSpringShowModeValues[mWhichMode][i] = val;
	}

	// write out the values defaults:
	for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i)
	{
		SetConfigurationSpringShowMode(modes->GetSelection(), mSpringShowModeValues[mWhichMode]);
	}

	for (size_t i = 0; i < MAX_SPR_LINES; ++i)
	{
		spr_line_text[i] = mYardText[i];
	}
	SetConfigSpringShowYardline();

	return true;
}

bool SpringShowModeSetup::ClearValuesToDefault()
{
	for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i)
	{
		ClearConfigurationSpringShowMode(i);
	}
	ClearConfigSpringShowYardline();
	Init();
	return TransferDataToWindow();
}

void SpringShowModeSetup::OnCmdChoice(wxCommandEvent&)
{
	// save off all the old values:
	mSpringShowModeValues[mWhichMode][0] = 0;
	for (size_t i = 0; i < 4; ++i)
	{
		wxCheckBox* checkbox = (wxCheckBox*) FindWindow(DISPLAY_YARDLINE_BELOW + i);
		mSpringShowModeValues[mWhichMode][0] |= checkbox->IsChecked() ? (1<<i) : 0;
	}
	for (size_t i = 1; i < kSpringShowModeValues; ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_BORDER_LEFT + i - 1);
		text->GetValue().ToLong(&val);
		mSpringShowModeValues[mWhichMode][i] = val;
	}
	wxChoice* modes = (wxChoice*) FindWindow(SPRING_MODE_CHOICE);
	mWhichMode = modes->GetSelection();

	wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_SHOW_LINE_VALUE);
	mYardText[mWhichYardLine] = text->GetValue();
	modes = (wxChoice*) FindWindow(SPRING_SHOW_LINE_MARKING);
	mWhichYardLine = modes->GetSelection();
	TransferDataToWindow();
}


BEGIN_EVENT_TABLE(CalChartPreferences, wxDialog)
EVT_BUTTON(wxID_RESET,CalChartPreferences::OnCmdResetAll)
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
    sBasicSizerFlags.Border(wxALL, 2).Center().Proportion(0);
	sLeftBasicSizerFlags.Border(wxALL, 2).Left().Proportion(0);
    sExpandSizerFlags.Border(wxALL, 2).Center().Proportion(0);
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

	mNotebook = new wxNotebook(this, wxID_ANY);
	topsizer->Add(mNotebook, sBasicSizerFlags );

	wxPanel* window0 = new GeneralSetup(mNotebook, wxID_ANY);
	mNotebook->AddPage(window0, wxT("General"));
	wxPanel* window2 = new PSPrintingSetUp(mNotebook, wxID_ANY);
	mNotebook->AddPage(window2, wxT("PS Printing"));
	wxPanel* window3 = new ShowModeSetup(mNotebook, wxID_ANY);
	mNotebook->AddPage(window3, wxT("Show Mode Setup"));
	wxPanel* window4 = new SpringShowModeSetup(mNotebook, wxID_ANY);
	mNotebook->AddPage(window4, wxT("SpringShow Mode Setup"));
	
	// the buttons on the bottom
	wxBoxSizer *okCancelBox = new wxBoxSizer( wxHORIZONTAL );
	topsizer->Add(okCancelBox, sBasicSizerFlags);

	okCancelBox->Add(new wxButton(this, wxID_APPLY), sBasicSizerFlags );
	okCancelBox->Add(new wxButton(this, wxID_RESET, wxT("&Reset All")), sBasicSizerFlags );
	okCancelBox->Add(new wxButton(this, wxID_OK), sBasicSizerFlags );
	okCancelBox->Add(new wxButton(this, wxID_CANCEL), sBasicSizerFlags );
}

bool CalChartPreferences::TransferDataToWindow()
{
	return true;
}

bool CalChartPreferences::TransferDataFromWindow()
{
	// transfer everything to the config...
	size_t pages = mNotebook->GetPageCount();
	for (size_t i = 0; i < pages; ++i)
	{
		PreferencePage* page = static_cast<PreferencePage*>(mNotebook->GetPage(i));
		page->TransferDataFromWindow();
	}
	wxMessageBox(wxT("If you change any preferences, you may have restart CalChart to have them take effect"), wxT("Restart CalChart"));
	return true;
}

void CalChartPreferences::OnCmdResetAll(wxCommandEvent&)
{
	// transfer everything to the config...
	size_t pages = mNotebook->GetPageCount();
	for (size_t i = 0; i < pages; ++i)
	{
		PreferencePage* page = static_cast<PreferencePage*>(mNotebook->GetPage(i));
		page->ClearValuesToDefault();
	}
}
