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
#include "basic_ui.h"
#include "calchartdoc.h"
#include "cc_shapes.h"
#include "draw.h"

#include <wx/colordlg.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/listbook.h>
#include <wx/dcbuffer.h>

// how the preferences work:
// preference dialog create a copy of the CalChart config from which to read and
// set values
// CalChart config doesn't automatically write values to main config, it must be
// flushed
// out when the user presses apply.
// first page will be general settings:
//   Auto save behavior: file location, time
// second page is Drawing preferences for edit menu
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

static void AddTextboxWithCaption(wxWindow* parent, wxBoxSizer* verticalsizer,
    int id, const wxString& caption,
    long style = 0)
{
    wxBoxSizer* textsizer = new wxBoxSizer(wxVERTICAL);
    textsizer->Add(new wxStaticText(parent, wxID_STATIC, caption,
                       wxDefaultPosition, wxDefaultSize, 0),
        sLeftBasicSizerFlags);
    textsizer->Add(new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition,
                       wxDefaultSize, style),
        sBasicSizerFlags);
    verticalsizer->Add(textsizer, sBasicSizerFlags);
}

static void AddCheckboxWithCaption(wxWindow* parent, wxBoxSizer* verticalsizer,
    int id, const wxString& caption,
    long style = 0)
{
    wxBoxSizer* textsizer = new wxBoxSizer(wxVERTICAL);
    textsizer->Add(new wxCheckBox(parent, id, caption, wxDefaultPosition,
                       wxDefaultSize, style),
        sBasicSizerFlags);
    verticalsizer->Add(textsizer, sBasicSizerFlags);
}

template <typename Function>
void AddTextboxWithCaptionAndAction(wxWindow* parent, wxBoxSizer* verticalsizer,
    int id, const wxString& caption,
    Function&& f, long style = 0)
{
    wxBoxSizer* textsizer = new wxBoxSizer(wxVERTICAL);
    textsizer->Add(new wxStaticText(parent, wxID_STATIC, caption,
                       wxDefaultPosition, wxDefaultSize, 0),
        sLeftBasicSizerFlags);
    auto textCtrl = new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, style);
    textCtrl->Bind((style & wxTE_PROCESS_ENTER) ? wxEVT_TEXT_ENTER : wxEVT_TEXT,
        f);
    textsizer->Add(textCtrl, sBasicSizerFlags);
    verticalsizer->Add(textsizer, sBasicSizerFlags);
}

// the basic class panel we use for all the pages.
// Each page gets a references to the CalChartConfig which will be used for
// getting and setting
class PreferencePage : public wxPanel {
    DECLARE_ABSTRACT_CLASS(GeneralSetup)
public:
    PreferencePage(CalChartConfiguration& config)
        : mConfig(config)
    {
        Init();
    }
    virtual ~PreferencePage() {}
    virtual void Init() {}
    virtual bool Create(wxWindow* parent, wxWindowID id, const wxString& caption,
        const wxPoint& pos, const wxSize& size, long style)
    {
        if (!wxPanel::Create(parent, id, pos, size, style, caption))
            return false;
        CreateControls();
        GetSizer()->Fit(this);
        GetSizer()->SetSizeHints(this);
        Center();
        return true;
    }

    // use these to get and set default values
    virtual bool TransferDataToWindow() = 0;
    virtual bool TransferDataFromWindow() = 0;
    virtual bool ClearValuesToDefault() = 0;

private:
    virtual void CreateControls() = 0;

protected:
    CalChartConfiguration& mConfig;
};

IMPLEMENT_ABSTRACT_CLASS(PreferencePage, wxPanel)

//////// General setup ////////
// handling autosave
////////

class GeneralSetup : public PreferencePage {
    DECLARE_CLASS(GeneralSetup)
    DECLARE_EVENT_TABLE()

public:
    GeneralSetup(CalChartConfiguration& config, wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("General Setup"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
        : PreferencePage(config)
    {
        Init();
        Create(parent, id, caption, pos, size, style);
    }
    virtual ~GeneralSetup() {}

    virtual void Init();
    virtual void CreateControls();

    // use these to get and set default values
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    virtual bool ClearValuesToDefault();

private:
    void OnCmdResetAll(wxCommandEvent&);

    wxString mAutoSave_Interval;
    bool mScroll_Natural;
    bool mSetSheet_Undo;
    bool mSelection_Undo;
};

enum {
    AUTOSAVE_INTERVAL = 1000,
    SCROLL_NATURAL,
    SETSHEET_UNDO,
    SELECTION_UNDO,
};

BEGIN_EVENT_TABLE(GeneralSetup, PreferencePage)
END_EVENT_TABLE()

IMPLEMENT_CLASS(GeneralSetup, PreferencePage)

void GeneralSetup::CreateControls()
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    wxStaticBoxSizer* boxsizer = new wxStaticBoxSizer(
        new wxStaticBox(this, -1, wxT("Autosave settings")), wxVERTICAL);
    topsizer->Add(boxsizer);

    wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    boxsizer->Add(sizer1, sLeftBasicSizerFlags);

    AddTextboxWithCaption(this, sizer1, AUTOSAVE_INTERVAL,
        wxT("Autosave Interval"));
    AddCheckboxWithCaption(this, sizer1, SCROLL_NATURAL,
        wxT("Scroll Direction: Natural"));
    AddCheckboxWithCaption(this, sizer1, SETSHEET_UNDO,
        wxT("Set Sheet is undo-able"));
    AddCheckboxWithCaption(this, sizer1, SELECTION_UNDO,
        wxT("Point selection is undo-able"));

    TransferDataToWindow();
}

void GeneralSetup::Init()
{
    mAutoSave_Interval.Printf(wxT("%ld"), mConfig.Get_AutosaveInterval());
    mScroll_Natural = mConfig.Get_ScrollDirectionNatural();
    mSetSheet_Undo = mConfig.Get_CommandUndoSetSheet();
    mSelection_Undo = mConfig.Get_CommandUndoSelection();
}

bool GeneralSetup::TransferDataToWindow()
{
    ((wxTextCtrl*)FindWindow(AUTOSAVE_INTERVAL))->SetValue(mAutoSave_Interval);
    ((wxCheckBox*)FindWindow(SCROLL_NATURAL))->SetValue(mScroll_Natural);
    ((wxCheckBox*)FindWindow(SETSHEET_UNDO))->SetValue(mSetSheet_Undo);
    ((wxCheckBox*)FindWindow(SELECTION_UNDO))->SetValue(mSelection_Undo);
    return true;
}

bool GeneralSetup::TransferDataFromWindow()
{
    // read out the values from the window
    mAutoSave_Interval = ((wxTextCtrl*)FindWindow(AUTOSAVE_INTERVAL))->GetValue();
    long val;
    if (mAutoSave_Interval.ToLong(&val)) {
        mConfig.Set_AutosaveInterval(val);
    }
    mScroll_Natural = ((wxCheckBox*)FindWindow(SCROLL_NATURAL))->GetValue();
    mConfig.Set_ScrollDirectionNatural(mScroll_Natural);
    mSetSheet_Undo = ((wxCheckBox*)FindWindow(SETSHEET_UNDO))->GetValue();
    mConfig.Set_CommandUndoSetSheet(mSetSheet_Undo);
    mSelection_Undo = ((wxCheckBox*)FindWindow(SELECTION_UNDO))->GetValue();
    mConfig.Set_CommandUndoSelection(mSelection_Undo);
    return true;
}

bool GeneralSetup::ClearValuesToDefault()
{
    mConfig.Clear_AutosaveInterval();
    Init();
    TransferDataToWindow();
    return true;
}

//////// Draw setup ////////
// handling Drawing colors
////////

class PrefCanvas : public ClickDragCtrlScrollCanvas {
    DECLARE_EVENT_TABLE()
    using super = ClickDragCtrlScrollCanvas;

public:
    PrefCanvas(CalChartConfiguration& config, wxWindow* parent);

