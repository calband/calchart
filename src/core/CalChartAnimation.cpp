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
#include "CalChartAnimationSheet.h"
#include "CalChartContinuity.h"
#include "CalChartMeasure.h"
#include "CalChartRanges.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include <optional>
#include <ranges>

auto gAnimateMeasure = CalChart::MeasureDuration{ "AnimateShow" };

namespace CalChart::Animate {

auto AnimateShow(const Show& show) -> Sheets
{
    auto snapshot = gAnimateMeasure.doMeasurement();

    // the variables are persistant through the entire compile process.
    Variables variablesStates;

    auto runningIndex = CalChart::Ranges::ToVector<unsigned>(show.AreSheetsInAnimation());
    std::exclusive_scan(runningIndex.begin(), runningIndex.end(), runningIndex.begin(), 0);

    // First, construct pairs of sheets, the animation start and end.  We use optional here as a
    // sentinel -- meaning if it is null we know we have the last sheet, which we treat specially.
    // Then, from the pairs construct AnimationData, which has all the information needed for the compile step.
    // Then CreateCompileResult with the AnimationData and Variables.  Collect the whole thing into commands,
    // and, viola, we have the compiled show.

    return Animate::Sheets{
        CalChart::Ranges::ToVector<Animate::Sheet>(
            CalChart::Ranges::adjacent_view<2>([](auto&& show) {
                // CalChart::Ranges don't work well on lvalues, so having a closure that returns what we need here.
                auto animationSheetsWithSentinel = CalChart::Ranges::ToVector<std::optional<CalChart::Sheet>>(show.SheetsInAnimation());
                animationSheetsWithSentinel.push_back(std::nullopt);
                return animationSheetsWithSentinel;
            }(show))
            | std::views::transform([&](auto&& curr_next) {
                  auto [curr_sheet, nextAnimationSheet] = curr_next;
                  auto numBeats = curr_sheet->GetBeats();
                  auto isLastSheet = !nextAnimationSheet.has_value();
                  auto theCommands = CalChart::Ranges::ToVector<Animate::CompileResult>(
                      std::ranges::iota_view(0, show.GetNumPoints()) | std::views::transform([=, curr_sheet = curr_sheet, nextAnimationSheet = nextAnimationSheet](auto whichMarcher) {
                          auto current_symbol = curr_sheet->GetSymbol(whichMarcher);
                          auto endPosition = [whichMarcher](auto&& nextAnimationSheet) -> std::optional<Coord> {
                              if (nextAnimationSheet) {
                                  return nextAnimationSheet->GetPosition(whichMarcher);
                              }
                              return std::nullopt;
                          }(nextAnimationSheet);
                          auto const& cont = curr_sheet->GetContinuityBySymbol(current_symbol).GetParsedContinuity();
                          auto empty_cont = std::vector<std::unique_ptr<Cont::Procedure>>{};
                          return AnimationData{
                              static_cast<unsigned>(whichMarcher),
                              curr_sheet->GetPoint(whichMarcher),
                              curr_sheet->ContinuityInUse(current_symbol) ? cont : empty_cont,
                              endPosition,
                              numBeats,
                              isLastSheet
                          };
                      })
                      | std::views::transform([&](auto animationData) {
                            return CalChart::Animate::CreateCompileResult(
                                animationData,
                                variablesStates);
                        }));

                  return Animate::Sheet{
                      curr_sheet->GetName(), numBeats, theCommands
                  };
              })),
        runningIndex
    };
}
}

namespace CalChart {
Animation::Animation(const Show& show)
    : mSheets{ Animate::AnimateShow(show) }
{
}

}
