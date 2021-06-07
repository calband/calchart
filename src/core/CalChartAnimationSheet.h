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

#include "CalChartAnimationTypes.h"
#include "CalChartCoord.h"

#include <string>
#include <vector>

/**
 * Animation Sheet
 * Continuities can be broken down into 3 distinct types:
 *  MarkTime: A direction to be facing
 *  Moving: A vector along which to be moving (indicating how far to move each point)
 *  Rotate: A point which to rotate, radius, start and end angles
 * AnimationCommand is an object that represents a particular part of a continuity.  When we decompose
 * continuities into these parts, we can then "transform" a point from a starting position to the end of the
 * Animation by "stepping" it along each AnimationCommand
 */

namespace CalChart {

class AnimationCommand;

using AnimationCommands = std::vector<std::shared_ptr<AnimationCommand>>;

// AnimationSheet is a snapshot of CalChartSheet
class AnimationSheet {
public:
    AnimationSheet(std::vector<Coord> const& thePoints, std::vector<AnimationCommands> const& theCommands, std::string const& s, unsigned beats)
        : mPoints(thePoints)
        , mCommands(theCommands)
        , name(s)
        , numbeats(beats)
    {
    }

    // make things copiable
    AnimationSheet(AnimationSheet const&);
    AnimationSheet& operator=(AnimationSheet);
    AnimationSheet(AnimationSheet&&) noexcept;
    AnimationSheet& operator=(AnimationSheet&&) noexcept;
    void swap(AnimationSheet&) noexcept;

    auto GetName() const { return name; }
    auto GetNumBeats() const { return numbeats; }
    auto GetPoints() const { return mPoints; }
    auto GetCommands(int which) const { return mCommands.at(which); }
    auto GetCommandsBegin(int which) const
    {
        return mCommands.at(which).begin();
    }
    auto GetCommandsBeginIndex(int /*which*/) const
    {
        return std::vector<AnimationCommands>::size_type(0);
    }
    auto GetCommandsEnd(int which) const
    {
        return mCommands.at(which).end();
    }
    auto GetCommandsEndIndex(int which) const
    {
        return mCommands.at(which).size();
    }
    auto GetCommandsAt(int which, int index) const
    {
        return mCommands.at(which).at(index);
    }

private:
    std::vector<Coord> mPoints; // should probably be const
    std::vector<AnimationCommands> mCommands;
    std::string name;
    unsigned numbeats;
};

}
