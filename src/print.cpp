/*
 * print.cpp
 * Handles all postscript printing to a file stream
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

#include "confgr.h"
#include "modes.h"
#include "show.h"
#include "cc_show.h"
#include "cc_sheet.h"

#include "prolog0.h"
#include "prolog1.h"
#include "prolog2.h"
#include "setup0.h"
#include "setup1.h"
#include "setup2.h"
#include "zllrbach.h"

#include <time.h>
#include <string>
#include <sstream>

static float width, height, real_width, real_height;
static float field_x, field_y, field_w, field_h;
static float stage_field_x, stage_field_y, stage_field_w, stage_field_h;
static float step_size;
static float spr_step_size;
static short num_pages;
static short step_width, step_offset;
static Coord max_s, max_n, clip_s, clip_n;
static short split_sheet;

#define DPI 72

static const char *dot_routines[] =
{
	"dotplain",
	"dotsolid",
	"dotbs",
	"dotsl",
	"dotx",
	"dotsolbs",
	"dotsolsl",
	"dotsolx",
};

static const char *fontnames[] =
{
	"CalChart",
	"contfont",
	"boldfont",
	"italfont",
	"bolditalfont",
};

static const size_t kBufferSize = 256;

void gen_cont_line(std::ostream& buffer, const CC_textline& line, PSFONT_TYPE *currfontnum, float fontsize);
void print_start_page(std::ostream& buffer, bool landscape);

int CC_show::PrintShowToPS(std::ostream& buffer, bool eps, bool overview, unsigned curr_ss, int min_yards) const
{
	char buf[kBufferSize];
	time_t t;
	CC_coord fullsize = mode->Size();
	CC_coord fieldsize = mode->FieldSize();
	float fullwidth = Coord2Float(fullsize.x);
	float fullheight = Coord2Float(fullsize.y);
	float fieldwidth = Coord2Float(fieldsize.x);
	float fieldheight = Coord2Float(fieldsize.y);

	std::string head_font_str(GetConfiguration_HeadFont().utf8_str());
	std::string main_font_str(GetConfiguration_MainFont().utf8_str());
	std::string number_font_str(GetConfiguration_NumberFont().utf8_str());
	std::string cont_font_str(GetConfiguration_ContFont().utf8_str());
	std::string bold_font_str(GetConfiguration_BoldFont().utf8_str());
	std::string ital_font_str(GetConfiguration_ItalFont().utf8_str());
	std::string bold_ital_font_str(GetConfiguration_BoldItalFont().utf8_str());

	num_pages = 0;
/* first, calculate dimensions */
	if (!overview)
	{
		switch (mode->GetType())
		{
			case ShowMode::SHOW_STANDARD:
				if (print_landscape)
				{
					width = GetConfiguration_PageHeight() * DPI;
					if (print_do_cont)
					{
						height = GetConfiguration_PageWidth() * (1.0 - GetConfiguration_ContRatio()) * DPI;
						field_y = GetConfiguration_PageWidth() * GetConfiguration_ContRatio() * DPI;
					}
					else
					{
						height = GetConfiguration_PageWidth() * DPI;
						field_y = 0;
					}
				}
				else
				{
					width = GetConfiguration_PageWidth() * DPI;
					if (print_do_cont)
					{
						height = GetConfiguration_PageHeight() * (1.0 - GetConfiguration_ContRatio()) * DPI;
						field_y = GetConfiguration_PageHeight() * GetConfiguration_ContRatio() * DPI;
					}
					else
					{
						height = GetConfiguration_PageHeight() * DPI;
						field_y = 0;
					}
				}
				real_width = GetConfiguration_PageWidth() * DPI;
				real_height = GetConfiguration_PageHeight() * DPI;
				step_width = (short)(width / (height / (fullheight+8.0)));
				if (step_width > Coord2Int(fieldsize.x))
					step_width = Coord2Int(fieldsize.x);
				min_yards = min_yards*16/10;
				if (step_width > min_yards)
				{
/* We can get the minimum size */
					step_width = (step_width / 8) * 8;
					field_w = step_width * (height / (fullheight+8.0));
					field_h = height * (fieldheight/(fullheight+8.0));
					if ((field_w/step_width*(step_width+4)) > width)
					{
/* Oops, we didn't made it big enough */
						field_h *= width*step_width/(step_width+4)/field_w;
						field_w = width*step_width/(step_width+4);
					}
				}
				else
				{
/* Decrease height to get minimum size */
					field_w = width;
					field_h = width * fieldheight / min_yards;
					step_width = min_yards;
				}
				step_size = field_w/step_width;
				field_x = (width - field_w) / 2;
				field_y += height - (field_h + step_size*16);
				break;
			case ShowMode::SHOW_SPRINGSHOW:
				if (print_landscape)
				{
					width = GetConfiguration_PageHeight() * DPI;
					if (print_do_cont)
					{
						height = GetConfiguration_PageWidth() * (1.0 - GetConfiguration_ContRatio()) * DPI;
						field_y = GetConfiguration_PageWidth() * GetConfiguration_ContRatio() * DPI;
					}
					else
					{
						height = GetConfiguration_PageWidth() * DPI;
						field_y = 0;
					}
				}
				else
				{
					width = GetConfiguration_PageWidth() * DPI;
					if (print_do_cont)
					{
						height = GetConfiguration_PageHeight() * (1.0 - GetConfiguration_ContRatio()) * DPI;
						field_y = GetConfiguration_PageHeight() * GetConfiguration_ContRatio() * DPI;
					}
					else
					{
						height = GetConfiguration_PageHeight() * DPI;
						field_y = 0;
					}
				}
				real_width = GetConfiguration_PageWidth() * DPI;
				real_height = GetConfiguration_PageHeight() * DPI;
				field_h = height / (1 + 16*((ShowModeSprShow*)mode)->FieldW()/
					((float)((ShowModeSprShow*)mode)->StageW()*80));
				field_w = field_h/((ShowModeSprShow*)mode)->StageH() *
					((ShowModeSprShow*)mode)->StageW();
				if (field_w > width)
				{
/* reduce stage to fit on page */
					field_h *= width/field_w;
					field_w = width;
				}
				stage_field_w = field_w * ((ShowModeSprShow*)mode)->FieldW() /
					((ShowModeSprShow*)mode)->StageW();
				stage_field_h = field_h * ((ShowModeSprShow*)mode)->FieldH() /
					((ShowModeSprShow*)mode)->StageH();
				step_size = stage_field_w / ((ShowModeSprShow*)mode)->StepsW();
				spr_step_size = (short)(field_w/80.0);
				field_x = (width - field_w) / 2;
				field_y += height - (field_h + spr_step_size*16);
				stage_field_x = field_w * ((ShowModeSprShow*)mode)->FieldX() /
					((ShowModeSprShow*)mode)->StageW();
				stage_field_y = field_h * ((ShowModeSprShow*)mode)->FieldY() /
					((ShowModeSprShow*)mode)->StageH();
				break;
		}
	}
	else
	{
		if (print_landscape)
		{
			width = GetConfiguration_PageHeight() * DPI;
			height = GetConfiguration_PageWidth() * DPI;
		}
		else
		{
			width = GetConfiguration_PageWidth() * DPI;
			height = GetConfiguration_PageHeight() * DPI;
		}
		if ((width * (fullheight / fullwidth)) > height)
		{
			width = height * (fullwidth / fullheight);
		}
		else
		{
			height = width * (fullheight / fullwidth);
		}
		width--; height--;
		if (print_landscape)
		{
			real_width = height;
			real_height = width;
		}
		else
		{
			real_width = width;
			real_height = height;
		}
		field_x = 8.0 / fullwidth * width;
		field_y = 8.0 / fullheight * height;
		width *= fieldwidth/fullwidth;
		height *= fieldheight/fullheight;
	}