    void OnPaint(wxPaintEvent& event);

private:
    void OnEraseBackground(wxEraseEvent& event);

    std::unique_ptr<CalChart::Show> mShow;
    std::unique_ptr<const CalChart::ShowMode> mMode;
    CalChartConfiguration& mConfig;
    std::vector<CalChart::DrawCommand> mPath;
    CalChart::Coord mPathEnd;
    std::vector<CalChart::DrawCommand> mShape;
};

BEGIN_EVENT_TABLE(PrefCanvas, ClickDragCtrlScrollCanvas)
EVT_PAINT(PrefCanvas::OnPaint)
EVT_ERASE_BACKGROUND(PrefCanvas::OnEraseBackground)
END_EVENT_TABLE()

PrefCanvas::PrefCanvas(CalChartConfiguration& config, wxWindow* parent)
    : super(parent, wxID_ANY, wxDefaultPosition, wxSize(640, 240))
    , mMode(CalChart::ShowModeStandard::CreateShowMode(
          "", CalChart::Coord(Int2CoordUnits(160), Int2CoordUnits(84)),
          CalChart::Coord(Int2CoordUnits(80), Int2CoordUnits(42)),
          CalChart::Coord(Int2CoordUnits(4), Int2CoordUnits(4)),
          CalChart::Coord(Int2CoordUnits(4), Int2CoordUnits(4)), Int2CoordUnits(32), Int2CoordUnits(52)))
    , mConfig(config)
{
    auto field_offset = mMode->FieldOffset();
    auto offset = mMode->Offset();
    SetCanvasSize(wxSize{ mMode->Size().x, mMode->Size().y });

    // Create a fake show with some points and selections to draw an example for
    // the user
    mShow = CalChart::Show::Create_CC_show();
    auto labels = std::vector<std::string>{
        "unsel", "unsel", "sel", "sel",
    };
    mShow->Create_SetShowInfoCommand(labels, 4, field_offset).first(*mShow);
    mShow->Create_SetShowInfoCommand(labels, 4, field_offset).first(*mShow);
    mShow->Create_SetSelectionCommand(SelectionList{ 0, 2 }).first(*mShow);
    mShow->Create_SetSymbolCommand(SYMBOL_X).first(*mShow);
    mShow->Create_SetSelectionCommand(SelectionList{ 1, 3 }).first(*mShow);
    mShow->Create_SetSymbolCommand(SYMBOL_SOLX).first(*mShow);
    mShow->Create_SetSelectionCommand(SelectionList{}).first(*mShow);

    for (auto i = 0; i < 4; ++i) {
        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 0).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 1).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 2).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 3).first(*mShow);

        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(i * 4), Int2CoordUnits(6)) } }, 1).first(*mShow);
    }

    mShow->Create_AddSheetsCommand(CalChart::Show::Sheet_container_t{ *static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet() }, 1).first(*mShow);
    mShow->Create_SetCurrentSheetCommand(1).first(*mShow);
    for (auto i = 0; i < 4; ++i) {
        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 0).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 1).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 2).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 3).first(*mShow);

        mShow->Create_MovePointsCommand({ { i, field_offset + CalChart::Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 6)) } }, 1).first(*mShow);
    }
    mShow->Create_SetCurrentSheetCommand(0).first(*mShow);

    auto point_start = offset + field_offset + CalChart::Coord(Int2CoordUnits(4), Int2CoordUnits(2));
    mPathEnd = point_start + CalChart::Coord(Int2CoordUnits(0), Int2CoordUnits(2));
    mPath.emplace_back(point_start, mPathEnd);
    point_start = mPathEnd;
    mPathEnd += CalChart::Coord(Int2CoordUnits(18), Int2CoordUnits(0));
    mPath.emplace_back(point_start, mPathEnd);

    auto shape_start = field_offset + CalChart::Coord(Int2CoordUnits(18), Int2CoordUnits(-2));
    auto shape_end = shape_start + CalChart::Coord(Int2CoordUnits(4), Int2CoordUnits(4));
    CalChart::Shape_rect rect(shape_start, shape_end);
    mShape = rect.GetCC_DrawCommand(offset.x, offset.y);
}

// Define the repainting behaviour
void PrefCanvas::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    // draw the background
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(mConfig.Get_CalChartBrushAndPen(COLOR_FIELD).first);
    dc.Clear();

    // Draw the field
    DrawMode(dc, mConfig, *mMode, ShowMode_kFieldView);

    auto sheet = static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet();
    auto nextSheet = sheet;
    ++nextSheet;

    SelectionList list;
    list.insert(2);
    list.insert(3);

    // draw the ghost sheet
    DrawGhostSheet(dc, mConfig, mMode->Offset(), list, mShow->GetNumPoints(),
        mShow->GetPointLabels(), *nextSheet, 0);

    // Draw the points
    DrawPoints(dc, mConfig, mMode->Offset(), list, mShow->GetNumPoints(),
        mShow->GetPointLabels(), *sheet, 0, true);
    DrawPoints(dc, mConfig, mMode->Offset(), list, mShow->GetNumPoints(),
        mShow->GetPointLabels(), *sheet, 1, false);

    // draw the path
    DrawPath(dc, mConfig, mPath, mPathEnd);

    // draw the shap
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(mConfig.Get_CalChartBrushAndPen(COLOR_SHAPES).second);
    DrawCC_DrawCommandList(dc, mShape);
}

// We have a empty erase background to improve redraw performance.
void PrefCanvas::OnEraseBackground(wxEraseEvent& event) {}

