/*
 * cc_command.h
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

#pragma once

#include "cc_continuity.h"
#include "cc_coord.h"
#include "calchartdoc.h"
#include "cc_show.h"
#include <ostream>
#include <deque>
#include <map>
#include <wx/cmdproc.h>

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

// AddNewBackgroundImage
// Add a new image to the sheet
class AddNewBackgroundImageCommand : public SetSheetCommand {
private:
    using super = SetSheetCommand;

public:
    AddNewBackgroundImageCommand(CalChartDoc& show, int x, int y, int width, int height,
		std::vector<unsigned char> const& data,
		std::vector<unsigned char> const& alpha);
    virtual ~AddNewBackgroundImageCommand();
protected:
    virtual void DoAction();

	int m_x;
	int m_y;
	int m_width;
	int m_height;
	std::vector<unsigned char> m_data;
	std::vector<unsigned char> m_alpha;
};

// AddNewBackgroundImage
// Add a new image to the sheet
class RemoveBackgroundImageCommand : public SetSheetCommand {
private:
    using super = SetSheetCommand;

public:
    RemoveBackgroundImageCommand(CalChartDoc& show, int which);
    virtual ~RemoveBackgroundImageCommand();
protected:
    virtual void DoAction();

	int m_which;
};

// MoveBackgroundImage
// Apply a move to a background image
class MoveBackgroundImage : public SetSheetCommand {
private:
    using super = SetSheetCommand;

public:
    MoveBackgroundImage(CalChartDoc& show, int which, int left, int top, int scaled_width, int scaled_height);
    virtual ~MoveBackgroundImage();
protected:
    virtual void DoAction();

	int m_which;
	int m_left;
	int m_top;
	int m_scaled_width;
	int m_scaled_height;
};
