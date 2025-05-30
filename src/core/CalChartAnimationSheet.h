#pragma once
/*
 * CalChartAnimation.h
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

#include "CalChartAnimationCommand.h"
#include "CalChartAnimationErrors.h"
#include "CalChartAnimationTypes.h"
#include "CalChartCoord.h"
#include "CalChartPoint.h"
#include "CalChartRanges.h"

#include <nlohmann/json.hpp>
#include <ranges>
#include <set>
#include <string>
#include <vector>

namespace CalChart::Animate {

using CompileResult = std::pair<std::vector<Command>, ErrorsEncountered>;

struct Info {
    int mIndex;
    CalChart::Coord::CollisionType mCollision = CalChart::Coord::CollisionType::none;
    MarcherInfo mMarcherInfo{};
};

// A sheet is a collection of all the Marcher's Commands.
// The Commands are the positions, directions, and style of each marcher at their beats.
// Because a Sheet sees all the points and where they are, the sheet can calculate all the
// collisions that exist.
class Sheet {
public:
    Sheet(std::string name, unsigned numBeats, std::vector<CompileResult> const& commands);

    [[nodiscard]] auto GetName() const { return mName; }
    [[nodiscard]] auto GetNumBeats() const { return mNumBeats; }

    [[nodiscard]] auto MarcherInfoAtBeat(size_t whichMarcher, Beats beat) const -> MarcherInfo;

    [[nodiscard]] auto AllMarcherInfoAtBeat(Beats beat) const
    {
        return std::views::iota(0UL, mCommands.size()) | std::views::transform([this, beat](auto whichMarcher) {
            return MarcherInfoAtBeat(whichMarcher, beat);
        });
    }

    [[nodiscard]] auto GetAllBeatsWithCollisions() const -> std::set<Beats>;
    [[nodiscard]] auto GetAllMarchersWithCollisionAtBeat(Beats beat) const -> CalChart::SelectionList;
    [[nodiscard]] auto GetAllMarchersWithCollisions() const -> CalChart::SelectionList
    {
        return std::accumulate(mCollisions.begin(), mCollisions.end(), CalChart::SelectionList{}, [](auto acc, auto item) {
            acc.insert(std::get<0>(std::get<0>(item)));
            return acc;
        });
    }
    [[nodiscard]] auto CollisionAtBeat(size_t whichMarcher, Beats beat) const -> Coord::CollisionType;
    [[nodiscard]] auto GeneratePathToDraw(int whichMarcher, Coord::units endRadius) const -> std::vector<Draw::DrawCommand>
    {
        return mCommands.at(whichMarcher).GeneratePathToDraw(endRadius);
    }

    [[nodiscard]] auto toOnlineViewerJSON(int whichMarcher) const -> std::vector<nlohmann::json>
    {
        return mCommands.at(whichMarcher).toOnlineViewerJSON();
    }

    [[nodiscard]] auto toOnlineViewerJSON() const -> std::vector<std::vector<nlohmann::json>>;
    [[nodiscard]] auto DebugAnimateInfoAtBeat(Beats beat, bool ignoreCollision) const -> std::vector<std::string>;

    [[nodiscard]] auto AllAnimateInfoAtBeat(Beats beat) const -> std::vector<Info>;

    [[nodiscard]] auto GetAnimationErrors() const
    {
        return mErrors;
    }

private:
    [[nodiscard]] auto FindAllCollisions() const -> std::map<std::tuple<size_t, Beats>, Coord::CollisionType>;
    std::string mName;
    Beats mNumBeats;
    std::vector<Commands> mCommands;
    std::map<std::tuple<size_t, Beats>, Coord::CollisionType> mCollisions;
    Errors mErrors;
};

class Sheets {
public:
    explicit Sheets(std::vector<Sheet> const& sheets, std::vector<unsigned> const& showSheetToAnimationSheet = {});
    [[nodiscard]] auto TotalSheets() const -> size_t { return mSheets.size(); }
    [[nodiscard]] auto TotalBeats() const -> Beats;
    [[nodiscard]] auto BeatToSheetOffsetAndBeat(Beats beat) const -> std::tuple<size_t, Beats>;
    [[nodiscard]] auto GetTotalNumberBeatsUpTo(int whichSheet) const -> Beats
    {
        if (whichSheet == 0) {
            return 0;
        }
        return mRunningBeatCount.at(whichSheet - 1);
    }
    [[nodiscard]] auto BeatForSheet(int whichSheet) const -> Beats { return mSheets.at(whichSheet).GetNumBeats(); }
    [[nodiscard]] auto MarcherInfoAtBeat(Beats beat, int whichMarcher) const -> MarcherInfo;
    [[nodiscard]] auto CollisionAtBeat(Beats beat, int whichMarcher) const -> Coord::CollisionType;
    [[nodiscard]] auto AnimateInfoAtBeat(Beats beat, int whichMarcher) const -> Info
    {
        return {
            whichMarcher,
            CollisionAtBeat(beat, whichMarcher),
            MarcherInfoAtBeat(beat, whichMarcher)
        };
    }

    [[nodiscard]] auto GetAnimationErrors() const -> std::vector<Errors>
    {
        return Ranges::ToVector<Errors>(
            mSheets | std::views::transform([](auto&& sheet) {
                return sheet.GetAnimationErrors();
            }));
    }

    [[nodiscard]] auto AllAnimateInfoAtBeat(Beats whichBeat) const -> std::vector<Info>;

    [[nodiscard]] auto DebugAnimateInfoAtBeat(Beats beat) const -> std::pair<std::string, std::vector<std::string>>;

    [[nodiscard]] auto BeatHasCollision(Beats whichBeat) const -> bool;
    [[nodiscard]] auto GetSheetName(int whichSheet) const { return mSheets.at(whichSheet).GetName(); }
    // Sheet -> selection of marchers who collided
    [[nodiscard]] auto SheetsToMarchersWhoCollided() const -> std::map<int, CalChart::SelectionList>
    {
        auto result = std::map<int, CalChart::SelectionList>{};
        for (auto whichSheet : std::views::iota(0UL, mSheets.size())) {
            auto marchersWithCollisions = mSheets[whichSheet].GetAllMarchersWithCollisions();
            if (marchersWithCollisions.size()) {
                result[whichSheet] = marchersWithCollisions;
            }
        }
        return result;
    }

    [[nodiscard]] auto GeneratePathToDraw(int whichSheet, int whichMarcher, Coord::units endRadius) const -> std::vector<Draw::DrawCommand>
    {
        return mSheets.at(whichSheet).GeneratePathToDraw(whichMarcher, endRadius);
    }

    [[nodiscard]] auto toOnlineViewerJSON() const -> std::vector<std::vector<std::vector<nlohmann::json>>>;

    [[nodiscard]] auto ShowSheetToAnimSheetTranslate(unsigned sheet) const { return mShowSheetToAnimationSheet.at(sheet); }

private:
    std::vector<Sheet> mSheets;
    std::vector<Beats> mRunningBeatCount;
    std::vector<unsigned> mShowSheetToAnimationSheet;
};

}
