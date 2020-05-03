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
#include "CalChartDoc.h"
#include "ContinuityBrowserPanel.h"
#include "ContinuityComposerDialog.h"
#include "cc_drawcommand.h"
#include "cc_shapes.h"
#include "cc_sheet.h"
#include "confgr.h"
#include "cont.h"
#include "draw.h"
#include "mode_dialog.h"
#include "mode_dialog_canvas.h"
#include "modes.h"

#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/stattext.h>

using namespace CalChart;

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
static auto sBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Center().Proportion(0);
static auto sLeftBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Left().Proportion(0);
static auto sRightBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Right().Proportion(0);
static auto sExpandSizerFlags = wxSizerFlags().Border(wxALL, 2).Center().Proportion(0);

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
    bool mBeep_On_Collisions;
    bool mScroll_Natural;
    bool mSetSheet_Undo;
    bool mSelection_Undo;
};

enum {
    AUTOSAVE_INTERVAL = 1000,
    BEEP_ON_COLLISIONS,
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
    AddCheckboxWithCaption(this, sizer1, BEEP_ON_COLLISIONS,
        wxT("Beep on animation collisions"));
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
    mBeep_On_Collisions = mConfig.Get_BeepOnCollisions();
    mScroll_Natural = mConfig.Get_ScrollDirectionNatural();
    mSetSheet_Undo = mConfig.Get_CommandUndoSetSheet();
    mSelection_Undo = mConfig.Get_CommandUndoSelection();
}