/* Now write postscript header */
	if (eps)
	{
		buffer<<"%!PS-Adobe-3.0 EPSF-3.0\n";
	}
	else
	{
		buffer<<"%!PS-Adobe-3.0\n";
	}
	snprintf(buf, sizeof(buf), "%%%%BoundingBox: %.0f %.0f %.0f %.0f\n",
		GetConfiguration_PageOffsetX() * DPI,
		(GetConfiguration_PaperLength() - GetConfiguration_PageOffsetY()) * DPI - real_height,
		GetConfiguration_PageOffsetX() * DPI + real_width,
		(GetConfiguration_PaperLength() - GetConfiguration_PageOffsetY()) * DPI);
	buffer<<buf;
	time(&t);
	buffer<<"%%CreationDate: "<<ctime(&t);
	buffer<<"%%Title: "<<std::string(GetTitle().utf8_str())<<"\n";
	buffer<<"%%Creator: CalChart\n";
	buffer<<"%%Pages: (atend)\n";
	buffer<<"%%PageOrder: Ascend\n";
	if (!overview)
	{
		snprintf(buf, sizeof(buf), "%%%%DocumentNeededResources: font %s %s %s %s %s %s %s\n",
			head_font_str.c_str(), main_font_str.c_str(),
			number_font_str.c_str(), cont_font_str.c_str(),
			bold_font_str.c_str(), ital_font_str.c_str(),
			bold_ital_font_str.c_str());
		buffer<<buf;
		snprintf(buf, sizeof(buf), "%%%%DocumentSuppliedResources: font CalChart\n");
		buffer<<buf;
		snprintf(buf, sizeof(buf), "%%%%BeginDefaults\n");
		buffer<<buf;
		snprintf(buf, sizeof(buf), "%%%%PageResources: font %s %s %s %s %s %s %s CalChart\n",
			head_font_str.c_str(), main_font_str.c_str(),
			number_font_str.c_str(), cont_font_str.c_str(),
			bold_font_str.c_str(), ital_font_str.c_str(),
			bold_ital_font_str.c_str());
		buffer<<buf;
		snprintf(buf, sizeof(buf), "%%%%EndDefaults\n");
		buffer<<buf;
	}
	snprintf(buf, sizeof(buf), "%%%%EndComments\n");
	buffer<<buf;
	if (!overview)
	{
		switch (mode->GetType())
		{
			case ShowMode::SHOW_STANDARD:
				snprintf(buf, sizeof(buf), "%%%%BeginProlog\n");
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/fieldw %.2f def\n", field_w);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/fieldh %.2f def\n", field_h);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/fieldy %.2f def\n", field_y);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/stepw %hd def\n", step_width);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/whash %hd def\n",
					((ShowModeStandard *)mode)->HashW());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/ehash %hd def\n",
					((ShowModeStandard *)mode)->HashE());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/headsize %.2f def\n", GetConfiguration_HeaderSize());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/yardsize %.2f def\n", GetConfiguration_YardsSize());
				buffer<<buf;
				// subtract 1 because we don't want to write the '\0' at the end
				buffer<<prolog0_ps;
				snprintf(buf, sizeof(buf), "%%%%EndProlog\n");
				buffer<<buf;
				snprintf(buf, sizeof(buf), "%%%%BeginSetup\n");
				buffer<<buf;
				snprintf(buf, sizeof(buf), "%%%%IncludeResources: font %s %s %s %s %s %s %s\n",
					head_font_str.c_str(), main_font_str.c_str(),
					number_font_str.c_str(), cont_font_str.c_str(),
					bold_font_str.c_str(), ital_font_str.c_str(),
					bold_ital_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/headfont0 /%s def\n", head_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/mainfont0 /%s def\n", main_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/numberfont0 /%s def\n", number_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/contfont0 /%s def\n", cont_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/boldfont0 /%s def\n", bold_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/italfont0 /%s def\n", ital_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/bolditalfont0 /%s def\n",
					bold_ital_font_str.c_str());
				buffer<<buf;
				// subtract 1 because we don't want to write the '\0' at the end
				buffer<<setup0_ps;
				buffer<<buf;
				snprintf(buf, sizeof(buf), "%%%%EndSetup\n");
				buffer<<buf;
				break;
			case ShowMode::SHOW_SPRINGSHOW:
				snprintf(buf, sizeof(buf), "%%%%BeginProlog\n");
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/fieldw %.2f def\n", field_w);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/fieldh %.2f def\n", field_h);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/fieldy %.2f def\n", field_y);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/sfieldw %.2f def\n", stage_field_w);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/sfieldh %.2f def\n", stage_field_h);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/sfieldx %.2f def\n", stage_field_x);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/sfieldy %.2f def\n", stage_field_y);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/stepsize %.6f def\n", step_size);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/sprstepsize %.6f def\n", spr_step_size);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/nfieldw %hd def\n",
					((ShowModeSprShow*)mode)->StepsW());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/nfieldh %hd def\n",
					((ShowModeSprShow*)mode)->StepsH());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/headsize %.2f def\n", GetConfiguration_HeaderSize());
				buffer<<buf;
				// subtract 1 because we don't want to write the '\0' at the end
				buffer<<prolog1_ps;
				snprintf(buf, sizeof(buf), "%%%%EndProlog\n");
				buffer<<buf;
				snprintf(buf, sizeof(buf), "%%%%BeginSetup\n");
				buffer<<buf;
				snprintf(buf, sizeof(buf), "%%%%IncludeResources: font %s %s %s %s %s %s %s\n",
					head_font_str.c_str(), main_font_str.c_str(),
					number_font_str.c_str(), cont_font_str.c_str(),
					bold_font_str.c_str(), ital_font_str.c_str(),
					bold_ital_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/headfont0 /%s def\n", head_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/mainfont0 /%s def\n", main_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/numberfont0 /%s def\n", number_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/contfont0 /%s def\n", cont_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/boldfont0 /%s def\n", bold_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/italfont0 /%s def\n", ital_font_str.c_str());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/bolditalfont0 /%s def\n",
					bold_ital_font_str.c_str());
				buffer<<buf;
				// subtract 1 because we don't want to write the '\0' at the end
				buffer<<setup1_ps;
				snprintf(buf, sizeof(buf), "%%%%EndSetup\n");
				buffer<<buf;
				break;
		}
	}
	else
	{
		snprintf(buf, sizeof(buf), "%%%%BeginProlog\n");
		buffer<<buf;
		snprintf(buf, sizeof(buf), "/whash %hd def\n",
			((ShowModeStandard *)mode)->HashW());
		buffer<<buf;
		snprintf(buf, sizeof(buf), "/ehash %hd def\n",
			((ShowModeStandard *)mode)->HashE());
		buffer<<buf;
		snprintf(buf, sizeof(buf), "/fieldw %.2f def\n", width);
		buffer<<buf;
		snprintf(buf, sizeof(buf), "/fieldh %.2f def\n", height);
		buffer<<buf;
		// subtract 1 because we don't want to write the '\0' at the end
		buffer<<prolog2_ps;
		snprintf(buf, sizeof(buf), "%%%%EndProlog\n");
		buffer<<buf;
		snprintf(buf, sizeof(buf), "%%%%BeginSetup\n");
		buffer<<buf;
		// subtract 1 because we don't want to write the '\0' at the end
		buffer<<setup2_ps;
		snprintf(buf, sizeof(buf), "%%%%EndSetup\n");
		buffer<<buf;
	}

