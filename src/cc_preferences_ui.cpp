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
#include "cc_drawcommand.h"
#include "confgr.h"
#include "modes.h"
#include "draw.h"
#include "cc_sheet.h"
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
				 long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU ) :
	mConfig(GetConfig())
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

	wxPen mCalChartPens[COLOR_NUM];
	wxBrush mCalChartBrushes[COLOR_NUM];

	wxString mAutoSave_Interval;
	CalChartConfiguration mConfig;

};

enum
{
	AUTOSAVE_INTERVAL = 1000,
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
#include "basic_ui.h"
#include "calchartdoc.h"

class PrefCanvas : public ClickDragCtrlScrollCanvas
{
	DECLARE_EVENT_TABLE()
	using super = ClickDragCtrlScrollCanvas;
public:
	PrefCanvas(wxWindow *parent, CalChartConfiguration& config);

	void OnPaint(wxPaintEvent& event);
	void PaintBackground(wxDC& dc);
	void OnEraseBackground(wxEraseEvent& event);
	std::unique_ptr<CC_show> mShow;
	std::unique_ptr<const ShowMode> mMode;
	CC_coord mOffset;
	CalChartConfiguration& mConfig;

};

BEGIN_EVENT_TABLE(PrefCanvas, ClickDragCtrlScrollCanvas)
EVT_PAINT(PrefCanvas::OnPaint)
EVT_ERASE_BACKGROUND(PrefCanvas::OnEraseBackground)
END_EVENT_TABLE()

void GeneralSetup::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxStaticBoxSizer* boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Autosave settings")), wxVERTICAL);
	topsizer->Add(boxsizer);

	wxBoxSizer *sizer1 = new wxBoxSizer(wxVERTICAL);
	boxsizer->Add(sizer1, sLeftBasicSizerFlags);

	AddTextboxWithCaption(this, sizer1, AUTOSAVE_INTERVAL, wxT("Autosave Interval"));

	boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Color settings")), wxVERTICAL);
	topsizer->Add(boxsizer);

	wxBoxSizer *horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	nameBox = new wxBitmapComboBox(this, NEW_COLOR_CHOICE, mConfig.GetColorNames().at(0), wxDefaultPosition, wxDefaultSize, COLOR_NUM, mConfig.GetColorNames().data(), wxCB_READONLY|wxCB_DROPDOWN);
	horizontalsizer->Add(nameBox, sBasicSizerFlags );
	
	for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i)+1))
	{
		wxBitmap temp_bitmap(16, 16);
		wxMemoryDC temp_dc;
		temp_dc.SelectObject(temp_bitmap);
		temp_dc.SetBackground(mConfig.Get_CalChartBrushAndPen(i).first);
		temp_dc.Clear();
		nameBox->SetItemBitmap(i, temp_bitmap);
	}
	nameBox->SetSelection(0);

	spin = new wxSpinCtrl(this, SPIN_WIDTH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mCalChartPens[nameBox->GetSelection()].GetWidth());
	spin->SetValue(mCalChartPens[nameBox->GetSelection()].GetWidth());
	horizontalsizer->Add(spin, sBasicSizerFlags );
	boxsizer->Add(horizontalsizer, sLeftBasicSizerFlags );

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );

	horizontalsizer->Add(new wxButton(this, BUTTON_SELECT, wxT("&Change Color")), sBasicSizerFlags );
	horizontalsizer->Add(new wxButton(this, BUTTON_RESTORE, wxT("&Reset Color")), sBasicSizerFlags );

	boxsizer->Add(horizontalsizer, sBasicSizerFlags );

	auto prefCanvas = new PrefCanvas(this, mConfig);
	// set scroll rate 1 to 1, so we can have even scrolling of whole field
	topsizer->Add(prefCanvas, 1, wxEXPAND);
//	mCanvas->SetScrollRate(1, 1);

	TransferDataToWindow();
}

