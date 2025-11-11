/*
 * CalChartDocCommand.cpp
 * Handle commands from view to doc
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

#include "CalChartDocCommand.h"
#include "CalChartDoc.h"

CalChartDocCommand::CalChartDocCommand(CalChartDoc& doc, std::string const& cmd_descr, CC_doc_command_pair const& cmds)
    : CalChartDocCommand(doc, cmd_descr, std::vector<CC_doc_command_pair>{ cmds })
{
}

CalChartDocCommand::CalChartDocCommand(CalChartDoc& doc, std::string const& cmd_descr, std::vector<CC_doc_command_pair> const& cmds)
    : wxCommand(true, cmd_descr)
    , mDoc(doc)
    , mDocModified(doc.IsModified())
    , mCmds(cmds)
{
}

// applying a command modifies the show
bool CalChartDocCommand::Do()
{
    for (auto&& cmd : mCmds) {
        cmd.first(mDoc);
    }
    mDoc.Modify(true);
    return true;
}

// undoing a command puts the state back to original
bool CalChartDocCommand::Undo()
{
    // apply undo in the reverse order
    for (auto iter = mCmds.rbegin(); iter != mCmds.rend(); ++iter) {
        iter->second(mDoc);
    }
    mDoc.Modify(mDocModified);
    return true;
}