/* print continuity sheets first */
	if (print_do_cont_sheet && !eps && !overview)
	{
		PrintSheets(buffer);
	}

/* do stuntsheet pages now */
	split_sheet = false;
	CC_show::const_CC_sheet_iterator_t curr_sheet = GetSheetBegin();
	if (eps)
	{
		curr_sheet += curr_ss;
	}
	while (curr_sheet != GetSheetEnd())
	{
		if (curr_sheet->IsPicked() || eps)
		{
			num_pages++;
			if (!overview)
			{
				switch (mode->GetType())
				{
					case ShowMode::SHOW_STANDARD:
						PrintStandard(buffer, *curr_sheet);
						break;
					case ShowMode::SHOW_SPRINGSHOW:
						PrintSpringshow(buffer, *curr_sheet);
						break;
				}
			}
			else
			{
				PrintOverview(buffer, *curr_sheet);
			}
			if (eps) break;
			else
			{
				snprintf(buf, sizeof(buf), "showpage\n");
				buffer<<buf;
			}
		}
		if (!split_sheet)
		{
			++curr_sheet;
		}
	}
/* finally, write trailer */
	snprintf(buf, sizeof(buf), "%%%%Trailer\n");
	buffer<<buf;
	snprintf(buf, sizeof(buf), "%%%%Pages: %hd\n", num_pages);
	buffer<<buf;
	snprintf(buf, sizeof(buf), "%%%%EOF\n");
	buffer<<buf;

	return num_pages;
}


