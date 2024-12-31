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
#include "CalChartConfiguration.h"
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
                      std::ranges::iota_view(0, show.GetNumPoints()) | std::views::transform([&variablesStates, numBeats, isLastSheet, curr_sheet = curr_sheet, nextAnimationSheet = nextAnimationSheet](auto whichMarcher) {
                          auto current_symbol = curr_sheet->GetSymbol(whichMarcher);
                          auto endPosition = [whichMarcher](auto&& nextAnimationSheet) -> std::optional<Coord> {
                              if (nextAnimationSheet) {
                                  return nextAnimationSheet->GetPosition(whichMarcher);
                              }
                              return std::nullopt;
                          }(nextAnimationSheet);
                          auto const& cont = curr_sheet->GetContinuityBySymbol(current_symbol).GetParsedContinuity();
                          auto empty_cont = std::vector<std::unique_ptr<Cont::Procedure>>{};
                          return CalChart::Animate::CreateCompileResult(
                              AnimationData{
                                  static_cast<unsigned>(whichMarcher),
                                  curr_sheet->GetPoint(whichMarcher),
                                  endPosition,
                                  numBeats,
                                  isLastSheet },
                              curr_sheet->ContinuityInUse(current_symbol) ? cont : empty_cont,
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

namespace {

template <std::ranges::input_range Range>
    requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Info>)
auto GeneratePointDrawCommand(Range&& infos) -> std::vector<CalChart::Draw::DrawCommand>
{
    return CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(infos | std::views::transform([](auto&& info) {
        auto size = CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(1) };
        return CalChart::Draw::Rectangle{
            info.mMarcherInfo.mPosition - size / 2, { CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(1) }
        };
    }));
}

template <std::ranges::input_range Range, typename Function>
    requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Info>)
auto GeneratePointDrawCommand(Range&& range, Function predicate, CalChart::BrushAndPen brushAndPen) -> CalChart::Draw::DrawCommand
{
    auto filteredRange = range | std::views::filter(predicate);
    if (!filteredRange.empty()) {
        return CalChart::Draw::withBrushAndPen(brushAndPen, GeneratePointDrawCommand(filteredRange));
    }
    return CalChart::Draw::Ignore{};
}

template <std::ranges::input_range Range>
    requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Info>)
auto AddDistanceFromPointInfo(Range&& range, CalChart::Coord origin) -> std::multimap<double, CalChart::Animate::Info>
{
    return std::accumulate(range.begin(), range.end(), std::multimap<double, CalChart::Animate::Info>{}, [origin](auto&& acc, auto&& item) {
        acc.insert({ origin.Distance(item.mMarcherInfo.mPosition.x), item });
        return acc;
    });
}

}

