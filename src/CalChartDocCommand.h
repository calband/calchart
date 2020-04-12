#pragma once
/*
 * CalChartDocCommand.h
 * Handles interaction from View to Doc
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

#include <vector>
#include <wx/cmdproc.h>

class CalChartDoc;

class CalChartDocCommand : public wxCommand {
public:
    using CC_doc_command = std::function<void(CalChartDoc&)>;
    using CC_doc_command_pair = std::pair<CC_doc_command, CC_doc_command>;

    CalChartDocCommand(CalChartDoc& doc, const wxString& cmd_descr, CC_doc_command_pair const& cmds);
    CalChartDocCommand(CalChartDoc& doc, const wxString& cmd_descr, std::vector<CC_doc_command_pair> const& cmds);

    virtual bool Do();
    virtual bool Undo();

protected:
    CalChartDoc& mDoc;
    bool mDocModified;
    std::vector<CC_doc_command_pair> mCmds;
};
