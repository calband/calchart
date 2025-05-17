/*
 * StackDrawPlayground.cpp
 */

/*
   Copyright (C) 2024  Richard Michael Powell

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

#include "StackDrawPlayground.h"
#include "CalChartDrawing.h"

#include <wx/splitter.h>

class StackDrawPreview : public wxScrolled<wxWindow> {
    using super = wxScrolled<wxWindow>;
    DECLARE_EVENT_TABLE()

public:
    StackDrawPreview(wxWindow* parent);

    void SetDrawCommand(CalChart::Draw::DrawCommand const& drawCommand)
    {
        mDrawCommand = drawCommand;
    }
    void SetUseOffset(bool offset)
    {
        mUseOffset = offset;
    }

private:
    CalChart::Draw::DrawCommand mDrawCommand{};
    bool mUseOffset{};

    void OnPaint(wxPaintEvent& event);
    void OnSizeEvent(wxSizeEvent& event);
};

BEGIN_EVENT_TABLE(StackDrawPreview, StackDrawPreview::super)
EVT_SIZE(StackDrawPreview::OnSizeEvent)
END_EVENT_TABLE()

static constexpr auto kStackDrawPreviewMinX = 256;
static constexpr auto kStackDrawPreviewMinY = 734 - 606;
StackDrawPreview::StackDrawPreview(wxWindow* parent)
    : wxScrolled<wxWindow>(parent, wxID_ANY)
{
    static const double kSizeX = 576, kSizeY = 734 - 606;
    SetVirtualSize(wxSize(kSizeX, kSizeY));
    SetScrollRate(10, 10);
    SetBackgroundColour(*wxWHITE);
    Connect(wxEVT_PAINT, wxPaintEventHandler(StackDrawPreview::OnPaint));
}

namespace {

[[nodiscard]] auto done(char const* begin, char const* end) -> bool
{
    return begin == end;
}

[[nodiscard]] auto increment(char const* begin, char const* end) -> char const*
{
    if (done(begin, end)) {
        throw std::runtime_error("Reached end");
    }
    ++begin;
    return begin;
}

static auto ParseStringHelper(char const* begin, char const* end) -> std::tuple<char const*, CalChart::Draw::DrawCommand>;

[[nodiscard]] auto ParseVector(char const* begin, char const* end) -> std::tuple<char const*, std::vector<CalChart::Draw::DrawCommand>>
{
    if (*begin != '(') {
        throw std::runtime_error("missing (");
    }
    begin = increment(begin, end);
    auto result = std::vector<CalChart::Draw::DrawCommand>{};
    while (*begin != ')') {
        auto [next, cmd] = ParseStringHelper(begin, end);
        result.push_back(cmd);
        begin = next;
    }
    begin = increment(begin, end);
    return { begin, result };
}

auto ParseNumberHelper(char const* begin, char const* end) -> std::tuple<char const*, int>
{
    auto result = 0;
    while (begin != end && isdigit(*begin)) {
        result *= 10;
        result += *begin - '0';
        begin = increment(begin, end);
    }
    return { begin, result };
}

// assumes that the char points to the open q
auto ParseWord(char const* begin, char const* end) -> std::tuple<char const*, CalChart::Draw::Text>
{
    if (*begin != 'q') {
        throw std::runtime_error("missing q");
    }

    begin = increment(begin, end);
    auto result = std::string{};

    while (*begin != 'q') {
        result.push_back(*begin);
        begin = increment(begin, end);
    }
    begin = increment(begin, end);
    return { begin, CalChart::Draw::Text{ CalChart::Coord{}, result } };
}

auto ParseEllipse(char const* begin, char const* end) -> std::tuple<char const*, CalChart::Draw::Ellipse>
{
    if (*begin != 'E') {
        throw std::runtime_error("missing E");
    }
    begin = increment(begin, end);
    if (*begin != '(') {
        throw std::runtime_error("missing (");
    }
    begin = increment(begin, end);
    auto c1 = CalChart::Coord::units{};
    std::tie(begin, c1) = ParseNumberHelper(begin, end);
    if (*begin != ',') {
        throw std::runtime_error("missing ,");
    }
    begin = increment(begin, end);
    auto c2 = CalChart::Coord::units{};
    std::tie(begin, c2) = ParseNumberHelper(begin, end);
    if (*begin != ')') {
        throw std::runtime_error("missing )");
    }
    begin = increment(begin, end);
    return { begin, CalChart::Draw::Ellipse{ { 0, 0 }, { c1, c2 } } };
}

auto ParseBox(char const* begin, char const* end) -> std::tuple<char const*, CalChart::Draw::ZStack>
{
    if (*begin != 'B') {
        throw std::runtime_error("missing B");
    }
    begin = increment(begin, end);
    if (*begin != '(') {
        throw std::runtime_error("missing (");
    }
    begin = increment(begin, end);
    auto c1 = CalChart::Coord::units{};
    std::tie(begin, c1) = ParseNumberHelper(begin, end);
    if (*begin != ',') {
        throw std::runtime_error("missing ,");
    }
    begin = increment(begin, end);
    auto c2 = CalChart::Coord::units{};
    std::tie(begin, c2) = ParseNumberHelper(begin, end);
    if (*begin != ')') {
        throw std::runtime_error("missing )");
    }
    begin = increment(begin, end);
    return {
        begin, CalChart::Draw::ZStack{ {
                   CalChart::Draw::Line{ { 0, 0 }, { c1, 0 } },
                   CalChart::Draw::Line{ { c1, 0 }, { c1, c2 } },
                   CalChart::Draw::Line{ { c1, c2 }, { 0, c2 } },
                   CalChart::Draw::Line{ { 0, c2 }, { 0, 0 } },
               } }
    };
}

auto CharToAlign = [](auto c) {
    switch (c) {
    case 'B':
        return CalChart::Draw::StackAlign::Begin;
    case 'E':
        return CalChart::Draw::StackAlign::End;
    case 'U':
        return CalChart::Draw::StackAlign::Uniform;
    case 'J':
        return CalChart::Draw::StackAlign::Justified;
    default:
        throw std::runtime_error("Wrong alignment");
    }
};

auto ParseStringHelper(char const* begin, char const* end) -> std::tuple<char const*, CalChart::Draw::DrawCommand>
{
    switch (*begin) {
    case 'A':
        return { increment(begin, end), CalChart::Draw::Ellipse{ { 0, 0 }, { 40, 40 } } };
    case 'B':
        return ParseBox(begin, end);
    case 'C':
        return { increment(begin, end), CalChart::Draw::Ellipse{ { 0, 0 }, { 20, 10 } } };
    case 'D':
        return {
            increment(begin, end), CalChart::Draw::Ellipse{ { 0, 0 }, { 10, 10 } }
        };
    case 'E':
        return ParseEllipse(begin, end);
    case 'F':
        return {
            increment(begin, end), CalChart::Draw::Ellipse{ { 0, 0 }, { 40, 40 } }
        };
    case 'q':
        return ParseWord(begin, end);
    case 'T':
        return {
            increment(begin, end), CalChart::Draw::Tab{}
        };
    case 'H': {
        begin = increment(begin, end);
        auto align = CharToAlign(*begin);
        begin = increment(begin, end);

        auto [next, values] = ParseVector(begin, end);
        return { next, CalChart::Draw::HStack{ values, align } };
    }
    case 'V': {
        begin = increment(begin, end);
        auto align = CharToAlign(*begin);
        // init
        begin = increment(begin, end);

        auto [next, values] = ParseVector(begin, end);
        return { next, CalChart::Draw::VStack{ values, align } };
    }
    case 'Z': {
        begin = increment(begin, end);
        auto align = CharToAlign(*begin);
        // init
        begin = increment(begin, end);

        auto [next, values] = ParseVector(begin, end);
        return { next, CalChart::Draw::ZStack{ values, align } };
    }
    }
    throw std::runtime_error("Missing things");
}

auto ParseString(char const* begin, char const* end) -> CalChart::Draw::DrawCommand
{
    auto [next, cmd] = ParseStringHelper(begin, end);
    return cmd;
}

auto ParseString(std::string const& description)
{
    return ParseString(description.data(), description.data() + description.size());
}

}

void StackDrawPreview::OnPaint(wxPaintEvent&)
{
    wxPaintDC dc(this);
    PrepareDC(dc);
    auto virtSize = GetVirtualSize();

    dc.Clear();
    dc.SetTextForeground(*wxBLACK);
    dc.DrawRectangle(wxRect(wxPoint(0, 0), virtSize));
    auto offset = mUseOffset ? CalChart::Coord(10, 10) : CalChart::Coord(0, 0);
    wxCalChart::Draw::DrawCommandList(dc, mDrawCommand + offset);
}

void StackDrawPreview::OnSizeEvent(wxSizeEvent& event)
{
    auto x = std::max(event.m_size.x, kStackDrawPreviewMinX);
    auto y = std::max(event.m_size.y, kStackDrawPreviewMinY);
    SetVirtualSize(wxSize(x, y));
    SetScrollRate(10, 10);
}

IMPLEMENT_CLASS(StackDrawPlayground, wxDialog)

static constexpr auto DefaultText
    = "VB(HU(qThis is contq)HB(qtestqTTTqtest2q))";

// private, use the CreatePreference method
StackDrawPlayground::StackDrawPlayground(wxWindow* parent)
    : super(parent, wxID_ANY, "Stack Draw")
{
    using namespace wxUI;
    VSizer{
        wxSizerFlags{}.Border(wxALL, 2).Left().Proportion(0),
        HSplitter{
            mPreview = [](wxWindow* parent) {
                return new StackDrawPreview(parent);
            },
            TextCtrl{}.bind([this] {
                          try {
                              mPreview->SetDrawCommand(ParseString(*mUserInput));
                              mStatus->SetStatusText("");
                          } catch (std::exception const& e) {
                              mPreview->SetDrawCommand({});
                              mStatus->SetStatusText(e.what());
                          }
                          Refresh();
                      })
                .withStyle(wxTE_MULTILINE | wxHSCROLL | wxTE_PROCESS_TAB)
                .withProxy(mUserInput),
        }
            .withSize(wxSize(500, 400)),
        HSizer{ Text{ "Apply (10,10) offset" }, CheckBox{}.bind([this]() {
                                                              mPreview->SetUseOffset(*mUseOffset);
                                                              Refresh();
                                                          })
                                                    .withProxy(mUseOffset) },
        Generic{ CreateStdDialogButtonSizer(wxOK) },
        mStatus = Generic<wxStatusBar>{ [](wxWindow* parent) {
            return new wxStatusBar(parent);
        } },
    }
        .fitTo(this);

    *mUserInput = DefaultText;

    try {
        mPreview->SetDrawCommand(ParseString(*mUserInput));
        mStatus->SetStatusText("");
    } catch (std::exception const& e) {
        mStatus->SetStatusText(e.what());
    }
}
