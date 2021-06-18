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

#include "ContinuityComposerDialog.h"
#include "CalChartAnimationTypes.h"
#include "CalChartConfiguration.h"
#include "CalChartContinuity.h"
#include "CalChartContinuityToken.h"
#include "ContinuityBoxDrawer.h"
#include "CustomListViewPanel.h"
#include "basic_ui.h"

class ContinuityComposerCanvas : public CustomListViewPanel {
    using super = CustomListViewPanel;

public:
    // Basic functions
    ContinuityComposerCanvas(CalChartConfiguration& config, wxWindow* parent, wxWindowID winid = wxID_ANY, wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxScrolledWindowStyle, wxString const& name = wxPanelNameStr);
    ~ContinuityComposerCanvas() override = default;
    void DoSetContinuity(CalChart::DrawableCont const& drawableCont, std::function<void(CalChart::DrawableCont const&)> action);

private:
    CalChartConfiguration& mConfig;
};

// Define a constructor for field canvas
ContinuityComposerCanvas::ContinuityComposerCanvas(CalChartConfiguration& config, wxWindow* parent, wxWindowID winid, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : super(parent, winid, pos, size, style, name)
    , mConfig(config)
{
    // use the config size to determine how big to make things,
    auto current_size = GetMinSize();
    current_size.y = ContinuityBoxDrawer::GetHeight(config);
    current_size.y += wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y);
    SetMinSize(current_size);
}

void ContinuityComposerCanvas::DoSetContinuity(CalChart::DrawableCont const& drawableCont, std::function<void(CalChart::DrawableCont const&)> action)
{
    std::vector<std::unique_ptr<DrawableCell>> contCells;
    contCells.emplace_back(std::make_unique<ContinuityBoxDrawer>(drawableCont, mConfig, action));
    SetCells(std::move(contCells));
    Refresh();
}

class ContinuityComposerPanel : public wxPanel {
    using super = wxPanel;
    DECLARE_CLASS(ContinuityComposerPanel)

public:
    ContinuityComposerPanel(std::unique_ptr<CalChart::ContProcedure> starting_continuity, CalChartConfiguration& config, wxWindow* parent, wxWindowID winid = wxID_ANY, wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER, wxString const& name = wxPanelNameStr);
    ~ContinuityComposerPanel() override = default;

    std::unique_ptr<CalChart::ContProcedure> GetContinuity();
    bool Validate() override;
    void SetOnUpdateIsValid(std::function<void(bool)> const& f) { mOnUpdateIsValid = f; }
    auto GetOnUpdateIsValid() const { return mOnUpdateIsValid; }

    void OnUpdate();

private:
    void Init();
    void CreateControls();

    // Event Handlers
    void OnDrawableContClick(CalChart::DrawableCont const& c);
    void OnCmdTextEnterKeyPressed(wxCommandEvent const& event);

    std::unique_ptr<CalChart::ContProcedure> mCont;
    CalChart::DrawableCont mDrawableCont;
    ContinuityComposerCanvas* mCanvas{};
    wxComboBox* mComboSelection{};
    CalChart::ContToken const* mCurrentSelected = nullptr;
    CalChart::ContToken* mCurrentParent = nullptr;
    std::function<void(CalChart::DrawableCont const& c)> const mAction;
    CalChartConfiguration& mConfig;
    std::function<void(bool)> mOnUpdateIsValid{};
};

IMPLEMENT_CLASS(ContinuityComposerPanel, wxPanel)

static const auto sRightBasicSizerFlags = wxSizerFlags{}.Border(wxALL, 2).Right().Proportion(0);
static const auto sExpandSizerFlags = wxSizerFlags{}.Border(wxALL, 2).Center().Proportion(0);

// find unselected and parent
std::pair<CalChart::ContToken const*, CalChart::ContToken*> first_unset(CalChart::DrawableCont const& cont)
{
    if (cont.type == CalChart::ContType::unset) {
        return { cont.self_ptr, cont.parent_ptr };
    }
    for (auto&& i : cont.args) {
        auto result = first_unset(i);
        if (result.first) {
            return result;
        }
    }
    return { nullptr, nullptr };
}

