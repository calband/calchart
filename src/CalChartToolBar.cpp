/*
 * CalChartToolBar.cpp
 * Header for adding toolbars to CalChart
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

#include "CalChartToolBar.h"
#include "CalChartSizes.h"
#include "platconf.h"
#include "ui_enums.h"

#include "tb_box.xbm"
#include "tb_bug.xbm"
#include "tb_curve.xbm"
#include "tb_gen.xbm"
#include "tb_lasso.xbm"
#include "tb_lbl_f.xbm"
#include "tb_lbl_hide.xbm"
#include "tb_lbl_l.xbm"
#include "tb_lbl_r.xbm"
#include "tb_lbl_show.xbm"
#include "tb_lbl_vis_toggle.xbm"
#include "tb_left.xbm"
#include "tb_line.xbm"
#include "tb_mv.xbm"
#include "tb_nbeat.xbm"
#include "tb_nshet.xbm"
#include "tb_pbeat.xbm"
#include "tb_play.xbm"
#include "tb_poly.xbm"
#include "tb_pshet.xbm"
#include "tb_ref.xbm"
#include "tb_right.xbm"
#include "tb_rot.xbm"
#include "tb_shape_box.xbm"
#include "tb_shape_cross.xbm"
#include "tb_shape_draw.xbm"
#include "tb_shape_ellipse.xbm"
#include "tb_shape_line.xbm"
#include "tb_shape_x.xbm"
#include "tb_shr.xbm"
#include "tb_siz.xbm"
#include "tb_stop.xbm"
#include "tb_swap.xbm"
#include "tb_sym0.xbm"
#include "tb_sym1.xbm"
#include "tb_sym2.xbm"
#include "tb_sym3.xbm"
#include "tb_sym4.xbm"
#include "tb_sym5.xbm"
#include "tb_sym6.xbm"
#include "tb_sym7.xbm"

#include <wx/aui/auibar.h>
#include <wx/toolbar.h>
#include <wx/wx.h>

struct ToolBarEntry {
    wxItemKind kind;
    std::string caption;
    wxBitmap bm;
    std::string desc;
    int id;
    bool space;
};

std::vector<ToolBarEntry> GetSymbolsToolBar()
{
    static auto const tb = std::vector<ToolBarEntry>{
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym0))), "plain", CALCHART__setsym0, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym1))), "solid", CALCHART__setsym1, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym2))), "backslash", CALCHART__setsym2, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym3))), "slash", CALCHART__setsym3, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym4))), "x", CALCHART__setsym4, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym5))), "solid backslash", CALCHART__setsym5, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym6))), "solid slash", CALCHART__setsym6, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym7))), "solid x", CALCHART__setsym7, true }
    };
    return tb;
}

std::vector<ToolBarEntry> GetHalfOfMainToolBar()
{
    static auto const tb = [] {
        auto result = std::vector<ToolBarEntry>{
            { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_left))), "Previous stuntsheet", CALCHART__prev_ss, {} },
            { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_right))), "Next stuntsheet", CALCHART__next_ss, true },
            { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_bug))), "Report a bug", CALCHART__file_bug, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_box))), "Select points with box", CALCHART__box, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_poly))), "Select points with polygon", CALCHART__poly, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lasso))), "Select points with lasso", CALCHART__lasso, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_swap))), "Swap points", CALCHART__swap, true },
            { wxITEM_CHECK, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_curve))), "Draw and Edit Curves", CALCHART__curve, true },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_mv))), "Translate points", CALCHART__move, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_line))), "Shape points in a line", CALCHART__shape_line, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_x))), "Shape points in an x", CALCHART__shape_x, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_cross))), "Shape points in a cross", CALCHART__shape_cross, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_box))), "Shape points in a box", CALCHART__shape_box, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_ellipse))), "Shape points in an ellipse", CALCHART__shape_ellipse, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_draw))), "Shape points by drawing", CALCHART__shape_draw, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_line))), "Move points into line", CALCHART__line, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_rot))), "Rotate block", CALCHART__rot, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shr))), "Shear block", CALCHART__shear, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_ref))), "Reflect block", CALCHART__reflect, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_siz))), "Resize block", CALCHART__size, {} },
            { wxITEM_RADIO, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_gen))), "Genius move", CALCHART__genius, true },
        };
        return result;
    }();
    return tb;
}

std::vector<ToolBarEntry> GetSecondHalfOfMainToolBar()
{
    static const auto tb = std::vector<ToolBarEntry>{
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_l))), "Label on left", CALCHART__label_left, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_f))), "Flip label", CALCHART__label_flip, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_r))), "Label on right", CALCHART__label_right, true },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_hide))), "Hide Label", CALCHART__label_hide, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_vis_toggle))), "Toggle Label Visibility", CALCHART__label_visibility_toggle, {} },
        { wxITEM_NORMAL, {}, ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_show))), "Show Label", CALCHART__label_show, true },
    };
    return tb;
}

auto GetSymbolsToolbar()
{
    auto first_half = GetSymbolsToolBar();
    auto second_half = GetSecondHalfOfMainToolBar();
    first_half.insert(first_half.end(), second_half.begin(), second_half.end());
    return first_half;
}

std::vector<wxBitmap> GetSymbolsBitmap()
{
    auto result = std::vector<wxBitmap>();
    for (auto&& i : GetSymbolsToolBar()) {
        result.push_back(i.bm);
    }
    return result;
}

template <typename T>
auto CreateAuiToolBar(wxAuiToolBar* tb, T toolbarBits)
{
    for (auto&& i : toolbarBits) {
        tb->AddTool(i.id, i.caption, i.bm, i.desc, i.kind);
        if (i.space) {
            tb->AddSeparator();
        }
    }
    tb->Realize();
    return tb;
}

wxAuiToolBar* CreateSelectAndMoves(wxWindow* parent, wxWindowID id, long style)
{

    return CreateAuiToolBar(new wxAuiToolBar(parent, id, wxDefaultPosition, wxDefaultSize, style), GetHalfOfMainToolBar());
}

wxAuiToolBar* CreateDotModifiers(wxWindow* parent, wxWindowID id, long style)
{
    return CreateAuiToolBar(new wxAuiToolBar(parent, id, wxDefaultPosition, wxDefaultSize, style), GetSymbolsToolbar());
}
