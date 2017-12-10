#pragma once
/*
 * modes.h
 * Definitions for the show mode classes
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

#include "cc_coord.h"

#include <string>
#include <array>
#include <memory>
#include <functional>

namespace CalChart {

static constexpr auto SPR_YARD_LEFT = 8;
static constexpr auto SPR_YARD_RIGHT = 4;
static constexpr auto SPR_YARD_ABOVE = 2;
static constexpr auto SPR_YARD_BELOW = 1;

static constexpr auto kYardTextValues = 53;
static constexpr auto kSprLineTextValues = 5;

static constexpr int kFieldStepSizeNorthSouth[2] = { 96, 160 };
static constexpr auto kFieldStepSizeEastWest = 84;
static constexpr int kFieldStepSizeSouthEdgeFromCenter[2] = { 48, 80 };
static constexpr auto kFieldStepSizeWestEdgeFromCenter = 42;
static constexpr auto kFieldStepWestHashFromWestSideline = 32;
static constexpr auto kFieldStepEastHashFromWestSideline = 52;

class ShowMode {
public:
    typedef enum { SHOW_STANDARD,
        SHOW_SPRINGSHOW } ShowType;

    virtual ~ShowMode();

public:
    virtual ShowType GetType() const = 0;
    auto Offset() const { return mOffset() + mBorder1(); }
    auto FieldOffset() const { return -mOffset(); }
    auto Size() const { return mSize() + mBorder1() + mBorder2(); }
    auto FieldSize() const { return mSize(); }
    auto MinPosition() const { return -(mOffset() + mBorder1()); }
    auto MaxPosition() const { return mSize() - mOffset() + mBorder2(); }
    auto Border1() const { return mBorder1(); }
    auto GetName() const { return mName; };
    CalChart::Coord ClipPosition(const CalChart::Coord& pos) const;

protected:
    // Users shouldn't create show modes, it should be done through derived
    // classes
    ShowMode(const std::string& name, const CalChart::Coord& size,
        const CalChart::Coord& offset, const CalChart::Coord& border1,
        const CalChart::Coord& border2);

    ShowMode(const std::string& name, const std::function<CalChart::Coord()>& size,
        const std::function<CalChart::Coord()>& offset,
        const std::function<CalChart::Coord()>& border1,
        const std::function<CalChart::Coord()>& border2);

private:
    const std::function<CalChart::Coord()> mSize;
    const std::function<CalChart::Coord()> mOffset;
    const std::function<CalChart::Coord()> mBorder1;
    const std::function<CalChart::Coord()> mBorder2;
    std::string mName;
};

class ShowModeStandard : public ShowMode {
public:
    enum ArrayValues {
        kwhash,
        kehash,
        kbord1_x,
        kbord1_y,
        kbord2_x,
        kbord2_y,
        koffset_x,
        koffset_y,
        ksize_x,
        ksize_y,
        kShowModeValues
    };
    using ShowModeInfo_t = std::array<long, kShowModeValues>;
    static std::unique_ptr<ShowMode> CreateShowMode(const std::string& which,
        const ShowModeInfo_t& values);
    // it is expected that the caller will keep the valueGetter alive
    static std::unique_ptr<ShowMode>
    CreateShowMode(const std::string& which,
        const std::function<ShowModeInfo_t()>& valueGetter);
    static std::unique_ptr<ShowMode>
    CreateShowMode(const std::string& name, CalChart::Coord size, CalChart::Coord offset,
        CalChart::Coord border1, CalChart::Coord border2, unsigned short whash,
        unsigned short ehash);

private:
    ShowModeStandard(const std::string& name, CalChart::Coord size, CalChart::Coord offset,
        CalChart::Coord border1, CalChart::Coord border2, unsigned short whash,
        unsigned short ehash);
    ShowModeStandard(const std::string& name,
        const std::function<CalChart::Coord()>& offset,
        const std::function<CalChart::Coord()>& border1,
        const std::function<CalChart::Coord()>& size,
        const std::function<CalChart::Coord()>& border2,
        const std::function<unsigned short()>& whash,
        const std::function<unsigned short()>& ehash);

public:
    virtual ~ShowModeStandard();

    virtual ShowType GetType() const;
    unsigned short HashW() const { return mHashW(); }
    unsigned short HashE() const { return mHashE(); }

private:
    const std::function<unsigned short()> mHashW, mHashE;
};

class ShowModeSprShow : public ShowMode {
public:
    enum ArrayValues {
        kwhich_spr_yards,
        kbord1_x,
        kbord1_y,
        kbord2_x,
        kbord2_y,
        kmode_steps_x,
        kmode_steps_y,
        kmode_steps_w,
        kmode_steps_h,
        keps_stage_x,
        keps_stage_y,
        keps_stage_w,
        keps_stage_h,
        keps_field_x,
        keps_field_y,
        keps_field_w,
        keps_field_h,
        keps_text_left,
        keps_text_right,
        keps_text_top,
        keps_text_bottom,
        kShowModeValues
    };
    using SpringShowModeInfo_t = std::array<long, kShowModeValues>;

    static std::unique_ptr<ShowMode>
    CreateSpringShowMode(const std::string& which,
        const SpringShowModeInfo_t& values);
    // it is expected that the caller will keep the valueGetter alive
    static std::unique_ptr<ShowMode> CreateSpringShowMode(
        const std::string& which,
        const std::function<SpringShowModeInfo_t()>& valueGetter);

private:
    // Look at calchart.cfg for description of arguments
    ShowModeSprShow(const std::string& name, CalChart::Coord border1, CalChart::Coord border2,
        unsigned char which, short stps_x, short stps_y, short stps_w,
        short stps_h, short stg_x, short stg_y, short stg_w,
        short stg_h, short fld_x, short fld_y, short fld_w,
        short fld_h, short txt_l, short txt_r, short txt_tp,
        short txt_bm);
    ShowModeSprShow(
        const std::string& name, const std::function<CalChart::Coord()>& border1,
        const std::function<CalChart::Coord()>& border2,
        const std::function<unsigned char()>& which,
        const std::function<short()>& stps_x,
        const std::function<short()>& stps_y,
        const std::function<short()>& stps_w,
        const std::function<short()>& stps_h, const std::function<short()>& stg_x,
        const std::function<short()>& stg_y, const std::function<short()>& stg_w,
        const std::function<short()>& stg_h, const std::function<short()>& fld_x,
        const std::function<short()>& fld_y, const std::function<short()>& fld_w,
        const std::function<short()>& fld_h, const std::function<short()>& txt_l,
        const std::function<short()>& txt_r, const std::function<short()>& txt_tp,
        const std::function<short()>& txt_bm);

public:
    virtual ~ShowModeSprShow();

    virtual ShowType GetType() const;
    unsigned char WhichYards() const { return which_yards(); }
    short StageX() const { return stage_x(); }
    short StageY() const { return stage_y(); }
    short StageW() const { return stage_w(); }
    short StageH() const { return stage_h(); }
    short FieldX() const { return field_x(); }
    short FieldY() const { return field_y(); }
    short FieldW() const { return field_w(); }
    short FieldH() const { return field_h(); }
    short StepsX() const { return steps_x(); }
    short StepsY() const { return steps_y(); }
    short StepsW() const { return steps_w(); }
    short StepsH() const { return steps_h(); }
    short TextLeft() const { return text_left(); }
    short TextRight() const { return text_right(); }
    short TextTop() const { return text_top(); }
    short TextBottom() const { return text_bottom(); }

private:
    const std::function<unsigned char()> which_yards;
    const std::function<short()> stage_x, stage_y, stage_w, stage_h;
    const std::function<short()> field_x, field_y, field_w, field_h;
    const std::function<short()> steps_x, steps_y, steps_w, steps_h;
    const std::function<short()> text_left, text_right, text_top, text_bottom;
};
}