PrefCanvas::PrefCanvas(wxWindow *parent, CalChartConfiguration& config) :
super(parent),
mMode(new ShowModeStandard(wxT(""), CC_coord(Int2Coord(160), Int2Coord(84)), CC_coord(Int2Coord(80), Int2Coord(42)), CC_coord(Int2Coord(4), Int2Coord(4)), CC_coord(Int2Coord(4), Int2Coord(4)), Int2Coord(32), Int2Coord(52) )),

mOffset(mMode->FieldOffset()),
mConfig(config)
{
	mShow = CC_show::Create_CC_show();
	mShow->SetupNewShow();
	mShow->SetNumPoints(4, 4, mOffset);
	mShow->SetPointLabel(std::vector<std::string>{
		"unsel",
		"unsel",
		"sel",
		"sel",
	});
	CC_show::CC_sheet_iterator_t sheet = mShow->GetCurrentSheet();
	sheet->GetPoint(0).SetSymbol(SYMBOL_X);
	sheet->GetPoint(1).SetSymbol(SYMBOL_SOLX);
	sheet->GetPoint(2).SetSymbol(SYMBOL_X);
	sheet->GetPoint(3).SetSymbol(SYMBOL_SOLX);
	
	for (auto i = 0; i < 4; ++i)
	{
		sheet->SetAllPositions(mOffset + CC_coord(Int2Coord(i*4), Int2Coord(2)), i);
		sheet->SetPosition(mOffset + CC_coord(Int2Coord(i*4), Int2Coord(6)), i, 1);
	}
}


// Define the repainting behaviour
void
PrefCanvas::OnPaint(wxPaintEvent& event)
{
	wxBufferedPaintDC dc(this);
	PrepareDC(dc);
	
	// draw the background
	PaintBackground(dc);

	dc.SetPen(mConfig.Get_CalChartBrushAndPen(COLOR_FIELD_DETAIL).second);
	dc.SetTextForeground(mConfig.Get_CalChartBrushAndPen(COLOR_FIELD_TEXT).second.GetColour());
	mMode->Draw(dc, mConfig);
	CC_show::const_CC_sheet_iterator_t sheet = mShow->GetCurrentSheet();

	SelectionList list;
	list.insert(2);
	list.insert(3);

	DrawPoints(dc, mConfig, mMode->Offset(), list, mShow->GetNumPoints(), mShow->GetPointLabels(), *sheet, 0, true);
	DrawPoints(dc, mConfig, mMode->Offset(), list, mShow->GetNumPoints(), mShow->GetPointLabels(), *sheet, 1, false);

	auto offset = mMode->FieldOffset();
	auto point_start = mMode->Offset() + offset + CC_coord(Int2Coord(4), Int2Coord(2));
	auto point_end = point_start + CC_coord(Int2Coord(0), Int2Coord(2));
	std::vector<CC_DrawCommand> cmds;
	cmds.push_back(CC_DrawCommand(point_start, point_end));
	point_start = point_end;
	point_end += CC_coord(Int2Coord(10), Int2Coord(0));
	cmds.push_back(CC_DrawCommand(point_start, point_end));
	
//	struct CC_DrawCommand

	DrawPath(dc, mConfig, cmds, point_end);

}

void
PrefCanvas::PaintBackground(wxDC& dc)
{
	// draw the background
	dc.SetBackgroundMode(wxTRANSPARENT);
	dc.SetBackground(mConfig.Get_CalChartBrushAndPen(COLOR_FIELD).first);
	dc.Clear();
}

// We have a empty erase background to improve redraw performance.
void
PrefCanvas::OnEraseBackground(wxEraseEvent& event)
{
}

void GeneralSetup::Init()
{
	// first read out the defaults:
	for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i)+1))
	{
		auto brushAndPen = mConfig.Get_CalChartBrushAndPen(i);
		mCalChartPens[i] = brushAndPen.second;
		mCalChartBrushes[i] = brushAndPen.first;
	}

	mAutoSave_Interval.Printf(wxT("%ld"), mConfig.Get_AutosaveInterval());
}