namespace CalChart {
Animation::Animation(const Show& show)
    : mSheets{ Animate::AnimateShow(show) }
{
}

auto Animation::GetBoundingBox(beats_t whichBeat) const -> std::pair<CalChart::Coord, CalChart::Coord>
{
    auto allPositions = GetAllAnimateInfo(whichBeat) | std::views::transform([](auto&& info) { return info.mMarcherInfo.mPosition; });
    return {
        std::accumulate(allPositions.begin(), allPositions.end(), CalChart::Coord{}, [](auto&& acc, auto&& pos) {
            return CalChart::Coord(std::min(acc.x, pos.x), std::min(acc.y, pos.y));
        }),
        std::accumulate(allPositions.begin(), allPositions.end(), CalChart::Coord{}, [](auto&& acc, auto&& pos) {
            return CalChart::Coord(std::max(acc.x, pos.x), std::max(acc.y, pos.y));
        })
    };
}

auto Animation::GetAnimateInfoWithDistanceFromPoint(beats_t whichBeat, CalChart::Coord origin) const -> std::multimap<double, Animate::Info>
{
    return AddDistanceFromPointInfo(GetAllAnimateInfo(whichBeat), origin);
}

auto Animation::GetAnimateInfoWithDistanceFromPoint(beats_t whichBeat, SelectionList const& selectionList, CalChart::Coord origin) const -> std::multimap<double, Animate::Info>
{
    auto allEnumeratedInfo = CalChart::Ranges::enumerate_view(GetAllAnimateInfo(whichBeat));
    auto allSelected = allEnumeratedInfo
        | std::views::filter([&selectionList](auto&& info) { return selectionList.contains(std::get<0>(info)); })
        | std::views::transform([](auto&& info) { return std::get<1>(info); });

    return std::accumulate(allSelected.begin(), allSelected.end(), std::multimap<double, Animate::Info>{}, [origin](auto&& acc, auto&& item) {
        acc.insert({ origin.Distance(item.mMarcherInfo.mPosition.x), item });
        return acc;
    });
}

auto Animation::GenerateDotsDrawCommands(beats_t whichBeat, SelectionList const& selectionList, bool drawCollisionWarning, CalChart::Configuration const& config) const -> std::vector<CalChart::Draw::DrawCommand>
{
    auto allEnumeratedInfo = CalChart::Ranges::enumerate_view(GetAllAnimateInfo(whichBeat));
    auto allInfo = allEnumeratedInfo
        | std::views::transform([](auto&& info) { return std::get<1>(info); });
    auto allSelected = allEnumeratedInfo
        | std::views::filter([&selectionList](auto&& info) { return selectionList.contains(std::get<0>(info)); })
        | std::views::transform([](auto&& info) { return std::get<1>(info); });
    auto allNotSelected = allEnumeratedInfo
        | std::views::filter([&selectionList](auto&& info) { return !selectionList.contains(std::get<0>(info)); })
        | std::views::transform([](auto&& info) { return std::get<1>(info); });

    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allNotSelected, [](auto&& info) { return FacingBack(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_BACK)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allNotSelected, [](auto&& info) { return FacingFront(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_FRONT)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allNotSelected, [](auto&& info) { return FacingSide(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_SIDE)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allSelected, [](auto&& info) { return FacingBack(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_HILIT_BACK)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allSelected, [](auto&& info) { return FacingFront(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_HILIT_FRONT)));
    CalChart::append(drawCmds,
        GeneratePointDrawCommand(
            allSelected, [](auto&& info) { return FacingSide(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_HILIT_SIDE)));

    if (drawCollisionWarning) {
        CalChart::append(drawCmds,
            GeneratePointDrawCommand(
                allInfo, [](auto&& info) { return CollisionWarning(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_COLLISION_WARNING)));
        CalChart::append(drawCmds,
            GeneratePointDrawCommand(
                allInfo, [](auto&& info) { return CollisionIntersect(info); }, config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_ANIM_COLLISION)));
    }
    return drawCmds;
}

auto Animation::GenerateSpritesDrawCommands(beats_t whichBeat, SelectionList const& selectionList, AngleStepToImageFunction imageFunction, std::optional<bool> onBeat, Configuration const& config) const -> std::vector<CalChart::Draw::DrawCommand>
{
    constexpr auto comp_X = 0.5;
    auto comp_Y = config.Get_SpriteBitmapOffsetY();

    auto drawCmds = CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(
        CalChart::Ranges::enumerate_view(GetAllAnimateInfo(whichBeat)) | std::views::transform([comp_Y, &selectionList, onBeat, imageFunction](auto&& enum_info) {
            auto&& [index, info] = enum_info;
            auto image_offset = [&]() -> ImageBeat {
                if (info.mMarcherInfo.mStepStyle == CalChart::MarchingStyle::Close) {
                    return ImageBeat::Standing;
                }
                if (!onBeat.has_value()) {
                    return ImageBeat::Standing;
                }
                return *onBeat ? ImageBeat::Left : ImageBeat::Right;
            }();
            auto image = imageFunction(info.mMarcherInfo.mFacingDirection, image_offset);
            auto position = info.mMarcherInfo.mPosition;
            auto offset = CalChart::Coord(image->image_width * comp_X, image->image_height * comp_Y);

            return CalChart::Draw::Image{ position, image, selectionList.contains(index) } - offset;
        }));
    return drawCmds;
}

auto Animation::GenerateDrawCommands(
    beats_t whichBeat,
    SelectionList const& selectionList,
    ShowMode const& showMode,
    Configuration const& config,
    bool drawCollisionWarning,
    std::optional<bool> onBeat,
    AngleStepToImageFunction imageFunction) const -> std::vector<CalChart::Draw::DrawCommand>
{
    auto drawCmds = CalChart::CreateModeDrawCommandsWithBorderOffset(config, showMode, CalChart::HowToDraw::Animation);
    auto useSprites = config.Get_UseSprites();
    if (useSprites) {
        CalChart::append(drawCmds,
            GenerateSpritesDrawCommands(
                whichBeat,
                selectionList,
                imageFunction,
                onBeat,
                config));
    } else {
        CalChart::append(drawCmds,
            GenerateDotsDrawCommands(
                whichBeat,
                selectionList,
                drawCollisionWarning,
                config));
    }
    return drawCmds + showMode.Offset();
}

}
