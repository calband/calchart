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
#include "CalChartSheet.h"
#include "CalChartShow.h"

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
        std::vector<std::vector<Animate::Command>> theCommands(points.size());
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
                        theCommands[j] = CalChart::Compile(variablesStates, errors, curr_sheet, show.GetSheetEnd(), j, current_symbol, continuity);
                    }
                }
            }
        }
        // Handle points that don't have continuity (shouldn't happen)
        for (unsigned j = 0; j < points.size(); j++) {
            if (theCommands[j].empty()) {
                theCommands[j] = CalChart::Compile(variablesStates, errors, curr_sheet, show.GetSheetEnd(), j, MAX_NUM_SYMBOLS, {});
            }
        }
        if (errors.AnyErrors()) {
            mAnimationErrors[std::distance(show.GetSheetBegin(), curr_sheet)] = errors;
        }
        std::vector<Coord> thePoints(points.size());
        for (unsigned i = 0; i < points.size(); i++) {
            thePoints.at(i) = curr_sheet->GetPosition(i);
        }
        sheets.emplace_back(curr_sheet->GetName(), curr_sheet->GetBeats(), theCommands);
    }

    mSheets = Animate::Sheets{ sheets };
}

}
