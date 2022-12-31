#pragma once
/*
 * basic_ui.h
 * Header for basic wxWindows classes
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

#include "CalChartSizes.h"

#include <vector>
#include <wx/hyperlink.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/toolbar.h>
#include <wx/wx.h>
#include <wxUI/wxUI.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

class CalChartView;

static constexpr auto kMultiple = "[multiple]";
static constexpr auto kDefaultInstrument = "default";
static constexpr auto kCustom = "custom...";

// Set icon to band's insignia
void SetBandIcon(wxFrame* frame);

// Set icon to band's insignia
wxStaticBitmap* BitmapWithBandIcon(wxWindow* parent, wxSize const& size = wxDefaultSize);
wxStaticText* TextStringWithSize(wxWindow* parent, std::string const& label, int pointSize);
wxHyperlinkCtrl* LinkStringWithSize(wxWindow* parent, std::string const& label, std::string const& url, int pointSize);
wxStaticLine* LineWithLength(wxWindow* parent, int length, long style = wxLI_HORIZONTAL);

// Define a text subwindow that can respond to drag-and-drop
class FancyTextWin : public wxTextCtrl {
    using super = wxTextCtrl;

public:
    FancyTextWin(wxWindow* parent, wxWindowID id,
        const wxString& value = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTE_MULTILINE | wxHSCROLL | wxTE_PROCESS_TAB);
    wxString GetValue(void) const override;
};

/**
 * A window that can be scrolled and zoomed in on.
 * To use, call PrepareDC on your OnPaint routines with the dc.
 */