void CC_show::PrintSheets(std::ostream& buffer) const
{
	char buf[kBufferSize];
	short lines_left = 0;
	bool need_eject = false;
	for (CC_show::const_CC_sheet_iterator_t sheet = GetSheetBegin(); sheet != GetSheetEnd(); ++sheet)
	{
		for (CC_textline_list::const_iterator text = sheet->continuity.begin();
			text != sheet->continuity.end();
			++text)
		{
			if (!text->on_main) continue;
			if (lines_left <= 0)
			{
				if (num_pages > 0)
				{
					buffer<<"showpage\n";
				}
				num_pages++;
				snprintf(buf, sizeof(buf), "%%%%Page: CONT%hd\n", num_pages);
				buffer<<buf;
				buffer<<"0 setgray\n";
				buffer<<"0.25 setlinewidth\n";
				snprintf(buf, sizeof(buf), "%.2f %.2f translate\n",
					GetConfiguration_PageOffsetX() * DPI,
					(GetConfiguration_PaperLength()-GetConfiguration_PageOffsetY())*DPI - real_height);
				buffer<<buf;
				lines_left = (short)(real_height / GetConfiguration_TextSize() - 0.5);
				snprintf(buf, sizeof(buf),
					"/contfont findfont %.2f scalefont setfont\n",
					GetConfiguration_TextSize());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/y %.2f def\n", real_height - GetConfiguration_TextSize());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/h %.2f def\n", GetConfiguration_TextSize());
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/lmargin 0 def /rmargin %.2f def\n",
					real_width);
				buffer<<buf;
				snprintf(buf, sizeof(buf), 
					"/tab1 %.2f def /tab2 %.2f def /tab3 %.2f def\n",
					real_width * 0.5 / 7.5, real_width * 1.5 / 7.5,
					real_width * 2.0 / 7.5);
				buffer<<buf;
				snprintf(buf, sizeof(buf), "/x lmargin def\n");
				buffer<<buf;
			}

			enum PSFONT_TYPE currfontnum = PSFONT_NORM;
			gen_cont_line(buffer, *text, &currfontnum, GetConfiguration_TextSize());

			buffer<<"/x lmargin def\n";
			buffer<<"/y y h sub def\n";
			lines_left--;
			need_eject = true;
		}
	}
	if (need_eject)
	{
		buffer<<"showpage\n";
	}
}