bool GeneralSetup::TransferDataToWindow()
{
    ((wxTextCtrl*)FindWindow(AUTOSAVE_INTERVAL))->SetValue(mAutoSave_Interval);
    ((wxCheckBox*)FindWindow(BEEP_ON_COLLISIONS))->SetValue(mBeep_On_Collisions);
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
    mBeep_On_Collisions = ((wxCheckBox*)FindWindow(BEEP_ON_COLLISIONS))->GetValue();
    mConfig.Set_BeepOnCollisions(mBeep_On_Collisions);
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
    mConfig.Clear_CalChartFrameAUILayout();
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
    CalChart::ShowMode mMode;
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
    , mMode(ShowMode::CreateShowMode(
          Coord(Int2CoordUnits(160), Int2CoordUnits(84)),
          Coord(Int2CoordUnits(80), Int2CoordUnits(42)),
          Coord(Int2CoordUnits(4), Int2CoordUnits(4)),
          Coord(Int2CoordUnits(4), Int2CoordUnits(4)), Int2CoordUnits(32), Int2CoordUnits(52),
          ShowMode::GetDefaultYardLines()))
    , mConfig(config)
{
    auto field_offset = mMode.FieldOffset();
    auto offset = mMode.Offset();
    SetCanvasSize(wxSize{ mMode.Size().x, mMode.Size().y });

    // Create a fake show with some points and selections to draw an example for
    // the user
    mShow = Show::Create_CC_show(ShowMode::GetDefaultShowMode());
    auto labels = std::vector<std::string>{
        "unsel",
        "unsel",
        "sel",
        "sel",
    };
    mShow->Create_SetShowInfoCommand(labels, 4, field_offset).first(*mShow);
    mShow->Create_SetShowInfoCommand(labels, 4, field_offset).first(*mShow);
    mShow->Create_SetSelectionCommand(SelectionList{ 0, 2 }).first(*mShow);
    mShow->Create_SetSymbolCommand(SYMBOL_X).first(*mShow);
    mShow->Create_SetSelectionCommand(SelectionList{ 1, 3 }).first(*mShow);
    mShow->Create_SetSymbolCommand(SYMBOL_SOLX).first(*mShow);
    mShow->Create_SetSelectionCommand(SelectionList{}).first(*mShow);

    for (auto i = 0; i < 4; ++i) {
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 0).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 1).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 2).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 3).first(*mShow);

        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(6)) } }, 1).first(*mShow);
    }

    mShow->Create_AddSheetsCommand(Show::Sheet_container_t{ *static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet() }, 1).first(*mShow);
    mShow->Create_SetCurrentSheetCommand(1).first(*mShow);
    for (auto i = 0; i < 4; ++i) {
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 0).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 1).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 2).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 3).first(*mShow);

        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 6)) } }, 1).first(*mShow);
    }
    mShow->Create_SetCurrentSheetCommand(0).first(*mShow);

    auto point_start = offset + field_offset + Coord(Int2CoordUnits(4), Int2CoordUnits(2));
    mPathEnd = point_start + Coord(Int2CoordUnits(0), Int2CoordUnits(2));
    mPath.emplace_back(point_start, mPathEnd);
    point_start = mPathEnd;
    mPathEnd += Coord(Int2CoordUnits(18), Int2CoordUnits(0));
    mPath.emplace_back(point_start, mPathEnd);

    auto shape_start = field_offset + Coord(Int2CoordUnits(18), Int2CoordUnits(-2));
    auto shape_end = shape_start + Coord(Int2CoordUnits(4), Int2CoordUnits(4));
    Shape_rect rect(shape_start, shape_end);
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
    DrawMode(dc, mConfig, mMode, ShowMode_kFieldView);

    auto sheet = static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet();
    auto nextSheet = sheet;
    ++nextSheet;

    SelectionList list;
    list.insert(2);
    list.insert(3);

    // draw the ghost sheet
    DrawGhostSheet(dc, mConfig, mMode.Offset(), list, mShow->GetNumPoints(),
        mShow->GetPointLabels(), *nextSheet, 0);

    // Draw the points
    DrawPoints(dc, mConfig, mMode.Offset(), list, mShow->GetNumPoints(),
        mShow->GetPointLabels(), *sheet, 0, true);
    DrawPoints(dc, mConfig, mMode.Offset(), list, mShow->GetNumPoints(),
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
    for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
        SetColor(i, mConfig.GetDefaultPenWidth()[i], mConfig.GetDefaultColors()[i]);
        mConfig.Clear_CalChartConfigColor(i);
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
    mConfig.Clear_CalChartConfigColor(static_cast<CalChartColors>(selection));
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
    CalChart::ShowMode::YardLinesInfo_t mYardText;
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

    modeSetupCanvas->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(mWhichMode)), mYardText));
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
    for (size_t i = 0; i < kYardTextValues; ++i) {
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
    for (size_t i = 0; i < kYardTextValues; ++i) {
        mConfig.Set_yard_text(i, mYardText[i]);
    }
    // now set the canvas
    ((ShowModeSetupCanvas*)FindWindow(CANVAS))->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(mWhichMode)), mYardText));

    return true;
}

bool ShowModeSetup::ClearValuesToDefault()
{
    for (size_t i = 0; i < SHOWMODE_NUM; ++i) {
        mConfig.Clear_ShowModeInfo(static_cast<CalChartShowModes>(i));
    }
    for (auto i = 0; i < kYardTextValues; ++i) {
        mConfig.Clear_yard_text(i);
    }
    Init();
    ((ShowModeSetupCanvas*)FindWindow(CANVAS))->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(mWhichMode)), mYardText));
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
    canvas->SetMode(CalChart::ShowMode::CreateShowMode(mConfig.Get_ShowModeInfo(static_cast<CalChartShowModes>(mWhichMode)), mYardText));

    wxTextCtrl* text = (wxTextCtrl*)FindWindow(SHOW_LINE_VALUE);
    mYardText[mWhichYardLine] = text->GetValue();
    // update mode
    for (size_t i = 0; i < kYardTextValues; ++i) {
        mConfig.Set_yard_text(i, mYardText[i]);
    }

    modes = (wxChoice*)FindWindow(SHOW_LINE_MARKING);
    mWhichYardLine = modes->GetSelection();
    TransferDataToWindow();
}

