/* cc_winlist.cpp
 * Member functions for winlist and winnode classes
 *
 */

/*
   Copyright (C) 1995-2010  Richard Powell

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

#ifdef __GNUG__
#pragma implementation
#endif

#include "cc_winlist.h"

#include <algorithm>

CC_WinNode::CC_WinNode(CC_WinList *lst)
: list(lst)
{
	list->Add(this);
}


CC_WinNode::~CC_WinNode() {}

void CC_WinNode::Remove()
{
	list->Remove(this);
}


CC_WinList::CC_WinList() {}

CC_WinList::~CC_WinList() {}

bool CC_WinList::MultipleWindows()
{
	return list.size() > 1;
}


void CC_WinList::Add(CC_WinNode *node)
{
	list.push_front(node);
}


void CC_WinList::Remove(CC_WinNode *node)
{
	list.erase(std::remove(list.begin(), list.end(), node), list.end());
	if (list.empty())
	{
		Empty();
	}
}


void CC_WinList::Empty() {}


void CC_WinList::ChangeNumPoints(wxWindow *win)
{
	for (NodeIter n = list.begin(); n != list.end(); ++n)
	{
		(*n)->ChangeNumPoints(win);
	}
}


void CC_WinList::GotoSheet(unsigned sht)
{
	for (NodeIter n = list.begin(); n != list.end(); ++n)
	{
		(*n)->GotoSheet(sht);
	}
}


void CC_WinList::GotoContLocation(unsigned sht, unsigned contnum, int line, int col)
{
	for (NodeIter n = list.begin(); n != list.end(); ++n)
	{
		(*n)->GotoContLocation(sht, contnum, line, col);
	}
}


void CC_WinList::FlushContinuity()
{
	for (NodeIter n = list.begin(); n != list.end(); ++n)
	{
		(*n)->FlushContinuity();
	}
}


