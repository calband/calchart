#pragma once
/*
 * cc_omniview_constants.h
 * CalChart omniview constants
 */

/*
 Copyright (C) 2012  Richard Michael Powell

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

#define _USE_MATH_DEFINES
#include <cmath>

static constexpr auto kViewPoint_x_1 = 0.0;
static constexpr auto kViewPoint_y_1 = -60.0;
static constexpr auto kViewPoint_z_1 = 20.0;
static constexpr auto kViewAngle_1 = M_PI / 2.0;
static constexpr auto kViewAngle_z_1 = -M_PI / 8.0;

static constexpr auto kViewPoint_x_2 = 0.0;
static constexpr auto kViewPoint_y_2 = -16.0;
static constexpr auto kViewPoint_z_2 = 2.5;
static constexpr auto kViewAngle_2 = M_PI / 2.0;
static constexpr auto kViewAngle_z_2 = 0.0;

static constexpr auto kViewPoint_x_3 = 60.0;
static constexpr auto kViewPoint_y_3 = -55.0;
static constexpr auto kViewPoint_z_3 = 20.0;
static constexpr auto kViewAngle_3 = 11.0 * M_PI / 16.0;
static constexpr auto kViewAngle_z_3 = -M_PI / 8.0;