class DrawingSetup : public PreferencePage {
    DECLARE_CLASS(DrawingSetup)
    DECLARE_EVENT_TABLE()

public:
    DrawingSetup(CalChartConfiguration& config, wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("General Setup"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
        : PreferencePage(config)
    {
        Init();
        Create(parent, id, caption, pos, size, style);
    }
    virtual ~DrawingSetup() {}

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
    void OnCmdTextChanged(wxCommandEvent&);

    void SetColor(int selection, int width, const wxColour& color);
    wxBitmapComboBox* nameBox;
    wxSpinCtrl* spin;

    wxPen mCalChartPens[COLOR_NUM];
    wxBrush mCalChartBrushes[COLOR_NUM];

    double mPrintValues[5];
};

enum {
    BUTTON_SELECT = 1000,
    BUTTON_RESTORE,
    SPIN_WIDTH,
    NEW_COLOR_CHOICE,
    DOTRATIO,
    NUMRATIO,
    PLINERATIO,
    SLINERATIO
};

BEGIN_EVENT_TABLE(DrawingSetup, PreferencePage)
EVT_BUTTON(BUTTON_SELECT, DrawingSetup::OnCmdSelectColors)
EVT_BUTTON(BUTTON_RESTORE, DrawingSetup::OnCmdResetColors)
EVT_SPINCTRL(SPIN_WIDTH, DrawingSetup::OnCmdSelectWidth)
EVT_COMBOBOX(NEW_COLOR_CHOICE, DrawingSetup::OnCmdChooseNewColor)
EVT_TEXT_ENTER(DOTRATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(NUMRATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(PLINERATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SLINERATIO, DrawingSetup::OnCmdTextChanged)
END_EVENT_TABLE()

IMPLEMENT_CLASS(DrawingSetup, PreferencePage)

void DrawingSetup::CreateControls()
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    wxStaticBoxSizer* boxsizer = new wxStaticBoxSizer(
        new wxStaticBox(this, -1, wxT("Color settings")), wxVERTICAL);
    topsizer->Add(boxsizer);

    wxBoxSizer* horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    nameBox = new wxBitmapComboBox(
        this, NEW_COLOR_CHOICE, mConfig.GetColorNames().at(0), wxDefaultPosition,
        wxDefaultSize, COLOR_NUM, mConfig.GetColorNames().data(),
        wxCB_READONLY | wxCB_DROPDOWN);
    horizontalsizer->Add(nameBox, sBasicSizerFlags);

    for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM;
         i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
        wxBitmap temp_bitmap(16, 16);
        wxMemoryDC temp_dc;
        temp_dc.SelectObject(temp_bitmap);
        temp_dc.SetBackground(mConfig.Get_CalChartBrushAndPen(i).first);
        temp_dc.Clear();
        nameBox->SetItemBitmap(i, temp_bitmap);
    }
    nameBox->SetSelection(0);

    spin = new wxSpinCtrl(this, SPIN_WIDTH, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, wxSP_ARROW_KEYS, 1, 10,
        mCalChartPens[nameBox->GetSelection()].GetWidth());
    spin->SetValue(mCalChartPens[nameBox->GetSelection()].GetWidth());
    horizontalsizer->Add(spin, sBasicSizerFlags);
    boxsizer->Add(horizontalsizer, sLeftBasicSizerFlags);

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);

    horizontalsizer->Add(new wxButton(this, BUTTON_SELECT, wxT("&Change Color")),
        sBasicSizerFlags);
    horizontalsizer->Add(new wxButton(this, BUTTON_RESTORE, wxT("&Reset Color")),
        sBasicSizerFlags);

    boxsizer->Add(horizontalsizer, sBasicSizerFlags);

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("ratios")),
        wxVERTICAL);
    topsizer->Add(boxsizer);

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    boxsizer->Add(horizontalsizer, sLeftBasicSizerFlags);

    AddTextboxWithCaption(this, horizontalsizer, DOTRATIO, wxT("Dot Ratio:"),
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaption(this, horizontalsizer, NUMRATIO, wxT("Num Ratio:"),
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaption(this, horizontalsizer, PLINERATIO, wxT("P-Line Ratio:"),
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaption(this, horizontalsizer, SLINERATIO, wxT("S-Line Ratio:"),
        wxTE_PROCESS_ENTER);

    auto prefCanvas = new PrefCanvas(mConfig, this);
    // set scroll rate 1 to 1, so we can have even scrolling of whole field
    topsizer->Add(prefCanvas, 1, wxEXPAND);
    //	mCanvas->SetScrollRate(1, 1);

    TransferDataToWindow();
}

void DrawingSetup::Init()
{
    // first read out the defaults:
    for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM;
         i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
        auto brushAndPen = mConfig.Get_CalChartBrushAndPen(i);
        mCalChartPens[i] = brushAndPen.second;
        mCalChartBrushes[i] = brushAndPen.first;
    }

    mPrintValues[0] = mConfig.Get_DotRatio();
    mPrintValues[1] = mConfig.Get_NumRatio();
    mPrintValues[2] = mConfig.Get_PLineRatio();
    mPrintValues[3] = mConfig.Get_SLineRatio();
    mPrintValues[4] = mConfig.Get_ContRatio();
}

bool DrawingSetup::TransferDataToWindow()
{
    wxString buf;
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(DOTRATIO);
    buf.Printf(wxT("%.2f"), mPrintValues[0]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(NUMRATIO);
    buf.Printf(wxT("%.2f"), mPrintValues[1]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(PLINERATIO);
    buf.Printf(wxT("%.2f"), mPrintValues[2]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(SLINERATIO);
    buf.Printf(wxT("%.2f"), mPrintValues[3]);
    text->SetValue(buf);

    return true;
}

bool DrawingSetup::TransferDataFromWindow()
{
    // write out the values defaults:
    for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM;
         i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
        mConfig.Set_CalChartBrushAndPen(i, mCalChartBrushes[i], mCalChartPens[i]);
    }
    return true;
}

bool DrawingSetup::ClearValuesToDefault()
{
    for (int i = 0; i < COLOR_NUM; ++i) {
        SetColor(i, mConfig.GetDefaultPenWidth()[i], mConfig.GetDefaultColors()[i]);
        mConfig.Clear_ConfigColor(i);
    }
    Init();
    TransferDataToWindow();
    return true;
}

void DrawingSetup::SetColor(int selection, int width, const wxColour& color)
{
    mCalChartPens[selection] = *wxThePenList->FindOrCreatePen(color, width, wxPENSTYLE_SOLID);
    mCalChartBrushes[selection] = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

    mConfig.Set_CalChartBrushAndPen(static_cast<CalChartColors>(selection),
        mCalChartBrushes[selection],
        mCalChartPens[selection]);

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

void DrawingSetup::OnCmdSelectColors(wxCommandEvent&)
{
    int selection = nameBox->GetSelection();
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(mCalChartBrushes[selection].GetColour());
    wxColourDialog dialog(this, &data);
    if (dialog.ShowModal() == wxID_OK) {
        wxColourData retdata = dialog.GetColourData();
        wxColour c = retdata.GetColour();
        SetColor(selection, mCalChartPens[selection].GetWidth(), c);
    }
}

void DrawingSetup::OnCmdSelectWidth(wxSpinEvent& e)
{
    int selection = nameBox->GetSelection();
    SetColor(selection, e.GetPosition(), mCalChartPens[selection].GetColour());
}

void DrawingSetup::OnCmdResetColors(wxCommandEvent&)
{
    int selection = nameBox->GetSelection();
    SetColor(selection, mConfig.GetDefaultPenWidth()[selection],
        mConfig.GetDefaultColors()[selection]);
    mConfig.Clear_ConfigColor(selection);
}

void DrawingSetup::OnCmdChooseNewColor(wxCommandEvent&)
{
    spin->SetValue(mCalChartPens[nameBox->GetSelection()].GetWidth());
}

void DrawingSetup::OnCmdTextChanged(wxCommandEvent& e)
{
    auto id = e.GetId();
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(id);
    double value;
    if (text->GetValue().ToDouble(&value)) {
        switch (id - DOTRATIO) {
        case 0:
            mConfig.Set_DotRatio(value);
            break;
        case 1:
            mConfig.Set_NumRatio(value);
            break;
        case 2:
            mConfig.Set_PLineRatio(value);
            break;
        case 3:
            mConfig.Set_SLineRatio(value);
            break;
        case 4:
            mConfig.Set_ContRatio(value);
            break;
        }
    }
    Refresh();
}

//////// General setup ////////
// setup pringing values and colors
////////

class PSPrintingSetUp : public PreferencePage {
    DECLARE_CLASS(PSPrintingSetUp)
    DECLARE_EVENT_TABLE()

public:
    PSPrintingSetUp(CalChartConfiguration& config, wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Printing Values"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
        : PreferencePage(config)
    {
        Init();
        Create(parent, id, caption, pos, size, style);
    }
    virtual ~PSPrintingSetUp() {}

    virtual void Init();
    virtual void CreateControls();

    // use these to get and set default values
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    virtual bool ClearValuesToDefault();

private:
    wxString mFontNames[7];
    double mPrintValues[8];
    // only one mode, not putting a choice controller in for now...
    int mWhichMode = 0;
    CalChartConfiguration::SpringShowModeInfo_t
        mSpringShowModeValues[SPRINGSHOWMODE_NUM];
};

typedef enum {
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
    CONTRATIO,
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
} PSPrintingSetUp_IDs;

BEGIN_EVENT_TABLE(PSPrintingSetUp, PreferencePage)
END_EVENT_TABLE()

IMPLEMENT_CLASS(PSPrintingSetUp, PreferencePage)

void PSPrintingSetUp::CreateControls()
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    wxBoxSizer* horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(horizontalsizer, sLeftBasicSizerFlags);

    AddTextboxWithCaption(this, horizontalsizer, HEADFONT, wxT("Header Font:"));
    AddTextboxWithCaption(this, horizontalsizer, MAINFONT, wxT("Main Font:"));
    AddTextboxWithCaption(this, horizontalsizer, NUMBERFONT, wxT("Number Font:"));
    AddTextboxWithCaption(this, horizontalsizer, CONTFONT,
        wxT("Continuity Font:"));

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(horizontalsizer, sLeftBasicSizerFlags);

    AddTextboxWithCaption(this, horizontalsizer, BOLDFONT, wxT("Bold Font:"));
    AddTextboxWithCaption(this, horizontalsizer, ITALFONT, wxT("Italic Font:"));
    AddTextboxWithCaption(this, horizontalsizer, BOLDITALFONT,
        wxT("Bold Italic Font:"));

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(horizontalsizer, sLeftBasicSizerFlags);

    AddTextboxWithCaption(this, horizontalsizer, HEADERSIZE, wxT("Header Size:"));
    AddTextboxWithCaption(this, horizontalsizer, YARDSSIZE, wxT("Yards Side:"));
    AddTextboxWithCaption(this, horizontalsizer, TEXTSIZE, wxT("Text Side:"));

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(horizontalsizer, sLeftBasicSizerFlags);

    AddTextboxWithCaption(this, horizontalsizer, CONTRATIO,
        wxT("Continuity Ratio:"));

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(horizontalsizer, sLeftBasicSizerFlags);
    AddTextboxWithCaption(this, horizontalsizer, EPS_STAGE_X, wxT("EPS Stage X"));
    AddTextboxWithCaption(this, horizontalsizer, EPS_STAGE_Y, wxT("EPS Stage Y"));
    AddTextboxWithCaption(this, horizontalsizer, EPS_STAGE_W, wxT("EPS Stage W"));
    AddTextboxWithCaption(this, horizontalsizer, EPS_STAGE_H, wxT("EPS Stage H"));

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(horizontalsizer, sLeftBasicSizerFlags);
    AddTextboxWithCaption(this, horizontalsizer, EPS_FIELD_X, wxT("EPS Field X"));
    AddTextboxWithCaption(this, horizontalsizer, EPS_FIELD_Y, wxT("EPS Field Y"));
    AddTextboxWithCaption(this, horizontalsizer, EPS_FIELD_W, wxT("EPS Field W"));
    AddTextboxWithCaption(this, horizontalsizer, EPS_FIELD_H, wxT("EPS Field H"));

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(horizontalsizer, sLeftBasicSizerFlags);
    AddTextboxWithCaption(this, horizontalsizer, EPS_TEXT_X, wxT("EPS Text X"));
    AddTextboxWithCaption(this, horizontalsizer, EPS_TEXT_Y, wxT("EPS Text Y"));
    AddTextboxWithCaption(this, horizontalsizer, EPS_TEXT_W, wxT("EPS Text W"));
    AddTextboxWithCaption(this, horizontalsizer, EPS_TEXT_H, wxT("EPS Text H"));

    TransferDataToWindow();
}

void PSPrintingSetUp::Init()
{
    mFontNames[0] = mConfig.Get_HeadFont();
    mFontNames[1] = mConfig.Get_MainFont();
    mFontNames[2] = mConfig.Get_NumberFont();
    mFontNames[3] = mConfig.Get_ContFont();
    mFontNames[4] = mConfig.Get_BoldFont();
    mFontNames[5] = mConfig.Get_ItalFont();
    mFontNames[6] = mConfig.Get_BoldItalFont();
    mPrintValues[0] = mConfig.Get_HeaderSize();
    mPrintValues[1] = mConfig.Get_YardsSize();
    mPrintValues[2] = mConfig.Get_TextSize();
    mPrintValues[7] = mConfig.Get_ContRatio();
    for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i) {
        mSpringShowModeValues[i] = mConfig.Get_SpringShowModeInfo(static_cast<CalChartSpringShowModes>(i));
    }
}

bool PSPrintingSetUp::TransferDataToWindow()
{
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(HEADFONT);
    text->SetValue(mFontNames[0]);
    text = (wxTextCtrl*)FindWindow(MAINFONT);
    text->SetValue(mFontNames[1]);
    text = (wxTextCtrl*)FindWindow(NUMBERFONT);
    text->SetValue(mFontNames[2]);
    text = (wxTextCtrl*)FindWindow(CONTFONT);
    text->SetValue(mFontNames[3]);
    text = (wxTextCtrl*)FindWindow(BOLDFONT);
    text->SetValue(mFontNames[4]);
    text = (wxTextCtrl*)FindWindow(ITALFONT);
    text->SetValue(mFontNames[5]);
    text = (wxTextCtrl*)FindWindow(BOLDITALFONT);
    text->SetValue(mFontNames[6]);
    text = (wxTextCtrl*)FindWindow(HEADERSIZE);
    wxString buf;
    buf.Printf(wxT("%.2f"), mPrintValues[0]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(YARDSSIZE);
    buf.Printf(wxT("%.2f"), mPrintValues[1]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(TEXTSIZE);
    buf.Printf(wxT("%.2f"), mPrintValues[2]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(CONTRATIO);
    buf.Printf(wxT("%.2f"), mPrintValues[7]);
    text->SetValue(buf);

    for (auto i = mSpringShowModeValues[mWhichMode].begin() + CalChart::ShowModeSprShow::keps_stage_x;
         i != mSpringShowModeValues[mWhichMode].end(); ++i) {
        wxString buf;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            EPS_STAGE_X + std::distance(mSpringShowModeValues[mWhichMode].begin(), i) - CalChart::ShowModeSprShow::keps_stage_x);
        buf.Printf(wxT("%ld"), *i);
        text->SetValue(buf);
    }

    return true;
}

bool PSPrintingSetUp::TransferDataFromWindow()
{
    // read out the values from the window
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(HEADFONT);
    mFontNames[0] = text->GetValue();
    text = (wxTextCtrl*)FindWindow(MAINFONT);
    mFontNames[1] = text->GetValue();
    text = (wxTextCtrl*)FindWindow(NUMBERFONT);
    mFontNames[2] = text->GetValue();
    text = (wxTextCtrl*)FindWindow(CONTFONT);
    mFontNames[3] = text->GetValue();
    text = (wxTextCtrl*)FindWindow(BOLDFONT);
    mFontNames[4] = text->GetValue();
    text = (wxTextCtrl*)FindWindow(ITALFONT);
    mFontNames[5] = text->GetValue();
    text = (wxTextCtrl*)FindWindow(BOLDITALFONT);
    mFontNames[6] = text->GetValue();
    text = (wxTextCtrl*)FindWindow(HEADERSIZE);
    text->GetValue().ToDouble(&mPrintValues[0]);
    text = (wxTextCtrl*)FindWindow(YARDSSIZE);
    text->GetValue().ToDouble(&mPrintValues[1]);
    text = (wxTextCtrl*)FindWindow(TEXTSIZE);
    text->GetValue().ToDouble(&mPrintValues[2]);
    text = (wxTextCtrl*)FindWindow(CONTRATIO);
    text->GetValue().ToDouble(&mPrintValues[7]);

    // write out the values defaults:
    mConfig.Set_HeadFont(mFontNames[0]);
    mConfig.Set_MainFont(mFontNames[1]);
    mConfig.Set_NumberFont(mFontNames[2]);
    mConfig.Set_ContFont(mFontNames[3]);
    mConfig.Set_BoldFont(mFontNames[4]);
    mConfig.Set_ItalFont(mFontNames[5]);
    mConfig.Set_BoldItalFont(mFontNames[6]);
    mConfig.Set_HeaderSize(mPrintValues[0]);
    mConfig.Set_YardsSize(mPrintValues[1]);
    mConfig.Set_TextSize(mPrintValues[2]);
    mConfig.Set_ContRatio(mPrintValues[7]);

    for (auto i = mSpringShowModeValues[mWhichMode].begin() + CalChart::ShowModeSprShow::keps_stage_x;
         i != mSpringShowModeValues[mWhichMode].end(); ++i) {
        long val;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            EPS_STAGE_X + std::distance(mSpringShowModeValues[mWhichMode].begin(), i) - CalChart::ShowModeSprShow::keps_stage_x);
        text->GetValue().ToLong(&val);
        *i = val;
    }

    // write out the values defaults:
    for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i) {
        mConfig.Set_SpringShowModeInfo(
            static_cast<CalChartSpringShowModes>(mWhichMode),
            mSpringShowModeValues[mWhichMode]);
    }
    return true;
}

bool PSPrintingSetUp::ClearValuesToDefault()
{
    mConfig.Clear_HeadFont();
    mConfig.Clear_MainFont();
    mConfig.Clear_NumberFont();
    mConfig.Clear_ContFont();
    mConfig.Clear_BoldFont();
    mConfig.Clear_ItalFont();
    mConfig.Clear_BoldItalFont();
    mConfig.Clear_HeaderSize();
    mConfig.Clear_YardsSize();
    mConfig.Clear_TextSize();
    mConfig.Clear_ContRatio();
    Init();
    return TransferDataToWindow();
}

//////// Show Mode setup ////////
// setup drawing characteristics
////////

const int zoom_amounts[] = { 500, 200, 150, 100, 75, 50, 25, 10 };

class ShowModeSetupCanvas : public ClickDragCtrlScrollCanvas {
    DECLARE_EVENT_TABLE()
    using super = ClickDragCtrlScrollCanvas;

public:
    ShowModeSetupCanvas(CalChartConfiguration& config, wxWindow* parent,
        wxWindowID id = wxID_ANY);

    void OnPaint(wxPaintEvent& event);
    void SetMode(const std::string&, CalChartShowModes);
    void SetMode(const std::string&, CalChartSpringShowModes);
    virtual void SetZoom(float factor);

private:
    CalChartConfiguration& mConfig;
    std::unique_ptr<const CalChart::ShowMode> mMode;
};

BEGIN_EVENT_TABLE(ShowModeSetupCanvas, ClickDragCtrlScrollCanvas)
EVT_PAINT(ShowModeSetupCanvas::OnPaint)
EVT_MOTION(ShowModeSetupCanvas::OnMouseMove)
EVT_MAGNIFY(ShowModeSetupCanvas::OnMousePinchToZoom)
EVT_MOUSEWHEEL(ShowModeSetupCanvas::OnMouseWheel)
END_EVENT_TABLE()

ShowModeSetupCanvas::ShowModeSetupCanvas(CalChartConfiguration& config,
    wxWindow* parent, wxWindowID id)
    : super(parent, id, wxDefaultPosition, wxSize(640, 240))
    , mConfig(config)
{
}

// Define the repainting behaviour
void ShowModeSetupCanvas::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    // draw the background
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(mConfig.Get_CalChartBrushAndPen(COLOR_FIELD).first);
    dc.Clear();

    // Draw the field
    if (mMode) {
        DrawMode(dc, mConfig, *mMode, ShowMode_kFieldView);
    }
}

void ShowModeSetupCanvas::SetMode(const std::string& which,
    CalChartShowModes item)
{
    mMode = CalChart::ShowModeStandard::CreateShowMode(
        which, [this, item]() { return this->mConfig.Get_ShowModeInfo(item); });
    SetCanvasSize(wxSize{ mMode->Size().x, mMode->Size().y });
    Refresh();
}

void ShowModeSetupCanvas::SetMode(const std::string& which,
    CalChartSpringShowModes item)
{
    mMode = CalChart::ShowModeSprShow::CreateSpringShowMode(which, [this, item]() {
        return this->mConfig.Get_SpringShowModeInfo(item);
    });
    SetCanvasSize(wxSize{ mMode->Size().x, mMode->Size().y });
    Refresh();
}

void ShowModeSetupCanvas::SetZoom(float factor)
{
    super::SetZoom(factor);
    Refresh();
}

class ShowModeSetup : public PreferencePage {
    DECLARE_CLASS(ShowModeSetup)
    DECLARE_EVENT_TABLE()

public:
    ShowModeSetup(CalChartConfiguration& config, wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Setup Modes"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
        : PreferencePage(config)
    {
        Init();
        Create(parent, id, caption, pos, size, style);
    }
    ~ShowModeSetup() {}

    virtual void Init();
    virtual void CreateControls();

    // use these to get and set default values
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    virtual bool ClearValuesToDefault();

private:
    void OnCmdLineText(wxCommandEvent&);
    void OnCmdChoice(wxCommandEvent&);
    CalChartConfiguration::ShowModeInfo_t mShowModeValues[SHOWMODE_NUM];
    wxString mYardText[CalChart::kYardTextValues];
    int mWhichMode;
    int mWhichYardLine;
};

enum {
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
    CANVAS,
};

BEGIN_EVENT_TABLE(ShowModeSetup, PreferencePage)
EVT_CHOICE(MODE_CHOICE, ShowModeSetup::OnCmdChoice)
EVT_CHOICE(SHOW_LINE_MARKING, ShowModeSetup::OnCmdChoice)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ShowModeSetup, PreferencePage)

void ShowModeSetup::CreateControls()
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    wxChoice* modes = new wxChoice(this, MODE_CHOICE, wxDefaultPosition,
        wxDefaultSize, SHOWMODE_NUM, kShowModeStrings);
    modes->SetSelection(0);
    topsizer->Add(modes, sLeftBasicSizerFlags);

    wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);

    auto refresh_action = [this](wxCommandEvent&) {
        this->TransferDataFromWindow();
        Refresh();
    };

    AddTextboxWithCaptionAndAction(this, sizer1, WESTHASH, wxT("West Hash"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, EASTHASH, wxT("East Hash"),
        refresh_action, wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
    AddTextboxWithCaptionAndAction(this, sizer1, BORDER_LEFT, wxT("Left Border"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, BORDER_TOP, wxT("Top Border"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, BORDER_RIGHT,
        wxT("Right Border"), refresh_action,
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, BORDER_BOTTOM,
        wxT("Bottom Border"), refresh_action,
        wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
    AddTextboxWithCaptionAndAction(this, sizer1, OFFSET_X, wxT("Offset X"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, OFFSET_Y, wxT("Offset Y"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, SIZE_X, wxT("Size X"),
        refresh_action, wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, SIZE_Y, wxT("Size Y"),
        refresh_action, wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
    wxBoxSizer* textsizer = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(textsizer, sBasicSizerFlags);
    textsizer->Add(new wxStaticText(this, wxID_STATIC,
                       wxT("Adjust yardline marker"),
                       wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    wxChoice* textchoice = new wxChoice(this, SHOW_LINE_MARKING, wxDefaultPosition, wxDefaultSize,
        wxArrayString{ mConfig.Get_yard_text_index().size(), mConfig.Get_yard_text_index().data() });
    textchoice->SetSelection(0);
    textsizer->Add(textchoice);
    auto show_line_value = new wxTextCtrl(this, SHOW_LINE_VALUE, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, wxTE_PROCESS_ENTER);
    show_line_value->Bind(wxEVT_TEXT_ENTER, refresh_action);
    textsizer->Add(show_line_value, sBasicSizerFlags);

    textsizer = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(textsizer, sBasicSizerFlags);
    textsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Zoom"),
                       wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    wxArrayString zoomtext;
    for (auto& i : zoom_amounts) {
        wxString buf;
        buf.sprintf(wxT("%d%%"), i);
        zoomtext.Add(buf);
    }
    auto zoomBox = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, zoomtext);
    zoomBox->Bind(wxEVT_CHOICE, [=](wxCommandEvent& event) {
        size_t sel = event.GetInt();
        float zoom_amount = zoom_amounts[sel] / 100.0;
        static_cast<ShowModeSetupCanvas*>(FindWindow(CANVAS))
            ->SetZoom(zoom_amount);
    });

    // set the text to the default zoom level
    textsizer->Add(zoomBox, sLeftBasicSizerFlags);

    auto modeSetupCanvas = new ShowModeSetupCanvas(mConfig, this, CANVAS);
    modeSetupCanvas->SetScrollRate(1, 1);
    topsizer->Add(modeSetupCanvas, 1, wxEXPAND);

    modeSetupCanvas->SetMode("", static_cast<CalChartShowModes>(mWhichMode));
    modeSetupCanvas->SetZoom(zoom_amounts[5] / 100.0);
    zoomBox->SetSelection(5);

    TransferDataToWindow();
}

void ShowModeSetup::Init()
{
    mWhichMode = 0;
    mWhichYardLine = 0;
    for (size_t i = 0; i < SHOWMODE_NUM; ++i) {
        mShowModeValues[i] = mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(i));
    }
    for (size_t i = 0; i < CalChart::kYardTextValues; ++i) {
        mYardText[i] = mConfig.Get_yard_text(i);
    }
}

bool ShowModeSetup::TransferDataToWindow()
{
    // standard show
    for (auto i = mShowModeValues[mWhichMode].begin();
         i != mShowModeValues[mWhichMode].end(); ++i) {
        wxString buf;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            WESTHASH + std::distance(mShowModeValues[mWhichMode].begin(), i));
        buf.Printf(wxT("%ld"), *i);
        text->ChangeValue(buf);
    }

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    text->SetValue(mYardText[mWhichYardLine]);
    return true;
}

bool ShowModeSetup::TransferDataFromWindow()
{
    // read out the values from the window
    // standard show
    for (auto i = mShowModeValues[mWhichMode].begin();
         i != mShowModeValues[mWhichMode].end(); ++i) {
        long val;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            WESTHASH + std::distance(mShowModeValues[mWhichMode].begin(), i));
        text->GetValue().ToLong(&val);
        *i = val;
    }
    // write out the values defaults:
    for (size_t i = 0; i < SHOWMODE_NUM; ++i) {
        mConfig.Set_ShowModeInfo(static_cast<CalChartShowModes>(i),
            mShowModeValues[i]);
    }

    // grab whatever's in the box
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();
    for (size_t i = 0; i < CalChart::kYardTextValues; ++i) {
        mConfig.Set_yard_text(i, mYardText[i]);
    }

    return true;
}

bool ShowModeSetup::ClearValuesToDefault()
{
    for (size_t i = 0; i < SHOWMODE_NUM; ++i) {
        mConfig.Clear_ShowModeInfo(static_cast<CalChartShowModes>(i));
    }
    for (auto i = 0; i < CalChart::kYardTextValues; ++i) {
        mConfig.Clear_yard_text(i);
    }
    Init();
    return TransferDataToWindow();
}

void ShowModeSetup::OnCmdChoice(wxCommandEvent&)
{
    // save off all the old values:
    for (auto i = mShowModeValues[mWhichMode].begin();
         i != mShowModeValues[mWhichMode].end(); ++i) {
        long val;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            WESTHASH + std::distance(mShowModeValues[mWhichMode].begin(), i));
        text->GetValue().ToLong(&val);
        *i = val;
    }
    wxChoice* modes = (wxChoice*)FindWindow(MODE_CHOICE);
    mWhichMode = modes->GetSelection();
    ShowModeSetupCanvas* canvas = (ShowModeSetupCanvas*)FindWindow(CANVAS);
    canvas->SetMode("", static_cast<CalChartShowModes>(mWhichMode));

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();
    // update mode
    for (size_t i = 0; i < CalChart::kYardTextValues; ++i) {
        mConfig.Set_yard_text(i, mYardText[i]);
    }

    modes = (wxChoice*)FindWindow(SHOW_LINE_MARKING);
    mWhichYardLine = modes->GetSelection();
    TransferDataToWindow();
}

//////// Spring Show Mode setup ////////
// setup pringing values and colors
////////

class SpringShowModeSetup : public PreferencePage {
    DECLARE_CLASS(SpringShowModeSetup)
    DECLARE_EVENT_TABLE()

public:
    SpringShowModeSetup(CalChartConfiguration& config, wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Setup Modes"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
        : PreferencePage(config)
    {
        Init();
        Create(parent, id, caption, pos, size, style);
    }
    ~SpringShowModeSetup() {}

    virtual void Init();
    virtual void CreateControls();

    // use these to get and set default values
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    virtual bool ClearValuesToDefault();

private:
    void OnCmdChoice(wxCommandEvent&);
    CalChartConfiguration::SpringShowModeInfo_t
        mSpringShowModeValues[SPRINGSHOWMODE_NUM];
    wxString mYardText[CalChart::kSprLineTextValues];
    int mWhichMode;
    int mWhichYardLine;
};

enum {
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
    SPRING_SHOW_LINE_MARKING,
    SPRING_SHOW_LINE_VALUE,
    SPRING_CANVAS,
};

BEGIN_EVENT_TABLE(SpringShowModeSetup, PreferencePage)
EVT_CHOICE(SPRING_MODE_CHOICE, SpringShowModeSetup::OnCmdChoice)
EVT_CHOICE(SPRING_SHOW_LINE_MARKING, SpringShowModeSetup::OnCmdChoice)
END_EVENT_TABLE()

IMPLEMENT_CLASS(SpringShowModeSetup, PreferencePage)

void SpringShowModeSetup::CreateControls()
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    wxChoice* modes = new wxChoice(this, SPRING_MODE_CHOICE, wxDefaultPosition, wxDefaultSize,
        SPRINGSHOWMODE_NUM, kSpringShowModeStrings);
    modes->SetSelection(0);
    topsizer->Add(modes, sLeftBasicSizerFlags);

    wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1);
    sizer1->Add(
        new wxCheckBox(this, DISPLAY_YARDLINE_LEFT, wxT("Display Yardline Left")),
        sBasicSizerFlags);
    sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_RIGHT,
                    wxT("Display Yardline Right")),
        sBasicSizerFlags);
    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
    sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_ABOVE,
                    wxT("Display Yardline Above")),
        sBasicSizerFlags);
    sizer1->Add(new wxCheckBox(this, DISPLAY_YARDLINE_BELOW,
                    wxT("Display Yardline Below")),
        sBasicSizerFlags);

    auto refresh_action = [this](wxCommandEvent&) {
        this->TransferDataFromWindow();
        Refresh();
    };

    static_cast<wxCheckBox*>(FindWindow(DISPLAY_YARDLINE_LEFT))
        ->Bind(wxEVT_CHECKBOX, refresh_action);
    static_cast<wxCheckBox*>(FindWindow(DISPLAY_YARDLINE_RIGHT))
        ->Bind(wxEVT_CHECKBOX, refresh_action);
    static_cast<wxCheckBox*>(FindWindow(DISPLAY_YARDLINE_ABOVE))
        ->Bind(wxEVT_CHECKBOX, refresh_action);
    static_cast<wxCheckBox*>(FindWindow(DISPLAY_YARDLINE_BELOW))
        ->Bind(wxEVT_CHECKBOX, refresh_action);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
    AddTextboxWithCaptionAndAction(this, sizer1, SPRING_BORDER_LEFT,
        wxT("Left Border"), refresh_action,
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, SPRING_BORDER_TOP,
        wxT("Top Border"), refresh_action,
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, SPRING_BORDER_RIGHT,
        wxT("Right Border"), refresh_action,
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, SPRING_BORDER_BOTTOM,
        wxT("Bottom Border"), refresh_action,
        wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
    AddTextboxWithCaptionAndAction(this, sizer1, MODE_STEPS_X,
        wxT("Mode Offset X"), refresh_action,
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, MODE_STEPS_Y,
        wxT("Mode Offset Y"), refresh_action,
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, MODE_STEPS_W,
        wxT("Mode Offset W"), refresh_action,
        wxTE_PROCESS_ENTER);
    AddTextboxWithCaptionAndAction(this, sizer1, MODE_STEPS_H,
        wxT("Mode Offset H"), refresh_action,
        wxTE_PROCESS_ENTER);

    sizer1 = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(sizer1, sLeftBasicSizerFlags);
    wxBoxSizer* textsizer = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(textsizer, sBasicSizerFlags);
    textsizer->Add(new wxStaticText(this, wxID_STATIC,
                       wxT("Adjust yardline marker"),
                       wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    wxChoice* textchoice = new wxChoice(this, SPRING_SHOW_LINE_MARKING, wxDefaultPosition,
        wxDefaultSize,
        wxArrayString{ mConfig.Get_spr_line_text_index().size(), mConfig.Get_spr_line_text_index().data() });
    textchoice->SetSelection(0);
    textsizer->Add(textchoice);
    auto show_line_value = new wxTextCtrl(this, SPRING_SHOW_LINE_VALUE, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    show_line_value->Bind(wxEVT_TEXT_ENTER, refresh_action);
    textsizer->Add(show_line_value, sBasicSizerFlags);

    textsizer = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(textsizer, sBasicSizerFlags);
    textsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Zoom"),
                       wxDefaultPosition, wxDefaultSize, 0),
        0, wxALIGN_LEFT | wxALL, 5);
    wxArrayString zoomtext;
    for (auto& i : zoom_amounts) {
        wxString buf;
        buf.sprintf(wxT("%d%%"), i);
        zoomtext.Add(buf);
    }
    auto zoomBox = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, zoomtext);
    zoomBox->Bind(wxEVT_CHOICE, [=](wxCommandEvent& event) {
        size_t sel = event.GetInt();
        float zoom_amount = zoom_amounts[sel] / 100.0;
        static_cast<ShowModeSetupCanvas*>(FindWindow(SPRING_CANVAS))
            ->SetZoom(zoom_amount);
    });

    // set the text to the default zoom level
    textsizer->Add(zoomBox, sLeftBasicSizerFlags);

    auto modeSetupCanvas = new ShowModeSetupCanvas(mConfig, this, SPRING_CANVAS);
    modeSetupCanvas->SetScrollRate(1, 1);
    topsizer->Add(modeSetupCanvas, 1, wxEXPAND);

    modeSetupCanvas->SetMode("",
        static_cast<CalChartSpringShowModes>(mWhichMode));
    modeSetupCanvas->SetZoom(zoom_amounts[5] / 100.0);
    zoomBox->SetSelection(5);

    TransferDataToWindow();
}

void SpringShowModeSetup::Init()
{
    mWhichMode = 0;
    mWhichYardLine = 0;
    for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i) {
        mSpringShowModeValues[i] = mConfig.Get_SpringShowModeInfo(static_cast<CalChartSpringShowModes>(i));
    }
    for (size_t i = 0; i < CalChart::kSprLineTextValues; ++i) {
        mYardText[i] = mConfig.Get_spr_line_text(i);
    }
}