ContinuityComposerPanel::ContinuityComposerPanel(std::unique_ptr<CalChart::ContProcedure> starting_continuity, CalChartConfiguration& config, wxWindow* parent,
    wxWindowID winid, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : wxPanel(parent, winid, pos, size, style, name)
    , mCont(std::move(starting_continuity))
    , mAction([this](CalChart::DrawableCont const& c) { this->OnDrawableContClick(c); })
    , mConfig(config)
{
    Init();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    OnUpdate();
}

void ContinuityComposerPanel::Init()
{
}

void ContinuityComposerPanel::CreateControls()
{
    mCanvas = new ContinuityComposerCanvas(mConfig, this);
    SetSizer(VStack([this](auto sizer) {
        sizer->Add(mCanvas, 1, wxEXPAND);

        mComboSelection = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, (long)wxTE_PROCESS_ENTER);
        sizer->Add(mComboSelection, 0, wxEXPAND | wxALL, 5);
        mComboSelection->Bind(wxEVT_TEXT_ENTER, [this](auto const& event) {
            this->OnCmdTextEnterKeyPressed(event);
        });
        mComboSelection->Bind(wxEVT_COMBOBOX, [this](auto const& event) {
            this->OnCmdTextEnterKeyPressed(event);
        });
    }));

    if (!mCont) {
        mCont = std::make_unique<CalChart::ContProcUnset>();
    }
    mDrawableCont = mCont->GetDrawableCont();
    std::tie(mCurrentSelected, mCurrentParent) = first_unset(mDrawableCont);
    // preset the highlighted
    mCanvas->DoSetContinuity(mDrawableCont, mAction);
    mCanvas->SetHighlight(mCurrentSelected);
}

enum class TokenType {
    Do_Continuity,
    Do_Values,
    Do_Points,
    Do_ValuesVarUnset,
};

enum class ContProc {
    Blam,
    CM,
    DMCM,
    DMHS,
    Even,
    EWNS,
    Fountain,
    FM,
    FMTO,
    Grid,
    HSCM,
    HSDM,
    Magic,
    March,
    MT,
    MTRM,
    NSEW,
    Rotate,
    Set,
    LAST
};

