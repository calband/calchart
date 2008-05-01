/* draw.cpp
 * Member functions for drawing stuntsheets
 *
 * Modification history:
 * 6-22-95    Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1995-2008  Garrick Brian Meeker

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

#include <wx/dc.h>
#include "show.h"
#include "confgr.h"
#include "modes.h"

extern wxFont *pointLabelFont;

void CC_sheet::Draw(wxDC *dc, unsigned ref, bool primary,
		    bool drawall, int point) {
  unsigned short i;
  unsigned firstpoint, lastpoint;
  wxBrush *fillBrush;
  unsigned long x, y;
  float offset, plineoff, slineoff, textoff;
  float textw, texth, textd;
  float circ_r;
  CC_coord origin;

  dc->BeginDrawing();
  dc->SetBackgroundMode(wxTRANSPARENT);

  if (drawall) {
    dc->Clear();
    dc->SetPen(CalChartPens[COLOR_FIELD_DETAIL]);
    dc->SetTextForeground(&CalChartPens[COLOR_FIELD_TEXT]->GetColour());
    dc->SetLogicalFunction(wxCOPY);
    show->mode->Draw(dc);
  }

  if (pts) {
    dc->SetFont(pointLabelFont);
    dc->SetTextForeground(&CalChartPens[COLOR_POINT_TEXT]->GetColour());
    circ_r = FLOAT2COORD(dot_ratio);
    offset = circ_r / 2;
    plineoff = offset * pline_ratio;
    slineoff = offset * sline_ratio;
    textoff = offset * 1.25;
    origin = show->mode->Offset();
    if (point < 0) {
      firstpoint = 0;
      lastpoint = show->GetNumPoints();
    } else {
      firstpoint = (unsigned)point;
      lastpoint = (unsigned)point + 1;
    }
    for (int selectd = 0; selectd < 2; selectd++) {
      for (i = firstpoint; i < lastpoint; i++) {
	if ((show->IsSelected(i) != 0) == (selectd != 0)) {
	  if (selectd) {
	    if (primary) {
	      dc->SetPen(CalChartPens[COLOR_POINT_HILIT]);
	      fillBrush = CalChartBrushes[COLOR_POINT_HILIT];
	    } else {
	      dc->SetPen(CalChartPens[COLOR_REF_POINT_HILIT]);
	      fillBrush = CalChartBrushes[COLOR_REF_POINT_HILIT];
	    }
	  } else {
	    if (primary) {
	      dc->SetPen(CalChartPens[COLOR_POINT]);
	      fillBrush = CalChartBrushes[COLOR_POINT];
	    } else {
	      dc->SetPen(CalChartPens[COLOR_REF_POINT]);
	      fillBrush = CalChartBrushes[COLOR_REF_POINT];
	    }
	  }
	  x = GetPosition(i, ref).x+origin.x;
	  y = GetPosition(i, ref).y+origin.y;
	  switch (pts[i].sym) {
	  case SYMBOL_SOL:
	  case SYMBOL_SOLBKSL:
	  case SYMBOL_SOLSL:
	  case SYMBOL_SOLX:
	    dc->SetBrush(fillBrush);
	    break;
	  default:
	    dc->SetBrush(wxTRANSPARENT_BRUSH);
	  }
	  dc->DrawEllipse(x - offset, y - offset, circ_r, circ_r);
	  switch (pts[i].sym) {
	  case SYMBOL_SL:
	  case SYMBOL_X:
	    dc->DrawLine(x - plineoff, y + plineoff,
			 x + plineoff, y - plineoff);
	    break;
	  case SYMBOL_SOLSL:
	  case SYMBOL_SOLX:
	    dc->DrawLine(x - slineoff, y + slineoff,
			 x + slineoff, y - slineoff);
	    break;
	  default:
	    break;
	  }
	  switch (pts[i].sym) {
	  case SYMBOL_BKSL:
	  case SYMBOL_X:
	    dc->DrawLine(x - plineoff, y - plineoff,
			 x + plineoff, y + plineoff);
	    break;
	  case SYMBOL_SOLBKSL:
	  case SYMBOL_SOLX:
	    dc->DrawLine(x - slineoff, y - slineoff,
			 x + slineoff, y + slineoff);
	    break;
	  default:
	    break;
	  }
	  dc->GetTextExtent(show->GetPointLabel(i), &textw, &texth, &textd);
	  dc->DrawText(show->GetPointLabel(i),
		       pts[i].GetFlip() ? x : (x - textw),
		       y - textoff - texth + textd);
	}
      }
    }
  }
  dc->EndDrawing();
  dc->SetFont(NULL);
}