bool SpringShowModeSetup::TransferDataToWindow()
{
    // standard show
    for (size_t i = 0; i < 4; ++i) {
        wxCheckBox* checkbox = (wxCheckBox*)FindWindow(DISPLAY_YARDLINE_BELOW + i);
        checkbox->SetValue((mSpringShowModeValues[mWhichMode][0] & (1 << i)) > 0);
    }
    for (auto i = mSpringShowModeValues[mWhichMode].begin() + 1;
         i != (mSpringShowModeValues[mWhichMode].begin() + CalChart::ShowModeSprShow::keps_stage_x);
         ++i) {
        wxString buf;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            SPRING_BORDER_LEFT + std::distance(mSpringShowModeValues[mWhichMode].begin(), i) - 1);
        buf.Printf(wxT("%ld"), *i);
        text->ChangeValue(buf);
    }

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SPRING_SHOW_LINE_VALUE);
    text->SetValue(mConfig.Get_spr_line_text(mWhichYardLine));
    return true;
}

bool SpringShowModeSetup::TransferDataFromWindow()
{
    // read out the values from the window
    // spring show
    wxChoice* modes = (wxChoice*)FindWindow(SPRING_MODE_CHOICE);

    mSpringShowModeValues[mWhichMode][0] = 0;
    for (size_t i = 0; i < 4; ++i) {
        wxCheckBox* checkbox = (wxCheckBox*)FindWindow(DISPLAY_YARDLINE_BELOW + i);
        mSpringShowModeValues[mWhichMode][0] |= checkbox->IsChecked() ? (1 << i) : 0;
    }
    for (auto i = mSpringShowModeValues[mWhichMode].begin() + 1;
         i != (mSpringShowModeValues[mWhichMode].begin() + CalChart::ShowModeSprShow::keps_stage_x);
         ++i) {
        long val;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            SPRING_BORDER_LEFT + std::distance(mSpringShowModeValues[mWhichMode].begin(), i) - 1);
        text->GetValue().ToLong(&val);
        *i = val;
    }

    // write out the values defaults:
    for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i) {
        mConfig.Set_SpringShowModeInfo(
            static_cast<CalChartSpringShowModes>(modes->GetSelection()),
            mSpringShowModeValues[mWhichMode]);
    }

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SPRING_SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();
    for (size_t i = 0; i < CalChart::kSprLineTextValues; ++i) {
        mConfig.Set_spr_line_text(i, mYardText[i]);
    }

    return true;
}