void PrintCont(std::ostream& buffer, const CC_sheet& sheet)
{
	char buf[kBufferSize];
	short cont_len = 0;

	float cont_height = field_y - step_size*10;
	for (CC_textline_list::const_iterator text = sheet.continuity.begin();
		text != sheet.continuity.end();
		++text)
	{
		if (text->on_sheet) cont_len++;
	}
	if (cont_len == 0) return;
	float this_size = cont_height / (cont_len + 0.5);
	if (this_size > GetConfiguration_TextSize()) this_size = GetConfiguration_TextSize();
	snprintf(buf, sizeof(buf), "/y %.2f def\n", cont_height - this_size);
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/h %.2f def\n", this_size);
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/lmargin 0 def /rmargin %.2f def\n", width);
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/tab1 %.2f def /tab2 %.2f def /tab3 %.2f def\n",
		width * 0.5 / 7.5, width * 1.5 / 7.5, width * 2.0 / 7.5);
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/contfont findfont %.2f scalefont setfont\n",
		this_size);
	buffer<<buf;
	for (CC_textline_list::const_iterator text = sheet.continuity.begin();
		text != sheet.continuity.end();
		++text)
	{
		if (!text->on_sheet) continue;
		snprintf(buf, sizeof(buf), "/x lmargin def\n");
		buffer<<buf;

		enum PSFONT_TYPE currfontnum = PSFONT_NORM;
		gen_cont_line(buffer, *text, &currfontnum, this_size);

		snprintf(buf, sizeof(buf), "/y y h sub def\n");
		buffer<<buf;
	}
}


void gen_cont_line(std::ostream& buffer, const CC_textline& line, PSFONT_TYPE *currfontnum,
float fontsize)
{
	char buf[kBufferSize];

	short tabstop = 0;
	for (CC_textchunk_list::const_iterator part = line.chunks.begin();
		part != line.chunks.end();
		++part)
	{
		if (part->font == PSFONT_TAB)
		{
			if (++tabstop > 3)
			{
				buffer<<"space_over\n";
			}
			else
			{
				buffer<<"tab"<<tabstop<<" do_tab\n";
			}
		}
		else
		{
			if (part->font != *currfontnum)
			{
				buffer<<"/"<<fontnames[part->font];
				snprintf(buf, sizeof(buf), " findfont %.2f scalefont setfont\n", fontsize);
				buffer<<buf;
				*currfontnum = part->font;
			}
			std::string textstr(part->text.utf8_str());
			const char *text = textstr.c_str();
			while (*text != 0)
			{
				std::string temp_buf = "";
				while (*text != 0)
				{
// Need backslash before parenthesis
					if ((*text == '(') || (*text == ')'))
					{
						temp_buf += "\\";
					}
					temp_buf += *(text++);
				}

				if (!temp_buf.empty())
				{
					if (line.center)
					{
						buffer<<"("<<temp_buf<<") centerText\n";
					}
					else
					{
						buffer<<"("<<temp_buf<<") leftText\n";
					}
				}
			}
		}
	}
}


void print_start_page(std::ostream& buffer, bool landscape)
{
	char buf[kBufferSize];

	buffer<<"0 setgray\n";
	buffer<<"0.25 setlinewidth\n";
	snprintf(buf, sizeof(buf), "%.2f %.2f translate\n",
		GetConfiguration_PageOffsetX() * DPI,
		(GetConfiguration_PaperLength() - GetConfiguration_PageOffsetY()) * DPI - real_height);
	buffer<<buf;
	if (landscape)
	{
		snprintf(buf, sizeof(buf), "%.2f 0 translate 90 rotate\n", real_width);
		buffer<<buf;
	}
}