const std::pair<std::string, std::function<std::unique_ptr<CalChart::ContProcedure>()>> ContMap[] = {
    { "BLAM", []() { return std::make_unique<CalChart::ContProcBlam>(); } },
    { "Counter March", []() { return std::make_unique<CalChart::ContProcCM>(std::make_unique<CalChart::ContPointUnset>(), std::make_unique<CalChart::ContPointUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "Diagonal Military Counter March", []() { return std::make_unique<CalChart::ContProcDMCM>(std::make_unique<CalChart::ContPointUnset>(), std::make_unique<CalChart::ContPointUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "Diagonal Military High Step", []() { return std::make_unique<CalChart::ContProcDMHS>(std::make_unique<CalChart::ContPointUnset>()); } },
    { "EVEN step", []() { return std::make_unique<CalChart::ContProcEven>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContPointUnset>()); } },
    { "East/West North/South", []() { return std::make_unique<CalChart::ContProcEWNS>(std::make_unique<CalChart::ContPointUnset>()); } },
    { "FOUNTAIN", []() { return std::make_unique<CalChart::ContProcFountain>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContPointUnset>()); } },
    { "Forward March", []() { return std::make_unique<CalChart::ContProcFM>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "Forward March TO", []() { return std::make_unique<CalChart::ContProcFMTO>(std::make_unique<CalChart::ContPointUnset>()); } },
    { "GRID", []() { return std::make_unique<CalChart::ContProcGrid>(std::make_unique<CalChart::ContValueUnset>()); } },
    { "High Steps Counter March", []() { return std::make_unique<CalChart::ContProcHSCM>(std::make_unique<CalChart::ContPointUnset>(), std::make_unique<CalChart::ContPointUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "High Steps Diagonal Military", []() { return std::make_unique<CalChart::ContProcHSDM>(std::make_unique<CalChart::ContPointUnset>()); } },
    { "MAGIC", []() { return std::make_unique<CalChart::ContProcMagic>(std::make_unique<CalChart::ContPointUnset>()); } },
    { "MARCH", []() { return std::make_unique<CalChart::ContProcMarch>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "MarkTime", []() { return std::make_unique<CalChart::ContProcMT>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "MarkTime ReMaining", []() { return std::make_unique<CalChart::ContProcMTRM>(std::make_unique<CalChart::ContValueUnset>()); } },
    { "North/South East/West", []() { return std::make_unique<CalChart::ContProcNSEW>(std::make_unique<CalChart::ContPointUnset>()); } },
    { "ROTATE", []() { return std::make_unique<CalChart::ContProcRotate>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContPointUnset>()); } },
    { "SET", []() { return std::make_unique<CalChart::ContProcSet>(std::make_unique<CalChart::ContValueVarUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
};

static_assert(sizeof(ContMap) / sizeof(ContMap[0]) == static_cast<int>(ContProc::LAST), "");

// based on usage, this is the order we should populate
const ContProc ContUsageOrder[] = {
    ContProc::MTRM,
    ContProc::MT,
    ContProc::NSEW,
    ContProc::EWNS,
    ContProc::FM,
    ContProc::March,
    ContProc::DMHS,
    ContProc::HSDM,
    ContProc::Rotate,
    ContProc::Even,
    ContProc::Set,
    ContProc::FMTO,
    ContProc::Magic,
    ContProc::Blam,
    ContProc::CM,
    ContProc::DMCM,
    ContProc::Fountain,
    ContProc::Grid,
    ContProc::HSCM,
};

enum class ContValue {
    // float is special
    Add,
    Sub,
    Mult,
    Divide,
    Neg,
    Remaining,
    DirTo,
    DirFrom,
    DistTo,
    DistFrom,
    Either,
    Opposite,
    StepDrill,
    A,
    B,
    C,
    D,
    X,
    Y,
    Z,
    DOF,
    DOH,
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW,
    HS,
    MM,
    SH,
    JS,
    GV,
    M,
    DM,
    LAST
};

// based on usage, this is the order we should populate
const ContValue ValueUsageOrder[] = {
    ContValue::N,
    ContValue::E,
    ContValue::S,
    ContValue::W,
    ContValue::NE,
    ContValue::SE,
    ContValue::SW,
    ContValue::NW,
    ContValue::StepDrill,
    ContValue::Remaining,
    ContValue::Neg,
    ContValue::Add,
    ContValue::Sub,
    ContValue::Mult,
    ContValue::Divide,
    ContValue::DirTo,
    ContValue::DirFrom,
    ContValue::DistTo,
    ContValue::DistFrom,
    ContValue::Opposite,
    ContValue::DOF,
    ContValue::DOH,
    ContValue::A,
    ContValue::B,
    ContValue::C,
    ContValue::D,
    ContValue::X,
    ContValue::Y,
    ContValue::Z,
    ContValue::HS,
    ContValue::MM,
    ContValue::SH,
    ContValue::JS,
    ContValue::GV,
    ContValue::M,
    ContValue::DM,
};

const ContValue SettableValueVar[] = {
    ContValue::A,
    ContValue::B,
    ContValue::C,
    ContValue::D,
    ContValue::X,
    ContValue::Y,
    ContValue::Z,
};

const std::pair<std::string, std::function<std::unique_ptr<CalChart::ContValue>()>> ValueMap[] = {
    { "+", []() { return std::make_unique<CalChart::ContValueAdd>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "-", []() { return std::make_unique<CalChart::ContValueSub>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "*", []() { return std::make_unique<CalChart::ContValueMult>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "/", []() { return std::make_unique<CalChart::ContValueDiv>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()); } },
    { "NEG", []() { return std::make_unique<CalChart::ContValueNeg>(std::make_unique<CalChart::ContValueUnset>()); } },
    { "REMaining", []() { return std::make_unique<CalChart::ContValueREM>(); } },
    { "DIRection TO", []() { return std::make_unique<CalChart::ContFuncDir>(std::make_unique<CalChart::ContPointUnset>()); } },
    { "DIRection From", []() { return std::make_unique<CalChart::ContFuncDirFrom>(std::make_unique<CalChart::ContPointUnset>(), std::make_unique<CalChart::ContPointUnset>()); } },
    { "DIStant To", []() { return std::make_unique<CalChart::ContFuncDist>(std::make_unique<CalChart::ContPointUnset>()); } },
    { "DIStant From", []() { return std::make_unique<CalChart::ContFuncDistFrom>(std::make_unique<CalChart::ContPointUnset>(), std::make_unique<CalChart::ContPointUnset>()); } },
    { "EITHER", []() { return std::make_unique<CalChart::ContFuncEither>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContPointUnset>()); } },
    { "OPPosite", []() { return std::make_unique<CalChart::ContFuncOpp>(std::make_unique<CalChart::ContValueUnset>()); } },
    { "STEP drill", []() { return std::make_unique<CalChart::ContFuncStep>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContPointUnset>()); } },
    { "A", []() { return std::make_unique<CalChart::ContValueVar>(CalChart::CONTVAR_A); } },
    { "B", []() { return std::make_unique<CalChart::ContValueVar>(CalChart::CONTVAR_B); } },
    { "C", []() { return std::make_unique<CalChart::ContValueVar>(CalChart::CONTVAR_C); } },
    { "D", []() { return std::make_unique<CalChart::ContValueVar>(CalChart::CONTVAR_D); } },
    { "X", []() { return std::make_unique<CalChart::ContValueVar>(CalChart::CONTVAR_X); } },
    { "Y", []() { return std::make_unique<CalChart::ContValueVar>(CalChart::CONTVAR_Y); } },
    { "Z", []() { return std::make_unique<CalChart::ContValueVar>(CalChart::CONTVAR_Z); } },
    { "Direction Of Flow", []() { return std::make_unique<CalChart::ContValueVar>(CalChart::CONTVAR_DOF); } },
    { "Direction Of Horn", []() { return std::make_unique<CalChart::ContValueVar>(CalChart::CONTVAR_DOH); } },
    { "North", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_N); } },
    { "NorthEast", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_NE); } },
    { "East", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_E); } },
    { "SouthEast", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_SE); } },
    { "South", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_S); } },
    { "SouthWest", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_SW); } },
    { "West", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_W); } },
    { "NorthWest", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_NW); } },
    { "HighStep", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_HS); } },
    { "Mini-Military", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_MM); } },
    { "ShowHigh", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_SH); } },
    { "Jerky Step", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_JS); } },
    { "GrapeVine", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_GV); } },
    { "Military", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_M); } },
    { "Diagonal Military", []() { return std::make_unique<CalChart::ContValueDefined>(CalChart::CC_DM); } },
};