bool GeneralSetup::TransferDataToWindow()
{
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(AUTOSAVE_INTERVAL);
	text->SetValue(mAutoSave_Interval);
	return true;
}

bool GeneralSetup::TransferDataFromWindow()
{
	// read out the values from the window
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(AUTOSAVE_INTERVAL);
	mAutoSave_Interval = text->GetValue();

	// write out the values defaults:
	for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i)+1))
	{
		mConfig.Set_CalChartBrushAndPen(i, mCalChartBrushes[i], mCalChartPens[i]);
	}
	long val;
	mAutoSave_Interval.ToLong(&val);
	mConfig.Set_AutosaveInterval(val);
	return true;
}

bool GeneralSetup::ClearValuesToDefault()
{
	for (int i = 0; i < COLOR_NUM; ++i)
	{
		SetColor(i, mConfig.GetDefaultPenWidth()[i], mConfig.GetDefaultColors()[i]);
		mConfig.Clear_ConfigColor(i);
	}
	mConfig.Clear_AutosaveInterval();
	Init();
	TransferDataToWindow();
	return true;
}

void GeneralSetup::SetColor(int selection, int width, const wxColour& color)
{
	mCalChartPens[selection] = *wxThePenList->FindOrCreatePen(color, width, wxSOLID);
	mCalChartBrushes[selection] = *wxTheBrushList->FindOrCreateBrush(color, wxSOLID);

	mConfig.Set_CalChartBrushAndPen(static_cast<CalChartColors>(selection), mCalChartBrushes[selection], mCalChartPens[selection]);

	// update the namebox list
	{
		wxBitmap test_bitmap(16, 16);
		wxMemoryDC temp_dc;
		temp_dc.SelectObject(test_bitmap);
		temp_dc.SetBackground(mCalChartBrushes[selection]);
		temp_dc.Clear();
		nameBox->SetItemBitmap(selection, test_bitmap);
	}
	Refresh();
}

void GeneralSetup::OnCmdSelectColors(wxCommandEvent&)
{
	int selection = nameBox->GetSelection();
	wxColourData data;
	data.SetChooseFull(true);
	data.SetColour(mCalChartBrushes[selection].GetColour());
	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxColourData retdata = dialog.GetColourData();
		wxColour c = retdata.GetColour();
		SetColor(selection, mCalChartPens[selection].GetWidth(), c);
	}
}

void GeneralSetup::OnCmdSelectWidth(wxSpinEvent& e)
{
	int selection = nameBox->GetSelection();
	SetColor(selection, e.GetPosition(), mCalChartPens[selection].GetColour());
}

void GeneralSetup::OnCmdResetColors(wxCommandEvent&)
{
	int selection = nameBox->GetSelection();
	SetColor(selection, mConfig.GetDefaultPenWidth()[selection], mConfig.GetDefaultColors()[selection]);
	mConfig.Clear_ConfigColor(selection);
}