void PrintStandard(std::ostream& buffer, const CC_sheet& sheet)
{
	char buf[kBufferSize];

	CC_coord fieldsize = sheet.show->GetMode().FieldSize();
	CC_coord fieldoff = sheet.show->GetMode().FieldOffset();
	Coord pmin = sheet.show->GetMode().MinPosition().x;
	Coord pmax = sheet.show->GetMode().MaxPosition().x;
	Coord fmin = sheet.show->GetMode().FieldOffset().x;
	Coord fmax = sheet.show->GetMode().FieldSize().x + fmin;

	std::string namestr(sheet.name.utf8_str());
	std::string numberstr(sheet.number.utf8_str());

	if (split_sheet)
	{
		buffer<<"%%Page: "<<namestr<<"(N)\n";
		if (!sheet.number.IsEmpty())
		{
			buffer<<"/pagenumtext ("<<numberstr<<"N) def\n";
		}
		else
		{
			buffer<<"/pagenumtext () def\n";
		}
		step_offset = Coord2Int(fieldsize.x) - step_width;
/* south yardline */
		clip_s = Int2Coord(step_offset) + fmin;
		clip_n = pmax;
		split_sheet = false;
	}
	else
	{
/* find bounds */
		max_s = fmax;
		max_n = fmin;
		for (unsigned i=0; i < sheet.show->GetNumPoints(); i++)
		{
			if (sheet.pts[i].pos.x < max_s) max_s = sheet.pts[i].pos.x;
			if (sheet.pts[i].pos.x > max_n) max_n = sheet.pts[i].pos.x;
		}
/* make sure bounds are on field */
		if (max_s < fmin) max_s = fmin;
		if (max_n > fmax) max_n = fmax;

		if (Coord2Int((max_n-max_s) +
			((max_s+fmin) % Int2Coord(8)))
			> step_width)
		{
/* Need to split into two pages */
			buffer<<"%%Page: "<<namestr<<"(S)\n";
			if (!sheet.number.IsEmpty())
			{
				buffer<<"/pagenumtext ("<<numberstr<<"S) def\n";
			}
			else
			{
				buffer<<"/pagenumtext () def\n";
			}
			step_offset = 0;
			split_sheet = true;
			clip_s = pmin;
			clip_n = Int2Coord(step_width) + fmin;/* north yardline */
		}
		else
		{
			buffer<<"%%Page: "<<namestr<<"\n";
			if (!sheet.number.IsEmpty())
			{
				buffer<<"/pagenumtext ("<<numberstr<<") def\n";
			}
			else
			{
				buffer<<"/pagenumtext () def\n";
			}
			step_offset = (Coord2Int(sheet.show->GetMode().FieldSize().x) - step_width) / 2;
			step_offset = (step_offset / 8) * 8;
			clip_s = pmin;
			clip_n = pmax;
			short x_s = Coord2Int(max_s) - Coord2Int(fmin);
			short x_n = Coord2Int(max_n) - Coord2Int(fmin);
			if ((x_s < step_offset) || (x_n > (step_offset + step_width)))
			{
/* Recenter formation */
				step_offset = x_s - (step_width-(x_n-x_s))/2;
				if (step_offset < 0) step_offset = 0;
				else if ((step_offset + step_width) >
					Coord2Int(sheet.show->GetMode().FieldSize().x))
					step_offset = Coord2Int(sheet.show->GetMode().FieldSize().x) - step_width;
				step_offset = (step_offset / 8) * 8;
			}
		}
	}
	print_start_page(buffer, sheet.show->GetBoolLandscape());

	snprintf(buf, sizeof(buf), "%.2f %.2f translate\n", field_x, field_y);
	buffer<<buf;

/* Draw field */
	buffer<<"drawfield\n";
	snprintf(buf, sizeof(buf), "/mainfont findfont %.2f scalefont setfont\n", step_size * GetConfiguration_YardsSize());
	buffer<<buf;

	for (short j=0; j <= step_width; j+=8)
	{
		snprintf(buf, sizeof(buf), "/lmargin %.2f def /rmargin %.2f def\n",
			step_size * j, step_size * j);
		buffer<<buf;
		snprintf(buf, sizeof(buf), "/y %.2f def\n", field_h + (step_size / 2));
		buffer<<buf;
		std::string yardstr(yard_text[(step_offset +
			(MAX_YARD_LINES-1)*4 +
			Coord2Int(fieldoff.x) + j)/8].utf8_str());
		buffer<<"("<<yardstr<<") dup centerText\n";
		snprintf(buf, sizeof(buf), "/y %.2f def\n", -(step_size * 2));
		buffer<<buf;
		snprintf(buf, sizeof(buf), "centerText\n");
		buffer<<buf;
	}


	float dot_w = step_size / 2 * GetConfiguration_DotRatio();
	snprintf(buf, sizeof(buf), "/w %.4f def\n", dot_w);
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/plinew %.4f def\n", dot_w * GetConfiguration_PLineRatio());
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/slinew %.4f def\n", dot_w * GetConfiguration_SLineRatio());
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/numberfont findfont %.2f scalefont setfont\n",
		dot_w * 2 * GetConfiguration_NumRatio());
	buffer<<buf;
	for (unsigned i = 0; i < sheet.show->GetNumPoints(); i++)
	{
		if ((sheet.pts[i].pos.x > clip_n) || (sheet.pts[i].pos.x < clip_s)) continue;
		float fieldheight = Coord2Float(fieldsize.y);
		float fieldoffx = Coord2Float(fieldoff.x);
		float fieldoffy = Coord2Float(fieldoff.y);
		float dot_x = (Coord2Float(sheet.pts[i].pos.x) - fieldoffx - step_offset) /
			step_width * field_w;
		float dot_y = (1.0 - (Coord2Float(sheet.pts[i].pos.y)-fieldoffy)/fieldheight)*field_h;
		snprintf(buf, sizeof(buf), "%.2f %.2f %s\n",
			dot_x, dot_y, dot_routines[sheet.pts[i].sym]);
		buffer<<buf;
		snprintf(buf, sizeof(buf), "(%s) %.2f %.2f %s\n",
			static_cast<const char*>(sheet.show->GetPointLabel(i).mb_str()), dot_x, dot_y,
			sheet.pts[i].GetFlip() ? "donumber2" : "donumber");
		buffer<<buf;
	}
	if (sheet.show->GetBoolDoCont())
	{
		snprintf(buf, sizeof(buf), "%.2f %.2f translate\n", -field_x, -field_y);
		buffer<<buf;
		PrintCont(buffer, sheet);
	}
}