bool SpringShowModeSetup::ClearValuesToDefault()
{
    for (size_t i = 0; i < SPRINGSHOWMODE_NUM; ++i) {
        mConfig.Clear_SpringShowModeInfo(static_cast<CalChartSpringShowModes>(i));
    }
    for (auto i = 0; i < CalChart::kSprLineTextValues; ++i) {
        mConfig.Clear_spr_line_text(i);
    }
    Init();
    return TransferDataToWindow();
}

void SpringShowModeSetup::OnCmdChoice(wxCommandEvent&)
{
    // save off all the old values:
    mSpringShowModeValues[mWhichMode][0] = 0;
    for (size_t i = 0; i < 4; ++i) {
        wxCheckBox* checkbox = (wxCheckBox*)FindWindow(DISPLAY_YARDLINE_BELOW + i);
        mSpringShowModeValues[mWhichMode][0] |= checkbox->IsChecked() ? (1 << i) : 0;
    }
    for (auto i = mSpringShowModeValues[mWhichMode].begin() + 1;
         i != (mSpringShowModeValues[mWhichMode].begin() + CalChart::ShowModeSprShow::keps_stage_x);
         ++i) {
        long val;
        wxTextCtrl* text = (wxTextCtrl*)FindWindow(
            SPRING_BORDER_LEFT + std::distance(mSpringShowModeValues[mWhichMode].begin(), i) - 1);
        text->GetValue().ToLong(&val);
        *i = val;
    }
    wxChoice* modes = (wxChoice*)FindWindow(SPRING_MODE_CHOICE);
    mWhichMode = modes->GetSelection();
    ShowModeSetupCanvas* canvas = (ShowModeSetupCanvas*)FindWindow(SPRING_CANVAS);
    canvas->SetMode("", static_cast<CalChartSpringShowModes>(mWhichMode));

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SPRING_SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();
    // update mode
    for (size_t i = 0; i < CalChart::kSprLineTextValues; ++i) {
        mConfig.Set_spr_line_text(i, mYardText[i]);
    }

    modes = (wxChoice*)FindWindow(SPRING_SHOW_LINE_MARKING);
    mWhichYardLine = modes->GetSelection();
    TransferDataToWindow();
}