void GeneralSetup::OnCmdChooseNewColor(wxCommandEvent&)
{
	spin->SetValue(mCalChartPens[nameBox->GetSelection()].GetWidth());
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
	mFontNames[0] = GetConfig().Get_HeadFont();
	mFontNames[1] = GetConfig().Get_MainFont();
	mFontNames[2] = GetConfig().Get_NumberFont();
	mFontNames[3] = GetConfig().Get_ContFont();
	mFontNames[4] = GetConfig().Get_BoldFont();
	mFontNames[5] = GetConfig().Get_ItalFont();
	mFontNames[6] = GetConfig().Get_BoldItalFont();
	mPrintValues[0] = GetConfig().Get_HeaderSize();
	mPrintValues[1] = GetConfig().Get_YardsSize();
	mPrintValues[2] = GetConfig().Get_TextSize();
	mPrintValues[3] = GetConfig().Get_DotRatio();
	mPrintValues[4] = GetConfig().Get_NumRatio();
	mPrintValues[5] = GetConfig().Get_PLineRatio();
	mPrintValues[6] = GetConfig().Get_SLineRatio();
	mPrintValues[7] = GetConfig().Get_ContRatio();
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
	GetConfig().Set_HeadFont(mFontNames[0]);
	GetConfig().Set_MainFont(mFontNames[1]);
	GetConfig().Set_NumberFont(mFontNames[2]);
	GetConfig().Set_ContFont(mFontNames[3]);
	GetConfig().Set_BoldFont(mFontNames[4]);
	GetConfig().Set_ItalFont(mFontNames[5]);
	GetConfig().Set_BoldItalFont(mFontNames[6]);
	GetConfig().Set_HeaderSize(mPrintValues[0]);
	GetConfig().Set_YardsSize(mPrintValues[1]);
	GetConfig().Set_TextSize(mPrintValues[2]);
	GetConfig().Set_DotRatio(mPrintValues[3]);
	GetConfig().Set_NumRatio(mPrintValues[4]);
	GetConfig().Set_PLineRatio(mPrintValues[5]);
	GetConfig().Set_SLineRatio(mPrintValues[6]);
	GetConfig().Set_ContRatio(mPrintValues[7]);
	return true;
}

bool PSPrintingSetUp::ClearValuesToDefault()
{
	GetConfig().Clear_HeadFont();
	GetConfig().Clear_MainFont();
	GetConfig().Clear_NumberFont();
	GetConfig().Clear_ContFont();
	GetConfig().Clear_BoldFont();
	GetConfig().Clear_ItalFont();
	GetConfig().Clear_BoldItalFont();
	GetConfig().Clear_HeaderSize();
	GetConfig().Clear_YardsSize();
	GetConfig().Clear_TextSize();
	GetConfig().Clear_DotRatio();
	GetConfig().Clear_NumRatio();
	GetConfig().Clear_PLineRatio();
	GetConfig().Clear_SLineRatio();
	GetConfig().Clear_ContRatio();
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
	std::vector<long> mShowModeValues[SHOWMODE_NUM];
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
	modes->SetSelection(0);
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
	wxChoice* textchoice = new wxChoice(this, SHOW_LINE_MARKING, wxDefaultPosition, wxDefaultSize, GetConfig().Get_yard_text_index().size(), GetConfig().Get_yard_text_index().data());
	textchoice->SetSelection(0);
	textsizer->Add(textchoice);
	textsizer->Add(new wxTextCtrl(this, SHOW_LINE_VALUE), sBasicSizerFlags );

	TransferDataToWindow();
}

void ShowModeSetup::Init()
{
	mWhichMode = 0;
	mWhichYardLine = 0;
	for (size_t i = 0; i < SHOWMODE_NUM; ++i)
	{
		mShowModeValues[i] = GetConfig().GetConfigurationShowMode(i);
	}
	for (size_t i = 0; i < MAX_YARD_LINES; ++i)
	{
		mYardText[i] = GetConfig().Get_yard_text(i);
	}
}