class ScrollZoomWindow : public wxScrolledWindow {
    using super = wxScrolledWindow;
    DECLARE_EVENT_TABLE()

public:
    ScrollZoomWindow(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~ScrollZoomWindow() override;

protected:
    virtual void PrepareDC(wxDC&) override;

public:
    // inform the scrolling window of the size of the underlying canvas.
    void SetCanvasSize(wxSize z);

    void SetZoom(float z);
    void SetZoom(float z, wxPoint center);
    float GetZoom() const;

    void ChangeOffset(wxPoint deltaOffset);

private:
    void HandleSizeEvent(wxSizeEvent& event);

    void SetupSize();
    wxPoint mOffset;
    float mZoomFactor;
    wxSize mCanvasSize;
};

/**
 * A canvas that scrolls according to the movement of the mouse. The amount that
 * the canvas
 * scrolls is directly proportional to the mouse movement.
 * To make this work, OnMouseMove(...) should be called in response to mouse
 * move events.
 */
class MouseMoveScrollCanvas : public ScrollZoomWindow {
    using super = ScrollZoomWindow;

public:
    MouseMoveScrollCanvas(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~MouseMoveScrollCanvas() override;

protected:
    // Inform the ctrl scrolling when the mouse moves
    virtual void OnMouseMove(wxMouseEvent& event);
    virtual void OnMousePinchToZoom(wxMouseEvent& event);
    virtual void OnMouseWheel(wxMouseEvent& event);

    virtual bool IsScrolling() const;

    virtual bool ShouldScrollOnMouseEvent(const wxMouseEvent& event) const = 0;

private:
    wxPoint mLastPos;
    bool mScrolledLastMove;
};

/**
 * A canvas that scrolls when the ctrl button is pressed and the user clicks and
 * drags the mouse.
 */
class ClickDragCtrlScrollCanvas : public MouseMoveScrollCanvas {
    using super = MouseMoveScrollCanvas;

public:
    ClickDragCtrlScrollCanvas(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~ClickDragCtrlScrollCanvas() override;

protected:
    virtual bool ShouldScrollOnMouseEvent(const wxMouseEvent& event) const override;
};

// common sizers all in one places
wxSizerFlags BasicSizerFlags();
wxSizerFlags LeftBasicSizerFlags();
wxSizerFlags RightBasicSizerFlags();
wxSizerFlags ExpandSizerFlags();
wxSizerFlags HorizontalSizerFlags();
wxSizerFlags VerticalSizerFlags();

template <typename T>
void AddToSizerBasic(wxSizer* sizer, T window)
{
    sizer->Add(window, BasicSizerFlags());
}

template <typename T>
void AddToSizerExpand(wxSizer* sizer, T window)
{
    sizer->Add(window, ExpandSizerFlags());
}

template <typename T>
void AddToSizerRight(wxSizer* sizer, T window)
{
    sizer->Add(window, RightBasicSizerFlags());
}

template <typename Brush>
auto CreateItemBitmap(Brush const& brush)
{
    wxBitmap temp_bitmap(GetColorBoxSize());
    wxMemoryDC temp_dc;
    temp_dc.SelectObject(temp_bitmap);
    temp_dc.SetBackground(brush);
    temp_dc.Clear();
    return temp_bitmap;
}

template <typename T, typename Int, typename Brush>
void CreateAndSetItemBitmap(T* target, Int which, Brush const& brush)
{
    target->SetItemBitmap(which, CreateItemBitmap(brush));
}

wxFont CreateFont(int pixelSize, wxFontFamily family = wxFONTFAMILY_SWISS, wxFontStyle style = wxFONTSTYLE_NORMAL, wxFontWeight weight = wxFONTWEIGHT_NORMAL);
wxFont ResizeFont(wxFont const& font, int pixelSize);

template <typename String>
inline auto StringSizeX(wxControl* controller, String const& string)
{
    return controller->GetTextExtent(string).x;
}

template <typename Strings>
inline auto BestSizeX(wxControl* controller, Strings const& strings)
{
    return controller->GetTextExtent(*std::max_element(std::begin(strings), std::end(strings), [controller](auto a, auto b) {
                         return controller->GetTextExtent(a).x < controller->GetTextExtent(b).x;
                     }))
        .x;
}

#pragma mark - These are intented to make something similar to SwiftUI.

// The HStacks and VStacks are conviences layouts to make the system more "declarative".  The idea is that
// when you want to lay out elements of the UI horizontally or vertically, you would have a block of code
// that adds the elements to a sizer.  So you would write something along the lines of:
// VStack(sizer, [](auto sizer) {
//     CreateItemOne(sizer);
//     CreateItemTwo(sizer);
// });

template <typename Handler>
inline auto HStack(wxSizer* sizer, Handler h)
{
    auto horizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    h(horizontalSizer);
    sizer->Add(horizontalSizer);
    return horizontalSizer;
}

template <typename Handler>
inline auto HStack(wxSizer* sizer, wxSizerFlags flags, Handler h)
{
    auto horizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    h(horizontalSizer);
    sizer->Add(horizontalSizer, flags);
    return horizontalSizer;
}

// generally this is the top level stack as it has no sizer to add itself to
template <typename Handler>
inline auto VStack(Handler h)
{
    auto vSizer = new wxBoxSizer(wxVERTICAL);
    h(vSizer);
    return vSizer;
}

template <typename Handler>
inline auto VStack(wxSizer* sizer, Handler h)
{
    auto vSizer = new wxBoxSizer(wxVERTICAL);
    h(vSizer);
    sizer->Add(vSizer);
    return vSizer;
}

template <typename Handler>
inline auto VStack(wxSizer* sizer, wxSizerFlags flags, Handler h)
{
    auto vSizer = new wxBoxSizer(wxVERTICAL);
    h(vSizer);
    sizer->Add(vSizer, flags);
    return vSizer;
}

template <typename String, typename Handler>
inline auto NamedVBoxStack(wxWindow* parent, wxSizer* sizer, String caption, Handler h)
{
    auto boxsizer = new wxStaticBoxSizer(new wxStaticBox(parent, -1, caption), wxVERTICAL);
    h(boxsizer);
    sizer->Add(boxsizer);
    return boxsizer;
}

template <typename String, typename Handler>
inline auto NamedVBoxStack(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, String caption, Handler h)
{
    auto boxsizer = new wxStaticBoxSizer(new wxStaticBox(parent, -1, caption), wxVERTICAL);
    h(boxsizer);
    sizer->Add(boxsizer, flags);
    return boxsizer;
}

template <typename String, typename Handler>
inline auto NamedHBoxStack(wxWindow* parent, wxSizer* sizer, String caption, Handler h)
{
    auto boxsizer = new wxStaticBoxSizer(new wxStaticBox(parent, -1, caption), wxHORIZONTAL);
    h(boxsizer);
    sizer->Add(boxsizer);
    return boxsizer;
}

template <typename String, typename Handler>
inline auto NamedHBoxStack(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, String caption, Handler h)
{
    auto boxsizer = new wxStaticBoxSizer(new wxStaticBox(parent, -1, caption), wxHORIZONTAL);
    h(boxsizer);
    sizer->Add(boxsizer, flags);
    return boxsizer;
}

inline auto CreateButton(wxWindow* parent, wxSizer* sizer, int id = wxID_ANY)
{
    auto button = new wxButton(parent, id);
    sizer->Add(button);
    return button;
}

inline auto CreateButton(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, int id)
{
    auto button = new wxButton(parent, id);
    sizer->Add(button, flags);
    return button;
}

template <typename String>
inline auto CreateButton(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, int id, String name)
{
    auto button = new wxButton(parent, id, name);
    sizer->Add(button, flags);
    return button;
}

template <typename String>
inline auto CreateButton(wxWindow* parent, wxSizer* sizer, int id, String name)
{
    auto button = new wxButton(parent, id, name);
    sizer->Add(button);
    return button;
}

template <typename String, typename Handler>
inline auto CreateButtonWithHandler(wxWindow* parent, String name, Handler handler)
{
    auto button = new wxButton(parent, wxID_ANY, name);
    button->Bind(wxEVT_BUTTON, [handler](wxCommandEvent&) {
        handler();
    });
    return button;
}

template <typename String, typename Handler>
inline auto CreateButtonWithHandler(wxWindow* parent, int id, String name, Handler handler)
{
    auto button = new wxButton(parent, id, name);
    button->Bind(wxEVT_BUTTON, [handler](wxCommandEvent&) {
        handler();
    });
    return button;
}

template <typename String, typename Handler>
inline auto CreateButtonWithHandler(wxWindow* parent, wxSizer* sizer, String name, Handler handler)
{
    auto button = CreateButtonWithHandler(parent, std::forward<String>(name), std::forward<Handler>(handler));
    sizer->Add(button);
    return button;
}

template <typename String, typename Handler>
inline auto CreateButtonWithHandler(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, String name, Handler handler)
{
    auto button = CreateButtonWithHandler(parent, std::forward<String>(name), std::forward<Handler>(handler));
    sizer->Add(button, flags);
    return button;
}

template <typename Image>
inline auto CreateBitmapButton(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, int id, Image bitmap)
{
    auto button = new wxBitmapButton(parent, id, bitmap);
    sizer->Add(button, flags);
    return button;
}

template <typename Image, typename Handler>
inline auto CreateBitmapButtonWithHandler(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, Image bitmap, Handler handler)
{
    auto button = new wxBitmapButton(parent, wxID_ANY, bitmap);
    button->Bind(wxEVT_BUTTON, [handler](wxCommandEvent&) {
        handler();
    });
    sizer->Add(button, flags);
    return button;
}

template <typename Image, typename Handler>
inline auto CreateBitmapToggleWithHandler(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, Image bitmap, Image pressed, Handler handler)
{
    auto button = new wxBitmapToggleButton(parent, wxID_ANY, bitmap, wxDefaultPosition);
    button->SetBitmapPressed(pressed);
    button->Bind(wxEVT_TOGGLEBUTTON, [handler](wxCommandEvent&) {
        handler();
    });
    sizer->Add(button, flags);
    return button;
}

// Creates a wxChoice with a handler that sets selection to 0
inline auto CreateChoiceWithHandler(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, int id, std::vector<wxString> const& choices)
{
    auto choice = new wxChoice(parent, id, wxDefaultPosition, wxDefaultSize, choices.size(), choices.data());
    choice->SetSelection(0);
    sizer->Add(choice, flags);
    return choice;
}

inline auto CreateChoiceWithHandler(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, int id, std::vector<std::string> const& choices)
{
    auto names = std::vector<wxString>{};
    std::copy(choices.cbegin(), choices.cend(), std::back_inserter(names));
    return CreateChoiceWithHandler(parent, sizer, flags, id, names);
}

template <typename Handler>
inline auto CreateChoiceWithHandler(wxWindow* parent, wxSizer* sizer, int id, std::vector<wxString> const& choices, Handler handler)
{
    auto choice = new wxChoice(parent, id, wxDefaultPosition, wxDefaultSize, choices.size(), choices.data());
    choice->Bind(wxEVT_CHOICE, [handler](wxCommandEvent& event) {
        handler(event);
    });
    choice->SetSelection(0);
    sizer->Add(choice);
    return choice;
}

template <typename Handler>
inline auto CreateChoiceWithHandler(wxWindow* parent, wxSizer* sizer, int id, std::vector<std::string> const& choices, Handler handler)
{
    auto names = std::vector<wxString>{};
    std::copy(choices.cbegin(), choices.cend(), std::back_inserter(names));
    return CreateChoiceWithHandler(parent, sizer, id, names, handler);
}

template <typename Handler>
inline auto CreateChoiceWithHandler(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, int id, std::vector<wxString> const& choices, Handler handler)
{
    auto choice = new wxChoice(parent, id, wxDefaultPosition, wxDefaultSize, choices.size(), choices.data());
    choice->Bind(wxEVT_CHOICE, [handler](wxCommandEvent& event) {
        handler(event);
    });
    choice->SetSelection(0);
    sizer->Add(choice, flags);
    return choice;
}

template <typename Handler>
inline auto CreateChoiceWithHandler(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, int id, std::vector<std::string> const& choices, Handler handler)
{
    auto names = std::vector<wxString>{};
    std::copy(choices.cbegin(), choices.cend(), std::back_inserter(names));
    return CreateChoiceWithHandler(parent, sizer, flags, id, names, handler);
}

template <typename Handler>
inline auto CreateChoiceWithHandler(wxWindow* parent, wxSizer* sizer, std::vector<wxString> const& choices, Handler handler)
{
    return CreateChoiceWithHandler(parent, sizer, wxID_ANY, choices, std::forward<Handler>(handler));
}

template <typename Handler>
inline auto CreateChoiceWithHandler(wxWindow* parent, wxSizer* sizer, std::vector<std::string> const& choices, Handler handler)
{
    return CreateChoiceWithHandler(parent, sizer, wxID_ANY, choices, std::forward<Handler>(handler));
}

template <typename String>
inline auto CreateText(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, String text)
{
    auto staticText = new wxStaticText(parent, wxID_STATIC, text);
    sizer->Add(staticText, flags);
    return staticText;
}

template <typename String>
inline auto CreateText(wxWindow* parent, wxSizer* sizer, String text)
{
    return CreateText(parent, sizer, BasicSizerFlags(), std::forward<String>(text));
}

inline auto CreateTextCtrl(wxWindow* parent, wxSizer* sizer, int id = wxID_ANY, long style = 0)
{
    auto textCtrl = new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxSize{ 100, -1 }, style);
    sizer->Add(textCtrl, 0, wxGROW | wxALL, 5);
    return textCtrl;
}

inline auto CreateTextCtrl(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, int id = wxID_ANY, long style = 0)
{
    auto textCtrl = new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxSize{ 100, -1 }, style);
    sizer->Add(textCtrl, flags);
    return textCtrl;
}

template <typename Function>
auto CreateTextboxWithAction(wxWindow* parent, wxSizer* sizer, int id, Function&& f, long style = 0)
{
    auto textCtrl = new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxSize{ 100, -1 }, style);
    textCtrl->Bind((style & wxTE_PROCESS_ENTER) ? wxEVT_TEXT_ENTER : wxEVT_TEXT, f);
    sizer->Add(textCtrl, BasicSizerFlags());
    return textCtrl;
}

