/*
 * new_cont_grammar.h
 * Grammar for continuity
 */

/*
   Copyright (C) 2014  Richard Michael Powell

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

#ifndef _NEW_CONT_GRAMMAR_H_
#define _NEW_CONT_GRAMMAR_H_

#include "new_cont.h"

#include <utility>
#include <string>

class AnimateCompile;
class CC_coord;

std::pair<bool, CC_coord> Parse_point_string(const std::string& test, AnimateCompile& a);
std::pair<bool, double> Parse_value_string(const std::string& test, AnimateCompile& a);
std::pair<bool, std::vector<calchart::continuity::Procedure>> Parse_procedure_string(const std::string& test);

#endif // _NEW_CONT_GRAMMAR_H_
