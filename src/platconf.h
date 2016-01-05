/*
 * platconf.h
 * Configuration of additional platform dependencies
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

#define cpp_cat(a, b) a##b

// For creating icons and bitmaps
// There are separate macros because XPM is used for icons
#define ICON_NAME(name) cpp_cat(name, _xpm)
#define BITMAP_NAME(name) \
    (char*) cpp_cat(name, _bits), cpp_cat(name, _width), cpp_cat(name, _height)

// Run external programs to print or just write to files
#ifndef __WXMSW__
#define PRINT__RUN_CMD
#endif

// Character used in paths
#ifdef __WXMSW__
#define PATH_SEPARATOR wxT("\\")
#else
#define PATH_SEPARATOR wxT("/")
#endif

// Wildcard in file selector
#define FILE_WILDCARDS                                                           \
    wxT("All shows (*.shw;*.mas)|*.shw;*.mas|New shows (*.shw)|*.shw|Old shows " \
        "(*.mas)|*.mas")
#define FILE_SAVE_WILDCARDS wxT("New shows (*.shw)|*.shw")

// Need to handle DOS-style text
#ifdef __WXMSW__
#define TEXT_DOS_STYLE
#endif