void PrintSpringshow(std::ostream& buffer, const CC_sheet& sheet)
{
	char buf[kBufferSize];
	std::string namestr(sheet.name.utf8_str());
	std::string numberstr(sheet.number.utf8_str());

	buffer<<"%%Page: "<<namestr<<"\n";
	if (!sheet.number.IsEmpty())
	{
		buffer<<"/pagenumtext ("<<numberstr<<") def\n";
	}
	else
	{
		buffer<<"/pagenumtext () def\n";
	}

	print_start_page(buffer, sheet.show->GetBoolLandscape());

	snprintf(buf, sizeof(buf), "%.2f %.2f translate\n", field_x, field_y);
	buffer<<buf;

	const ShowModeSprShow *modesprshow = dynamic_cast<const ShowModeSprShow*>(&sheet.show->GetMode());
	buffer<<"   BeginEPSF\n";
	snprintf(buf, sizeof(buf), "   %.2f %.2f scale\n",
		field_w / modesprshow->StageW(),
		field_h / modesprshow->StageH());
	buffer<<buf;
	snprintf(buf, sizeof(buf), "   %hd %hd translate\n",
		static_cast<short>(-modesprshow->StageX()), static_cast<short>(-modesprshow->StageY()));
	buffer<<buf;
	buffer<<"%%BeginDocument: zllrbach.eps\n";
	// sprint show is special.  We put down the special stage:
	// subtract 1 because we don't want to write the '\0' at the end
	buffer<<zllrbach_eps;
	buffer<<"%%EndDocument\n";
	buffer<<"EndEPSF\n";
/* Draw field */
	buffer<<"drawfield\n";
	if (modesprshow->WhichYards())
	{
		snprintf(buf, sizeof(buf), "/mainfont findfont %.2f scalefont setfont\n",
			step_size * GetConfiguration_YardsSize());
		buffer<<buf;
		if (modesprshow->WhichYards() & (SPR_YARD_ABOVE | SPR_YARD_BELOW))
			for (short j=0; j <= modesprshow->StepsW(); j+=8)
		{
			snprintf(buf, sizeof(buf), "/lmargin %.2f def /rmargin %.2f def\n",
				stage_field_x + step_size * j,
				stage_field_x + step_size * j);
			buffer<<buf;
			if (modesprshow->WhichYards() & SPR_YARD_ABOVE)
			{
				snprintf(buf, sizeof(buf), "/y %.2f def\n",
					field_h*
					(modesprshow->TextTop()-modesprshow->StageX())/
					modesprshow->StageH());
				buffer<<buf;
				std::string yardstr(yard_text[(modesprshow->StepsX() +
					(MAX_YARD_LINES-1)*4 + j)/8].utf8_str());
				snprintf(buf, sizeof(buf), "(%s) centerText\n", yardstr.c_str());
				buffer<<buf;
			}
			if (modesprshow->WhichYards() & SPR_YARD_BELOW)
			{
				snprintf(buf, sizeof(buf), "/y %.2f def\n",
					field_h *
					(modesprshow->TextBottom() -
					modesprshow->StageX()) /
					modesprshow->StageH() -(step_size*GetConfiguration_YardsSize()));
				buffer<<buf;
				std::string yardstr(yard_text[(modesprshow->StepsX() +
					(MAX_YARD_LINES-1)*4 + j)/8].utf8_str());
				snprintf(buf, sizeof(buf), "(%s) centerText\n", yardstr.c_str());
				buffer<<buf;
			}
		}
		if (modesprshow->WhichYards() & (SPR_YARD_LEFT | SPR_YARD_RIGHT))
			for (short j=0; j <= modesprshow->StepsH(); j+=8)
		{
			snprintf(buf, sizeof(buf), "/y %.2f def\n",
				stage_field_y + stage_field_h *
				(modesprshow->StepsH()-j-GetConfiguration_YardsSize()/2) /
				modesprshow->StepsH());
			buffer<<buf;
			snprintf(buf, sizeof(buf), "/x %.2f def /rmargin %.2f def\n",
				field_w*
				(modesprshow->TextRight()-modesprshow->StageX()) /
				modesprshow->StageW(),
				field_w*
				(modesprshow->TextLeft()-modesprshow->StageX()) /
				modesprshow->StageW());
			buffer<<buf;
			std::string spr_text_str(spr_line_text[j / 8].utf8_str());
			if (modesprshow->WhichYards() & SPR_YARD_RIGHT)
			{
				buffer<<"("<<spr_text_str<<") leftText\n";
			}
			if (modesprshow->WhichYards() & SPR_YARD_LEFT)
			{
				buffer<<"("<<spr_text_str<<") rightText\n";
			}
		}
	}

	float dot_w = step_size / 2 * GetConfiguration_DotRatio();
	snprintf(buf, sizeof(buf), "/w %.4f def\n", dot_w);
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/plinew %.4f def\n", dot_w * GetConfiguration_PLineRatio());
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/slinew %.4f def\n", dot_w * GetConfiguration_SLineRatio());
	buffer<<buf;
	snprintf(buf, sizeof(buf), "/numberfont findfont %.2f scalefont setfont\n",
		dot_w * 2 * GetConfiguration_NumRatio());
	buffer<<buf;
	for (unsigned i = 0; i < sheet.show->GetNumPoints(); i++)
	{
		float dot_x = stage_field_x +
			(Coord2Float(sheet.pts[i].pos.x) - modesprshow->StepsX()) /
			modesprshow->StepsW() * stage_field_w;
		float dot_y = stage_field_y + stage_field_h * (1.0 -
			(Coord2Float(sheet.pts[i].pos.y)-
			modesprshow->StepsY())/
			modesprshow->StepsH());
		snprintf(buf, sizeof(buf), "%.2f %.2f %s\n",
			dot_x, dot_y, dot_routines[sheet.pts[i].sym]);
		buffer<<buf;
		snprintf(buf, sizeof(buf), "(%s) %.2f %.2f %s\n",
			static_cast<const char*>(sheet.show->GetPointLabel(i).mb_str()), dot_x, dot_y,
			(sheet.pts[i].GetFlip()) ? "donumber2" : "donumber");
		buffer<<buf;
	}
	if (sheet.show->GetBoolDoCont())
	{
		snprintf(buf, sizeof(buf), "%.2f %.2f translate\n", -field_x, -field_y);
		buffer<<buf;
		PrintCont(buffer, sheet);
	}
}


