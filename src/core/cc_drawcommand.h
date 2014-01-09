/*
 * CC_DrawCommand.h
 * Class for how to draw
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

#ifndef _CC_DRAWCOMMAND_H_
#define _CC_DRAWCOMMAND_H_

/**
 * An instruction defining how something should be drawn.
 */
struct CC_DrawCommand
{
	typedef enum { Ignore, Line, Arc } DrawType;
	/**
	 * Defines how the draw command's data should be interpreted when
	 * drawing.
	 */
	DrawType mType;
	/**
	 * Start x position.
	 */
	int x1;
	/**
	 * Start y position.
	 */
	int y1;
	/** 
	 * End x position.
	 */
	int x2;
	/**
	 * End y position.
	 */
	int y2;
	/**
	 * Center x position.
	 */
	int xc;
	/**
	 * Center y position.
	 */
	int yc;
	// nothing version
	CC_DrawCommand() : mType(Ignore) {}

	// Line version
	/**
	 * Makes a command that will draw a line.
	 */
	CC_DrawCommand(int startx, int starty, int endx, int endy) :
	mType(Line),
	x1(startx),
	y1(starty),
	x2(endx),
	y2(endy)
	{}

	// Arc version
	/**
	 * Makes a command that will draw an arc.
	 */
	CC_DrawCommand(int startx, int starty, int endx, int endy, int centerx, int centery) :
	mType(Arc),
	x1(startx),
	y1(starty),
	x2(endx),
	y2(endy),
	xc(centerx),
	yc(centery)
	{}

};

#endif // _CC_DRAWCOMMAND_H_
