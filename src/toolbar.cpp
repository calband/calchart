/*
 * toolbar.cpp
 * Handle adding toolbars
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

#include "toolbar.h"
#include "ui_enums.h"
#include "platconf.h"

#include "tb_left.xbm"
#include "tb_right.xbm"
#include "tb_box.xbm"
#include "tb_poly.xbm"
#include "tb_lasso.xbm"
#include "tb_mv.xbm"
#include "tb_swap.xbm"
#include "tb_line.xbm"
#include "tb_rot.xbm"
#include "tb_shr.xbm"
#include "tb_ref.xbm"
#include "tb_siz.xbm"
#include "tb_gen.xbm"
#include "tb_lbl_l.xbm"
#include "tb_lbl_r.xbm"
#include "tb_lbl_f.xbm"
#include "tb_lbl_show.xbm"
#include "tb_lbl_hide.xbm"
#include "tb_lbl_vis_toggle.xbm"
#include "tb_sym0.xbm"
#include "tb_sym1.xbm"
#include "tb_sym2.xbm"
#include "tb_sym3.xbm"
#include "tb_sym4.xbm"
#include "tb_sym5.xbm"
#include "tb_sym6.xbm"
#include "tb_sym7.xbm"
#include "tb_stop.xbm"
#include "tb_play.xbm"
#include "tb_pbeat.xbm"
#include "tb_nbeat.xbm"
#include "tb_pshet.xbm"
#include "tb_nshet.xbm"

void AddCoolToolBar(const std::vector<ToolBarEntry>& entries, wxFrame& frame)
{
    wxToolBar* tb = frame.CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL);

    for (std::vector<ToolBarEntry>::const_iterator i = entries.begin();
         i != entries.end(); i++) {
        tb->AddTool(i->id, wxT(""), *(i->bm), i->desc, i->kind);
        if (i->space) {
            tb->AddSeparator();
        }
    }
    tb->Realize();
}

std::vector<ToolBarEntry> GetSymbolsToolBar()
{
    static const ToolBarEntry tb[] = {
        { wxITEM_NORMAL, NULL, wxT("plainmen"), CALCHART__setsym0 },
        { wxITEM_NORMAL, NULL, wxT("solidmen"), CALCHART__setsym1 },
        { wxITEM_NORMAL, NULL, wxT("backslash men"), CALCHART__setsym2 },
        { wxITEM_NORMAL, NULL, wxT("slash men"), CALCHART__setsym3 },
        { wxITEM_NORMAL, NULL, wxT("x men"), CALCHART__setsym4 },
        { wxITEM_NORMAL, NULL, wxT("solid backslash men"), CALCHART__setsym5 },
        { wxITEM_NORMAL, NULL, wxT("solid slash men"), CALCHART__setsym6 },
        { wxITEM_NORMAL, NULL, wxT("solid x men"), CALCHART__setsym7 }
    };
    static std::vector<ToolBarEntry> sTB(tb, tb + sizeof(tb) / sizeof(tb[0]));
    // technically this is a race condition, but not worth fixing (how often do we
    // call this function synchronously...?
    static bool sFirstTime = true;
    if (sFirstTime) {
        sFirstTime = false;
        std::vector<ToolBarEntry>::iterator i = sTB.begin();
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_sym0));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_sym1));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_sym2));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_sym3));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_sym4));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_sym5));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_sym6));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_sym7));
    }
    return sTB;
}

std::vector<ToolBarEntry> GetHalfOfMainToolBar()
{
    static const ToolBarEntry tb[] = {
        { wxITEM_NORMAL, NULL, wxT("Previous stuntsheet"), CALCHART__prev_ss },
        { wxITEM_NORMAL, NULL, wxT("Next stuntsheet"), CALCHART__next_ss, true },
        { wxITEM_RADIO, NULL, wxT("Select points with box"), CALCHART__box },
        { wxITEM_RADIO, NULL, wxT("Select points with polygon"), CALCHART__poly },
        { wxITEM_RADIO, NULL, wxT("Select points with lasso"), CALCHART__lasso,
         true },
        { wxITEM_RADIO, NULL, wxT("Translate points"), CALCHART__move },
        { wxITEM_RADIO, NULL, wxT("Swap points"), CALCHART__swap },
        { wxITEM_RADIO, NULL, wxT("Move points into line"), CALCHART__line },
        { wxITEM_RADIO, NULL, wxT("Rotate block"), CALCHART__rot },
        { wxITEM_RADIO, NULL, wxT("Shear block"), CALCHART__shear },
        { wxITEM_RADIO, NULL, wxT("Reflect block"), CALCHART__reflect },
        { wxITEM_RADIO, NULL, wxT("Resize block"), CALCHART__size },
        { wxITEM_RADIO, NULL, wxT("Genius move"), CALCHART__genius, true },
        { wxITEM_NORMAL, NULL, wxT("Label on left"), CALCHART__label_left },
        { wxITEM_NORMAL, NULL, wxT("Flip label"), CALCHART__label_flip },
        { wxITEM_NORMAL, NULL, wxT("Label on right"), CALCHART__label_right, true },
        { wxITEM_NORMAL, NULL, wxT("Hide Label"), CALCHART__label_hide },
        { wxITEM_NORMAL, NULL, wxT("Toggle Label Visibility"),
         CALCHART__label_visibility_toggle },
        { wxITEM_NORMAL, NULL, wxT("Show Label"), CALCHART__label_show, true },
    };
    static std::vector<ToolBarEntry> sTB(tb, tb + sizeof(tb) / sizeof(tb[0]));
    static bool sFirstTime = true;
    if (sFirstTime) {
        sFirstTime = false;
        std::vector<ToolBarEntry>::iterator i = sTB.begin();
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_left));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_right));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_box));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_poly));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_lasso));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_mv));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_swap));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_line));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_rot));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_shr));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_ref));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_siz));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_gen));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_lbl_l));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_lbl_f));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_lbl_r));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_lbl_hide));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_lbl_vis_toggle));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_lbl_show));
    }
    return sTB;
}

std::vector<ToolBarEntry> GetMainToolBar()
{
    auto first_half = GetHalfOfMainToolBar();
    auto second_half = GetSymbolsToolBar();
    first_half.insert(first_half.end(), second_half.begin(), second_half.end());
    return first_half;
}

std::vector<ToolBarEntry> GetAnimationToolBar()
{
    static const ToolBarEntry anim_tb[] = {
        { wxITEM_NORMAL, NULL, wxT("Stop (space toggle)"), CALCHART__anim_stop },
        { wxITEM_NORMAL, NULL, wxT("Play (space toggle)"), CALCHART__anim_play },
        { wxITEM_NORMAL, NULL, wxT("Previous beat (left arrow)"),
         CALCHART__anim_prev_beat },
        { wxITEM_NORMAL, NULL, wxT("Next beat (right arrow)"),
         CALCHART__anim_next_beat },
        { wxITEM_NORMAL, NULL, wxT("Previous stuntsheet"),
         CALCHART__anim_prev_sheet },
        { wxITEM_NORMAL, NULL, wxT("Next stuntsheet"), CALCHART__anim_next_sheet }
    };
    static std::vector<ToolBarEntry> sAnimTB(
        anim_tb, anim_tb + sizeof(anim_tb) / sizeof(anim_tb[0]));
    static bool sFirstTime = true;
    if (sFirstTime) {
        sFirstTime = false;
        std::vector<ToolBarEntry>::iterator i = sAnimTB.begin();
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_stop));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_play));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_pbeat));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_nbeat));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_pshet));
        (i++)->bm = new wxBitmap(BITMAP_NAME(tb_nshet));
    }
    return sAnimTB;
}
