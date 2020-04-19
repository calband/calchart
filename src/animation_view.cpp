/*
 * AnimationView.cpp
 * Animation user interface
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

#include "animation_view.h"
#include "animation_frame.h"
#include "basic_ui.h"
#include "CalChartApp.h"
#include "CalChartDoc.h"
#include "cc_shapes.h"
#include "cc_sheet.h"
#include "confgr.h"
#include "draw.h"
#include "modes.h"

#include <wx/dcbuffer.h>

#include <fstream>

using namespace CalChart;
// IMPLEMENT_DYNAMIC_CLASS(AnimationView, wxView)

AnimationView::AnimationView()
    : mErrorOccurred(false)
    , mCollisionWarningType(COLLISION_RESPONSE_SHOW)
{
}

void AnimationView::OnDraw(wxDC* dc)
{
    auto& config = CalChartConfiguration::GetGlobalConfig();
    OnDraw(*dc, config);
}

void AnimationView::OnDraw(wxDC& dc, const CalChartConfiguration& config)
{
    dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_FIELD_DETAIL).second);
    DrawMode(dc, config, GetShow()->GetShowMode(), ShowMode_kAnimation);
    const bool checkForCollision = mCollisionWarningType != COLLISION_RESPONSE_NONE;
    if (mAnimation) {
        for (auto i = 0; i < GetShow()->GetNumPoints(); ++i) {
            auto info = mAnimation->GetAnimateInfo(i);

            if (checkForCollision && info.mCollision) {
                if (info.mCollision == Coord::COLLISION_WARNING) {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(
                        COLOR_POINT_ANIM_COLLISION_WARNING);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } else if (info.mCollision == Coord::COLLISION_INTERSECT) {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_COLLISION);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                }
            } else if (GetShow()->IsSelected(i)) {
                switch (info.mDirection) {
                case ANIMDIR_SW:
                case ANIMDIR_W:
                case ANIMDIR_NW: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_HILIT_BACK);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } break;
                case ANIMDIR_SE:
                case ANIMDIR_E:
                case ANIMDIR_NE: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_HILIT_FRONT);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } break;
                default: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_HILIT_SIDE);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                }
                }
            } else {
                switch (info.mDirection) {
                case ANIMDIR_SW:
                case ANIMDIR_W:
                case ANIMDIR_NW: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_BACK);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } break;
                case ANIMDIR_SE:
                case ANIMDIR_E:
                case ANIMDIR_NE: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_FRONT);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                } break;
                default: {
                    auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_SIDE);
                    dc.SetBrush(brushAndPen.first);
                    dc.SetPen(brushAndPen.second);
                }
                }
            }
            Coord position = info.mPosition;
            auto x = position.x + GetShow()->GetShowMode().Offset().x;
            auto y = position.y + GetShow()->GetShowMode().Offset().y;
            dc.DrawRectangle(x - Int2CoordUnits(1) / 2, y - Int2CoordUnits(1) / 2,
                Int2CoordUnits(1), Int2CoordUnits(1));
        }
    }
}

void AnimationView::OnUpdate(wxView* sender, wxObject* hint)
{
    wxView::OnUpdate(sender, hint);
    if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_modified))) {
        if (mAnimation) {
            mAnimation.reset();
            RefreshFrame();
        }
    }
    if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_FinishedLoading))) {
        Generate();
    }
}

void AnimationView::RefreshFrame()
{
    GetAnimationFrame()->UpdatePanel();
    GetAnimationFrame()->Refresh();
}

void AnimationView::SetCollisionType(CollisionWarning col)
{
    mCollisionWarningType = col;
    if (mAnimation) {
        mAnimation->SetCollisionAction(col == COLLISION_RESPONSE_BEEP ? wxBell
                                                                      : NULL);
    }
}

void AnimationView::SelectCollisions()
{
    if (mAnimation) {
        SelectionList select;
        for (auto i = 0; i < GetShow()->GetNumPoints(); i++) {
            auto info = mAnimation->GetAnimateInfo(i);

            if (info.mCollision) {
                select.insert(i);
            }
        }
        GetShow()->SetSelection(select);
    }
}

//#define GENERATE_SHOW_DUMP 1

void AnimationView::Generate()
{
    GetAnimationFrame()->SetStatusText(wxT("Compiling..."));
    mAnimation.reset();

    // flush out the show
    GetShow()->FlushAllTextWindows(); // get all changes in text windows

    // set our error indicator:
    mErrorOccurred = false;
    // (wxMessageBox(wxT("Ignore errors?"), wxT("Animate"), wxYES_NO) != wxYES);
    NotifyStatus notifyStatus = [this](const std::string& notice) {
        this->OnNotifyStatus(notice);
    };
    NotifyErrorList notifyErrorList = [this](
                                          const std::map<AnimateError, ErrorMarker>& error_markers,
                                          unsigned sheetnum, const std::string& message) {
        return this->OnNotifyErrorList(error_markers, sheetnum, message);
    };
    mAnimation = GetShow()->NewAnimation(notifyStatus, notifyErrorList);
    if (mAnimation && (mAnimation->GetNumberSheets() == 0)) {
        mAnimation.reset();
    }
    if (mAnimation) {
#if defined(GENERATE_SHOW_DUMP) && GENERATE_SHOW_DUMP
        std::ofstream output(
            (GetShow()->GetFilename() + wxT(".txt")).ToStdString().c_str());
        mAnimation->GotoSheet(0);
        do {
            output << mAnimation->GetCurrentInfo();
        } while (mAnimation->NextBeat());
#endif // GENERATE_SHOW_DUMP

        mAnimation->SetCollisionAction(
            mCollisionWarningType == COLLISION_RESPONSE_BEEP ? wxBell : NULL);
        mAnimation->GotoSheet(GetShow()->GetCurrentSheetNum());
    }
    GetAnimationFrame()->UpdatePanel();
    GetAnimationFrame()->SetStatusText(
        mErrorOccurred ? wxT("Error during compilation") : wxT("Ready"));
    GetAnimationFrame()->Refresh();
    if (mErrorOccurred) {
        wxMessageBox(wxT("Error during compilation.  See Error pulldown below."),
            wxT("Errors occurred"), wxOK);
    }
}

// true if changes made
bool AnimationView::PrevBeat()
{
    if (mAnimation && mAnimation->PrevBeat()) {
        RefreshFrame();
        return true;
    }
    return false;
}

bool AnimationView::NextBeat()
{
    if (mAnimation && mAnimation->NextBeat()) {
        RefreshFrame();
        return true;
    }
    return false;
}

void AnimationView::GotoBeat(unsigned i)
{
    if (mAnimation) {
        mAnimation->GotoBeat(i);
        RefreshFrame();
    }
}

bool AnimationView::PrevSheet()
{
    if (mAnimation && mAnimation->PrevSheet()) {
        RefreshFrame();
        return true;
    }
    return false;
}

bool AnimationView::NextSheet()
{
    if (mAnimation && mAnimation->NextSheet()) {
        RefreshFrame();
        return true;
    }
    return false;
}

void AnimationView::GotoSheet(unsigned i)
{
    if (mAnimation) {
        mAnimation->GotoSheet(i);
        RefreshFrame();
    }
}

void AnimationView::GotoAnimationSheet(unsigned i)
{
    if (mAnimation) {
        mAnimation->GotoAnimationSheet(i);
        RefreshFrame();
    }
}

void AnimationView::SetSelection(const SelectionList& sl)
{
    GetShow()->SetSelection(sl);
}

wxString AnimationView::GetStatusText() const
{
    wxString tempbuf;

    if (mAnimation) {
        tempbuf.Printf(wxT("Beat %u of %u  Sheet %d of %d \"%.32s\""),
            mAnimation->GetCurrentBeat(), mAnimation->GetNumberBeats(),
            mAnimation->GetCurrentSheet() + 1,
            mAnimation->GetNumberSheets(),
            mAnimation->GetCurrentSheetName().c_str());
    } else {
        tempbuf = wxT("No animation available");
    }
    return tempbuf;
}

// Return a bounding box of the show
std::pair<Coord, Coord> AnimationView::GetShowSizeAndOffset() const
{
    auto size = GetShow()->GetShowMode().Size();
    return { size, Coord(0, 0) };
}

// Return a bounding box of the show of where the marchers are.  If they are
// outside the show, we don't see them.
std::pair<Coord, Coord> AnimationView::GetMarcherSizeAndOffset() const
{
    auto mode_size = GetShow()->GetShowMode().Size();
    Coord bounding_box_upper_left = mode_size;
    Coord bounding_box_low_right(0, 0);

    for (auto i = 0; mAnimation && i < GetShow()->GetNumPoints(); ++i) {
        Coord position = mAnimation->GetAnimateInfo(i).mPosition;
        bounding_box_upper_left = Coord(std::min(bounding_box_upper_left.x, position.x),
            std::min(bounding_box_upper_left.y, position.y));
        bounding_box_low_right = Coord(std::max(bounding_box_low_right.x, position.x),
            std::max(bounding_box_low_right.y, position.y));
    }

    return { bounding_box_low_right - bounding_box_upper_left,
        mode_size / 2 + bounding_box_upper_left };
}

void AnimationView::UnselectMarchers()
{
    GetShow()->SetSelection(GetShow()->MakeUnselectAll());
}

void AnimationView::SelectMarchersInBox(long mouseXStart, long mouseYStart,
    long mouseXEnd, long mouseYEnd,
    bool altDown)
{
    // otherwise, Select points within rectangle
    auto x_off = GetShow()->GetShowMode().Offset().x;
    auto y_off = GetShow()->GetShowMode().Offset().y;
    Lasso lasso(Coord(mouseXStart - x_off, mouseYStart - y_off));
    lasso.Append(Coord(mouseXStart - x_off, mouseYEnd - y_off));
    lasso.Append(Coord(mouseXEnd - x_off, mouseYEnd - y_off));
    lasso.Append(Coord(mouseXEnd - x_off, mouseYStart - y_off));
    lasso.End();
    SelectionList pointlist;
    for (auto i = 0; i < GetShow()->GetNumPoints(); ++i) {
        Coord position = mAnimation->GetAnimateInfo(i).mPosition;
        if (lasso.Inside(position)) {
            pointlist.insert(i);
        }
    }
    if (altDown) {
        GetShow()->SetSelection(GetShow()->MakeToggleSelection(pointlist));
    } else {
        GetShow()->SetSelection(GetShow()->MakeAddToSelection(pointlist));
    }
}

// Keystroke command toggles the timer, but the timer
// lives in the Frame.  So we have this weird path...
void AnimationView::ToggleTimer() { GetAnimationFrame()->ToggleTimer(); }

bool AnimationView::OnBeat() const { return GetAnimationFrame()->OnBeat(); }

Continuity
AnimationView::GetContinuityOnSheet(unsigned whichSheet,
    SYMBOL_TYPE whichSymbol) const
{
    auto current_sheet = GetShow()->GetNthSheet(whichSheet);
    return current_sheet->GetContinuityBySymbol(whichSymbol);
}

void AnimationView::OnNotifyStatus(const wxString& status)
{
    GetAnimationFrame()->SetStatusText(status);
}

bool AnimationView::OnNotifyErrorList(
    const std::map<AnimateError, ErrorMarker>& error_markers, unsigned sheetnum,
    const wxString& message)
{
    GetAnimationFrame()->OnNotifyErrorList(error_markers, sheetnum, message);
    mErrorOccurred = true;
    return false;
}

const Animation* AnimationView::GetAnimation() const
{
    return mAnimation.get();
}

AnimationFrame* AnimationView::GetAnimationFrame()
{
    return static_cast<AnimationFrame*>(GetFrame());
}

const AnimationFrame* AnimationView::GetAnimationFrame() const
{
    return static_cast<const AnimationFrame*>(GetFrame());
}

CalChartDoc* AnimationView::GetShow()
{
    return static_cast<CalChartDoc*>(GetDocument());
}

const CalChartDoc* AnimationView::GetShow() const
{
    return static_cast<const CalChartDoc*>(GetDocument());
}