static_assert(sizeof(ValueMap) / sizeof(ValueMap[0]) == static_cast<int>(ContValue::LAST), "");

enum class ContPoint {
    Point,
    StartPoint,
    NextPoint,
    RefPoint1,
    RefPoint2,
    RefPoint3,
    LAST
};

// based on usage, this is the order we should populate
const ContPoint PointUsageOrder[] = {
    ContPoint::Point,
    ContPoint::StartPoint,
    ContPoint::NextPoint,
    ContPoint::RefPoint1,
    ContPoint::RefPoint2,
    ContPoint::RefPoint3,
};

const std::pair<std::string, std::function<std::unique_ptr<CalChart::ContPoint>()>> PointMap[] = {
    { "Point", []() { return std::make_unique<CalChart::ContPoint>(); } },
    { "Starting Point", []() { return std::make_unique<CalChart::ContStartPoint>(); } },
    { "Next Point", []() { return std::make_unique<CalChart::ContNextPoint>(); } },
    { "Ref Point 1", []() { return std::make_unique<CalChart::ContRefPoint>(1); } },
    { "Ref Point 2", []() { return std::make_unique<CalChart::ContRefPoint>(2); } },
    { "Ref Point 3", []() { return std::make_unique<CalChart::ContRefPoint>(3); } },
};

static_assert(sizeof(ValueMap) / sizeof(ValueMap[0]) == static_cast<int>(ContValue::LAST), "");

