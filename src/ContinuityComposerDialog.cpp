/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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
#include <wxUI/wxUI.hpp>

class ContinuityComposerCanvas : public CustomListViewPanel {
    using super = CustomListViewPanel;

public:
    // Basic functions
    ContinuityComposerCanvas(CalChart::Configuration const& config, wxWindow* parent, wxWindowID winid = wxID_ANY, wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxScrolledWindowStyle, wxString const& name = wxPanelNameStr);
    ~ContinuityComposerCanvas() override = default;
    void DoSetContinuity(CalChart::Cont::Drawable const& drawableCont, std::function<void(CalChart::Cont::Drawable const&)> action);

private:
    CalChart::Configuration const& mConfig;
};

// Define a constructor for field canvas
ContinuityComposerCanvas::ContinuityComposerCanvas(CalChart::Configuration const& config, wxWindow* parent, wxWindowID winid, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : super(parent, winid, pos, size, style, name)
    , mConfig(config)
{
    // use the config size to determine how big to make things,
    auto current_size = GetMinSize();
    current_size.y = ContinuityBoxDrawer::GetHeight(config);
    current_size.y += wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y);
    current_size.x = ContinuityBoxDrawer::GetMinWidth(config);
    SetMinSize(current_size);
}

void ContinuityComposerCanvas::DoSetContinuity(CalChart::Cont::Drawable const& drawableCont, std::function<void(CalChart::Cont::Drawable const&)> action)
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
    ContinuityComposerPanel(std::unique_ptr<CalChart::Cont::Procedure> starting_continuity, CalChart::Configuration const& config, wxWindow* parent, wxWindowID winid = wxID_ANY, wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER, wxString const& name = wxPanelNameStr);
    ~ContinuityComposerPanel() override = default;

    std::unique_ptr<CalChart::Cont::Procedure> GetContinuity();
    bool Validate() override;
    void SetOnUpdateIsValid(std::function<void(bool)> const& f) { mOnUpdateIsValid = f; }
    auto GetOnUpdateIsValid() const { return mOnUpdateIsValid; }

    void OnUpdate();

private:
    void Init();
    void CreateControls();

    // Event Handlers
    void OnDrawableContClick(CalChart::Cont::Drawable const& c);
    void OnCmdTextEnterKeyPressed(wxCommandEvent const& event);
    void OnComboPressed(wxCommandEvent const& event);
    void OnComboText(wxCommandEvent const& event);

    std::unique_ptr<CalChart::Cont::Procedure> mCont;
    CalChart::Cont::Drawable mDrawableCont;
    wxUI::Generic<ContinuityComposerCanvas>::Proxy mCanvas{};
    wxUI::ComboBox::Proxy mComboSelection{};
    CalChart::Cont::Token const* mCurrentSelected = nullptr;
    CalChart::Cont::Token* mCurrentParent = nullptr;
    std::function<void(CalChart::Cont::Drawable const& c)> const mAction;
    CalChart::Configuration const& mConfig;
    std::function<void(bool)> mOnUpdateIsValid{};
    std::string mLastValue;
};

IMPLEMENT_CLASS(ContinuityComposerPanel, wxPanel)

static const auto sRightBasicSizerFlags = wxSizerFlags{}.Border(wxALL, 2).Right().Proportion(0);
static const auto sExpandSizerFlags = wxSizerFlags{}.Border(wxALL, 2).Center().Proportion(0);