////////////////
class ContCellSetup : public PreferencePage {
    DECLARE_CLASS(ContCellSetup)
    DECLARE_EVENT_TABLE()

public:
    ContCellSetup(CalChartConfiguration& config, wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("ContCell Setup"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
        : PreferencePage(config)
    {
        Init();
        Create(parent, id, caption, pos, size, style);
    }
    virtual ~ContCellSetup() {}

    virtual void Init();
    virtual void CreateControls();

    // use these to get and set default values
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    virtual bool ClearValuesToDefault();

private:
    void OnCmdLongForm(wxCommandEvent&);
    void OnCmdFontSize(wxSpinEvent&);
    void OnCmdRounding(wxSpinEvent&);
    void OnCmdTextPadding(wxSpinEvent&);
    void OnCmdBoxPadding(wxSpinEvent&);
    void OnCmdSelectWidth(wxSpinEvent&);

    void OnCmdSelectColors(wxCommandEvent&);
    void OnCmdChooseNewColor(wxCommandEvent&);
    void OnCmdResetColors(wxCommandEvent&);
    void SetColor(int selection, const wxColour& color);

    // we can set up the Font, colors, size.
    wxBitmapComboBox* nameBox;
    wxBrush mContCellBrushes[COLOR_CONTCELLS_NUM];
};

enum {
    CHECK_LongForm,
    SPIN_Font_Size,
    SPIN_Rouding,
    SPIN_Text_Padding,
    SPIN_Box_Padding,
};

BEGIN_EVENT_TABLE(ContCellSetup, PreferencePage)
EVT_CHECKBOX(CHECK_LongForm, ContCellSetup::OnCmdLongForm)
EVT_SPINCTRL(SPIN_Font_Size, ContCellSetup::OnCmdFontSize)
EVT_SPINCTRL(SPIN_Rouding, ContCellSetup::OnCmdRounding)
EVT_SPINCTRL(SPIN_Text_Padding, ContCellSetup::OnCmdTextPadding)
EVT_SPINCTRL(SPIN_Box_Padding, ContCellSetup::OnCmdBoxPadding)
EVT_BUTTON(BUTTON_SELECT, ContCellSetup::OnCmdSelectColors)
EVT_BUTTON(BUTTON_RESTORE, ContCellSetup::OnCmdResetColors)
EVT_COMBOBOX(NEW_COLOR_CHOICE, ContCellSetup::OnCmdChooseNewColor)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ContCellSetup, PreferencePage)

template <typename T>
static auto do_cloning(T const& cont)
{
    std::vector<std::unique_ptr<ContProcedure>> copied_cont;
    for (auto&& i : cont.GetParsedContinuity()) {
        copied_cont.emplace_back(i->clone());
    }
    return copied_cont;
}

void ContCellSetup::CreateControls()
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    auto boxsizer = new wxStaticBoxSizer(
        new wxStaticBox(this, -1, wxT("Color settings")), wxVERTICAL);
    topsizer->Add(boxsizer);

    auto horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    boxsizer->Add(horizontalsizer, sLeftBasicSizerFlags);
    nameBox = new wxBitmapComboBox(
        this, NEW_COLOR_CHOICE, mConfig.GetContCellColorNames().at(0), wxDefaultPosition,
        wxDefaultSize, COLOR_CONTCELLS_NUM, mConfig.GetContCellColorNames().data(),
        wxCB_READONLY | wxCB_DROPDOWN);
    horizontalsizer->Add(nameBox, sBasicSizerFlags);

    for (auto i = 0; i < COLOR_CONTCELLS_NUM; ++i) {
        wxBitmap temp_bitmap(16, 16);
        wxMemoryDC temp_dc;
        temp_dc.SelectObject(temp_bitmap);
        temp_dc.SetBackground(mConfig.Get_ContCellBrushAndPen(static_cast<ContCellColors>(i)).first);
        temp_dc.Clear();
        nameBox->SetItemBitmap(i, temp_bitmap);
    }
    nameBox->SetSelection(0);

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    boxsizer->Add(horizontalsizer, sLeftBasicSizerFlags);
    horizontalsizer->Add(new wxButton(this, BUTTON_SELECT, wxT("&Change Color")), sBasicSizerFlags);
    horizontalsizer->Add(new wxButton(this, BUTTON_RESTORE, wxT("&Reset Color")), sBasicSizerFlags);

    horizontalsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Cont Cell settings")), wxHORIZONTAL);
    topsizer->Add(horizontalsizer);

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Long form")), wxVERTICAL);
    horizontalsizer->Add(boxsizer, sBasicSizerFlags);
    auto checkbox = new wxCheckBox(this, CHECK_LongForm, wxT("Long form"));
    checkbox->SetValue(mConfig.Get_ContCellLongForm());
    boxsizer->Add(checkbox, sBasicSizerFlags);

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Font Size")), wxVERTICAL);
    horizontalsizer->Add(boxsizer, sBasicSizerFlags);
    auto spin = new wxSpinCtrl(this, SPIN_Font_Size, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 30, mConfig.Get_ContCellFontSize());
    boxsizer->Add(spin, sBasicSizerFlags);

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Rounding")), wxVERTICAL);
    horizontalsizer->Add(boxsizer, sBasicSizerFlags);
    spin = new wxSpinCtrl(this, SPIN_Rouding, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mConfig.Get_ContCellRounding());
    boxsizer->Add(spin, sBasicSizerFlags);

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Text Padding")), wxVERTICAL);
    horizontalsizer->Add(boxsizer, sBasicSizerFlags);
    spin = new wxSpinCtrl(this, SPIN_Text_Padding, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mConfig.Get_ContCellTextPadding());
    boxsizer->Add(spin, sBasicSizerFlags);

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Box Padding")), wxVERTICAL);
    horizontalsizer->Add(boxsizer, sBasicSizerFlags);
    spin = new wxSpinCtrl(this, SPIN_Box_Padding, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mConfig.Get_ContCellBoxPadding());
    boxsizer->Add(spin, sBasicSizerFlags);

    auto canvas = new ContinuityBrowserPanel(SYMBOL_PLAIN, mConfig, this);
    topsizer->Add(canvas, 1, wxEXPAND);
    auto basic_cont = CalChart::Continuity{ "ewns np\nX = distfrom(sp r2)\nmt (24-X)w\nmarch gv dist(np) dir(np) w\nmtrm e" };
    auto clonedOut = do_cloning(basic_cont);
    clonedOut.emplace_back(std::make_unique<CalChart::ContProcMT>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()));
    canvas->DoSetContinuity(CalChart::Continuity{ std::move(clonedOut) });

    TransferDataToWindow();
}

void ContCellSetup::Init()
{
    // first read out the defaults:
    for (auto i = 0; i < COLOR_CONTCELLS_NUM; ++i) {
        auto brushAndPen = mConfig.Get_ContCellBrushAndPen(static_cast<ContCellColors>(i));
        mContCellBrushes[i] = brushAndPen.first;
    }
}

bool ContCellSetup::TransferDataToWindow()
{
    return true;
}

bool ContCellSetup::TransferDataFromWindow()
{
    return true;
}

bool ContCellSetup::ClearValuesToDefault()
{
    mConfig.Clear_ContCellLongForm();
    mConfig.Clear_ContCellFontSize();
    mConfig.Clear_ContCellRounding();
    mConfig.Clear_ContCellTextPadding();
    mConfig.Clear_ContCellBoxPadding();

    for (ContCellColors i = COLOR_CONTCELLS_PROC; i < COLOR_CONTCELLS_NUM; i = static_cast<ContCellColors>(static_cast<int>(i) + 1)) {
        SetColor(i, mConfig.GetContCellDefaultColors()[i]);
        mConfig.Clear_ContCellConfigColor(i);
    }
    Init();
    TransferDataToWindow();
    return true;
}

void ContCellSetup::OnCmdLongForm(wxCommandEvent& e)
{
    mConfig.Set_ContCellLongForm(e.IsChecked());
    Refresh();
}

void ContCellSetup::OnCmdFontSize(wxSpinEvent& e)
{
    mConfig.Set_ContCellFontSize(e.GetValue());
    Refresh();
}

