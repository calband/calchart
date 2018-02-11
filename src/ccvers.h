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

#ifndef CC_VERSION

#define CC_MAJOR_VERSION 3
#define CC_MINOR_VERSION 5
#define CC_SUB_MINOR_VERSION 1

#define STRINGIZE_HELPER(name) #name
#define STRINGIZE(x) STRINGIZE_HELPER(x)

#define MK_CC_VERSION(major, minor, subminor) \
    "v" STRINGIZE(major) "." STRINGIZE(minor) "." STRINGIZE(subminor)
#define CC_VERSION \
    MK_CC_VERSION(CC_MAJOR_VERSION, CC_MINOR_VERSION, CC_SUB_MINOR_VERSION)

#endif
