/* show.cpp
 * Member functions for show classes
 *
 * Modification history:
 * 4-16-95    Garrick Meeker              Created from previous CalPrint
 * 7-28-95    Garrick Meeker              Added continuity parser from
 *                                           previous CalPrint
 *
 */

/*
   Copyright (C) 1994-2008  Garrick Brian Meeker

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

#include "cc_continuity.h"


wxString Capitalize(const wxString &str)
{
	wxString Result = str.Lower();
	if (Result.Length() > 0)
		Result[0] = toupper(Result.c_str()[0]);

	return Result;
}

CC_continuity::CC_continuity(const wxString& s, unsigned n)
: num(n), name(Capitalize(s))
{
}

CC_continuity::~CC_continuity()
{
}

const wxString& CC_continuity::GetName() const
{
	return name;
}

unsigned CC_continuity::GetNum() const
{
	return num;
}

void CC_continuity::SetText(const wxString& s)
{
	text = s;
}

const wxString& CC_continuity::GetText() const
{
	return text;
}

void CC_continuity::AppendText(const wxString& s)
{
	text.Append(s);
}