void ContCellSetup::OnCmdRounding(wxSpinEvent& e)
{
    mConfig.Set_ContCellRounding(e.GetValue());
    Refresh();
}

void ContCellSetup::OnCmdTextPadding(wxSpinEvent& e)
{
    mConfig.Set_ContCellTextPadding(e.GetValue());
    Refresh();
}

void ContCellSetup::OnCmdBoxPadding(wxSpinEvent& e)
{
    mConfig.Set_ContCellBoxPadding(e.GetValue());
    Refresh();
}

void ContCellSetup::SetColor(int selection, const wxColour& color)
{
    auto pen = *wxThePenList->FindOrCreatePen(color, 1, wxPENSTYLE_SOLID);
    mContCellBrushes[selection] = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

    mConfig.Set_ContCellBrushAndPen(static_cast<ContCellColors>(selection),
        mContCellBrushes[selection],
        pen);

    // update the namebox list
    {
        wxBitmap test_bitmap(16, 16);
        wxMemoryDC temp_dc;
        temp_dc.SelectObject(test_bitmap);
        temp_dc.SetBackground(mContCellBrushes[selection]);
        temp_dc.Clear();
        nameBox->SetItemBitmap(selection, test_bitmap);
    }
    Refresh();
}

void ContCellSetup::OnCmdSelectColors(wxCommandEvent&)
{
    int selection = nameBox->GetSelection();
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(mContCellBrushes[selection].GetColour());
    wxColourDialog dialog(this, &data);
    if (dialog.ShowModal() == wxID_OK) {
        wxColourData retdata = dialog.GetColourData();
        wxColour c = retdata.GetColour();
        SetColor(selection, c);
    }
    Refresh();
}

void ContCellSetup::OnCmdResetColors(wxCommandEvent&)
{
    int selection = nameBox->GetSelection();
    SetColor(selection, mConfig.GetContCellDefaultColors()[selection]);
    mConfig.Clear_ContCellConfigColor(static_cast<ContCellColors>(selection));
    Refresh();
}

void ContCellSetup::OnCmdChooseNewColor(wxCommandEvent&)
{
    Refresh();
}

////////////////

BEGIN_EVENT_TABLE(CalChartPreferences, wxDialog)
EVT_BUTTON(wxID_RESET, CalChartPreferences::OnCmdResetAll)
END_EVENT_TABLE()

IMPLEMENT_CLASS(CalChartPreferences, wxDialog)

CalChartPreferences::CalChartPreferences(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, caption, pos, size, style)
    , mConfig(CalChartConfiguration::GetGlobalConfig())
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    mNotebook = new wxNotebook(this, wxID_ANY);
    topsizer->Add(mNotebook, sBasicSizerFlags);

    mNotebook->AddPage(new GeneralSetup(mConfig, mNotebook, wxID_ANY), wxT("General"));
    mNotebook->AddPage(new ContCellSetup(mConfig, mNotebook, wxID_ANY), wxT("Continuity"));
    mNotebook->AddPage(new DrawingSetup(mConfig, mNotebook, wxID_ANY), wxT("Drawing"));
    mNotebook->AddPage(new PSPrintingSetUp(mConfig, mNotebook, wxID_ANY), wxT("PS Printing"));
    mNotebook->AddPage(new ShowModeSetup(mConfig, mNotebook, wxID_ANY), wxT("Show Mode Setup"));

    // the buttons on the bottom
    wxBoxSizer* okCancelBox = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(okCancelBox, sBasicSizerFlags);

    okCancelBox->Add(new wxButton(this, wxID_APPLY), sBasicSizerFlags);
    okCancelBox->Add(new wxButton(this, wxID_RESET, wxT("&Reset All")), sBasicSizerFlags);
    okCancelBox->Add(new wxButton(this, wxID_OK), sBasicSizerFlags);
    okCancelBox->Add(new wxButton(this, wxID_CANCEL), sBasicSizerFlags);

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);
    Center();
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
