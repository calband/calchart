/*
 * draw.cpp
 * Member functions for drawing stuntsheets
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

#include "draw.h"

#include <wx/dc.h>
#include <wx/dcmemory.h>
#include "confgr.h"
#include "modes.h"
#include "calchartdoc.h"
#include "cc_show.h"
#include "cc_sheet.h"
#include "cc_point.h"
#include "cc_text.h"
#include "animate.h"
#include "animatecommand.h"
#include <memory>

extern wxFont *pointLabelFont;

// helper classes for saving and restoring state
class SaveAndRestore_DeviceOrigin
{
	wxDC& dc;
	wxCoord origX, origY;
public:
	SaveAndRestore_DeviceOrigin(wxDC& dc_) : dc(dc_) { dc.GetDeviceOrigin(&origX, &origY); }
	~SaveAndRestore_DeviceOrigin() { dc.SetDeviceOrigin(origX, origY); }
};

class SaveAndRestore_UserScale
{
	wxDC& dc;
	double origXscale, origYscale;
public:
	SaveAndRestore_UserScale(wxDC& dc_) : dc(dc_) { dc.GetUserScale(&origXscale, &origYscale); }
	~SaveAndRestore_UserScale() { dc.SetUserScale(origXscale, origYscale); }
};

class SaveAndRestore_TextForeground
{
	wxDC& dc;
	wxColour origForegroundColor;
public:
	SaveAndRestore_TextForeground(wxDC& dc_) : dc(dc_), origForegroundColor(dc.GetTextForeground()) {}
	~SaveAndRestore_TextForeground() { dc.SetTextForeground(origForegroundColor); }
};

class SaveAndRestore_Font
{
	wxDC& dc;
	wxFont origFont;
public:
	SaveAndRestore_Font(wxDC& dc_) : dc(dc_), origFont(dc.GetFont()) {}
	~SaveAndRestore_Font() { dc.SetFont(origFont); }
};

class SaveAndRestore_Brush
{
	wxDC& dc;
	wxBrush origBrush;
public:
	SaveAndRestore_Brush(wxDC& dc_) : dc(dc_), origBrush(dc.GetBrush()) {}
	~SaveAndRestore_Brush() { dc.SetBrush(origBrush); }
};

// draw text centered around x (though still at y down)
void DrawCenteredText(wxDC& dc, const wxString& text, const wxPoint& pt)
{
	wxCoord w, h;
	dc.GetTextExtent(text, &w, &h);
	w = std::max(0, pt.x - w/2);
	dc.DrawText(text, w, pt.y);
}

// draw text centered around x (though still at y down) with a line over it.
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

// calculate the distance for tab stops
size_t TabStops(size_t which, bool landscape)
{
	size_t tab = 0;
	while (which > 4)
	{
		which--;
		tab += (landscape)?8:6;
	}
	switch (which)
	{
		case 3:
			tab += 8;
		case 2:
			tab += (landscape)?18:14;
		case 1:
			tab += (landscape)?10:6;
	}
	return tab;
}

// these are the sizes that the page is set up to do.
static const double kBitmapScale = 2.0; // the factor to scale the bitmap
static const double kSizeX = 576, kSizeY = 734;
static const double kSizeXLandscape = 917, kSizeYLandscape = 720;
static const double kHeaderLocation[2][2] = { { 0.5, 18/kSizeY }, { 0.5, 22/kSizeYLandscape } };
static const wxString kHeader = wxT("UNIVERSITY OF CALIFORNIA MARCHING BAND");
static const double kUpperNumberPosition[2][2] = { { 1.0 - 62/kSizeX, 36/kSizeY }, { 1.0 - 96/kSizeXLandscape, 36/kSizeYLandscape } };
static const double kLowerNumberPosition[2][2] = { { 1.0 - 62/kSizeX, 714/kSizeY }, { 1.0 - 96/kSizeXLandscape, 680/kSizeYLandscape } };
static const double kLowerNumberBox[2][4] = { { 1.0 - 90/kSizeX, 708/kSizeY, 56/kSizeX, 22/kSizeY }, { 1.0 - 124/kSizeXLandscape, 674/kSizeYLandscape, 56/kSizeXLandscape, 28/kSizeYLandscape } };
static const double kMusicLabelPosition[2][3] = { { 0.5, 60/kSizeY, 240/kSizeX }, { 0.5, 60/kSizeYLandscape, 400/kSizeXLandscape } };
static const wxString kMusicLabel = wxT("Music");
static const double kFormationLabelPosition[2][3] = { { 0.5, 82/kSizeY, 240/kSizeX }, { 0.5, 82/kSizeYLandscape, 400/kSizeXLandscape } };
static const wxString kFormationLabel = wxT("Formation");

static const double kGameLabelPosition[2][3] = { { 62/kSizeX, 50/kSizeY, 64/kSizeX }, { 96/kSizeXLandscape, 54/kSizeYLandscape, 78/kSizeXLandscape } };
static const wxString kGameLabel = wxT("game");
static const double kPageLabelPosition[2][3] = { { 1.0 - 62/kSizeX, 50/kSizeY, 64/kSizeX }, { 1.0 - 96/kSizeXLandscape, 54/kSizeYLandscape, 78/kSizeXLandscape } };
static const wxString kPageLabel = wxT("page");
static const double kSideLabelPosition[2][2] = { { 0.5, 580/kSizeY }, { 0.5, 544/kSizeYLandscape } };
static const wxString kSideLabel = wxT("CAL SIDE");

static const double kUpperSouthPosition[2][2] = { { 52/kSizeX, (76-8)/kSizeY }, { 76/kSizeXLandscape, (80-8)/kSizeYLandscape } };
static const wxString kUpperSouthLabel = wxT("south");
static const double kUpperSouthArrow[2][3] = { { 52/kSizeX, (76)/kSizeY, 40/kSizeX }, { 76/kSizeXLandscape, (80)/kSizeYLandscape, 40/kSizeXLandscape } };
static const double kUpperNorthPosition[2][2] = { { 1.0 - 52/kSizeX, (76-8)/kSizeY }, { 1.0 - 76/kSizeXLandscape, (80-8)/kSizeYLandscape } };
static const wxString kUpperNorthLabel = wxT("north");
static const double kUpperNorthArrow[2][3] = { { 1.0 - 52/kSizeX, (76)/kSizeY, 40/kSizeX }, { 1.0 - 76/kSizeXLandscape, (80)/kSizeYLandscape, 40/kSizeXLandscape } };

static const double kLowerSouthPosition[2][2] = { { 52/kSizeX, (570+8)/kSizeY }, { 76/kSizeXLandscape, (536+8)/kSizeYLandscape } };
static const wxString kLowerSouthLabel = wxT("south");
static const double kLowerSouthArrow[2][3] = { { 52/kSizeX, (570)/kSizeY, 40/kSizeX }, { 76/kSizeXLandscape, (536)/kSizeYLandscape, 40/kSizeXLandscape } };
static const double kLowerNorthPosition[2][2] = { { 1.0 - 52/kSizeX, (570+8)/kSizeY }, { 1.0 - 76/kSizeXLandscape, (536+8)/kSizeYLandscape } };
static const wxString kLowerNorthLabel = wxT("north");
static const double kLowerNorthArrow[2][3] = { { 1.0 - 52/kSizeX, (570)/kSizeY, 40/kSizeX }, { 1.0 - 76/kSizeXLandscape, (536)/kSizeYLandscape, 40/kSizeXLandscape } };

static const double kContinuityStart[2] = { 606/kSizeY, 556/kSizeYLandscape };

void DrawSheetPoints(wxDC& dc, const CalChartDoc& show, const CC_sheet& sheet, unsigned ref, CalChartColors unselectedColor, CalChartColors selectedColor, CalChartColors unselectedTextColor, CalChartColors selectedTextColor)
{
	SaveAndRestore_Font orig_font(dc);
	wxFont *pointLabelFont = wxTheFontList->FindOrCreateFont((int)Float2Coord(GetConfiguration_DotRatio() * GetConfiguration_NumRatio()),
		wxSWISS, wxNORMAL, wxNORMAL);
	dc.SetFont(*pointLabelFont);
	CC_coord origin = show.GetMode().Offset();
	for (size_t i = 0; i < show.GetNumPoints(); i++)
	{
		if (show.IsSelected(i))
		{
			dc.SetPen(GetCalChartPen(selectedColor));
			dc.SetBrush(GetCalChartBrush(selectedColor));
			dc.SetTextForeground(GetCalChartPen(selectedTextColor).GetColour());
		}
		else
		{
			dc.SetPen(GetCalChartPen(unselectedColor));
			dc.SetBrush(GetCalChartBrush(unselectedColor));
			dc.SetTextForeground(GetCalChartPen(unselectedTextColor).GetColour());
		}
		DrawPoint(sheet.GetPoint(i), dc, ref, origin, show.GetPointLabel(i));
	}
}

void DrawGhostSheet(wxDC& dc, const CalChartDoc& show, const CC_sheet& sheet, unsigned ref) {
	DrawSheetPoints(dc, show, sheet, ref, COLOR_GHOST_POINT, COLOR_GHOST_POINT_HLIT, COLOR_GHOST_POINT_TEXT, COLOR_GHOST_POINT_HLIT_TEXT);
}

void Draw(wxDC& dc, const CalChartDoc& show, const CC_sheet& sheet, unsigned ref, bool primary)
{
	CalChartColors unselectedColor, selectedColor, unselectedTextColor, selectedTextColor;
	if (primary) {
		unselectedColor = COLOR_POINT;
		selectedColor = COLOR_POINT_HILIT;
		unselectedTextColor = COLOR_POINT_TEXT;
		selectedTextColor = COLOR_POINT_HILIT_TEXT;
	} else {
		unselectedColor = COLOR_REF_POINT;
		selectedColor = COLOR_REF_POINT_HILIT;
		unselectedTextColor = COLOR_REF_POINT_TEXT;
		selectedTextColor = COLOR_REF_POINT_HILIT_TEXT;
	}
	DrawSheetPoints(dc, show, sheet, ref, unselectedColor, selectedColor, unselectedTextColor, selectedTextColor);
}

// draw the continuity starting at a specific offset
void DrawCont(wxDC& dc, const CC_textline_list& print_continuity, const wxRect& bounding, bool landscape)
{
	SaveAndRestore_DeviceOrigin orig_dev(dc);
	SaveAndRestore_UserScale orig_scale(dc);
	SaveAndRestore_TextForeground orig_text(dc);
	SaveAndRestore_Font orig_font(dc);
#if DEBUG
	dc.DrawRectangle(bounding);
#endif

	dc.SetTextForeground(*wxBLACK);
	
	int pageMiddle = (bounding.GetWidth()/2);

	size_t numLines = 0;
	for (auto text = print_continuity.begin(); text != print_continuity.end(); ++text)
	{
		if (text->GetOnSheet())
		{
			++numLines;
		}
	}

	int font_size = ((bounding.GetBottom() - bounding.GetTop()) - (numLines - 1)*2)/(numLines ? numLines : 1);
	//font size, we scale to be no more than 256 pixels.
	font_size = std::min(font_size, 10);

	wxFont *contPlainFont = wxTheFontList->FindOrCreateFont(font_size, wxMODERN, wxNORMAL, wxNORMAL);
	wxFont *contBoldFont = wxTheFontList->FindOrCreateFont(font_size, wxMODERN, wxNORMAL, wxBOLD);
	wxFont *contItalFont = wxTheFontList->FindOrCreateFont(font_size, wxMODERN, wxITALIC, wxNORMAL);
	wxFont *contBoldItalFont = wxTheFontList->FindOrCreateFont(font_size, wxMODERN, wxITALIC, wxBOLD);
	
	dc.SetFont(*contPlainFont);
	const wxCoord maxtexth = contPlainFont->GetPointSize()+2;

	float y = bounding.GetTop();
	const wxCoord charWidth = dc.GetCharWidth();
	for (auto& cont : print_continuity)
	{
		float x = bounding.GetLeft();
		// Determine how to center the text
		if (cont.GetCenter())
		{
			x += pageMiddle;
			auto chunks = cont.GetChunks();
			for (auto& c : chunks)
			{
				bool do_tab = false;
				switch (c.font)
				{
					case PSFONT_SYMBOL:
					{
						wxCoord textw, texth;
						dc.GetTextExtent(wxT("O"), &textw, &texth);
						x += textw * c.text.length();
					}
						break;
					case PSFONT_NORM:
						dc.SetFont(*contPlainFont);
						break;
					case PSFONT_BOLD:
						dc.SetFont(*contBoldFont);
						break;
					case PSFONT_ITAL:
						dc.SetFont(*contItalFont);
						break;
					case PSFONT_BOLDITAL:
						dc.SetFont(*contBoldItalFont);
						break;
					case PSFONT_TAB:
						do_tab = true;
						break;
				}
				if (!do_tab && (c.font != PSFONT_SYMBOL))
				{
					wxCoord textw, texth;
					dc.GetTextExtent(c.text, &textw, &texth);
					x -= textw/2;
				}
			}
		}
		// now draw the text
		unsigned tabnum = 0;
		auto chunks = cont.GetChunks();
		for (auto& c : chunks)
		{
			bool do_tab = false;
			switch (c.font)
			{
				case PSFONT_NORM:
				case PSFONT_SYMBOL:
					dc.SetFont(*contPlainFont);
					break;
				case PSFONT_BOLD:
					dc.SetFont(*contBoldFont);
					break;
				case PSFONT_ITAL:
					dc.SetFont(*contItalFont);
					break;
				case PSFONT_BOLDITAL:
					dc.SetFont(*contBoldItalFont);
					break;
				case PSFONT_TAB:
				{
					tabnum++;
					wxCoord textw = bounding.GetLeft() + charWidth * TabStops(tabnum, landscape);
					if (textw >= x) x = textw;
					else x += charWidth;
					do_tab = true;
				}
					break;
				default:
					break;
			}
			if (c.font == PSFONT_SYMBOL)
			{
				wxCoord textw, texth, textd;
				dc.GetTextExtent(wxT("O"), &textw, &texth, &textd);
				const float d = textw;
				const float top_y = y + texth - textd - textw;

				for (std::string::const_iterator s = c.text.begin(); s != c.text.end(); s++)
				{
					{
						dc.SetPen(*wxBLACK_PEN);
						SYMBOL_TYPE sym = (SYMBOL_TYPE)(*s - 'A');
						switch (sym)
						{
							case SYMBOL_SOL:
							case SYMBOL_SOLBKSL:
							case SYMBOL_SOLSL:
							case SYMBOL_SOLX:
								dc.SetBrush(*wxBLACK_BRUSH);
								break;
							default:
								dc.SetBrush(*wxTRANSPARENT_BRUSH);
						}
						dc.DrawEllipse(x, top_y, d, d);
						switch (sym)
						{
							case SYMBOL_SL:
							case SYMBOL_X:
							case SYMBOL_SOLSL:
							case SYMBOL_SOLX:
								dc.DrawLine(x-1, top_y + d+1, x + d+1, top_y-1);
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
								dc.DrawLine(x-1, top_y-1, x + d+1, top_y + d+1);
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
					wxCoord textw, texth;
					dc.GetTextExtent(c.text, &textw, &texth);
					dc.DrawText(c.text, x, y);
					x += textw;
				}
			}
		}
		y += maxtexth;
	}
#if DEBUG
	char buffer[100];
	snprintf(buffer, sizeof(buffer), "TopLeft %d, %d", bounding.GetTopLeft().x, bounding.GetTopLeft().y);
	dc.DrawText(buffer, bounding.GetTopLeft());
	snprintf(buffer, sizeof(buffer), "BottomRight %d, %d", bounding.GetBottomRight().x, bounding.GetBottomRight().y);
	dc.DrawText(buffer, bounding.GetBottomRight());
#endif
}

static std::auto_ptr<ShowMode> CreateFieldForPrinting(bool landscape)
{
	CC_coord bord1(Int2Coord(8),Int2Coord(8)), bord2(Int2Coord(8),Int2Coord(8));
	CC_coord siz, off;
	uint32_t whash = 32;
	uint32_t ehash = 52;
	bord1.x = Int2Coord((landscape)?12:6);
	bord1.y = Int2Coord((landscape)?21:19);
	bord2.x = Int2Coord((landscape)?12:6);
	bord2.y = Int2Coord((landscape)?12:6);
	siz.x = Int2Coord((landscape)?160:96);
	siz.y = Int2Coord(84);
	off.x = Int2Coord((landscape)?80:48);
	off.y = Int2Coord(42);

	return std::auto_ptr<ShowMode>(new ShowModeStandard(wxT("Standard"), siz, off, bord1, bord2, whash, ehash));
}

void DrawForPrinting(wxDC *printerdc, const CalChartDoc& show, const CC_sheet& sheet, unsigned ref, bool landscape)
{
	unsigned short i;
	unsigned long x, y;
	float circ_r;
	CC_coord origin;

	std::auto_ptr<wxBitmap> membm(new wxBitmap((landscape?kSizeXLandscape:kSizeX)*kBitmapScale, (landscape?kSizeYLandscape:kSizeY)*kBitmapScale));
	std::auto_ptr<wxDC> dc(new wxMemoryDC(*membm));
	wxCoord origX, origY;
	double origXscale, origYscale;
	dc->GetDeviceOrigin(&origX, &origY);
	dc->GetUserScale(&origXscale, &origYscale);

	if (landscape)
	{
		dc->SetDeviceOrigin(10, 0);
	}

	// create a field for drawing:
	std::auto_ptr<ShowMode> mode = CreateFieldForPrinting(landscape);

	int pageW, pageH;
	dc->GetSize(&pageW, &pageH);

	// set the scaling for drawing the field
	dc->SetUserScale(pageW/(double)mode->Size().x, pageW/(double)mode->Size().x);

	// draw the field.
	dc->Clear();
	dc->SetPen(*wxBLACK_PEN);
	dc->SetTextForeground(*wxBLACK);
	dc->SetLogicalFunction(wxCOPY);
	mode->Draw(*dc);

	const std::vector<CC_point> pts = sheet.GetPoints();
	if (!pts.empty())
	{
		wxFont *pointLabelFont = wxTheFontList->FindOrCreateFont((int)Float2Coord(GetConfiguration_DotRatio() * GetConfiguration_NumRatio()),
																 wxSWISS, wxNORMAL, wxNORMAL);
		dc->SetFont(*pointLabelFont);
		circ_r = Float2Coord(GetConfiguration_DotRatio());
		const float offset = circ_r / 2;
		const float plineoff = offset * GetConfiguration_PLineRatio();
		const float slineoff = offset * GetConfiguration_SLineRatio();
		const float textoff = offset * 1.25;
		origin = mode->Offset();
		for (int selectd = 0; selectd < 2; selectd++)
		{
			for (i = 0; i < pts.size(); i++)
			{
				x = pts.at(i).GetPos(ref).x+origin.x;
				y = pts.at(i).GetPos(ref).y+origin.y;
				switch (pts.at(i).GetSymbol())
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
				switch (pts.at(i).GetSymbol())
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
				switch (pts.at(i).GetSymbol())
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
				wxCoord textw, texth, textd;
				dc->GetTextExtent(show.GetPointLabel(i), &textw, &texth, &textd);
				dc->DrawText(show.GetPointLabel(i),
					pts.at(i).GetFlip() ? x : (x - textw),
					y - textoff - texth + textd);
			}
		}
	}

	dc->SetBrush(*wxTRANSPARENT_BRUSH);
	// set the page for drawing:
	dc->GetSize(&pageW, &pageH);
	if (landscape)
	{
		dc->SetUserScale(pageW/kSizeXLandscape, pageH/kSizeYLandscape);
		pageW = kSizeXLandscape;
		pageH = kSizeYLandscape;
	}
	else
	{
		dc->SetUserScale(pageW/kSizeX, pageH/kSizeY);
		pageW = kSizeX;
		pageH = kSizeY;
	}

	// draw the header
	dc->SetFont(*wxTheFontList->FindOrCreateFont(16, wxROMAN, wxNORMAL, wxBOLD));
	dc->SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 1, wxSOLID));

	DrawCenteredText(*dc, kHeader, wxPoint(pageW*kHeaderLocation[landscape][0], pageH*kHeaderLocation[landscape][1]));

	DrawCenteredText(*dc, sheet.GetNumber(), wxPoint(pageW*kUpperNumberPosition[landscape][0], pageH*kUpperNumberPosition[landscape][1]));
	DrawCenteredText(*dc, sheet.GetNumber(), wxPoint(pageW*kLowerNumberPosition[landscape][0], pageH*kLowerNumberPosition[landscape][1]));
	dc->DrawRectangle(pageW*kLowerNumberBox[landscape][0], pageH*kLowerNumberBox[landscape][1], pageW*kLowerNumberBox[landscape][2], pageH*kLowerNumberBox[landscape][3]);

	dc->SetFont(*wxTheFontList->FindOrCreateFont(8, wxSWISS, wxNORMAL, wxNORMAL));

	DrawLineOverText(*dc, kMusicLabel, wxPoint(pageW*kMusicLabelPosition[landscape][0], pageH*kMusicLabelPosition[landscape][1]), pageW*kMusicLabelPosition[landscape][2]);
	DrawLineOverText(*dc, kFormationLabel, wxPoint(pageW*kFormationLabelPosition[landscape][0], pageH*kFormationLabelPosition[landscape][1]), pageW*kFormationLabelPosition[landscape][2]);
	DrawLineOverText(*dc, kGameLabel, wxPoint(pageW*kGameLabelPosition[landscape][0], pageH*kGameLabelPosition[landscape][1]), pageW*kGameLabelPosition[landscape][2]);
	DrawLineOverText(*dc, kPageLabel, wxPoint(pageW*kPageLabelPosition[landscape][0], pageH*kPageLabelPosition[landscape][1]), pageW*kPageLabelPosition[landscape][2]);
	DrawCenteredText(*dc, kSideLabel, wxPoint(pageW*kSideLabelPosition[landscape][0], pageH*kSideLabelPosition[landscape][1]));

	// draw arrows
	DrawCenteredText(*dc, kUpperSouthLabel, wxPoint(pageW*kUpperSouthPosition[landscape][0], pageH*kUpperSouthPosition[landscape][1]));
	DrawArrow(*dc, wxPoint(pageW*kUpperSouthArrow[landscape][0], pageH*kUpperSouthArrow[landscape][1]), pageW*kUpperSouthArrow[landscape][2], false);
	DrawCenteredText(*dc, kUpperNorthLabel, wxPoint(pageW*kUpperNorthPosition[landscape][0], pageH*kUpperNorthPosition[landscape][1]));
	DrawArrow(*dc, wxPoint(pageW*kUpperNorthArrow[landscape][0], pageH*kUpperNorthArrow[landscape][1]), pageW*kUpperNorthArrow[landscape][2], true);
	DrawCenteredText(*dc, kLowerSouthLabel, wxPoint(pageW*kLowerSouthPosition[landscape][0], pageH*kLowerSouthPosition[landscape][1]));
	DrawArrow(*dc, wxPoint(pageW*kLowerSouthArrow[landscape][0], pageH*kLowerSouthArrow[landscape][1]), pageW*kLowerSouthArrow[landscape][2], false);
	DrawCenteredText(*dc, kLowerNorthLabel, wxPoint(pageW*kLowerNorthPosition[landscape][0], pageH*kLowerNorthPosition[landscape][1]));
	DrawArrow(*dc, wxPoint(pageW*kLowerNorthArrow[landscape][0], pageH*kLowerNorthArrow[landscape][1]), pageW*kLowerNorthArrow[landscape][2], true);

	DrawCont(*dc, sheet.GetPrintableContinuity(), wxRect(wxPoint(10, pageH*kContinuityStart[landscape]), wxSize(pageW-20, pageH-pageH*kContinuityStart[landscape])), landscape);

	dc->SetUserScale(origXscale, origYscale);
	dc->SetDeviceOrigin(origX, origY);

	dc->SetFont(wxNullFont);
	
	int printerW, printerH;
	printerdc->GetSize(&printerW, &printerH);
	printerdc->SetUserScale(printerW/((landscape?kSizeXLandscape:kSizeX)*kBitmapScale), printerH/((landscape?kSizeYLandscape:kSizeY)*kBitmapScale));
	printerdc->Blit(0, 0, membm->GetWidth(), membm->GetHeight(), dc.get(), 0, 0);
}

void
DrawPointHelper(const CC_coord& pos, const CC_point& point, wxDC& dc, const wxString& label)
{
	SaveAndRestore_Brush restore(dc);
	float circ_r = Float2Coord(GetConfiguration_DotRatio());
	float offset = circ_r / 2;
	float plineoff = offset * GetConfiguration_PLineRatio();
	float slineoff = offset * GetConfiguration_SLineRatio();
	float textoff = offset * 1.25;
	
	long x = pos.x;
	long y = pos.y;
	switch (point.GetSymbol())
	{
		case SYMBOL_SOL:
		case SYMBOL_SOLBKSL:
		case SYMBOL_SOLSL:
		case SYMBOL_SOLX:
			break;
		default:
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
	}
	dc.DrawEllipse(x - offset, y - offset, circ_r, circ_r);
	switch (point.GetSymbol())
	{
		case SYMBOL_SL:
		case SYMBOL_X:
			dc.DrawLine(x - plineoff, y + plineoff,
						x + plineoff, y - plineoff);
			break;
		case SYMBOL_SOLSL:
		case SYMBOL_SOLX:
			dc.DrawLine(x - slineoff, y + slineoff,
						x + slineoff, y - slineoff);
			break;
		default:
			break;
	}
	switch (point.GetSymbol())
	{
		case SYMBOL_BKSL:
		case SYMBOL_X:
			dc.DrawLine(x - plineoff, y - plineoff,
						x + plineoff, y + plineoff);
			break;
		case SYMBOL_SOLBKSL:
		case SYMBOL_SOLX:
			dc.DrawLine(x - slineoff, y - slineoff,
						x + slineoff, y + slineoff);
			break;
		default:
			break;
	}
	if (point.LabelIsVisible()) {
		wxCoord textw, texth, textd;
		dc.GetTextExtent(label, &textw, &texth, &textd);
		dc.DrawText(label,
			point.GetFlip() ? x : (x - textw),
			y - textoff - texth + textd);
	}
}

void
DrawPoint(const CC_point& point, wxDC& dc, unsigned reference, const CC_coord& origin, const wxString& label)
{
	DrawPointHelper(point.GetPos(reference) + origin, point, dc, label);
}

void
DrawPhatomPoints(wxDC& dc, const CalChartDoc& show, const CC_sheet& sheet, const std::map<unsigned, CC_coord>& positions)
{
	SaveAndRestore_Font orig_font(dc);
	wxFont *pointLabelFont = wxTheFontList->FindOrCreateFont((int)Float2Coord(GetConfiguration_DotRatio() * GetConfiguration_NumRatio()), wxSWISS, wxNORMAL, wxNORMAL);
	dc.SetFont(*pointLabelFont);
	CC_coord origin = show.GetMode().Offset();
	dc.SetPen(GetCalChartPen(COLOR_GHOST_POINT));
	dc.SetBrush(GetCalChartBrush(COLOR_GHOST_POINT));
	dc.SetTextForeground(GetCalChartPen(COLOR_GHOST_POINT_TEXT).GetColour());
	
	for (auto& i : positions)
	{
		DrawPointHelper(i.second + origin, sheet.GetPoint(i.first), dc, show.GetPointLabel(i.first));
	}
}

void DrawCC_DrawCommandList(wxDC& dc, const std::vector<CC_DrawCommand>& draw_commands)
{
	for (auto iter = draw_commands.begin(); iter != draw_commands.end(); ++iter)
	{
		switch (iter->mType)
		{
			case CC_DrawCommand::Line:
				dc.DrawLine(iter->x1, iter->y1, iter->x2, iter->y2);
				break;
			case CC_DrawCommand::Arc:
				dc.DrawArc(iter->x1, iter->y1, iter->x2, iter->y2, iter->xc, iter->yc);
				break;
			case CC_DrawCommand::Ignore:
				break;
		}
	}
}

void DrawPath(wxDC& dc, const std::vector<CC_DrawCommand>& draw_commands, const CC_coord& end)
{
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(GetCalChartPen(COLOR_PATHS));
	DrawCC_DrawCommandList(dc, draw_commands);
	dc.SetBrush(GetCalChartBrush(COLOR_PATHS));
	float circ_r = Float2Coord(GetConfiguration_DotRatio());
	dc.DrawEllipse(end.x - circ_r/2, end.y - circ_r/2, circ_r, circ_r);
}

//void DrawShape(wxDC& dc, const CC_shape& shape, float x, float y)
//{
//	DrawCC_DrawCommandList(dc, shape.GetCC_DrawCommand(x, y));
//}


