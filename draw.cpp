/* draw.cc
 * Member functions for drawing stuntsheets
 *
 * Modification history:
 * 6-22-95    Garrick Meeker              Created
 *
 */

#include <wx_dc.h>
#include "show.h"
#include "confgr.h"
#include "modes.h"

extern wxFont *pointLabelFont;
extern wxBrush *hilitBrush;
extern wxPen *hilitPen;

void CC_sheet::Draw(wxDC *dc, Bool drawall, int point) {
  unsigned short i;
  unsigned lastpoint;
  wxBrush *fillBrush;
  unsigned long x, y;
  float offset, lineoff, textoff;
  float textw, texth, textd;
  float circ_r;
  CC_coord origin;

  dc->BeginDrawing();
  dc->SetBackgroundMode(wxTRANSPARENT);
  if (dc->Colour) {
    dc->SetPen(wxWHITE_PEN);
    fillBrush = wxWHITE_BRUSH;
  } else {
    dc->SetPen(wxBLACK_PEN);
    fillBrush = wxBLACK_BRUSH;
  }
  dc->SetTextForeground(wxBLACK);
  dc->SetLogicalFunction(wxCOPY);

  if (drawall) {
    dc->Clear();
    show->mode->Draw(dc);
  }

  if (pts) {
    dc->SetFont(pointLabelFont);
    circ_r = FLOAT2COORD(dot_ratio);
    offset = circ_r / 2;
    lineoff = offset * pline_ratio;
    textoff = offset * 1.25;
    origin = show->mode->Offset();
    if (point < 0) {
      i = 0;
      lastpoint = show->GetNumPoints();
    } else {
      i = (unsigned)point;
      lastpoint = (unsigned)point + 1;
    }
    for (; i < lastpoint; i++) {
      if (dc->Colour) {
	if (show->IsSelected(i)) {
	  dc->SetPen(hilitPen);
	  fillBrush = hilitBrush;
	} else {
	  dc->SetPen(wxWHITE_PEN);
	  fillBrush = wxWHITE_BRUSH;
	}
      } else {
	dc->SetPen(wxBLACK_PEN);
	fillBrush = wxBLACK_BRUSH;
      }
      x = pts[i].pos.x+origin.x;
      y = pts[i].pos.y+origin.y;
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
      case SYMBOL_SOLSL:
      case SYMBOL_SOLX:
	dc->DrawLine(x - lineoff, y + lineoff, x + lineoff, y - lineoff);
	break;
      default:
	break;
      }
      switch (pts[i].sym) {
      case SYMBOL_BKSL:
      case SYMBOL_X:
      case SYMBOL_SOLBKSL:
      case SYMBOL_SOLX:
	dc->DrawLine(x - lineoff, y - lineoff, x + lineoff, y + lineoff);
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
  dc->EndDrawing();
  dc->SetFont(NULL);
}
