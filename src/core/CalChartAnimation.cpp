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
    : mSheets({})
    , mAnimationErrors(show.GetNumSheets())
{
    auto points = std::vector<Coord>(show.GetNumPoints());
    // the variables are persistant through the entire compile process.
    AnimationVariables variablesStates;

    auto sheets = std::vector<Animate::Sheet>{};

    for (auto curr_sheet = show.GetSheetBegin(); curr_sheet != show.GetSheetEnd(); ++curr_sheet) {

        if (!curr_sheet->IsInAnimation()) {
            continue;
        }

        // Now parse continuity
        AnimationErrors errors;
        std::vector<AnimationCommands> theCommands(points.size());
        std::vector<std::vector<Animate::Command>> theCommands2(points.size());
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
                for (unsigned j = 0; j < points.size(); j++) {
                    if (curr_sheet->GetSymbol(j) == current_symbol) {
                        auto compileResults = CalChart::Compile(variablesStates, errors, curr_sheet, show.GetSheetEnd(), j, current_symbol, continuity);
                        theCommands[j] = compileResults.first;
                        theCommands2[j] = compileResults.second;
                    }
                }
            }
        }
        // Handle points that don't have continuity (shouldn't happen)
        for (unsigned j = 0; j < points.size(); j++) {
            if (theCommands[j].empty()) {
                auto compileResults = CalChart::Compile(variablesStates, errors, curr_sheet, show.GetSheetEnd(), j, MAX_NUM_SYMBOLS, {});
                theCommands[j] = compileResults.first;
                theCommands2[j] = compileResults.second;
            }
        }
        if (errors.AnyErrors()) {
            mAnimationErrors[std::distance(show.GetSheetBegin(), curr_sheet)] = errors;
        }
        std::vector<Coord> thePoints(points.size());
        for (unsigned i = 0; i < points.size(); i++) {
            thePoints.at(i) = curr_sheet->GetPosition(i);
        }
        sheets.emplace_back(curr_sheet->GetName(), curr_sheet->GetBeats(), theCommands2);
        // here's where we would put in another sheet, and compare to see if all the positions are the same,
        // if it makes the same json movements.
    }

    mSheets = Animate::Sheets{ sheets };
}

Animation::~Animation() = default;

auto Animation::BeatHasCollision(beats_t whichBeat) const -> bool
{
    return mSheets.BeatHasCollision(whichBeat);
}

auto Animation::GetAnimateInfo(beats_t whichBeat, int which) const -> Animate::Info
{
    return mSheets.AnimateInfoAtBeat(whichBeat, which);
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
    return mSheets.AllAnimateInfoAtBeat(whichBeat);
}

int Animation::GetNumberSheets() const { return static_cast<int>(mSheets.TotalSheets()); }

auto Animation::GetTotalNumberBeatsUpTo(int sheet) const -> beats_t
{
    return mSheets.GetTotalNumberBeatsUpTo(sheet);
}

auto Animation::GenPathToDraw(unsigned whichSheet, unsigned point, Coord::units endRadius) const -> std::vector<Draw::DrawCommand>
{
    return mSheets.GeneratePathToDraw(whichSheet, point, endRadius);
}

auto Animation::GetCurrentInfo(beats_t whichBeat) const -> std::pair<std::string, std::vector<std::string>>
{
    return mSheets.DebugAnimateInfoAtBeat(whichBeat);
}

std::vector<AnimationErrors> Animation::GetAnimationErrors() const
{
    return mAnimationErrors;
}

auto Animation::toOnlineViewerJSON() const -> std::vector<std::vector<std::vector<nlohmann::json>>>
{
    return mSheets.toOnlineViewerJSON();
}

auto Animation::GetAnimationCollisions() const -> std::map<int, CalChart::SelectionList>
{
    return mSheets.SheetsToMarchersWhoCollided();
}

}
