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
bool drawall, int point)
{
	unsigned short i;
	unsigned firstpoint, lastpoint;
	const wxBrush *fillBrush;
	unsigned long x, y;
	float offset, plineoff, slineoff, textoff;
	wxCoord textw, texth, textd;
	float circ_r;
	CC_coord origin;

	dc->SetBackgroundMode(wxTRANSPARENT);
	dc->SetBackground(*CalChartBrushes[COLOR_FIELD]);

	if (drawall)
	{
		dc->Clear();
		dc->SetPen(*CalChartPens[COLOR_FIELD_DETAIL]);
		dc->SetTextForeground(CalChartPens[COLOR_FIELD_TEXT]->GetColour());
		dc->SetLogicalFunction(wxCOPY);
		show->GetMode().Draw(dc);
	}

	if (!pts.empty())
	{
		dc->SetFont(*pointLabelFont);
		dc->SetTextForeground(CalChartPens[COLOR_POINT_TEXT]->GetColour());
		circ_r = FLOAT2COORD(dot_ratio);
		offset = circ_r / 2;
		plineoff = offset * pline_ratio;
		slineoff = offset * sline_ratio;
		textoff = offset * 1.25;
		origin = show->GetMode().Offset();
		if (point < 0)
		{
			firstpoint = 0;
			lastpoint = show->GetNumPoints();
		}
		else
		{
			firstpoint = (unsigned)point;
			lastpoint = (unsigned)point + 1;
		}
		for (int selectd = 0; selectd < 2; selectd++)
		{
			for (i = firstpoint; i < lastpoint; i++)
			{
				if ((show->IsSelected(i) != 0) == (selectd != 0))
				{
					if (selectd)
					{
						if (primary)
						{
							dc->SetPen(*CalChartPens[COLOR_POINT_HILIT]);
							fillBrush = CalChartBrushes[COLOR_POINT_HILIT];
						}
						else
						{
							dc->SetPen(*CalChartPens[COLOR_REF_POINT_HILIT]);
							fillBrush = CalChartBrushes[COLOR_REF_POINT_HILIT];
						}
					}
					else
					{
						if (primary)
						{
							dc->SetPen(*CalChartPens[COLOR_POINT]);
							fillBrush = CalChartBrushes[COLOR_POINT];
						}
						else
						{
							dc->SetPen(*CalChartPens[COLOR_REF_POINT]);
							fillBrush = CalChartBrushes[COLOR_REF_POINT];
						}
					}
					x = GetPosition(i, ref).x+origin.x;
					y = GetPosition(i, ref).y+origin.y;
					switch (pts[i].sym)
					{
						case SYMBOL_SOL:
						case SYMBOL_SOLBKSL:
						case SYMBOL_SOLSL:
						case SYMBOL_SOLX:
							dc->SetBrush(*fillBrush);
							break;
						default:
							dc->SetBrush(*wxTRANSPARENT_BRUSH);
					}
					dc->DrawEllipse(x - offset, y - offset, circ_r, circ_r);
					switch (pts[i].sym)
					{
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
					switch (pts[i].sym)
					{
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
	dc->SetFont(wxNullFont);
}

void DrawCenteredText(wxDC& dc, const wxString& text, const wxPoint& pt)
{
	wxCoord w, h;
	dc.GetTextExtent(text, &w, &h);
	w = std::max(0, pt.x - w/2);
	dc.DrawText(text, w, pt.y);
}

void DrawLineOverText(wxDC& dc, const wxString& text, const wxPoint& pt, wxCoord lineLength)
{
	DrawCenteredText(dc, text, pt + wxPoint(0,2));
	dc.DrawLine(pt - wxPoint(lineLength/2,0), pt + wxPoint(lineLength/2,0));
}

static const size_t ArrowSize = 4;
void DrawArrow(wxDC& dc, const wxPoint& pt, wxCoord lineLength, bool pointRight)
{
	dc.DrawLine(pt + wxPoint(-lineLength/2,ArrowSize), pt + wxPoint(lineLength/2,ArrowSize));
	if (pointRight)
	{
		dc.DrawLine(pt + wxPoint(lineLength/2 - ArrowSize,0), pt + wxPoint(lineLength/2,ArrowSize));
		dc.DrawLine(pt + wxPoint(lineLength/2 - ArrowSize,ArrowSize*2), pt + wxPoint(lineLength/2,ArrowSize));
	}
	else
	{
		dc.DrawLine(pt + wxPoint(-(lineLength/2 - ArrowSize),0), pt + wxPoint(-lineLength/2,ArrowSize));
		dc.DrawLine(pt + wxPoint(-(lineLength/2 - ArrowSize),ArrowSize*2), pt + wxPoint(-lineLength/2,ArrowSize));
	}
}


size_t TabStops(size_t which)
{
	size_t tab = 0;
	while (which > 4)
	{
		which--;
		tab += 6;
	}
	switch (which)
	{
		case 3:
			tab += 8;
		case 2:
			tab += 14;
		case 1:
			tab += 6;
	}
	return tab;
}

size_t TabStopsLandscape(size_t which)
{
	size_t tab = 0;
	while (which > 4)
	{
		which--;
		tab += 8;
	}
	switch (which)
	{
		case 3:
			tab += 8;
		case 2:
			tab += 18;
		case 1:
			tab += 10;
	}
	return tab;
}

// draw the continuity starting at a specific offset
void CC_sheet::DrawCont(wxDC *dc, const wxCoord yStart) const
{
	float x, y;
	wxCoord textw, texth, textd, maxtexth;

	wxCoord origX, origY;
	double origXscale, origYscale;

	dc->GetDeviceOrigin(&origX, &origY);
	dc->GetUserScale(&origXscale, &origYscale);
	const wxColour& origForegroundColor = dc->GetTextForeground();
	const wxFont& origFont = dc->GetFont();

	// we set the font large then scale down to give fine detail on the page
	wxFont *contPlainFont = wxTheFontList->FindOrCreateFont(20, wxMODERN, wxNORMAL, wxNORMAL);
	wxFont *contBoldFont = wxTheFontList->FindOrCreateFont(20, wxMODERN, wxNORMAL, wxBOLD);
	wxFont *contItalFont = wxTheFontList->FindOrCreateFont(20, wxMODERN, wxITALIC, wxNORMAL);
	wxFont *contBoldItalFont = wxTheFontList->FindOrCreateFont(20, wxMODERN, wxITALIC, wxBOLD);

	dc->SetDeviceOrigin(20, yStart);
	dc->SetUserScale(0.42, 0.42);
	dc->SetTextForeground(*wxBLACK);
	dc->SetFont(*contPlainFont);
	
	int pageW, pageH;
	dc->GetSize(&pageW, &pageH);
	int pageMiddle = pageW/2 - 20;
	pageMiddle /= 0.42;

	y = 0.0;
	const wxCoord charWidth = dc->GetCharWidth();
	CC_textline_list::const_iterator cont(continuity.lines.begin());
	while (cont != continuity.lines.end())
	{
		bool do_tab;
		CC_textchunk_list::const_iterator c;
		x = 0.0;
		if (cont->center)
		{
			x += pageMiddle;
			for (c = cont->chunks.begin();
				c != cont->chunks.end();
				++c)
			{
				do_tab = false;
				switch (c->font)
				{
					case PSFONT_SYMBOL:
						dc->GetTextExtent(wxT("O"), &textw, &texth, &textd);
						x += textw * c->text.length();
						break;
					case PSFONT_NORM:
						dc->SetFont(*contPlainFont);
						break;
					case PSFONT_BOLD:
						dc->SetFont(*contBoldFont);
						break;
					case PSFONT_ITAL:
						dc->SetFont(*contItalFont);
						break;
					case PSFONT_BOLDITAL:
						dc->SetFont(*contBoldItalFont);
						break;
					case PSFONT_TAB:
						do_tab = true;
						break;
				}
				if (!do_tab && (c->font != PSFONT_SYMBOL))
				{
					dc->GetTextExtent(c->text, &textw, &texth, &textd);
					x -= textw/2;
				}
			}
		}
		maxtexth = contPlainFont->GetPointSize()+2;
		unsigned tabnum = 0;
		for (c = cont->chunks.begin();
			c != cont->chunks.end();
			++c)
		{
			do_tab = false;
			switch (c->font)
			{
				case PSFONT_NORM:
				case PSFONT_SYMBOL:
					dc->SetFont(*contPlainFont);
					break;
				case PSFONT_BOLD:
					dc->SetFont(*contBoldFont);
					break;
				case PSFONT_ITAL:
					dc->SetFont(*contItalFont);
					break;
				case PSFONT_BOLDITAL:
					dc->SetFont(*contBoldItalFont);
					break;
				case PSFONT_TAB:
					tabnum++;
					textw = charWidth * TabStops(tabnum);
					if (textw >= x) x = textw;
					else x += charWidth;
					do_tab = true;
					break;
				default:
					break;
			}
			if (c->font == PSFONT_SYMBOL)
			{
				dc->GetTextExtent(wxT("O"), &textw, &texth, &textd);
				float d = textw;
				SYMBOL_TYPE sym;

				float top_y = y + texth - textd - d;

				for (const wxChar *s = c->text; *s; s++)
				{
					{
						dc->SetPen(*wxBLACK_PEN);
						sym = (SYMBOL_TYPE)(*s - 'A');
						switch (sym)
						{
							case SYMBOL_SOL:
							case SYMBOL_SOLBKSL:
							case SYMBOL_SOLSL:
							case SYMBOL_SOLX:
								dc->SetBrush(*wxBLACK_BRUSH);
								break;
							default:
								dc->SetBrush(*wxTRANSPARENT_BRUSH);
						}
						dc->DrawEllipse(x, top_y, d, d);
						switch (sym)
						{
							case SYMBOL_SL:
							case SYMBOL_X:
							case SYMBOL_SOLSL:
							case SYMBOL_SOLX:
								dc->DrawLine(x-1, top_y + d+1, x + d+1, top_y-1);
								break;
							default:
								break;
						}
						switch (sym)
						{
							case SYMBOL_BKSL:
							case SYMBOL_X:
							case SYMBOL_SOLBKSL:
							case SYMBOL_SOLX:
								dc->DrawLine(x-1, top_y-1, x + d+1, top_y + d+1);
								break;
							default:
								break;
						}
					}
					x += d;
				}
			}
			else
			{
				if (!do_tab)
				{
					dc->GetTextExtent(c->text, &textw, &texth, &textd);
					printf("drawing text at %f\n", x);
					dc->DrawText(c->text, x, y);
					x += textw;
				}
			}
		}
		y += maxtexth;
		++cont;
	}

	// restore everything
	dc->SetUserScale(origXscale, origYscale);
	dc->SetDeviceOrigin(origX, origY);
	dc->SetTextForeground(origForegroundColor);
	dc->SetFont(origFont);
}


void CC_sheet::DrawForPrinting(wxDC *dc, unsigned ref) const
{
	unsigned short i;
	unsigned firstpoint, lastpoint;
	unsigned long x, y;
	wxCoord textw, texth, textd;
	float circ_r;
	CC_coord origin;

	double originalScaleX, originalScaleY;
	dc->GetUserScale(&originalScaleX, &originalScaleY);

	// create a field for drawing:
	CC_coord bord1(INT2COORD(8),INT2COORD(8)), bord2(INT2COORD(8),INT2COORD(8));
	CC_coord siz, off;
	uint32_t whash = 32;
	uint32_t ehash = 52;
	bord1.x = INT2COORD(6);
	bord1.y = INT2COORD(19);
	bord2.x = INT2COORD(6);
	bord2.y = INT2COORD(6);
	siz.x = INT2COORD(96);
	siz.y = INT2COORD(84);
	off.x = INT2COORD(48);
	off.y = INT2COORD(42);
	ShowMode* mode = new ShowModeStandard(wxT("Standard"), bord1, bord2, siz, off, whash, ehash);


	// set the scaling for drawing the field
	{
		int pageW, pageH;
		dc->GetSize(&pageW, &pageH);
		dc->SetUserScale(pageW/(double)mode->Size().x, pageW/(double)mode->Size().x);
	}

	// draw the field.
	dc->Clear();
	dc->SetPen(*wxBLACK_PEN);
	dc->SetTextForeground(*wxBLACK);
	dc->SetLogicalFunction(wxCOPY);
	mode->Draw(dc);

	if (!pts.empty())
	{
		dc->SetFont(*pointLabelFont);
		circ_r = FLOAT2COORD(dot_ratio);
		const float offset = circ_r / 2;
		const float plineoff = offset * pline_ratio;
		const float slineoff = offset * sline_ratio;
		const float textoff = offset * 1.25;
		origin = mode->Offset();
		firstpoint = 0;
		lastpoint = show->GetNumPoints();
		for (int selectd = 0; selectd < 2; selectd++)
		{
			for (i = firstpoint; i < lastpoint; i++)
			{
				x = GetPosition(i, ref).x+origin.x;
				y = GetPosition(i, ref).y+origin.y;
				switch (pts[i].sym)
				{
					case SYMBOL_SOL:
					case SYMBOL_SOLBKSL:
					case SYMBOL_SOLSL:
					case SYMBOL_SOLX:
						dc->SetBrush(*wxBLACK_BRUSH);
						break;
					default:
						dc->SetBrush(*wxTRANSPARENT_BRUSH);
				}
				dc->DrawEllipse(x - offset, y - offset, circ_r, circ_r);
				switch (pts[i].sym)
				{
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
				switch (pts[i].sym)
				{
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

	dc->SetBrush(*wxTRANSPARENT_BRUSH);
	// draw the header
	dc->SetUserScale(originalScaleX, originalScaleY);
	dc->SetFont(*wxTheFontList->FindOrCreateFont(16, wxROMAN, wxNORMAL, wxBOLD));
	dc->SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 1, wxSOLID));
	int pageW, pageH;
	dc->GetSize(&pageW, &pageH);
	wxPoint pt(pageW/2, 18);
	DrawCenteredText(*dc, wxT("UNIVERSITY OF CALIFORNIA MARCHING BAND"), pt);

	DrawCenteredText(*dc, GetNumber(), wxPoint(pageW-(62), 36));
	DrawCenteredText(*dc, GetNumber(), wxPoint(pageW-(62), 720));
	dc->DrawRectangle(pageW-(90), 714, 56, 28);

	dc->SetFont(*wxTheFontList->FindOrCreateFont(8, wxSWISS, wxNORMAL, wxNORMAL));

	DrawLineOverText(*dc, wxT("Music"), wxPoint(pageW/2, 60), 240);
	DrawLineOverText(*dc, wxT("Formation"), wxPoint(pageW/2, 82), 240);

	DrawLineOverText(*dc, wxT("game"), wxPoint(62, 50), 64);
	DrawLineOverText(*dc, wxT("page"), wxPoint(pageW-(62), 50), 64);

	// draw arrows
	pt = wxPoint(52, 76);
	DrawCenteredText(*dc, wxT("south"), pt-wxPoint(0,8));
	DrawArrow(*dc, pt, 40, false);
	pt = wxPoint(pageW-(52), 76);
	DrawCenteredText(*dc, wxT("north"), pt-wxPoint(0,8));
	DrawArrow(*dc, pt, 40, true);

	// draw arrows
	pt = wxPoint(52, 570);
	DrawCenteredText(*dc, wxT("south"), pt+wxPoint(0,8));
	DrawArrow(*dc, pt, 40, false);
	pt = wxPoint(pageW-(52), 570);
	DrawCenteredText(*dc, wxT("north"), pt+wxPoint(0,8));
	DrawArrow(*dc, pt, 40, true);

	DrawCenteredText(*dc, wxT("CAL SIDE"), wxPoint(pageW/2,580));

	DrawCont(dc, 606);

	dc->SetFont(wxNullFont);
	
}

// draw the continuity starting at a specific offset
void CC_sheet::DrawContLandscape(wxDC *dc, const wxCoord yStart) const
{
	float x, y;
	wxCoord textw, texth, textd, maxtexth;

	wxCoord origX, origY;
	double origXscale, origYscale;

	dc->GetDeviceOrigin(&origX, &origY);
	dc->GetUserScale(&origXscale, &origYscale);
	const wxColour& origForegroundColor = dc->GetTextForeground();
	const wxFont& origFont = dc->GetFont();

	// we set the font large then scale down to give fine detail on the page
	wxFont *contPlainFont = wxTheFontList->FindOrCreateFont(20, wxMODERN, wxNORMAL, wxNORMAL);
	wxFont *contBoldFont = wxTheFontList->FindOrCreateFont(20, wxMODERN, wxNORMAL, wxBOLD);
	wxFont *contItalFont = wxTheFontList->FindOrCreateFont(20, wxMODERN, wxITALIC, wxNORMAL);
	wxFont *contBoldItalFont = wxTheFontList->FindOrCreateFont(20, wxMODERN, wxITALIC, wxBOLD);

	dc->SetDeviceOrigin(20, yStart);
	dc->SetUserScale(0.42, 0.42);
	dc->SetTextForeground(*wxBLACK);
	dc->SetFont(*contPlainFont);

	int pageW, pageH;
	dc->GetSize(&pageW, &pageH);
	int pageMiddle = pageW/2 - 20;
	pageMiddle /= 0.42;

	y = 0.0;
	const wxCoord charWidth = dc->GetCharWidth();
	CC_textline_list::const_iterator cont(continuity.lines.begin());
	while (cont != continuity.lines.end())
	{
		bool do_tab;
		CC_textchunk_list::const_iterator c;
		x = 0.0;
		if (cont->center)
		{
			x += pageMiddle;
			for (c = cont->chunks.begin();
				c != cont->chunks.end();
				++c)
			{
				do_tab = false;
				switch (c->font)
				{
					case PSFONT_SYMBOL:
						dc->GetTextExtent(wxT("O"), &textw, &texth, &textd);
						x += textw * c->text.length();
						break;
					case PSFONT_NORM:
						dc->SetFont(*contPlainFont);
						break;
					case PSFONT_BOLD:
						dc->SetFont(*contBoldFont);
						break;
					case PSFONT_ITAL:
						dc->SetFont(*contItalFont);
						break;
					case PSFONT_BOLDITAL:
						dc->SetFont(*contBoldItalFont);
						break;
					case PSFONT_TAB:
						do_tab = true;
						break;
				}
				if (!do_tab && (c->font != PSFONT_SYMBOL))
				{
					dc->GetTextExtent(c->text, &textw, &texth, &textd);
					x -= textw/2;
				}
			}
		}
		maxtexth = contPlainFont->GetPointSize()+2;
		unsigned tabnum = 0;
		for (c = cont->chunks.begin();
			c != cont->chunks.end();
			++c)
		{
			do_tab = false;
			switch (c->font)
			{
				case PSFONT_NORM:
				case PSFONT_SYMBOL:
					dc->SetFont(*contPlainFont);
					break;
				case PSFONT_BOLD:
					dc->SetFont(*contBoldFont);
					break;
				case PSFONT_ITAL:
					dc->SetFont(*contItalFont);
					break;
				case PSFONT_BOLDITAL:
					dc->SetFont(*contBoldItalFont);
					break;
				case PSFONT_TAB:
					tabnum++;
					textw = charWidth * TabStopsLandscape(tabnum);
					if (textw >= x) x = textw;
					else x += charWidth;
					do_tab = true;
					break;
				default:
					break;
			}
			if (c->font == PSFONT_SYMBOL)
			{
				dc->GetTextExtent(wxT("O"), &textw, &texth, &textd);
				float d = textw;
				SYMBOL_TYPE sym;

				float top_y = y + texth - textd - d;

				for (const wxChar *s = c->text; *s; s++)
				{
					{
						dc->SetPen(*wxBLACK_PEN);
						sym = (SYMBOL_TYPE)(*s - 'A');
						switch (sym)
						{
							case SYMBOL_SOL:
							case SYMBOL_SOLBKSL:
							case SYMBOL_SOLSL:
							case SYMBOL_SOLX:
								dc->SetBrush(*wxBLACK_BRUSH);
								break;
							default:
								dc->SetBrush(*wxTRANSPARENT_BRUSH);
						}
						dc->DrawEllipse(x, top_y, d, d);
						switch (sym)
						{
							case SYMBOL_SL:
							case SYMBOL_X:
							case SYMBOL_SOLSL:
							case SYMBOL_SOLX:
								dc->DrawLine(x-1, top_y + d+1, x + d+1, top_y-1);
								break;
							default:
								break;
						}
						switch (sym)
						{
							case SYMBOL_BKSL:
							case SYMBOL_X:
							case SYMBOL_SOLBKSL:
							case SYMBOL_SOLX:
								dc->DrawLine(x-1, top_y-1, x + d+1, top_y + d+1);
								break;
							default:
								break;
						}
					}
					x += d;
				}
			}
			else
			{
				if (!do_tab)
				{
					dc->GetTextExtent(c->text, &textw, &texth, &textd);
					dc->DrawText(c->text, x, y);
					x += textw;
				}
			}
		}
		y += maxtexth;
		++cont;
	}

	// restore everything
	dc->SetUserScale(origXscale, origYscale);
	dc->SetDeviceOrigin(origX, origY);
	dc->SetTextForeground(origForegroundColor);
	dc->SetFont(origFont);
}

void CC_sheet::DrawForPrintingLandscape(wxDC *dc, unsigned ref) const
{
	unsigned short i;
	unsigned firstpoint, lastpoint;
	unsigned long x, y;
	wxCoord textw, texth, textd;
	float circ_r;
	CC_coord origin;

	wxCoord origX, origY;
	double origXscale, origYscale;
	dc->GetDeviceOrigin(&origX, &origY);
	dc->GetUserScale(&origXscale, &origYscale);

	dc->SetDeviceOrigin(10, 0);

	// create a field for drawing:
	CC_coord bord1(INT2COORD(8),INT2COORD(8)), bord2(INT2COORD(8),INT2COORD(8));
	CC_coord siz, off;
	uint32_t whash = 32;
	uint32_t ehash = 52;
	bord1.x = INT2COORD(12);
	bord1.y = INT2COORD(21);
	bord2.x = INT2COORD(12);
	bord2.y = INT2COORD(12);
	siz.x = INT2COORD(160);
	siz.y = INT2COORD(84);
	off.x = INT2COORD(80);
	off.y = INT2COORD(42);
	ShowMode* mode = new ShowModeStandard(wxT("Standard"), bord1, bord2, siz, off, whash, ehash);


	// set the scaling for drawing the field
	{
		int pageW, pageH;
		dc->GetSize(&pageW, &pageH);
		dc->SetUserScale(pageW/(double)mode->Size().x, pageW/(double)mode->Size().x);
	}

	// draw the field.
	dc->Clear();
	dc->SetPen(*wxBLACK_PEN);
	dc->SetTextForeground(*wxBLACK);
	dc->SetLogicalFunction(wxCOPY);
	mode->Draw(dc);

	if (!pts.empty())
	{
		dc->SetFont(*pointLabelFont);
		circ_r = FLOAT2COORD(dot_ratio);
		const float offset = circ_r / 2;
		const float plineoff = offset * pline_ratio;
		const float slineoff = offset * sline_ratio;
		const float textoff = offset * 1.25;
		origin = mode->Offset();
		firstpoint = 0;
		lastpoint = show->GetNumPoints();
		for (int selectd = 0; selectd < 2; selectd++)
		{
			for (i = firstpoint; i < lastpoint; i++)
			{
				x = GetPosition(i, ref).x+origin.x;
				y = GetPosition(i, ref).y+origin.y;
				switch (pts[i].sym)
				{
					case SYMBOL_SOL:
					case SYMBOL_SOLBKSL:
					case SYMBOL_SOLSL:
					case SYMBOL_SOLX:
						dc->SetBrush(*wxBLACK_BRUSH);
						break;
					default:
						dc->SetBrush(*wxTRANSPARENT_BRUSH);
				}
				dc->DrawEllipse(x - offset, y - offset, circ_r, circ_r);
				switch (pts[i].sym)
				{
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
				switch (pts[i].sym)
				{
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

	dc->SetBrush(*wxTRANSPARENT_BRUSH);
	// draw the header
	dc->SetUserScale(0.8, 0.8);

	int pageW, pageH;
	dc->GetSize(&pageW, &pageH);
	pageW /= 0.8;
	pageH /= 0.8;

	dc->SetFont(*wxTheFontList->FindOrCreateFont(16, wxROMAN, wxNORMAL, wxBOLD));
	dc->SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 1, wxSOLID));
//	int pageW, pageH;
//	// for landscape the sizes are reversed
//	dc->GetSize(&pageH, &pageW);
	wxPoint pt(pageW/2, 22);
	DrawCenteredText(*dc, wxT("UNIVERSITY OF CALIFORNIA MARCHING BAND"), pt);

	DrawCenteredText(*dc, GetNumber(), wxPoint(pageW-(96), 36));
	DrawCenteredText(*dc, GetNumber(), wxPoint(pageW-(96), 680));
	dc->DrawRectangle(pageW-(124), 674, 56, 28);

	dc->SetFont(*wxTheFontList->FindOrCreateFont(8, wxSWISS, wxNORMAL, wxNORMAL));

	DrawLineOverText(*dc, wxT("Music"), wxPoint(pageW/2, 60), 400);
	DrawLineOverText(*dc, wxT("Formation"), wxPoint(pageW/2, 82), 400);

	DrawLineOverText(*dc, wxT("game"), wxPoint(96, 54), 78);
	DrawLineOverText(*dc, wxT("page"), wxPoint(pageW-(96), 54), 78);

	// draw arrows
	pt = wxPoint(76, 80);
	DrawCenteredText(*dc, wxT("south"), pt-wxPoint(0,8));
	DrawArrow(*dc, pt, 40, false);
	pt = wxPoint(pageW-(76), 80);
	DrawCenteredText(*dc, wxT("north"), pt-wxPoint(0,8));
	DrawArrow(*dc, pt, 40, true);

	// draw arrows
	pt = wxPoint(76, 536);
	DrawCenteredText(*dc, wxT("south"), pt+wxPoint(0,8));
	DrawArrow(*dc, pt, 40, false);
	pt = wxPoint(pageW-(76), 536);
	DrawCenteredText(*dc, wxT("north"), pt+wxPoint(0,8));
	DrawArrow(*dc, pt, 40, true);

	DrawCenteredText(*dc, wxT("CAL SIDE"), wxPoint(pageW/2,544));

	dc->SetUserScale(origXscale, origYscale);
	dc->SetDeviceOrigin(origX, origY);

	DrawContLandscape(dc, 464);

	dc->SetFont(wxNullFont);
	
}