// find unselected and parent
std::pair<CalChart::Cont::Token const*, CalChart::Cont::Token*> first_unset(CalChart::Cont::Drawable const& cont)
{
    if (cont.type == CalChart::Cont::Type::unset) {
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

std::pair<CalChart::Cont::Token const*, CalChart::Cont::Token*> getFirstIfNoUnset(CalChart::Cont::Drawable const& cont)
{
    auto result = first_unset(cont);
    if (result == std::pair<CalChart::Cont::Token const*, CalChart::Cont::Token*>{ nullptr, nullptr }) {
        return { cont.self_ptr, cont.parent_ptr };
    }
    return result;
}

ContinuityComposerPanel::ContinuityComposerPanel(std::unique_ptr<CalChart::Cont::Procedure> starting_continuity, CalChart::Configuration const& config, wxWindow* parent,
    wxWindowID winid, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : wxPanel(parent, winid, pos, size, style, name)
    , mCont(std::move(starting_continuity))
    , mAction([this](CalChart::Cont::Drawable const& c) { this->OnDrawableContClick(c); })
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
    wxUI::VSizer{
        mCanvas = wxUI::Generic<ContinuityComposerCanvas>{
            wxSizerFlags(1).Expand(),
            [this](wxWindow* parent) {
                return new ContinuityComposerCanvas(mConfig, parent);
            },
        },
        wxUI::ComboBox{}.withStyle(wxTE_PROCESS_ENTER).bind(wxEVT_TEXT_ENTER, [this](auto const& event) {
                                                          OnCmdTextEnterKeyPressed(event);
                                                      })
            .bind(wxEVT_COMBOBOX, [this](auto const& event) {
                OnComboPressed(event);
            })
            .bind(wxEVT_TEXT, [this](auto const& event) {
                OnComboText(event);
            })
            .withFlags(wxSizerFlags(1).Expand().Border())
            .withProxy(mComboSelection),
    }
        .fitTo(this);

    if (!mCont) {
        mCont = std::make_unique<CalChart::Cont::ProcUnset>();
    }
    mDrawableCont = mCont->GetDrawable();
    std::tie(mCurrentSelected, mCurrentParent) = getFirstIfNoUnset(mDrawableCont);
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
    Close,
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
    StandAndPlay,
    LAST
};

const std::pair<std::string, std::function<std::unique_ptr<CalChart::Cont::Procedure>()>> ContMap[] = {
    { "BLAM", []() { return std::make_unique<CalChart::Cont::ProcBlam>(); } },
    { "Close", []() { return std::make_unique<CalChart::Cont::ProcClose>(std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "Counter March", []() { return std::make_unique<CalChart::Cont::ProcCM>(std::make_unique<CalChart::Cont::PointUnset>(), std::make_unique<CalChart::Cont::PointUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "Diagonal Military Counter March", []() { return std::make_unique<CalChart::Cont::ProcDMCM>(std::make_unique<CalChart::Cont::PointUnset>(), std::make_unique<CalChart::Cont::PointUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "Diagonal Military High Step", []() { return std::make_unique<CalChart::Cont::ProcDMHS>(std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "EVEN step", []() { return std::make_unique<CalChart::Cont::ProcEven>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "East/West North/South", []() { return std::make_unique<CalChart::Cont::ProcEWNS>(std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "FOUNTAIN", []() { return std::make_unique<CalChart::Cont::ProcFountain>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "Forward March", []() { return std::make_unique<CalChart::Cont::ProcFM>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "Forward March TO", []() { return std::make_unique<CalChart::Cont::ProcFMTO>(std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "GRID", []() { return std::make_unique<CalChart::Cont::ProcGrid>(std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "High Steps Counter March", []() { return std::make_unique<CalChart::Cont::ProcHSCM>(std::make_unique<CalChart::Cont::PointUnset>(), std::make_unique<CalChart::Cont::PointUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "High Steps Diagonal Military", []() { return std::make_unique<CalChart::Cont::ProcHSDM>(std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "MAGIC", []() { return std::make_unique<CalChart::Cont::ProcMagic>(std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "MARCH", []() { return std::make_unique<CalChart::Cont::ProcMarch>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "MarkTime", []() { return std::make_unique<CalChart::Cont::ProcMT>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "MarkTime ReMaining", []() { return std::make_unique<CalChart::Cont::ProcMTRM>(std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "North/South East/West", []() { return std::make_unique<CalChart::Cont::ProcNSEW>(std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "ROTATE", []() { return std::make_unique<CalChart::Cont::ProcRotate>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "SET", []() { return std::make_unique<CalChart::Cont::ProcSet>(std::make_unique<CalChart::Cont::ValueVarUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "Stand & Play", []() { return std::make_unique<CalChart::Cont::ProcStandAndPlay>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
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
    ContProc::Close,
    ContProc::StandAndPlay,
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

const std::pair<std::string, std::function<std::unique_ptr<CalChart::Cont::Value>()>> ValueMap[] = {
    { "+", []() { return std::make_unique<CalChart::Cont::ValueAdd>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "-", []() { return std::make_unique<CalChart::Cont::ValueSub>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "*", []() { return std::make_unique<CalChart::Cont::ValueMult>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "/", []() { return std::make_unique<CalChart::Cont::ValueDiv>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "NEG", []() { return std::make_unique<CalChart::Cont::ValueNeg>(std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "REMaining", []() { return std::make_unique<CalChart::Cont::ValueREM>(); } },
    { "DIRection TO", []() { return std::make_unique<CalChart::Cont::FuncDir>(std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "DIRection From", []() { return std::make_unique<CalChart::Cont::FuncDirFrom>(std::make_unique<CalChart::Cont::PointUnset>(), std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "DIStant To", []() { return std::make_unique<CalChart::Cont::FuncDist>(std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "DIStant From", []() { return std::make_unique<CalChart::Cont::FuncDistFrom>(std::make_unique<CalChart::Cont::PointUnset>(), std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "EITHER", []() { return std::make_unique<CalChart::Cont::FuncEither>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "OPPosite", []() { return std::make_unique<CalChart::Cont::FuncOpp>(std::make_unique<CalChart::Cont::ValueUnset>()); } },
    { "STEP drill", []() { return std::make_unique<CalChart::Cont::FuncStep>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::PointUnset>()); } },
    { "A", []() { return std::make_unique<CalChart::Cont::ValueVar>(CalChart::Cont::Variable::A); } },
    { "B", []() { return std::make_unique<CalChart::Cont::ValueVar>(CalChart::Cont::Variable::B); } },
    { "C", []() { return std::make_unique<CalChart::Cont::ValueVar>(CalChart::Cont::Variable::C); } },
    { "D", []() { return std::make_unique<CalChart::Cont::ValueVar>(CalChart::Cont::Variable::D); } },
    { "X", []() { return std::make_unique<CalChart::Cont::ValueVar>(CalChart::Cont::Variable::X); } },
    { "Y", []() { return std::make_unique<CalChart::Cont::ValueVar>(CalChart::Cont::Variable::Y); } },
    { "Z", []() { return std::make_unique<CalChart::Cont::ValueVar>(CalChart::Cont::Variable::Z); } },
    { "Direction Of Flow", []() { return std::make_unique<CalChart::Cont::ValueVar>(CalChart::Cont::Variable::DOF); } },
    { "Direction Of Horn", []() { return std::make_unique<CalChart::Cont::ValueVar>(CalChart::Cont::Variable::DOH); } },
    { "North", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_N); } },
    { "NorthEast", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_NE); } },
    { "East", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_E); } },
    { "SouthEast", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_SE); } },
    { "South", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_S); } },
    { "SouthWest", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_SW); } },
    { "West", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_W); } },
    { "NorthWest", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_NW); } },
    { "HighStep", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_HS); } },
    { "Mini-Military", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_MM); } },
    { "ShowHigh", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_SH); } },
    { "Jerky Step", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_JS); } },
    { "GrapeVine", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_GV); } },
    { "Military", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_M); } },
    { "Diagonal Military", []() { return std::make_unique<CalChart::Cont::ValueDefined>(CalChart::Cont::CC_DM); } },
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

const std::pair<std::string, std::function<std::unique_ptr<CalChart::Cont::Point>()>> PointMap[] = {
    { "Point", []() { return std::make_unique<CalChart::Cont::Point>(); } },
    { "Starting Point", []() { return std::make_unique<CalChart::Cont::StartPoint>(); } },
    { "Next Point", []() { return std::make_unique<CalChart::Cont::NextPoint>(); } },
    { "Ref Point 1", []() { return std::make_unique<CalChart::Cont::RefPoint>(1); } },
    { "Ref Point 2", []() { return std::make_unique<CalChart::Cont::RefPoint>(2); } },
    { "Ref Point 3", []() { return std::make_unique<CalChart::Cont::RefPoint>(3); } },
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

auto WhatType(CalChart::Cont::Token const* ptr)
{
    if (dynamic_cast<CalChart::Cont::Procedure const*>(ptr)) {
        return TokenType::Do_Continuity;
    }
    if (dynamic_cast<CalChart::Cont::ValueVarUnset const*>(ptr)) {
        return TokenType::Do_ValuesVarUnset;
    }
    if (dynamic_cast<CalChart::Cont::Value const*>(ptr)) {
        return TokenType::Do_Values;
    }
    if (dynamic_cast<CalChart::Cont::Point const*>(ptr)) {
        return TokenType::Do_Points;
    }
    throw std::runtime_error("unknown pointer type");
}

auto GetCurrentList(CalChart::Cont::Token const* selected)
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
        the_string = mComboSelection.control()->GetString(event.GetSelection()).ToStdString();
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
                    mCurrentParent->replace(mCurrentSelected, std::make_unique<CalChart::Cont::ValueFloat>(value));
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
        } catch (CalChart::Cont::ProcSet::ReplaceError_NotAVar const& e) {
            wxString msg("Not a valid variable\n");
            wxMessageBox(msg, wxT("Invalid variable name"), wxICON_INFORMATION | wxOK);
        }
        if (changed) {
            mDrawableCont = mCont->GetDrawable();
            std::tie(mCurrentSelected, mCurrentParent) = first_unset(mDrawableCont);
            // clear out the text
            *mComboSelection = "";
        }
    }
    OnUpdate();
}

// how this works:  Go through each char of input str and find the in order matches of the
// input list given.
// We do this by keeping a tally of the match points, and prune off the strings that don't match
auto findStringsThatMatchString(std::string const& str, std::vector<std::string> const& input)
{
    auto listOfStrings = std::vector<std::pair<std::string, size_t>>{};
    std::transform(input.begin(), input.end(), std::back_inserter(listOfStrings), [](auto&& i) -> std::pair<std::string, size_t> {
        return { i, 0 };
    });
    for (auto&& l : str) {
        std::transform(listOfStrings.begin(), listOfStrings.end(), listOfStrings.begin(), [l](auto&& i) -> std::pair<std::string, size_t> {
            auto result = std::min(i.first.find(tolower(l), i.second), i.first.find(toupper(l), i.second));
            return { i.first, result };
        });
        listOfStrings.erase(std::remove_copy_if(listOfStrings.begin(), listOfStrings.end(), listOfStrings.begin(), [](auto&& i) { return i.second == std::string::npos; }), listOfStrings.cend());
    }
    auto result = std::vector<std::string>{};
    std::transform(listOfStrings.cbegin(), listOfStrings.cend(), std::back_inserter(result), [](auto&& i) { return i.first; });

    return result;
}

// we take the list of strings, filter them based on the char.
void ContinuityComposerPanel::OnUpdate()
{
    // Get the master list.
    auto list_of_strings = GetCurrentList(mCurrentSelected);

    // filter out the selected
    auto filteredStrings = findStringsThatMatchString(*mComboSelection, std::vector(list_of_strings.begin(), list_of_strings.end()));
    auto stringsCont = std::vector<wxString>(filteredStrings.begin(), filteredStrings.end());
    // if we *not* in the middle of browsing the list, or there's currently no strings, put in the culled list.
    if (wxNOT_FOUND == mComboSelection.selection().get() || mComboSelection.control()->GetCount() == 0) {
        while (mComboSelection.control()->GetCount()) {
            mComboSelection.control()->Delete(0);
        }
        if (!stringsCont.empty()) {
            mComboSelection.control()->Insert(stringsCont, 0);
        }
    }

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

// currently unused
void ContinuityComposerPanel::OnComboPressed(wxCommandEvent const& /*event*/)
{
}

void ContinuityComposerPanel::OnComboText(wxCommandEvent const& /*event*/)
{
    // we only do updates when the text changes
    if (mLastValue != static_cast<std::string>(*mComboSelection)) {
        mLastValue = *mComboSelection;
        OnUpdate();
    }
}

void ContinuityComposerPanel::OnDrawableContClick(CalChart::Cont::Drawable const& c)
{
    mCurrentSelected = c.self_ptr;
    mCurrentParent = c.parent_ptr;
    OnUpdate();
}

bool ContinuityComposerPanel::Validate()
{
    return first_unset(mDrawableCont).first == nullptr;
}

std::unique_ptr<CalChart::Cont::Procedure>
ContinuityComposerPanel::GetContinuity()
{
    return mCont->clone();
}

IMPLEMENT_CLASS(ContinuityComposerDialog, wxDialog)

ContinuityComposerDialog::ContinuityComposerDialog(std::unique_ptr<CalChart::Cont::Procedure> starting_continuity, CalChart::Configuration const& config, wxWindow* parent)
    : super(parent, wxID_ANY, "Compose Continuity")
{
    ContinuityComposerPanel* panel = new ContinuityComposerPanel(std::move(starting_continuity), config, this);
    wxUI::VSizer{
        sRightBasicSizerFlags,
        mPanel = wxUI::Generic<ContinuityComposerPanel>{
            wxSizerFlags{ 0 }.Expand().Border(wxALL, 5),
            panel },
        wxUI::VSizer{
            sRightBasicSizerFlags,
            wxUI::HSizer{
                wxSizerFlags{ 0 },
                wxUI::Button{ wxID_CANCEL, "&Cancel" },
                wxUI::Button{ wxID_OK, "&Done" }.setDefault().setEnabled(panel->Validate()),
            },
        },
    }
        .fitTo(this);

    mPanel->SetOnUpdateIsValid([this](bool enable) {
        FindWindow(wxID_OK)->Enable(enable);
    });

    Update();
}

std::unique_ptr<CalChart::Cont::Procedure>
ContinuityComposerDialog::GetContinuity()
{
    return mPanel->GetContinuity();
}

bool ContinuityComposerDialog::Validate()
{
    return mPanel->Validate();
}