BEGIN_EVENT_TABLE(CalChartPreferences, wxDialog)
EVT_BUTTON(wxID_RESET, CalChartPreferences::OnCmdResetAll)
END_EVENT_TABLE()

IMPLEMENT_CLASS(CalChartPreferences, wxDialog)

CalChartPreferences::CalChartPreferences() { Init(); }

CalChartPreferences::CalChartPreferences(wxWindow* parent, wxWindowID id,
    const wxString& caption,
    const wxPoint& pos, const wxSize& size,
    long style)
    : // make a copy of the config
    mConfig(CalChartConfiguration::GetGlobalConfig())
{
    Init();

    Create(parent, id, caption, pos, size, style);
}

CalChartPreferences::~CalChartPreferences() {}

void CalChartPreferences::Init()
{
    sBasicSizerFlags.Border(wxALL, 2).Center().Proportion(0);
    sLeftBasicSizerFlags.Border(wxALL, 2).Left().Proportion(0);
    sExpandSizerFlags.Border(wxALL, 2).Center().Proportion(0);
}

bool CalChartPreferences::Create(wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
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
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    mNotebook = new wxNotebook(this, wxID_ANY);
    topsizer->Add(mNotebook, sBasicSizerFlags);

    wxPanel* window0 = new DrawingSetup(mConfig, mNotebook, wxID_ANY);
    mNotebook->AddPage(window0, wxT("Drawing"));
    wxPanel* window1 = new GeneralSetup(mConfig, mNotebook, wxID_ANY);
    mNotebook->AddPage(window1, wxT("General"));
    wxPanel* window2 = new PSPrintingSetUp(mConfig, mNotebook, wxID_ANY);
    mNotebook->AddPage(window2, wxT("PS Printing"));
    wxPanel* window3 = new ShowModeSetup(mConfig, mNotebook, wxID_ANY);
    mNotebook->AddPage(window3, wxT("Show Mode Setup"));
    wxPanel* window4 = new SpringShowModeSetup(mConfig, mNotebook, wxID_ANY);
    mNotebook->AddPage(window4, wxT("SpringShow Mode Setup"));

    // the buttons on the bottom
    wxBoxSizer* okCancelBox = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(okCancelBox, sBasicSizerFlags);

    okCancelBox->Add(new wxButton(this, wxID_APPLY), sBasicSizerFlags);
    okCancelBox->Add(new wxButton(this, wxID_RESET, wxT("&Reset All")),
        sBasicSizerFlags);
    okCancelBox->Add(new wxButton(this, wxID_OK), sBasicSizerFlags);
    okCancelBox->Add(new wxButton(this, wxID_CANCEL), sBasicSizerFlags);
}

bool CalChartPreferences::TransferDataToWindow() { return true; }

bool CalChartPreferences::TransferDataFromWindow()
{
    // transfer everything to the config...
    size_t pages = mNotebook->GetPageCount();
    for (size_t i = 0; i < pages; ++i) {
        PreferencePage* page = static_cast<PreferencePage*>(mNotebook->GetPage(i));
        page->TransferDataFromWindow();
    }
    CalChartConfiguration::AssignConfig(mConfig);
    return true;
}

void CalChartPreferences::OnCmdResetAll(wxCommandEvent&)
{
    // transfer everything to the config...
    size_t pages = mNotebook->GetPageCount();
    for (size_t i = 0; i < pages; ++i) {
        PreferencePage* page = static_cast<PreferencePage*>(mNotebook->GetPage(i));
        page->ClearValuesToDefault();
    }
    CalChartConfiguration::AssignConfig(mConfig);
}
