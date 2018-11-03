/*
 * animate.cpp
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

#include "animate.h"
#include "animatecommand.h"
#include "cc_continuity.h"
#include "cc_drawcommand.h"
#include "cc_point.h"
#include "cc_sheet.h"
#include "cc_show.h"
#include "cont.h"
#include "math_utils.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

extern int parsecontinuity();
extern const char* yyinputbuffer;
extern std::list<std::unique_ptr<CalChart::ContProcedure>> ParsedContinuity;

namespace CalChart {

AnimateDir AnimGetDirFromAngle(float ang)
{
    ang = NormalizeAngle(ang);
    // rotate angle by 22.5:
    ang += 22.5;
    size_t quadrant = ang / 45.0;
    switch (quadrant) {
    case 0:
        return ANIMDIR_N;
    case 1:
        return ANIMDIR_NW;
    case 2:
        return ANIMDIR_W;
    case 3:
        return ANIMDIR_SW;
    case 4:
        return ANIMDIR_S;
    case 5:
        return ANIMDIR_SE;
    case 6:
        return ANIMDIR_E;
    case 7:
        return ANIMDIR_NE;
    case 8:
        return ANIMDIR_N;
    }
    return ANIMDIR_N;
}

std::list<std::unique_ptr<ContProcedure>>
Animation::ParseContinuity(std::string const& continuity, AnimationErrors& errors, SYMBOL_TYPE current_symbol)
{
    yyinputbuffer = continuity.c_str();
    // parse out the error
    if (parsecontinuity() != 0) {
        // Supply a generic parse error
        ContToken dummy;
        errors.RegisterError(ANIMERR_SYNTAX, &dummy, 0, current_symbol);
    }
    return std::move(ParsedContinuity);
}

Animation::Animation(const Show& show, NotifyStatus notifyStatus, NotifyErrorList notifyErrorList)
    : pts(show.GetNumPoints())
    , curr_cmds(pts.size())
    , curr_sheetnum(0)
    , mCollisionAction(NULL)
{
    // the variables are persistant through the entire compile process.
    AnimationVariables variablesStates;

    int newSheetIndex = 0;
    int prevSheetIndex = 0;
    for (auto curr_sheet = show.GetSheetBegin(); curr_sheet != show.GetSheetEnd(); ++curr_sheet) {
        
        if (!curr_sheet->IsInAnimation())
        {
            mAnimSheetIndices.push_back(prevSheetIndex);
            continue;
        }

        mAnimSheetIndices.push_back(newSheetIndex);
        prevSheetIndex = newSheetIndex;
        newSheetIndex++;

        // Now parse continuity
        AnimationErrors errors;
        std::vector<AnimateCommands> theCommands(pts.size());
        for (auto& current_symbol : k_symbols) {
            if (curr_sheet->ContinuityInUse(current_symbol)) {
                auto& current_continuity = curr_sheet->GetContinuityBySymbol(current_symbol);
                auto continuity = ParseContinuity(current_continuity.GetText(), errors, current_symbol);
                if (notifyStatus) {
                    std::string message("Compiling \"");
                    message += curr_sheet->GetName().substr(0, 32);
                    message += ("\" ");
                    message += GetNameForSymbol(current_symbol).substr(0, 32);
                    message += ("...");
                    notifyStatus(message);
                }
#if 0 // enable to see dump of continuity
                {
                    for (auto& proc : continuity) {
                        std::cout << *proc << "\n";
                    }
                }
#endif
                for (unsigned j = 0; j < pts.size(); j++) {
                    if (curr_sheet->GetPoint(j).GetSymbol() == current_symbol) {
                        theCommands[j] = AnimateCompile::Compile(show, variablesStates, errors, curr_sheet, j, current_symbol, continuity);
                    }
                }
            }
        }
        // Handle points that don't have continuity (shouldn't happen)
        if (notifyStatus) {
            std::string message("Compiling \"");
            message += curr_sheet->GetName().substr(0, 32);
            message += ("\"...");
            notifyStatus(message);
        }
        for (unsigned j = 0; j < pts.size(); j++) {
            if (theCommands[j].empty()) {
                theCommands[j] = AnimateCompile::Compile(show, variablesStates, errors, curr_sheet, j, MAX_NUM_SYMBOLS, {});
            }
        }
        if (errors.AnyErrors() && notifyErrorList) {
            std::string message("Errors for \"");
            message += curr_sheet->GetName().substr(0, 32);
            message += ("\"");
            if (notifyErrorList(errors.GetErrors(), std::distance(show.GetSheetBegin(), curr_sheet), message)) {
                break;
            }
        }
        std::vector<AnimatePoint> thePoints(pts.size());
        for (unsigned i = 0; i < pts.size(); i++) {
            thePoints.at(i) = curr_sheet->GetPosition(i);
        }
        sheets.emplace_back(thePoints, theCommands, curr_sheet->GetName(), curr_sheet->GetBeats());
    }
}

Animation::~Animation() {}

bool Animation::PrevSheet()
{
    if (curr_beat == 0) {
        if (curr_sheetnum > 0) {
            curr_sheetnum--;
        }
    }
    RefreshSheet();
    CheckCollisions();
    return true;
}

bool Animation::NextSheet()
{
    if ((curr_sheetnum + 1) != sheets.size()) {
        curr_sheetnum++;
        RefreshSheet();
        CheckCollisions();
    } else {
        if (curr_beat >= sheets.at(curr_sheetnum).GetNumBeats()) {
            if (sheets.at(curr_sheetnum).GetNumBeats() == 0) {
                curr_beat = 0;
            } else {
                curr_beat = sheets.at(curr_sheetnum).GetNumBeats() - 1;
            }
        }
        return false;
    }
    return true;
}

bool Animation::PrevBeat()
{
    unsigned i;

    if (curr_beat == 0) {
        if (curr_sheetnum == 0)
            return false;
        curr_sheetnum--;
        for (i = 0; i < pts.size(); i++) {
            curr_cmds[i] = sheets.at(curr_sheetnum).GetCommandsEnd(i) - 1;
            EndCmd(i);
        }
        curr_beat = sheets.at(curr_sheetnum).GetNumBeats();
    }
    for (i = 0; i < pts.size(); i++) {
        if (!(*curr_cmds.at(i))->PrevBeat(pts[i])) {
            // Advance to prev command, skipping zero beat commands
            if (curr_cmds[i] != sheets.at(curr_sheetnum).GetCommandsBegin(i)) {
                --curr_cmds[i];
                EndCmd(i);
                // Set to next-to-last beat of this command
                // Should always return true
                (*curr_cmds[i])->PrevBeat(pts[i]);
            }
        }
    }
    if (curr_beat > 0)
        curr_beat--;
    CheckCollisions();
    return true;
}

bool Animation::NextBeat()
{
    unsigned i;

    curr_beat++;
    if (curr_beat >= sheets.at(curr_sheetnum).GetNumBeats()) {
        return NextSheet();
    }
    for (i = 0; i < pts.size(); i++) {
        if (!(*curr_cmds.at(i))->NextBeat(pts[i])) {
            // Advance to next command, skipping zero beat commands
            if ((curr_cmds[i] + 1) != sheets.at(curr_sheetnum).GetCommandsEnd(i)) {
                ++curr_cmds[i];
                BeginCmd(i);
            }
        }
    }
    CheckCollisions();
    return true;
}

void Animation::GotoBeat(unsigned i)
{
    while (curr_beat > i) {
        PrevBeat();
    }
    while (curr_beat < i) {
        NextBeat();
    }
}

void Animation::GotoSheet(unsigned i)
{
    GotoAnimationSheet(mAnimSheetIndices[i]);
}

void Animation::GotoAnimationSheet(unsigned i)
{
    curr_sheetnum = i;
    RefreshSheet();
    CheckCollisions();
}

void Animation::BeginCmd(unsigned i)
{
    while (!(*curr_cmds.at(i))->Begin(pts[i])) {
        if ((curr_cmds[i] + 1) != sheets.at(curr_sheetnum).GetCommandsEnd(i))
            return;
        ++curr_cmds[i];
    }
}

void Animation::EndCmd(unsigned i)
{
    while (!(*curr_cmds.at(i))->End(pts[i])) {
        if ((curr_cmds[i]) == sheets.at(curr_sheetnum).GetCommandsBegin(i))
            return;
        --curr_cmds[i];
    }
}

void Animation::RefreshSheet()
{
    pts = sheets.at(curr_sheetnum).GetPoints();
    for (auto i = 0u; i < pts.size(); i++) {
        curr_cmds[i] = sheets.at(curr_sheetnum).GetCommandsBegin(i);
        BeginCmd(i);
    }
    curr_beat = 0;
}

void Animation::CheckCollisions()
{
    mCollisions.clear();
    for (unsigned i = 0; i < pts.size(); i++) {
        for (unsigned j = i + 1; j < pts.size(); j++) {
            auto collisionResult = pts[i].DetectCollision(pts[j]);
            if (collisionResult) {
                if (!mCollisions.count(i) || mCollisions[i] < collisionResult) {
                    mCollisions[i] = collisionResult;
                }
                if (!mCollisions.count(j) || mCollisions[j] < collisionResult) {
                    mCollisions[j] = collisionResult;
                }
            }
        }
    }
    if (!mCollisions.empty() && mCollisionAction) {
        mCollisionAction();
    }
}

Animation::animate_info_t Animation::GetAnimateInfo(int which) const
{
    return Animation::animate_info_t(
        mCollisions.count(which) ? mCollisions.find(which)->second
                                 : Coord::COLLISION_NONE,
        (*curr_cmds.at(which))->Direction(),
        (*curr_cmds.at(which))->RealDirection(), pts.at(which));
}

int Animation::GetNumberSheets() const { return static_cast<int>(sheets.size()); }

AnimateCommands Animation::GetCommands(unsigned whichPoint) const
{
    return sheets.at(curr_sheetnum).GetCommands(whichPoint);
}

std::vector<DrawCommand>
Animation::GenPathToDraw(unsigned point, const Coord& offset) const
{
    auto animation_commands = GetCommands(point);
    auto position = pts.at(point);
    std::vector<DrawCommand> draw_commands;
    for (auto commands = animation_commands.begin();
         commands != animation_commands.end(); ++commands) {
        draw_commands.push_back((*commands)->GenCC_DrawCommand(position, offset));
        (*commands)->ApplyForward(position);
    }
    return draw_commands;
}

AnimatePoint Animation::EndPosition(unsigned point,
    const Coord& offset) const
{
    auto animation_commands = GetCommands(point);
    auto position = pts.at(point);
    for (auto commands = animation_commands.begin();
         commands != animation_commands.end(); ++commands) {
        (*commands)->ApplyForward(position);
    }
    position += offset;
    return position;
}

std::pair<std::string, std::vector<std::string>>
Animation::GetCurrentInfo() const
{
    std::vector<std::string> each;
    for (auto i = 0; i < static_cast<int>(pts.size()); ++i) {
        std::ostringstream each_string;
        auto info = GetAnimateInfo(i);
        each_string << "pt " << i << ": (" << info.mPosition.x << ", "
                    << info.mPosition.y << "), dir=" << info.mDirection
                    << ", realdir=" << info.mRealDirection
                    << (info.mCollision ? ", collision!" : "");
        each.push_back(each_string.str());
    }
    std::ostringstream output;
    output << GetCurrentSheetName() << " (" << GetCurrentSheet() << " of "
           << GetNumberSheets() << ")\n";
    output << "beat " << GetCurrentBeat() << " of " << GetNumberBeats() << "\n";
    return std::pair<std::string, std::vector<std::string>>(output.str(), each);
}

std::vector<AnimateSheet>::const_iterator Animation::sheetsBegin() const
{
    return sheets.begin();
}

std::vector<AnimateSheet>::const_iterator Animation::sheetsEnd() const
{
    return sheets.end();
}
}