static const auto ContMapStrings = []() {
    std::vector<std::string> result;
    for (auto&& i : ContUsageOrder) {
        result.push_back(ContMap[static_cast<int>(i)].first);
    }
    return result;
}();

static const auto ValueMapStrings = []() {
    std::vector<std::string> result;
    for (auto&& i : ValueUsageOrder) {
        result.push_back(ValueMap[static_cast<int>(i)].first);
    }
    return result;
}();

static const auto ValueVarMapStrings = []() {
    std::vector<std::string> result;
    for (auto&& i : SettableValueVar) {
        result.push_back(ValueMap[static_cast<int>(i)].first);
    }
    return result;
}();

const auto PointMapStrings = []() {
    std::vector<std::string> result;
    for (auto&& i : PointUsageOrder) {
        result.push_back(PointMap[static_cast<int>(i)].first);
    }
    return result;
}();

// if the entry matches the upper case letters
bool string_matches_upper_case(std::string const& target, std::string const& check_against)
{
    std::string new_string;
    std::copy_if(check_against.begin(), check_against.end(), std::back_inserter(new_string), [](auto&& i) { return isupper(i); });
    return std::equal(target.begin(), target.end(), new_string.begin(), new_string.end(), [](auto&& a, auto&& b) { return tolower(a) == tolower(b); });
}

auto WhatType(CalChart::ContToken const* ptr)
{
    if (dynamic_cast<CalChart::ContProcedure const*>(ptr)) {
        return TokenType::Do_Continuity;
    }
    if (dynamic_cast<CalChart::ContValueVarUnset const*>(ptr)) {
        return TokenType::Do_ValuesVarUnset;
    }
    if (dynamic_cast<CalChart::ContValue const*>(ptr)) {
        return TokenType::Do_Values;
    }
    if (dynamic_cast<CalChart::ContPoint const*>(ptr)) {
        return TokenType::Do_Points;
    }
    throw std::runtime_error("unknown pointer type");
}

std::vector<std::string> GetCurrentList(CalChart::ContToken const* selected, std::string text_entered)
{
    std::vector<std::string> result;
    if (selected) {
        // set up the pull down with all the stuff
        switch (WhatType(selected)) {
        case TokenType::Do_Continuity:
            result = ContMapStrings;
            break;
        case TokenType::Do_Values:
            result = ValueMapStrings;
            break;
        case TokenType::Do_ValuesVarUnset:
            result = ValueVarMapStrings;
            break;
        case TokenType::Do_Points:
            result = PointMapStrings;
            break;
        }
    }
    return result;
}

template <typename Table, typename Value>
auto find_exact_match_then_string_match(Table const& table, Value const& v)
{
    // first see if there's an exact match:
    auto theEntry = std::find_if(std::begin(table), std::end(table), [&v](auto&& entry) {
        return v == entry.first;
    });
    if (theEntry == std::end(table)) {
        theEntry = std::find_if(std::begin(table), std::end(table), [&v](auto&& entry) {
            return string_matches_upper_case(v, entry.first);
        });
    }
    return theEntry;
}