inline auto CreateHLine(wxWindow* parent, wxSizer* sizer)
{
    auto line = new wxStaticLine(parent, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    sizer->Add(line, 0, wxGROW | wxALL, 5);
    return line;
}

template <typename String, typename Handler>
auto CreateCheckbox(wxWindow* parent, wxSizer* sizer, String caption, Handler handler)
{
    auto checkBox = new wxCheckBox(parent, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize);
    checkBox->Bind(wxEVT_CHECKBOX, [handler](wxCommandEvent& event) {
        handler(event);
    });
    sizer->Add(checkBox);
    return checkBox;
}

template <typename String>
auto CreateCheckboxWithCaption(wxWindow* parent, wxSizer* sizer, int id, String caption)
{
    auto checkBox = new wxCheckBox(parent, id, caption, wxDefaultPosition, wxDefaultSize);
    VStack(sizer, LeftBasicSizerFlags(), [checkBox](auto& sizer) {
        sizer->Add(checkBox);
    });
    return checkBox;
}

template <typename String>
auto CreateTextboxWithCaption(wxWindow* parent, wxSizer* sizer, int id, String caption, long style = 0)
{
    VStack(sizer, LeftBasicSizerFlags(), [parent, caption, id, style](auto& sizer) {
        sizer->Add(new wxStaticText(parent, wxID_STATIC, caption, wxDefaultPosition, wxDefaultSize, 0), LeftBasicSizerFlags());
        sizer->Add(new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxSize{ 100, -1 }, style), BasicSizerFlags());
    });
}

template <typename String, typename Function>
auto CreateTextboxWithCaptionAndAction(wxWindow* parent, wxSizer* sizer, int id, String caption, Function&& f, long style = 0)
{
    VStack(sizer, LeftBasicSizerFlags(), [parent, caption, id, style, f](auto& sizer) {
        sizer->Add(new wxStaticText(parent, wxID_STATIC, caption, wxDefaultPosition, wxDefaultSize, 0), LeftBasicSizerFlags());
        auto textCtrl = new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxSize{ 100, -1 }, style);
        textCtrl->Bind((style & wxTE_PROCESS_ENTER) ? wxEVT_TEXT_ENTER : wxEVT_TEXT, f);
        sizer->Add(textCtrl, BasicSizerFlags());
    });
}

// Give a wxUI::Widgets a vertical label
template <wxUI::details::Widget W>
auto VLabelWidget(std::string caption, W widget)
{
    return wxUI::Custom{
        [widget = std::move(widget), caption](wxWindow* window, wxSizer* sizer, wxSizerFlags flags) {
            wxUI::VSizer{
                wxUI::Text{ caption },
                std::move(widget),
            }
                .createAndAdd(window, sizer, flags);
        }
    };
}

#pragma GCC diagnostic pop
