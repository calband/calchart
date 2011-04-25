/* cc_winlist.h
 * Definitions for the winlist and winnode classes
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

#ifndef _CC_WINLIST_H_
#define _CC_WINLIST_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <wx/defs.h>							  // For basic wx defines
#include <wx/string.h>
#include <wx/list.h>

#include <vector>
#include <deque>

class wxWindow;
class wxDC;
class CC_show;

class CC_WinList;
class CC_WinNode
{
public:
	CC_WinNode(CC_WinList *lst);
	virtual ~CC_WinNode();

	void Remove();
	inline CC_WinList* GetList() { return list; }

	virtual void UpdateSelections(wxWindow* win = NULL, int point = -1) {}
	virtual void UpdatePointsOnSheet(unsigned sht, int ref = -1) {}
	virtual void ChangeNumPoints(wxWindow *win) {}
	virtual void ChangePointLabels(wxWindow *win)  {}
	virtual void ChangeShowMode(wxWindow *win) {}
	virtual void UpdateStatusBar() {}
	virtual void GotoSheet(unsigned sht) {}
	virtual void GotoContLocation(unsigned sht, unsigned contnum, int line = -1, int col = -1) { GotoSheet(sht); }
	virtual void AddSheet(unsigned sht) {}
	virtual void DeleteSheet(unsigned sht) {}
	virtual void AppendSheets() {}
	virtual void RemoveSheets(unsigned num) {}
	virtual void ChangeTitle(unsigned sht) {}
	virtual void SelectSheet(wxWindow* win, unsigned sht) {}
	virtual void AddContinuity(unsigned sht, unsigned cont) {}
	virtual void DeleteContinuity(unsigned sht, unsigned cont) {}
	virtual void FlushContinuity() {}
	virtual void SetContinuity(wxWindow* win, unsigned sht, unsigned cont) {}

protected:
	CC_WinList *list;
};

class CC_WinList
{
public:
	CC_WinList();
	virtual ~CC_WinList();

	bool MultipleWindows();

	void Add(CC_WinNode *node);
	void Remove(CC_WinNode *node);
	virtual void Empty();

	virtual void UpdateSelections(wxWindow* win = NULL, int point = -1);
	virtual void UpdatePointsOnSheet(unsigned sht, int ref = -1);
	virtual void ChangeNumPoints(wxWindow *win);
	virtual void ChangePointLabels(wxWindow *win);
	virtual void ChangeShowMode(wxWindow *win);
	virtual void UpdateStatusBar();
	virtual void GotoSheet(unsigned sht);
	virtual void GotoContLocation(unsigned sht, unsigned contnum,
		int line = -1, int col = -1);
	virtual void AddSheet(unsigned sht);
	virtual void DeleteSheet(unsigned sht);
	virtual void AppendSheets();
	virtual void RemoveSheets(unsigned num);
	virtual void ChangeTitle(unsigned sht);
	virtual void SelectSheet(wxWindow* win, unsigned sht);
	virtual void AddContinuity(unsigned sht, unsigned cont);
	virtual void DeleteContinuity(unsigned sht, unsigned cont);
	virtual void FlushContinuity();
	virtual void SetContinuity(wxWindow* win, unsigned sht, unsigned cont);

private:
	typedef std::deque<CC_WinNode*> WinNodeList;
	typedef WinNodeList::iterator NodeIter;
	typedef WinNodeList::const_iterator NodeCIter;
	WinNodeList list;
};

#endif
