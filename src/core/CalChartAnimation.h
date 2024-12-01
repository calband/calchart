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

#include "CalChartAngles.h"
#include "CalChartAnimationErrors.h"
#include "CalChartAnimationSheet.h"
#include "CalChartAnimationTypes.h"
#include "CalChartCoord.h"
#include "CalChartDrawCommand.h"

#include <map>
#include <vector>

namespace CalChart {

class Configuration;
class Show;
class ShowMode;

namespace Animate {
    inline auto FacingBack(Info const& info)
    {
        auto direction = CalChart::AngleToDirection(info.mMarcherInfo.mFacingDirection);
        return direction == CalChart::Direction::SouthWest
            || direction == CalChart::Direction::West
            || direction == CalChart::Direction::NorthWest;
    }

    inline auto FacingFront(Info const& info)
    {
        auto direction = CalChart::AngleToDirection(info.mMarcherInfo.mFacingDirection);
        return direction == CalChart::Direction::SouthEast
            || direction == CalChart::Direction::East
            || direction == CalChart::Direction::NorthEast;
    }

    inline auto FacingSide(Info const& info)
    {
        return !FacingBack(info) && !FacingFront(info);
    }

    inline auto CollisionWarning(Info const& info)
    {
        return info.mCollision == CalChart::Coord::CollisionType::warning;
    }

    inline auto CollisionIntersect(Info const& info)
    {
        return info.mCollision == CalChart::Coord::CollisionType::intersect;
    }

}

using beats_t = unsigned;

class Animation {
public:
    explicit Animation(const Show& show);

    [[nodiscard]] auto GetAnimateInfo(beats_t whichBeat, int which) const -> Animate::Info { return mSheets.AnimateInfoAtBeat(whichBeat, which); }
    [[nodiscard]] auto GetAllAnimateInfo(beats_t whichBeat) const -> std::vector<Animate::Info> { return mSheets.AllAnimateInfoAtBeat(whichBeat); }

    [[nodiscard]] auto GetAnimateInfoWithDistanceFromPoint(beats_t whichBeat, CalChart::Coord origin) const -> std::multimap<double, Animate::Info>;
    [[nodiscard]] auto GetAnimateInfoWithDistanceFromPoint(beats_t whichBeat, SelectionList const& selectionList, CalChart::Coord origin) const -> std::multimap<double, Animate::Info>;

    [[nodiscard]] auto GetNumberSheets() const { return mSheets.TotalSheets(); }
    [[nodiscard]] auto GetTotalNumberBeatsUpTo(int sheet) const -> beats_t { return mSheets.GetTotalNumberBeatsUpTo(sheet); }
    [[nodiscard]] auto GetTotalNumberBeats() const { return mSheets.TotalBeats(); }

    [[nodiscard]] auto GetErrors() const { return mSheets.GetAnimationErrors(); }

    // Sheet -> selection of marchers who collided
    [[nodiscard]] auto GetCollisions() const -> std::map<int, CalChart::SelectionList> { return mSheets.SheetsToMarchersWhoCollided(); }
    [[nodiscard]] auto BeatHasCollision(beats_t whichBeat) const { return mSheets.BeatHasCollision(whichBeat); }

    // bounds that encompasses all marchers
    [[nodiscard]] auto GetBoundingBox(beats_t whichBeat) const -> std::pair<CalChart::Coord, CalChart::Coord>;

    // collection of position of each point, for debugging purposes
    [[nodiscard]] auto GetCurrentInfo(beats_t whichBeat) const -> std::pair<std::string, std::vector<std::string>> { return mSheets.DebugAnimateInfoAtBeat(whichBeat); }

    [[nodiscard]] auto GenPathToDraw(unsigned whichSheet, unsigned point, Coord::units endRadius) const { return mSheets.GeneratePathToDraw(whichSheet, point, endRadius); }

    [[nodiscard]] auto ShowSheetToAnimSheetTranslate(unsigned showSheet) const { return mSheets.ShowSheetToAnimSheetTranslate(showSheet); }
    [[nodiscard]] auto GetBeatForShowSheet(unsigned showSheet) const { return GetTotalNumberBeatsUpTo(ShowSheetToAnimSheetTranslate(showSheet)); }

    /*!
     * @brief Generates JSON that could represent of all the marchers in an Online Viewer '.viewer' file.
     * @param pointsOverSheets All of the points in all of the sheets.
     * @return A JSON which could represent all the animations in a '.viewer' file.
     */
    [[nodiscard]] auto toOnlineViewerJSON() const { return mSheets.toOnlineViewerJSON(); }

    enum class ImageBeat {
        Standing,
        Left,
        Right,
        Size
    };
    using AngleStepToImageFunction = std::function<std::shared_ptr<ImageData>(Radian, ImageBeat)>;

    // Drawing commands
    [[nodiscard]] auto GenerateDotsDrawCommands(beats_t whichBeat, SelectionList const& selectionList, bool drawCollisionWarning, Configuration const& config) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateSpritesDrawCommands(beats_t whichBeat, SelectionList const& selectionList, AngleStepToImageFunction imageFunction, std::optional<bool> onBeat, Configuration const& config) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateDrawCommands(
        beats_t whichBeat,
        SelectionList const& selectionList,
        ShowMode const& showMode,
        Configuration const& config,
        bool drawCollisionWarning,
        std::optional<bool> onBeat,
        AngleStepToImageFunction imageFunction) const -> std::vector<CalChart::Draw::DrawCommand>;

private:
    // There are two types of data, the ones that are set when we are created, and the ones that modify over time.
    Animate::Sheets mSheets;
};

}
