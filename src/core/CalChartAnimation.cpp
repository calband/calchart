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
#include "CalChartMeasure.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"

auto gAnimateMeasure = CalChart::MeasureDuration{ "AnimateShow" };
namespace CalChart::Animate {

namespace {
    auto getEndPosition(
        Show::const_Sheet_iterator_t c_sheet,
        Show::const_Sheet_iterator_t endSheet,
        unsigned whichMarcher) -> std::optional<Coord>
    {
        // find the first animation sheet.
        auto nextAnimationSheet = std::find_if(c_sheet + 1, endSheet, [](auto& sheet) { return sheet.IsInAnimation(); });
        auto isLastAnimationSheet = nextAnimationSheet == endSheet;
        // End position is the position of the next valid animation sheet.
        return
            [=]() -> std::optional<Coord> {
                if (isLastAnimationSheet) {
                    return std::optional<Coord>{};
                }
                return nextAnimationSheet->GetPosition(whichMarcher);
            }();
    }
}

auto AnimateShow(const Show& show) -> std::tuple<Sheets, std::vector<Errors>>
{
    auto snapshot = gAnimateMeasure.doMeasurement();
    auto animationErrors = std::vector<Animate::Errors>(show.GetNumSheets());

    auto points = std::vector<Coord>(show.GetNumPoints());
    // the variables are persistant through the entire compile process.
    Variables variablesStates;

    auto sheets = std::vector<Animate::Sheet>{};
    auto endSheet = show.GetSheetEnd();

    auto runningIndex = std::vector<unsigned>{};
    for (auto curr_sheet = show.GetSheetBegin(); curr_sheet != endSheet; ++curr_sheet) {
        runningIndex.push_back(curr_sheet->IsInAnimation() ? 1 : 0);
    }
    std::exclusive_scan(runningIndex.begin(), runningIndex.end(), runningIndex.begin(), 0);

    for (auto curr_sheet = show.GetSheetBegin(); curr_sheet != endSheet; ++curr_sheet) {

        if (!curr_sheet->IsInAnimation()) {
            continue;
        }

        // find the first animation sheet.
        auto nextAnimationSheet = std::find_if(curr_sheet + 1, endSheet, [](auto& sheet) { return sheet.IsInAnimation(); });
        auto isLastAnimationSheet = nextAnimationSheet == endSheet;
        auto numBeats = curr_sheet->GetBeats();

        // Now parse continuity
        Errors errors;
        std::vector<std::vector<Animate::Command>> theCommands(points.size());
        for (auto current_symbol : k_symbols) {
            if (curr_sheet->ContinuityInUse(current_symbol)) {
                auto const& current_continuity = curr_sheet->GetContinuityBySymbol(current_symbol);
                auto const& continuity = current_continuity.GetParsedContinuity();
#if 0 // enable to see dump of continuity
                {
                    for (auto& proc : continuity) {
                        std::cout << *proc << "\n";
                    }
                }
#endif
                for (unsigned j = 0; j < points.size(); j++) {
                    if (curr_sheet->GetSymbol(j) == current_symbol) {
                        auto endPosition = getEndPosition(curr_sheet, endSheet, j);
                        theCommands[j] = CreateCompileResult(
                            variablesStates,
                            errors,
                            j,
                            curr_sheet->GetPoint(j),
                            numBeats,
                            isLastAnimationSheet,
                            endPosition,
                            continuity);
                    }
                }
            }
        }
        // Handle points that don't have continuity (shouldn't happen)
        for (unsigned j = 0; j < points.size(); j++) {
            if (theCommands[j].empty()) {
                auto endPosition = getEndPosition(curr_sheet, endSheet, j);

                theCommands[j] = CreateCompileResult(
                    variablesStates,
                    errors,
                    j,
                    curr_sheet->GetPoint(j),
                    numBeats,
                    isLastAnimationSheet,
                    endPosition,
                    {});
            }
        }
        if (AnyErrors(errors)) {
            animationErrors[std::distance(show.GetSheetBegin(), curr_sheet)] = errors;
        }
        sheets.emplace_back(curr_sheet->GetName(), numBeats, theCommands);
    }

    return { Animate::Sheets{ sheets, runningIndex }, animationErrors };
}
}

namespace CalChart {
Animation::Animation(const Show& show)
    : mSheets({})
    , mErrors(show.GetNumSheets())
{
    std::tie(mSheets, mErrors) = Animate::AnimateShow(show);
}

}
