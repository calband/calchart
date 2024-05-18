/*
 * CalChartAnimations.cpp
 * Classes for animating shows
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

#include "CalChartAnimation.h"
#include "CalChartAnimationCommand.h"
#include "CalChartAnimationCompile.h"
#include "CalChartAnimationErrors.h"
#include "CalChartContinuity.h"
#include "CalChartDrawCommand.h"
#include "CalChartPoint.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "CalChartUtils.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>

namespace CalChart {

Animation::Animation(const Show& show)
    : mPoints(show.GetNumPoints())
    , mCurrentCmdIndex(mPoints.size())
    , mCurrentSheetNumber(0)
    , mAnimationErrors(show.GetNumSheets())
{
    // the variables are persistant through the entire compile process.
    AnimationVariables variablesStates;

    int newSheetIndex = 0;
    int prevSheetIndex = 0;
    for (auto curr_sheet = show.GetSheetBegin(); curr_sheet != show.GetSheetEnd(); ++curr_sheet) {

        if (!curr_sheet->IsInAnimation()) {
            mAnimSheetIndices.push_back(prevSheetIndex);
            continue;
        }

        mAnimSheetIndices.push_back(newSheetIndex);
        prevSheetIndex = newSheetIndex;
        newSheetIndex++;

        // Now parse continuity
        AnimationErrors errors;
        std::vector<AnimationCommands> theCommands(mPoints.size());
        for (auto& current_symbol : k_symbols) {
            if (curr_sheet->ContinuityInUse(current_symbol)) {
                auto& current_continuity = curr_sheet->GetContinuityBySymbol(current_symbol);
                auto& continuity = current_continuity.GetParsedContinuity();
#if 0 // enable to see dump of continuity
                {
                    for (auto& proc : continuity) {
                        std::cout << *proc << "\n";
                    }
                }
#endif
                for (unsigned j = 0; j < mPoints.size(); j++) {
                    if (curr_sheet->GetSymbol(j) == current_symbol) {
                        theCommands[j] = CalChart::Compile(variablesStates, errors, curr_sheet, show.GetSheetEnd(), j, current_symbol, continuity);
                    }
                }
            }
        }
        // Handle points that don't have continuity (shouldn't happen)
        for (unsigned j = 0; j < mPoints.size(); j++) {
            if (theCommands[j].empty()) {
                theCommands[j] = CalChart::Compile(variablesStates, errors, curr_sheet, show.GetSheetEnd(), j, MAX_NUM_SYMBOLS, {});
            }
        }
        if (errors.AnyErrors()) {
            mAnimationErrors[std::distance(show.GetSheetBegin(), curr_sheet)] = errors;
        }
        std::vector<Coord> thePoints(mPoints.size());
        for (unsigned i = 0; i < mPoints.size(); i++) {
            thePoints.at(i) = curr_sheet->GetPosition(i);
        }
        mSheets.emplace_back(thePoints, theCommands, curr_sheet->GetName(), curr_sheet->GetBeats());
    }
    FindAllCollisions();
    RefreshSheet();
}

Animation::~Animation() { }

bool Animation::PrevSheet()
{
    if (mCurrentBeatNumber == 0) {
        if (mCurrentSheetNumber > 0) {
            mCurrentSheetNumber--;
        }
    }
    RefreshSheet();
    FindAllCollisions();
    return true;
}

bool Animation::NextSheet()
{
    if ((mCurrentSheetNumber + 1) != mSheets.size()) {
        mCurrentSheetNumber++;
        RefreshSheet();
    } else {
        if (mCurrentBeatNumber >= mSheets.at(mCurrentSheetNumber).GetNumBeats()) {
            if (mSheets.at(mCurrentSheetNumber).GetNumBeats() == 0) {
                mCurrentBeatNumber = 0;
            } else {
                mCurrentBeatNumber = mSheets.at(mCurrentSheetNumber).GetNumBeats() - 1;
            }
        }
        return false;
    }
    return true;
}

bool Animation::PrevBeat()
{
    unsigned i;

    if (mCurrentBeatNumber == 0) {
        if (mCurrentSheetNumber == 0)
            return false;
        mCurrentSheetNumber--;
        for (i = 0; i < mPoints.size(); i++) {
            mCurrentCmdIndex[i] = mSheets.at(mCurrentSheetNumber).GetCommandsEndIndex(i) - 1;
            EndCmd(i);
        }
        mCurrentBeatNumber = mSheets.at(mCurrentSheetNumber).GetNumBeats();
    }
    for (i = 0; i < mPoints.size(); i++) {
        if (!GetCommand(mCurrentSheetNumber, i).PrevBeat(mPoints[i])) {
            // Advance to prev command, skipping zero beat commands
            if (mCurrentCmdIndex[i] != mSheets.at(mCurrentSheetNumber).GetCommandsBeginIndex(i)) {
                --mCurrentCmdIndex[i];
                EndCmd(i);
                // Set to next-to-last beat of this command
                // Should always return true
                GetCommand(mCurrentSheetNumber, i).PrevBeat(mPoints[i]);
            }
        }
    }
    if (mCurrentBeatNumber > 0)
        mCurrentBeatNumber--;
    return true;
}

bool Animation::NextBeat()
{
    unsigned i;

    mCurrentBeatNumber++;
    if (mCurrentBeatNumber >= mSheets.at(mCurrentSheetNumber).GetNumBeats()) {
        return NextSheet();
    }
    for (i = 0; i < mPoints.size(); i++) {
        if (!GetCommand(mCurrentSheetNumber, i).NextBeat(mPoints[i])) {
            // Advance to next command, skipping zero beat commands
            if ((mCurrentCmdIndex[i] + 1) != mSheets.at(mCurrentSheetNumber).GetCommandsEndIndex(i)) {
                ++mCurrentCmdIndex[i];
                BeginCmd(i);
            }
        }
    }
    return true;
}

void Animation::GotoBeat(unsigned i)
{
    while (mCurrentBeatNumber > i) {
        PrevBeat();
    }
    while (mCurrentBeatNumber < i) {
        NextBeat();
    }
}

void Animation::GotoSheet(unsigned i)
{
    mCurrentSheetNumber = mAnimSheetIndices.at(i);
    RefreshSheet();
}

void Animation::GotoTotalBeat(int i)
{
    while (GetTotalCurrentBeat() > i) {
        PrevBeat();
    }
    while (GetTotalCurrentBeat() < i) {
        NextBeat();
    }
}

void Animation::BeginCmd(unsigned i)
{
    while (!GetCommand(mCurrentSheetNumber, i).Begin(mPoints[i])) {
        if ((mCurrentCmdIndex[i] + 1) != mSheets.at(mCurrentSheetNumber).GetCommandsEndIndex(i))
            return;
        ++mCurrentCmdIndex[i];
    }
}

void Animation::EndCmd(unsigned i)
{
    while (!GetCommand(mCurrentSheetNumber, i).End(mPoints[i])) {
        if ((mCurrentCmdIndex[i]) == mSheets.at(mCurrentSheetNumber).GetCommandsBeginIndex(i))
            return;
        --mCurrentCmdIndex[i];
    }
}

void Animation::RefreshSheet()
{
    mPoints = mSheets.at(mCurrentSheetNumber).GetPoints();
    for (auto i = 0u; i < mPoints.size(); i++) {
        mCurrentCmdIndex[i] = mSheets.at(mCurrentSheetNumber).GetCommandsBeginIndex(i);
        BeginCmd(i);
    }
    mCurrentBeatNumber = 0;
}

void Animation::FindAllCollisions()
{
    mCollisions.clear();
    auto oldSheet = mCurrentSheetNumber;
    auto oldBeat = mCurrentBeatNumber;
    mCurrentSheetNumber = 0;
    mCurrentBeatNumber = 0;
    RefreshSheet();
    while (NextBeat()) {
        for (unsigned i = 0; i < mPoints.size(); i++) {
            for (unsigned j = i + 1; j < mPoints.size(); j++) {
                auto collisionResult = mPoints[i].DetectCollision(mPoints[j]);
                if (collisionResult != Coord::CollisionType::none) {
                    if (!mCollisions.count({ i, mCurrentSheetNumber, mCurrentBeatNumber }) || mCollisions[{ i, mCurrentSheetNumber, mCurrentBeatNumber }] < collisionResult) {
                        mCollisions[{ i, mCurrentSheetNumber, mCurrentBeatNumber }] = collisionResult;
                    }
                    if (!mCollisions.count({ j, mCurrentSheetNumber, mCurrentBeatNumber }) || mCollisions[{ j, mCurrentSheetNumber, mCurrentBeatNumber }] < collisionResult) {
                        mCollisions[{ j, mCurrentSheetNumber, mCurrentBeatNumber }] = collisionResult;
                    }
                }
            }
        }
    }
    mCurrentSheetNumber = oldSheet;
    mCurrentBeatNumber = oldBeat;
    RefreshSheet();
}

bool Animation::CurrentBeatHasCollision() const
{
    for (unsigned i = 0; i < mPoints.size(); i++) {
        if (mCollisions.count({ i, mCurrentSheetNumber, mCurrentBeatNumber })) {
            return true;
        }
    }
    return false;
}

Animation::animate_info_t Animation::GetAnimateInfo(int which) const
{
    auto const& cmd = GetCommand(mCurrentSheetNumber, which);
    return {
        which,
        mCollisions.count({ which, mCurrentSheetNumber, mCurrentBeatNumber }) ? mCollisions.find({ which, mCurrentSheetNumber, mCurrentBeatNumber })->second : Coord::CollisionType::none,
        CalChart::Radian{ cmd.FacingDirection() },
        mPoints.at(which),
        cmd.StepStyle()
    };
}

std::vector<Animation::animate_info_t> Animation::GetAllAnimateInfo() const
{
    auto points = std::vector<int>(mPoints.size());
    std::iota(points.begin(), points.end(), 0);
    auto animates = std::vector<Animation::animate_info_t>();
    std::transform(points.begin(), points.end(), std::back_inserter(animates), [this](auto which) -> Animation::animate_info_t {
        return GetAnimateInfo(which);
    });
    std::sort(animates.begin(), animates.end(), [](auto& a, auto& b) {
        return a.mPosition < b.mPosition;
    });
    return animates;
}

int Animation::GetNumberSheets() const { return static_cast<int>(mSheets.size()); }

unsigned Animation::GetTotalNumberBeatsUpTo(int sheet) const
{
    return std::accumulate(mSheets.cbegin(), mSheets.cbegin() + sheet, 0, [](auto&& a, auto&& b) {
        return a + b.GetNumBeats();
    });
}

int Animation::GetTotalCurrentBeat() const
{
    return GetTotalNumberBeatsUpTo(mCurrentSheetNumber) + mCurrentBeatNumber;
}

AnimationCommands Animation::GetCommands(unsigned whichSheet, unsigned whichPoint) const
{
    return mSheets.at(whichSheet).GetCommands(whichPoint);
}

AnimationCommand& Animation::GetCommand(unsigned whichSheet, unsigned whichPoint) const
{
    return *GetCommands(whichSheet, whichPoint).at(mCurrentCmdIndex.at(whichPoint));
}

auto Animation::GenPathToDraw(unsigned whichSheet, unsigned point, Coord::units endRadius) const -> std::vector<Draw::DrawCommand>
{
    auto animation_commands = GetCommands(whichSheet, point);
    auto position = mSheets.at(whichSheet).GetPoints().at(point);
    std::vector<Draw::DrawCommand> draw_commands;
    for (auto&& commands : animation_commands) {
        draw_commands.push_back(commands->GenCC_DrawCommand(position));
        commands->ApplyForward(position);
    }
    // now at this point we should put in a circle for end point
    draw_commands.push_back(CalChart::Draw::Circle{ position, endRadius, true });
    return draw_commands;
}

std::pair<std::string, std::vector<std::string>>
Animation::GetCurrentInfo() const
{
    std::vector<std::string> each;
    for (auto i = 0; i < static_cast<int>(mPoints.size()); ++i) {
        std::ostringstream each_string;
        auto info = GetAnimateInfo(i);
        each_string << "pt " << i << ": (" << info.mPosition.x << ", "
                    << info.mPosition.y << "), dir=" << CalChart::Degree{ info.mFacingDirection }.getValue()
                    << ((info.mCollision != CalChart::Coord::CollisionType::none) ? ", collision!" : "");
        each.push_back(each_string.str());
    }
    std::ostringstream output;
    output << GetCurrentSheetName() << " (" << GetCurrentSheet() << " of "
           << GetNumberSheets() << ")\n";
    output << "beat " << GetCurrentBeat() << " of " << GetNumberBeats() << "\n";
    return std::pair<std::string, std::vector<std::string>>(output.str(), each);
}

std::vector<AnimationErrors> Animation::GetAnimationErrors() const
{
    return mAnimationErrors;
}

auto Animation::toOnlineViewerJSON(std::vector<std::vector<CalChart::Point>> const& pointsOverSheets) const -> std::vector<std::vector<std::vector<nlohmann::json>>>
{
    auto results = std::vector<std::vector<std::vector<nlohmann::json>>>{};
    for (auto i : std::views::iota(0UL, pointsOverSheets.size())) {
        results.push_back(mSheets.at(i).toOnlineViewerJSON(pointsOverSheets.at(i)));
    }
    return results;
}

}