void ContinuityComposerPanel::OnCmdTextEnterKeyPressed(wxCommandEvent const& event)
{
    // fast path, if no things set, treat as an OK
    if (mCurrentSelected == nullptr && Validate()) {
        QueueEvent(new wxCommandEvent{ wxEVT_BUTTON, wxID_OK });
        return;
    }
    auto the_string = event.GetString().ToStdString();
    if (the_string == "") {
        the_string = mComboSelection->GetString(event.GetSelection()).ToStdString();
    }
    if (mCurrentSelected) {
        auto changed = false;
        // setting a VarValue can result in an exception...
        try {
            switch (WhatType(mCurrentSelected)) {
            case TokenType::Do_Continuity: {
                auto theEntry = find_exact_match_then_string_match(ContMap, the_string);
                if (theEntry != std::end(ContMap)) {
                    mCont = theEntry->second();
                    changed = true;
                }
            } break;
            case TokenType::Do_Points: {
                auto theEntry = find_exact_match_then_string_match(PointMap, the_string);
                if (theEntry != std::end(PointMap)) {
                    mCurrentParent->replace(mCurrentSelected, theEntry->second());
                    changed = true;
                }
            } break;
            case TokenType::Do_ValuesVarUnset: {
                auto theEntry = find_exact_match_then_string_match(ValueMap, the_string);
                if (theEntry != std::end(ValueMap)) {
                    mCurrentParent->replace(mCurrentSelected, theEntry->second());
                    changed = true;
                }
            } break;
            case TokenType::Do_Values: {
                // special, if it just numbers, then it's a float.
                double value;
                if (event.GetString().ToDouble(&value)) {
                    mCurrentParent->replace(mCurrentSelected, std::make_unique<CalChart::ContValueFloat>(value));
                    changed = true;
                    break;
                }
                auto theEntry = find_exact_match_then_string_match(ValueMap, the_string);
                if (theEntry != std::end(ValueMap)) {
                    mCurrentParent->replace(mCurrentSelected, theEntry->second());
                    changed = true;
                }
            } break;
            }
        } catch (CalChart::ContProcSet::ReplaceError_NotAVar const& e) {
            wxString msg("Not a valid variable\n");
            wxMessageBox(msg, wxT("Invalid variable name"), wxICON_INFORMATION | wxOK);
        }
        if (changed) {
            mDrawableCont = mCont->GetDrawableCont();
            std::tie(mCurrentSelected, mCurrentParent) = first_unset(mDrawableCont);
        }
    }
    OnUpdate();
}

void ContinuityComposerPanel::OnUpdate()
{
    // save off what's in the list.
    auto list_of_strings = GetCurrentList(mCurrentSelected, mComboSelection->GetValue().ToStdString());
    // this is where we would filter things out.
    std::vector<wxString> wxStringsCont(list_of_strings.begin(), list_of_strings.end());
    mComboSelection->Set(wxStringsCont.size(), wxStringsCont.data());

    mCanvas->DoSetContinuity(mDrawableCont, mAction);
    // is it valid?
    // do highlighting
    mCanvas->SetHighlight(mCurrentSelected);
    mComboSelection->SetFocus();

    if (mOnUpdateIsValid) {
        mOnUpdateIsValid(Validate());
    }
    Refresh();
}

void ContinuityComposerPanel::OnDrawableContClick(CalChart::DrawableCont const& c)
{
    mCurrentSelected = c.self_ptr;
    mCurrentParent = c.parent_ptr;
    OnUpdate();
}

bool ContinuityComposerPanel::Validate()
{
    return first_unset(mDrawableCont).first == nullptr;
}

std::unique_ptr<CalChart::ContProcedure>
ContinuityComposerPanel::GetContinuity()
{
    return mCont->clone();
}

IMPLEMENT_CLASS(ContinuityComposerDialog, wxDialog)

ContinuityComposerDialog::ContinuityComposerDialog(std::unique_ptr<CalChart::ContProcedure> starting_continuity, wxWindow* parent, wxWindowID id, wxString const& caption, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : super(parent, id, caption, pos, size, style)
{
    // create a sizer for laying things out top down:
    mPanel = new ContinuityComposerPanel(std::move(starting_continuity), CalChartConfiguration::GetGlobalConfig(), this);
    SetSizer(VStack([this](auto sizer) {
        sizer->Add(mPanel, 0, wxEXPAND | wxALL, 5);

        HStack(sizer, sRightBasicSizerFlags, [this](auto sizer) {
            CreateButton(this, sizer, wxID_CANCEL, wxT("&Cancel"));
            mCloseButton = CreateButton(this, sizer, wxID_OK, wxT("&Done"));
            mCloseButton->SetDefault();
            mCloseButton->Enable(mPanel->Validate());
        });
    }));
    mPanel->SetOnUpdateIsValid([this](bool enable) {
        static_cast<wxButton*>(FindWindow(wxID_OK))->Enable(enable);
    });

    Update();
}

std::unique_ptr<CalChart::ContProcedure>
ContinuityComposerDialog::GetContinuity()
{
    return mPanel->GetContinuity();
}

bool ContinuityComposerDialog::Validate()
{
    return mPanel->Validate();
}
