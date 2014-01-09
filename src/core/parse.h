/*
 * parse.h
 * Classes for parsing continuity
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

#ifndef _PARSE_H_
#define _PARSE_H_

/**
 * TODO
 */
struct YYLTYPE
{
	int first_line;
	int first_column;
};

extern YYLTYPE yylloc;

/**
 * A list of Procedures entered through the Continuity Editor.
 */
struct proclist
{
	/**
	 * The first procedure in the list. Each procedure has a link to the next
	 * one, so this gives access to the entire list.
	 */
	ContProcedure* list;
	/**
	 * The last procedure in the list.
	 */
	ContProcedure* last;
};
#endif
