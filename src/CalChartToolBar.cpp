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
    wxString caption;
    wxBitmap bm;
    wxString desc;
    int id;
    bool space;
};

std::vector<ToolBarEntry> GetSymbolsToolBar()
{
    static auto const tb = std::vector<ToolBarEntry>{
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym0))), wxT("plain"), CALCHART__setsym0, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym1))), wxT("solid"), CALCHART__setsym1, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym2))), wxT("backslash"), CALCHART__setsym2, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym3))), wxT("slash"), CALCHART__setsym3, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym4))), wxT("x"), CALCHART__setsym4, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym5))), wxT("solid backslash"), CALCHART__setsym5, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym6))), wxT("solid slash"), CALCHART__setsym6, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_sym7))), wxT("solid x"), CALCHART__setsym7, true }
    };
    return tb;
}

std::vector<ToolBarEntry> GetHalfOfMainToolBar()
{
    static auto const tb = [] {
        auto result = std::vector<ToolBarEntry>{
            { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_left))), wxT("Previous stuntsheet"), CALCHART__prev_ss, {} },
            { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_right))), wxT("Next stuntsheet"), CALCHART__next_ss, true },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_box))), wxT("Select points with box"), CALCHART__box, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_poly))), wxT("Select points with polygon"), CALCHART__poly, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lasso))), wxT("Select points with lasso"), CALCHART__lasso, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_swap))), wxT("Swap points"), CALCHART__swap, true },
            { wxITEM_CHECK, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_curve))), wxT("Draw and Edit Curves"), CALCHART__curve, true },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_mv))), wxT("Translate points"), CALCHART__move, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_line))), wxT("Shape points in a line"), CALCHART__shape_line, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_x))), wxT("Shape points in an x"), CALCHART__shape_x, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_cross))), wxT("Shape points in a cross"), CALCHART__shape_cross, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_box))), wxT("Shape points in a box"), CALCHART__shape_box, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_ellipse))), wxT("Shape points in an ellipse"), CALCHART__shape_ellipse, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shape_draw))), wxT("Shape points by drawing"), CALCHART__shape_draw, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_line))), wxT("Move points into line"), CALCHART__line, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_rot))), wxT("Rotate block"), CALCHART__rot, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_shr))), wxT("Shear block"), CALCHART__shear, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_ref))), wxT("Reflect block"), CALCHART__reflect, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_siz))), wxT("Resize block"), CALCHART__size, {} },
            { wxITEM_RADIO, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_gen))), wxT("Genius move"), CALCHART__genius, true },
        };
        return result;
    }();
    return tb;
}

std::vector<ToolBarEntry> GetSecondHalfOfMainToolBar()
{
    static const auto tb = std::vector<ToolBarEntry>{
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_l))), wxT("Label on left"), CALCHART__label_left, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_f))), wxT("Flip label"), CALCHART__label_flip, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_r))), wxT("Label on right"), CALCHART__label_right, true },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_hide))), wxT("Hide Label"), CALCHART__label_hide, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_vis_toggle))), wxT("Toggle Label Visibility"), CALCHART__label_visibility_toggle, {} },
        { wxITEM_NORMAL, wxT(""), ScaleButtonBitmap(wxBitmap(BITMAP_NAME(tb_lbl_show))), wxT("Show Label"), CALCHART__label_show, true },
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