bool ShowModeSetup::TransferDataToWindow()
{
	// standard show
	for (auto i = mShowModeValues[mWhichMode].begin(); i != mShowModeValues[mWhichMode].end(); ++i)
	{
		wxString buf;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(WESTHASH + std::distance(mShowModeValues[mWhichMode].begin(), i));
		buf.Printf(wxT("%ld"), *i);
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
	for (auto i = mShowModeValues[mWhichMode].begin(); i != mShowModeValues[mWhichMode].end(); ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(WESTHASH + std::distance(mShowModeValues[mWhichMode].begin(), i));
		text->GetValue().ToLong(&val);
		*i = val;
	}
	// write out the values defaults:
	for (size_t i = 0; i < SHOWMODE_NUM; ++i)
	{
		GetConfig().SetConfigurationShowMode(i, mShowModeValues[i]);
	}

	// grab whatever's in the box
	wxTextCtrl* text = (wxTextCtrl*) FindWindow(SHOW_LINE_VALUE);
	mYardText[mWhichYardLine] = text->GetValue();
	for (size_t i = 0; i < MAX_YARD_LINES; ++i)
	{
		GetConfig().Set_yard_text(i, mYardText[i]);
	}

	return true;
}

bool ShowModeSetup::ClearValuesToDefault()
{
	for (size_t i = 0; i < SHOWMODE_NUM; ++i)
	{
		GetConfig().ClearConfigurationShowMode(i);
	}
	GetConfig().ClearConfigShowYardline();
	Init();
	return TransferDataToWindow();
}

void ShowModeSetup::OnCmdChoice(wxCommandEvent&)
{
	// save off all the old values:
	for (auto i = mShowModeValues[mWhichMode].begin(); i != mShowModeValues[mWhichMode].end(); ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(WESTHASH + std::distance(mShowModeValues[mWhichMode].begin(), i));
		text->GetValue().ToLong(&val);
		*i = val;
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
	std::vector<long> mSpringShowModeValues[SPRINGSHOWMODE_NUM];
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
	modes->SetSelection(0);
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
	wxChoice* textchoice = new wxChoice(this, SPRING_SHOW_LINE_MARKING, wxDefaultPosition, wxDefaultSize, GetConfig().Get_spr_line_text_index().size(), GetConfig().Get_spr_line_text_index().data());
	textchoice->SetSelection(0);
	textsizer->Add(textchoice);
	textsizer->Add(new wxTextCtrl(this, SPRING_SHOW_LINE_VALUE), sBasicSizerFlags );

	TransferDataToWindow();
}

void SpringShowModeSetup::Init()
{
	mWhichMode = 0;
	mWhichYardLine = 0;
	for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i)
	{
		mSpringShowModeValues[i] = GetConfig().GetConfigurationSpringShowMode(i);
	}
	for (size_t i = 0; i < MAX_SPR_LINES; ++i)
	{
		mYardText[i] = GetConfig().Get_spr_line_text(i);
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
	for (auto i = mSpringShowModeValues[mWhichMode].begin() + 1; i != mSpringShowModeValues[mWhichMode].end(); ++i)
	{
		wxString buf;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_BORDER_LEFT + std::distance(mSpringShowModeValues[mWhichMode].begin(), i) - 1);
		buf.Printf(wxT("%ld"), *i);
		text->SetValue(buf);
	}

	wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_SHOW_LINE_VALUE);
	text->SetValue(GetConfig().Get_spr_line_text(mWhichYardLine));
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
	for (auto i = mSpringShowModeValues[mWhichMode].begin() + 1; i != mSpringShowModeValues[mWhichMode].end(); ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_BORDER_LEFT + std::distance(mSpringShowModeValues[mWhichMode].begin(), i) - 1);
		text->GetValue().ToLong(&val);
		*i = val;
	}

	// write out the values defaults:
	for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i)
	{
		GetConfig().SetConfigurationSpringShowMode(modes->GetSelection(), mSpringShowModeValues[mWhichMode]);
	}

	wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_SHOW_LINE_VALUE);
	mYardText[mWhichYardLine] = text->GetValue();
	for (size_t i = 0; i < MAX_SPR_LINES; ++i)
	{
		GetConfig().Set_spr_line_text(i, mYardText[i]);
	}

	return true;
}

bool SpringShowModeSetup::ClearValuesToDefault()
{
	for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i)
	{
		GetConfig().ClearConfigurationSpringShowMode(i);
	}
	GetConfig().ClearConfigSpringShowYardline();
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
	for (auto i = mSpringShowModeValues[mWhichMode].begin() + 1; i != mSpringShowModeValues[mWhichMode].end(); ++i)
	{
		long val;
		wxTextCtrl* text = (wxTextCtrl*) FindWindow(SPRING_BORDER_LEFT + std::distance(mSpringShowModeValues[mWhichMode].begin(), i) - 1);
		text->GetValue().ToLong(&val);
		*i = val;
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
