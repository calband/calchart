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
#include "CalChartRanges.h"
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
    : mSheets2({})
    , mPoints(show.GetNumPoints())
    , mCurrentCmdIndex(mPoints.size())
    , mCurrentSheetNumber(0)
    , mAnimationErrors(show.GetNumSheets())
{
    // the variables are persistant through the entire compile process.
    AnimationVariables variablesStates;

    int newSheetIndex = 0;
    int prevSheetIndex = 0;
    auto sheets2 = std::vector<Animate::Sheet>{};

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
        std::vector<std::vector<Animate::Command>> theCommands2(mPoints.size());
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
                        auto compileResults = CalChart::Compile(variablesStates, errors, curr_sheet, show.GetSheetEnd(), j, current_symbol, continuity);
                        theCommands[j] = compileResults.first;
                        theCommands2[j] = compileResults.second;
                    }
                }
            }
        }
        // Handle points that don't have continuity (shouldn't happen)
        for (unsigned j = 0; j < mPoints.size(); j++) {
            if (theCommands[j].empty()) {
                auto compileResults = CalChart::Compile(variablesStates, errors, curr_sheet, show.GetSheetEnd(), j, MAX_NUM_SYMBOLS, {});
                theCommands[j] = compileResults.first;
                theCommands2[j] = compileResults.second;
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
        sheets2.emplace_back(curr_sheet->GetName(), curr_sheet->GetBeats(), theCommands2);
        // here's where we would put in another sheet, and compare to see if all the positions are the same,
        // if it makes the same json movements.
    }

    mSheets2 = Animate::Sheets{ sheets2 };

    FindAllCollisions();
    RefreshSheet();
}

Animation::~Animation() { }

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

void Animation::BeginCmd(unsigned i)
{
    while (!GetCommand(mCurrentSheetNumber, i).Begin(mPoints[i])) {
        if ((mCurrentCmdIndex[i] + 1) != mSheets.at(mCurrentSheetNumber).GetCommandsEndIndex(i))
            return;
        ++mCurrentCmdIndex[i];
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

auto Animation::BeatHasCollision(beats_t whichBeat) const -> bool
{
    return mSheets2.BeatHasCollision(whichBeat);
}

auto Animation::GetAnimateInfo(beats_t whichBeat, int which) const -> Animate::Info
{
    return {
        mSheets2.CollisionAtBeat(whichBeat, which),
        mSheets2.MarcherInfoAtBeat(whichBeat, which)
    };
}

namespace {
    // Because we need to draw sprites and other animations without overlap, sort them so the "closer" ones are first
    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Info>)
    auto SortForSprites(Range input)
    {
        std::ranges::sort(input, [](auto&& a, auto&& b) {
            return a.mMarcherInfo.mPosition < b.mMarcherInfo.mPosition;
        });
        return input;
    }

}

auto Animation::GetAllAnimateInfo(beats_t whichBeat) const -> std::vector<Animate::Info>
{
    auto points = std::vector<int>(mPoints.size());
    std::iota(points.begin(), points.end(), 0);
    auto animates = std::vector<Animate::Info>{};
    std::transform(points.begin(), points.end(), std::back_inserter(animates), [this, whichBeat](auto which) {
        return GetAnimateInfo(whichBeat, which);
    });
    return CalChart::Ranges::ToVector<Animate::Info>(SortForSprites(animates));
}

int Animation::GetNumberSheets() const { return static_cast<int>(mSheets.size()); }

auto Animation::GetTotalNumberBeatsUpTo(int sheet) const -> beats_t
{
    return std::accumulate(mSheets.cbegin(), mSheets.cbegin() + sheet, 0, [](auto&& a, auto&& b) {
        return a + b.GetNumBeats();
    });
}

auto Animation::GetTotalCurrentBeat() const -> beats_t
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
    return mSheets2.GeneratePathToDraw(whichSheet, point, endRadius);
}

std::pair<std::string, std::vector<std::string>>
Animation::GetCurrentInfo(beats_t whichBeat) const
{
    std::vector<std::string> each;
    for (auto i = 0; i < static_cast<int>(mPoints.size()); ++i) {
        std::ostringstream each_string;
        auto info = GetAnimateInfo(whichBeat, i);
        each_string << "pt " << i << ": (" << info.mMarcherInfo.mPosition.x << ", "
                    << info.mMarcherInfo.mPosition.y << "), dir=" << CalChart::Degree{ info.mMarcherInfo.mFacingDirection }.getValue()
                    << ((info.mCollision != CalChart::Coord::CollisionType::none) ? ", collision!" : "");
        each.push_back(each_string.str());
    }
    std::ostringstream output;
    auto [sheet, beat] = mSheets2.BeatToSheetOffsetAndBeat(whichBeat);
    output << mSheets2.GetSheetName(sheet) << " (" << sheet << " of "
           << GetNumberSheets() << ")\n";
    output << "beat " << beat << " of " << mSheets2.BeatForSheet(sheet) << "\n";
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

auto Animation::GetAnimationCollisions() const -> std::map<int, CalChart::SelectionList>
{
    return mSheets2.SheetsToMarchersWhoCollided();
    // first map all the collisions to a sheet with a point group.
    auto result = std::map<int, CalChart::SelectionList>{};
    for (auto [marcherInfo, collisionType] : mCollisions) {
        (void)collisionType;
        auto [whichMarcher, whichSheet, whichBeat] = marcherInfo;
        (void)whichBeat;
        result[whichSheet].insert(whichMarcher);
    }
    return result;
}

}