void PrintOverview(std::ostream& buffer, const CC_sheet& sheet)
{
	char buf[kBufferSize];
	buffer<<"%%Page: "<<std::string(sheet.name.utf8_str())<<"\n";

	print_start_page(buffer, sheet.show->GetBoolLandscape());
	snprintf(buf, sizeof(buf), "%.2f %.2f translate\n", field_x, field_y);
	buffer<<buf;
	buffer<<"drawfield\n";
	CC_coord fieldoff = sheet.show->GetMode().FieldOffset();
	CC_coord fieldsize = sheet.show->GetMode().FieldSize();
	float fieldwidth = Coord2Float(fieldsize.x);
	snprintf(buf, sizeof(buf), "/w %.2f def\n", width / fieldwidth * 2.0 / 3.0);
	buffer<<buf;

	float fieldx = Coord2Float(fieldoff.x);
	float fieldy = Coord2Float(fieldoff.y);
	float fieldheight = Coord2Float(fieldsize.y);

	for (unsigned i = 0; i < sheet.show->GetNumPoints(); i++)
	{
		snprintf(buf, sizeof(buf), "%.2f %.2f dotbox\n",
			(Coord2Float(sheet.pts[i].pos.x)-fieldx) / fieldwidth * width,
			(1.0 - (Coord2Float(sheet.pts[i].pos.y)-
			fieldy)/fieldheight) * height);
		buffer<<buf;
	}
}

