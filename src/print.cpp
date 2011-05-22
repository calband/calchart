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

#include <time.h>
#include <string>

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

static const wxChar *nofile = wxT("Unable to open print config files");
static const wxChar *badfile = wxT("Error writing to print file");

static const wxChar *gen_cont_line(const CC_textline& line, PSFONT_TYPE *currfontnum,
float fontsize, FILE *fp);
static const wxChar *print_start_page(FILE *fp, bool landscape);
static bool copy_ps_file(const wxString& name, FILE *fp);

#define CHECKPRINT0(a) if ((a) < 0) { error = badfile; return 0; }
#define CHECKPRINT1(a) if ((a) < 0) { error = badfile; return; }
#define CHECKPRINT(a) if ((a) < 0) return badfile;

int CC_show::Print(FILE *fp, bool eps, bool overview, unsigned curr_ss,
int min_yards) const
{
	time_t t;
	CC_coord fullsize = mode->Size();
	CC_coord fieldsize = mode->FieldSize();
	float fullwidth = COORD2FLOAT(fullsize.x);
	float fullheight = COORD2FLOAT(fullsize.y);
	float fieldwidth = COORD2FLOAT(fieldsize.x);
	float fieldheight = COORD2FLOAT(fieldsize.y);

	std::string head_font_str(head_font.utf8_str());
	std::string main_font_str(main_font.utf8_str());
	std::string number_font_str(number_font.utf8_str());
	std::string cont_font_str(cont_font.utf8_str());
	std::string bold_font_str(bold_font.utf8_str());
	std::string ital_font_str(ital_font.utf8_str());
	std::string bold_ital_font_str(bold_ital_font.utf8_str());

	num_pages = 0;
/* first, calculate dimensions */
	if (!overview)
	{
		switch (mode->GetType())
		{
			case SHOW_STANDARD:
				if (print_landscape)
				{
					width = page_height * DPI;
					if (print_do_cont)
					{
						height = page_width * (1.0 - cont_ratio) * DPI;
						field_y = page_width * cont_ratio * DPI;
					}
					else
					{
						height = page_width * DPI;
						field_y = 0;
					}
				}
				else
				{
					width = page_width * DPI;
					if (print_do_cont)
					{
						height = page_height * (1.0 - cont_ratio) * DPI;
						field_y = page_height * cont_ratio * DPI;
					}
					else
					{
						height = page_height * DPI;
						field_y = 0;
					}
				}
				real_width = page_width * DPI;
				real_height = page_height * DPI;
				step_width = (short)(width / (height / (fullheight+8.0)));
				if (step_width > COORD2INT(fieldsize.x))
					step_width = COORD2INT(fieldsize.x);
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
			case SHOW_SPRINGSHOW:
				if (print_landscape)
				{
					width = page_height * DPI;
					if (print_do_cont)
					{
						height = page_width * (1.0 - cont_ratio) * DPI;
						field_y = page_width * cont_ratio * DPI;
					}
					else
					{
						height = page_width * DPI;
						field_y = 0;
					}
				}
				else
				{
					width = page_width * DPI;
					if (print_do_cont)
					{
						height = page_height * (1.0 - cont_ratio) * DPI;
						field_y = page_height * cont_ratio * DPI;
					}
					else
					{
						height = page_height * DPI;
						field_y = 0;
					}
				}
				real_width = page_width * DPI;
				real_height = page_height * DPI;
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
			width = page_height * DPI;
			height = page_width * DPI;
		}
		else
		{
			width = page_width * DPI;
			height = page_height * DPI;
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
		CHECKPRINT0(fprintf(fp, "%%!PS-Adobe-3.0 EPSF-3.0\n"));
	}
	else
	{
		CHECKPRINT0(fprintf(fp, "%%!PS-Adobe-3.0\n"));
	}
	CHECKPRINT0(fprintf(fp, "%%%%BoundingBox: %.0f %.0f %.0f %.0f\n",
		page_offset_x * DPI,
		(paper_length - page_offset_y) * DPI - real_height,
		page_offset_x * DPI + real_width,
		(paper_length - page_offset_y) * DPI));
	time(&t);
	CHECKPRINT0(fprintf(fp, "%%%%CreationDate: %s", ctime(&t)));
	std::string namestr(GetTitle().utf8_str());
	CHECKPRINT0(fprintf(fp, "%%%%Title: %s\n", namestr.c_str()));
	CHECKPRINT0(fprintf(fp, "%%%%Creator: CalChart\n"));
	CHECKPRINT0(fprintf(fp, "%%%%Pages: (atend)\n"));
	CHECKPRINT0(fprintf(fp, "%%%%PageOrder: Ascend\n"));
	if (!overview)
	{
		CHECKPRINT0(fprintf(fp,"%%%%DocumentNeededResources: font %s %s %s %s %s %s %s\n",
			head_font_str.c_str(), main_font_str.c_str(),
			number_font_str.c_str(), cont_font_str.c_str(),
			bold_font_str.c_str(), ital_font_str.c_str(),
			bold_ital_font_str.c_str()));
		CHECKPRINT0(fprintf(fp, "%%%%DocumentSuppliedResources: font CalChart\n"));
		CHECKPRINT0(fprintf(fp, "%%%%BeginDefaults\n"));
		CHECKPRINT0(fprintf(fp, "%%%%PageResources: font %s %s %s %s %s %s %s CalChart\n",
			head_font_str.c_str(), main_font_str.c_str(),
			number_font_str.c_str(), cont_font_str.c_str(),
			bold_font_str.c_str(), ital_font_str.c_str(),
			bold_ital_font_str.c_str()));
		CHECKPRINT0(fprintf(fp, "%%%%EndDefaults\n"));
	}
	CHECKPRINT0(fprintf(fp, "%%%%EndComments\n"));
	if (!overview)
	{
		switch (mode->GetType())
		{
			case SHOW_STANDARD:
				CHECKPRINT0(fprintf(fp, "%%%%BeginProlog\n"));
				CHECKPRINT0(fprintf(fp, "/fieldw %.2f def\n", field_w));
				CHECKPRINT0(fprintf(fp, "/fieldh %.2f def\n", field_h));
				CHECKPRINT0(fprintf(fp, "/fieldy %.2f def\n", field_y));
				CHECKPRINT0(fprintf(fp, "/stepw %hd def\n", step_width));
				CHECKPRINT0(fprintf(fp, "/whash %hd def\n",
					((ShowModeStandard *)mode)->HashW()));
				CHECKPRINT0(fprintf(fp, "/ehash %hd def\n",
					((ShowModeStandard *)mode)->HashE()));
				CHECKPRINT0(fprintf(fp, "/headsize %.2f def\n", header_size));
				CHECKPRINT0(fprintf(fp, "/yardsize %.2f def\n", yards_size));
				// subtract 1 because we don't want to write the '\0' at the end
				CHECKPRINT0(fwrite(prolog0_ps, sizeof(char), sizeof(prolog0_ps)-1, fp));
				CHECKPRINT0(fprintf(fp, "%%%%EndProlog\n"));
				CHECKPRINT0(fprintf(fp, "%%%%BeginSetup\n"));
				CHECKPRINT0(fprintf(fp, "%%%%IncludeResources: font %s %s %s %s %s %s %s\n",
					head_font_str.c_str(), main_font_str.c_str(),
					number_font_str.c_str(), cont_font_str.c_str(),
					bold_font_str.c_str(), ital_font_str.c_str(),
					bold_ital_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/headfont0 /%s def\n", head_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/mainfont0 /%s def\n", main_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/numberfont0 /%s def\n", number_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/contfont0 /%s def\n", cont_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/boldfont0 /%s def\n", bold_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/italfont0 /%s def\n", ital_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/bolditalfont0 /%s def\n",
					bold_ital_font_str.c_str()));
				// subtract 1 because we don't want to write the '\0' at the end
				CHECKPRINT0(fwrite(setup0_ps, sizeof(char), sizeof(setup0_ps)-1, fp));
				CHECKPRINT0(fprintf(fp, "%%%%EndSetup\n"));
				break;
			case SHOW_SPRINGSHOW:
				CHECKPRINT0(fprintf(fp, "%%%%BeginProlog\n"));
				CHECKPRINT0(fprintf(fp, "/fieldw %.2f def\n", field_w));
				CHECKPRINT0(fprintf(fp, "/fieldh %.2f def\n", field_h));
				CHECKPRINT0(fprintf(fp, "/fieldy %.2f def\n", field_y));
				CHECKPRINT0(fprintf(fp, "/sfieldw %.2f def\n", stage_field_w));
				CHECKPRINT0(fprintf(fp, "/sfieldh %.2f def\n", stage_field_h));
				CHECKPRINT0(fprintf(fp, "/sfieldx %.2f def\n", stage_field_x));
				CHECKPRINT0(fprintf(fp, "/sfieldy %.2f def\n", stage_field_y));
				CHECKPRINT0(fprintf(fp, "/stepsize %.6f def\n", step_size));
				CHECKPRINT0(fprintf(fp, "/sprstepsize %.6f def\n", spr_step_size));
				CHECKPRINT0(fprintf(fp, "/nfieldw %hd def\n",
					((ShowModeSprShow*)mode)->StepsW()));
				CHECKPRINT0(fprintf(fp, "/nfieldh %hd def\n",
					((ShowModeSprShow*)mode)->StepsH()));
				CHECKPRINT0(fprintf(fp, "/headsize %.2f def\n", header_size));
				// subtract 1 because we don't want to write the '\0' at the end
				CHECKPRINT0(fwrite(prolog1_ps, sizeof(char), sizeof(prolog1_ps)-1, fp));
				CHECKPRINT0(fprintf(fp, "%%%%EndProlog\n"));
				CHECKPRINT0(fprintf(fp, "%%%%BeginSetup\n"));
				CHECKPRINT0(fprintf(fp, "%%%%IncludeResources: font %s %s %s %s %s %s %s\n",
					head_font_str.c_str(), main_font_str.c_str(),
					number_font_str.c_str(), cont_font_str.c_str(),
					bold_font_str.c_str(), ital_font_str.c_str(),
					bold_ital_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/headfont0 /%s def\n", head_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/mainfont0 /%s def\n", main_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/numberfont0 /%s def\n", number_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/contfont0 /%s def\n", cont_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/boldfont0 /%s def\n", bold_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/italfont0 /%s def\n", ital_font_str.c_str()));
				CHECKPRINT0(fprintf(fp, "/bolditalfont0 /%s def\n",
					bold_ital_font_str.c_str()));
				// subtract 1 because we don't want to write the '\0' at the end
				CHECKPRINT0(fwrite(setup1_ps, sizeof(char), sizeof(setup1_ps)-1, fp));
				CHECKPRINT0(fprintf(fp, "%%%%EndSetup\n"));
				break;
		}
	}
	else
	{
		CHECKPRINT0(fprintf(fp, "%%%%BeginProlog\n"));
		CHECKPRINT0(fprintf(fp, "/whash %hd def\n",
			((ShowModeStandard *)mode)->HashW()));
		CHECKPRINT0(fprintf(fp, "/ehash %hd def\n",
			((ShowModeStandard *)mode)->HashE()));
		CHECKPRINT0(fprintf(fp, "/fieldw %.2f def\n", width));
		CHECKPRINT0(fprintf(fp, "/fieldh %.2f def\n", height));
		// subtract 1 because we don't want to write the '\0' at the end
		CHECKPRINT0(fwrite(prolog2_ps, sizeof(char), sizeof(prolog2_ps)-1, fp));
		CHECKPRINT0(fprintf(fp, "%%%%EndProlog\n"));
		CHECKPRINT0(fprintf(fp, "%%%%BeginSetup\n"));
		// subtract 1 because we don't want to write the '\0' at the end
		CHECKPRINT0(fwrite(setup2_ps, sizeof(char), sizeof(setup2_ps)-1, fp));
		CHECKPRINT0(fprintf(fp, "%%%%EndSetup\n"));
	}

/* print continuity sheets first */
	if (print_do_cont_sheet && !eps && !overview)
	{
		PrintSheets(fp);
		if (!Ok()) return 0;
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
		if (curr_sheet->picked || eps)
		{
			num_pages++;
			if (!overview)
			{
				switch (mode->GetType())
				{
					case SHOW_STANDARD:
						error = PrintStandard(fp, *curr_sheet);
						if (!Ok()) return 0;
						break;
					case SHOW_SPRINGSHOW:
						error = PrintSpringshow(fp, *curr_sheet);
						if (!Ok()) return 0;
						break;
				}
			}
			else
			{
				error = PrintOverview(fp, *curr_sheet);
				if (!Ok()) return 0;
			}
			if (eps) break;
			else
			{
				CHECKPRINT0(fprintf(fp, "showpage\n"));
			}
		}
		if (!split_sheet)
		{
			++curr_sheet;
		}
	}
/* finally, write trailer */
	CHECKPRINT0(fprintf(fp, "%%%%Trailer\n"));
	CHECKPRINT0(fprintf(fp, "%%%%Pages: %hd\n", num_pages));
	CHECKPRINT0(fprintf(fp, "%%%%EOF\n"));

	return num_pages;
}


void CC_show::PrintSheets(FILE *fp) const
{
	enum PSFONT_TYPE currfontnum = PSFONT_NORM;
	short lines_left = 0;
	short need_eject = false;

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
					CHECKPRINT1(fprintf(fp, "showpage\n"));
				}
				num_pages++;
				CHECKPRINT1(fprintf(fp, "%%%%Page: CONT%hd\n", num_pages));
				CHECKPRINT1(fprintf(fp, "0 setgray\n"));
				CHECKPRINT1(fprintf(fp, "0.25 setlinewidth\n"));
				CHECKPRINT1(fprintf(fp, "%.2f %.2f translate\n",
					page_offset_x * DPI,
					(paper_length-page_offset_y)*DPI - real_height));
				lines_left = (short)(real_height / text_size - 0.5);
				CHECKPRINT1(fprintf(fp,
					"/contfont findfont %.2f scalefont setfont\n",
					text_size));
				CHECKPRINT1(fprintf(fp, "/y %.2f def\n", real_height - text_size));
				CHECKPRINT1(fprintf(fp, "/h %.2f def\n", text_size));
				CHECKPRINT1(fprintf(fp, "/lmargin 0 def /rmargin %.2f def\n",
					real_width));
				CHECKPRINT1(fprintf(fp,
					"/tab1 %.2f def /tab2 %.2f def /tab3 %.2f def\n",
					real_width * 0.5 / 7.5, real_width * 1.5 / 7.5,
					real_width * 2.0 / 7.5));
				CHECKPRINT1(fprintf(fp, "/x lmargin def\n"));
			}

			error = gen_cont_line(*text, &currfontnum, text_size, fp);
			if (!Ok()) return;

			CHECKPRINT1(fprintf(fp, "/x lmargin def\n"));
			CHECKPRINT1(fprintf(fp, "/y y h sub def\n"));
			lines_left--;
			need_eject = true;
		}
	}
	if (need_eject)
	{
		CHECKPRINT1(fprintf(fp, "showpage\n"));
	}
}


const wxChar *PrintCont(FILE *fp, const CC_sheet& sheet)
{
	enum PSFONT_TYPE currfontnum = PSFONT_NORM;
	short cont_len = 0;
	float cont_height, this_size;
	const wxChar *error;

	cont_height = field_y - step_size*10;
	for (CC_textline_list::const_iterator text = sheet.continuity.begin();
		text != sheet.continuity.end();
		++text)
	{
		if (text->on_sheet) cont_len++;
	}
	if (cont_len == 0) return NULL;
	this_size = cont_height / (cont_len + 0.5);
	if (this_size > text_size) this_size = text_size;
	CHECKPRINT(fprintf(fp, "/y %.2f def\n", cont_height - this_size));
	CHECKPRINT(fprintf(fp, "/h %.2f def\n", this_size));
	CHECKPRINT(fprintf(fp, "/lmargin 0 def /rmargin %.2f def\n", width));
	CHECKPRINT(fprintf(fp, "/tab1 %.2f def /tab2 %.2f def /tab3 %.2f def\n",
		width * 0.5 / 7.5, width * 1.5 / 7.5, width * 2.0 / 7.5));
	CHECKPRINT(fprintf(fp, "/contfont findfont %.2f scalefont setfont\n",
		this_size));
	for (CC_textline_list::const_iterator text = sheet.continuity.begin();
		text != sheet.continuity.end();
		++text)
	{
		if (!text->on_sheet) continue;
		CHECKPRINT(fprintf(fp, "/x lmargin def\n"));

		error = gen_cont_line(*text, &currfontnum, this_size, fp);
		if (error) return error;

		CHECKPRINT(fprintf(fp, "/y y h sub def\n"));
	}
	return NULL;
}


const wxChar *gen_cont_line(const CC_textline& line, PSFONT_TYPE *currfontnum,
float fontsize, FILE *fp)
{
	const char *text;
	short tabstop;
	std::string temp_buf;

	tabstop = 0;
	for (CC_textchunk_list::const_iterator part = line.chunks.begin();
		part != line.chunks.end();
		++part)
	{
		if (part->font == PSFONT_TAB)
		{
			if (++tabstop > 3)
			{
				CHECKPRINT(fprintf(fp, "space_over\n"));
			}
			else
			{
				CHECKPRINT(fprintf(fp, "tab%hd do_tab\n", tabstop));
			}
		}
		else
		{
			if (part->font != *currfontnum)
			{
				CHECKPRINT(fprintf(fp, "/%s findfont %.2f scalefont setfont\n",
					fontnames[part->font], fontsize));
				*currfontnum = part->font;
			}
			std::string textstr(part->text.utf8_str());
			text = textstr.c_str();
			while (*text != 0)
			{
				temp_buf = "";
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
						CHECKPRINT(fprintf(fp, "(%s) centerText\n", temp_buf.c_str()));
					}
					else
					{
						CHECKPRINT(fprintf(fp, "(%s) leftText\n", temp_buf.c_str()));
					}
				}
			}
		}
	}
	return NULL;
}


const wxChar *print_start_page(FILE *fp, bool landscape)
{
	CHECKPRINT(fprintf(fp, "0 setgray\n"));
	CHECKPRINT(fprintf(fp, "0.25 setlinewidth\n"));
	CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n",
		page_offset_x * DPI,
		(paper_length - page_offset_y) * DPI - real_height));
	if (landscape)
	{
		CHECKPRINT(fprintf(fp, "%.2f 0 translate 90 rotate\n", real_width));
	}
	return NULL;
}


const wxChar *PrintStandard(FILE *fp, const CC_sheet& sheet)
{
	float dot_x, dot_y, dot_w;
	short x_s, x_n;
	unsigned short i;
	short j;
	const wxChar *error;
	CC_coord fieldsize = sheet.show->GetMode().FieldSize();
	CC_coord fieldoff = sheet.show->GetMode().FieldOffset();
	Coord pmin = sheet.show->GetMode().MinPosition().x;
	Coord pmax = sheet.show->GetMode().MaxPosition().x;
	Coord fmin = sheet.show->GetMode().FieldOffset().x;
	Coord fmax = sheet.show->GetMode().FieldSize().x + fmin;
	float fieldheight = COORD2FLOAT(fieldsize.y);
	float fieldoffx = COORD2FLOAT(fieldoff.x);
	float fieldoffy = COORD2FLOAT(fieldoff.y);

	std::string namestr(sheet.name.utf8_str());
	std::string numberstr(sheet.number.utf8_str());

	if (split_sheet)
	{
		CHECKPRINT(fprintf(fp, "%%%%Page: %s(N)\n", namestr.c_str()));
		if (sheet.number)
		{
			CHECKPRINT(fprintf(fp, "/pagenumtext (%sN) def\n",numberstr.c_str()));
		}
		else
		{
			CHECKPRINT(fprintf(fp, "/pagenumtext () def\n"));
		}
		step_offset = COORD2INT(fieldsize.x) - step_width;
/* south yardline */
		clip_s = INT2COORD(step_offset) + fmin;
		clip_n = pmax;
		split_sheet = false;
	}
	else
	{
/* find bounds */
		max_s = fmax;
		max_n = fmin;
		for (i=0; i < sheet.show->GetNumPoints(); i++)
		{
			if (sheet.pts[i].pos.x < max_s) max_s = sheet.pts[i].pos.x;
			if (sheet.pts[i].pos.x > max_n) max_n = sheet.pts[i].pos.x;
		}
/* make sure bounds are on field */
		if (max_s < fmin) max_s = fmin;
		if (max_n > fmax) max_n = fmax;

		if (COORD2INT((max_n-max_s) +
			((max_s+fmin) % INT2COORD(8)))
			> step_width)
		{
/* Need to split into two pages */
			CHECKPRINT(fprintf(fp, "%%%%Page: %s(S)\n", namestr.c_str()));
			if (sheet.number)
			{
				CHECKPRINT(fprintf(fp, "/pagenumtext (%sS) def\n",
					numberstr.c_str()));
			}
			else
			{
				CHECKPRINT(fprintf(fp, "/pagenumtext () def\n"));
			}
			step_offset = 0;
			split_sheet = true;
			clip_s = pmin;
			clip_n = INT2COORD(step_width) + fmin;/* north yardline */
		}
		else
		{
			CHECKPRINT(fprintf(fp, "%%%%Page: %s\n", namestr.c_str()));
			if (sheet.number)
			{
				CHECKPRINT(fprintf(fp, "/pagenumtext (%s) def\n",
					numberstr.c_str()));
			}
			else
			{
				CHECKPRINT(fprintf(fp, "/pagenumtext () def\n"));
			}
			step_offset = (COORD2INT(sheet.show->GetMode().FieldSize().x) - step_width) / 2;
			step_offset = (step_offset / 8) * 8;
			clip_s = pmin;
			clip_n = pmax;
			x_s = COORD2INT(max_s) - COORD2INT(fmin);
			x_n = COORD2INT(max_n) - COORD2INT(fmin);
			if ((x_s < step_offset) || (x_n > (step_offset + step_width)))
			{
/* Recenter formation */
				step_offset = x_s - (step_width-(x_n-x_s))/2;
				if (step_offset < 0) step_offset = 0;
				else if ((step_offset + step_width) >
					COORD2INT(sheet.show->GetMode().FieldSize().x))
					step_offset = COORD2INT(sheet.show->GetMode().FieldSize().x) - step_width;
				step_offset = (step_offset / 8) * 8;
			}
		}
	}
	error = print_start_page(fp, sheet.show->GetBoolLandscape());
	if (error) return error;

	CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", field_x, field_y));

/* Draw field */
	CHECKPRINT(fprintf(fp, "drawfield\n"));
	CHECKPRINT(fprintf(fp, "/mainfont findfont %.2f scalefont setfont\n",
		step_size * yards_size));
	for (j=0; j <= step_width; j+=8)
	{
		CHECKPRINT(fprintf(fp, "/lmargin %.2f def /rmargin %.2f def\n",
			step_size * j, step_size * j));
		CHECKPRINT(fprintf(fp, "/y %.2f def\n", field_h + (step_size / 2)));
		std::string yardstr(yard_text[(step_offset +
			(MAX_YARD_LINES-1)*4 +
			COORD2INT(fieldoff.x) + j)/8].utf8_str());
		CHECKPRINT(fprintf(fp, "(%s) dup centerText\n", yardstr.c_str()));
		CHECKPRINT(fprintf(fp, "/y %.2f def\n", -(step_size * 2)));
		CHECKPRINT(fprintf(fp, "centerText\n"));
	}

	dot_w = step_size / 2 * dot_ratio;
	CHECKPRINT(fprintf(fp, "/w %.4f def\n", dot_w));
	CHECKPRINT(fprintf(fp, "/plinew %.4f def\n", dot_w * pline_ratio));
	CHECKPRINT(fprintf(fp, "/slinew %.4f def\n", dot_w * sline_ratio));
	CHECKPRINT(fprintf(fp, "/numberfont findfont %.2f scalefont setfont\n",
		dot_w * 2 * num_ratio));
	for (i = 0; i < sheet.show->GetNumPoints(); i++)
	{
		if ((sheet.pts[i].pos.x > clip_n) || (sheet.pts[i].pos.x < clip_s)) continue;
		dot_x = (COORD2FLOAT(sheet.pts[i].pos.x) - fieldoffx - step_offset) /
			step_width * field_w;
		dot_y = (1.0 - (COORD2FLOAT(sheet.pts[i].pos.y)-fieldoffy)/fieldheight)*field_h;
		CHECKPRINT(fprintf(fp, "%.2f %.2f %s\n",
			dot_x, dot_y, dot_routines[sheet.pts[i].sym]));
		CHECKPRINT(fprintf(fp, "(%s) %.2f %.2f %s\n",
			static_cast<const char*>(sheet.show->GetPointLabel(i).mb_str()), dot_x, dot_y,
			sheet.pts[i].GetFlip() ? "donumber2" : "donumber"));
	}
	if (sheet.show->GetBoolDoCont())
	{
		CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", -field_x, -field_y));
		error = PrintCont(fp, sheet);
		if (error) return error;
	}
	return NULL;
}


const wxChar *PrintSpringshow(FILE *fp, const CC_sheet& sheet)
{
	float dot_x, dot_y, dot_w;
	unsigned short i;
	short j;
	const wxChar *error;
	const ShowModeSprShow *modesprshow = dynamic_cast<const ShowModeSprShow*>(&sheet.show->GetMode());

	std::string namestr(sheet.name.utf8_str());
	std::string numberstr(sheet.number.utf8_str());

	CHECKPRINT(fprintf(fp, "%%%%Page: %s\n", namestr.c_str()));
	if (sheet.number)
	{
		CHECKPRINT(fprintf(fp, "/pagenumtext (%s) def\n", numberstr.c_str()));
	}
	else
	{
		CHECKPRINT(fprintf(fp, "/pagenumtext () def\n"));
	}

	error = print_start_page(fp, sheet.show->GetBoolLandscape());
	if (error) return error;

	CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", field_x, field_y));

/* Draw stage */
	CHECKPRINT(fprintf(fp, "   BeginEPSF\n"));
	CHECKPRINT(fprintf(fp, "   %.2f %.2f scale\n",
		field_w / modesprshow->StageW(),
		field_h / modesprshow->StageH()));
	CHECKPRINT(fprintf(fp, "   %hd %hd translate\n",
		-modesprshow->StageX(), -modesprshow->StageY()));
	std::string stagefilestr(modesprshow->StageFile().utf8_str());
	CHECKPRINT(fprintf(fp, "%%%%BeginDocument: %s\n", stagefilestr.c_str()));
	if (!copy_ps_file(modesprshow->StageFile(), fp))
	{
		return nofile;
	}
	CHECKPRINT(fprintf(fp, "%%%%EndDocument\n"));
	CHECKPRINT(fprintf(fp, "EndEPSF\n"));
/* Draw field */
	CHECKPRINT(fprintf(fp, "drawfield\n"));
	if (modesprshow->WhichYards())
	{
		CHECKPRINT(fprintf(fp, "/mainfont findfont %.2f scalefont setfont\n",
			step_size * yards_size));
		if (modesprshow->WhichYards() & (SPR_YARD_ABOVE | SPR_YARD_BELOW))
			for (j=0; j <= modesprshow->StepsW(); j+=8)
		{
			CHECKPRINT(fprintf(fp, "/lmargin %.2f def /rmargin %.2f def\n",
				stage_field_x + step_size * j,
				stage_field_x + step_size * j));
			if (modesprshow->WhichYards() & SPR_YARD_ABOVE)
			{
				CHECKPRINT(fprintf(fp, "/y %.2f def\n",
					field_h*
					(modesprshow->TextTop()-modesprshow->StageX())/
					modesprshow->StageH()));
				std::string yardstr(yard_text[(modesprshow->StepsX() +
					(MAX_YARD_LINES-1)*4 + j)/8].utf8_str());
				CHECKPRINT(fprintf(fp, "(%s) centerText\n", yardstr.c_str()));
			}
			if (modesprshow->WhichYards() & SPR_YARD_BELOW)
			{
				CHECKPRINT(fprintf(fp, "/y %.2f def\n",
					field_h *
					(modesprshow->TextBottom() -
					modesprshow->StageX()) /
					modesprshow->StageH() -(step_size*yards_size)));
				std::string yardstr(yard_text[(modesprshow->StepsX() +
					(MAX_YARD_LINES-1)*4 + j)/8].utf8_str());
				CHECKPRINT(fprintf(fp, "(%s) centerText\n", yardstr.c_str()));
			}
		}
		if (modesprshow->WhichYards() & (SPR_YARD_LEFT | SPR_YARD_RIGHT))
			for (j=0; j <= modesprshow->StepsH(); j+=8)
		{
			CHECKPRINT(fprintf(fp, "/y %.2f def\n",
				stage_field_y + stage_field_h *
				(modesprshow->StepsH()-j-yards_size/2) /
				modesprshow->StepsH()));
			CHECKPRINT(fprintf(fp, "/x %.2f def /rmargin %.2f def\n",
				field_w*
				(modesprshow->TextRight()-modesprshow->StageX()) /
				modesprshow->StageW(),
				field_w*
				(modesprshow->TextLeft()-modesprshow->StageX()) /
				modesprshow->StageW()));
			std::string spr_text_str(spr_line_text[j / 8].utf8_str());
			if (modesprshow->WhichYards() & SPR_YARD_RIGHT)
			{
				CHECKPRINT(fprintf(fp, "(%s) leftText\n", spr_text_str.c_str()));
			}
			if (modesprshow->WhichYards() & SPR_YARD_LEFT)
			{
				CHECKPRINT(fprintf(fp, "(%s) rightText\n", spr_text_str.c_str()));
			}
		}
	}

	dot_w = step_size / 2 * dot_ratio;
	CHECKPRINT(fprintf(fp, "/w %.4f def\n", dot_w));
	CHECKPRINT(fprintf(fp, "/plinew %.4f def\n", dot_w * pline_ratio));
	CHECKPRINT(fprintf(fp, "/slinew %.4f def\n", dot_w * sline_ratio));
	CHECKPRINT(fprintf(fp, "/numberfont findfont %.2f scalefont setfont\n",
		dot_w * 2 * num_ratio));
	for (i = 0; i < sheet.show->GetNumPoints(); i++)
	{
		dot_x = stage_field_x +
			(COORD2FLOAT(sheet.pts[i].pos.x) - modesprshow->StepsX()) /
			modesprshow->StepsW() * stage_field_w;
		dot_y = stage_field_y + stage_field_h * (1.0 -
			(COORD2FLOAT(sheet.pts[i].pos.y)-
			modesprshow->StepsY())/
			modesprshow->StepsH());
		CHECKPRINT(fprintf(fp, "%.2f %.2f %s\n",
			dot_x, dot_y, dot_routines[sheet.pts[i].sym]));
		CHECKPRINT(fprintf(fp, "(%s) %.2f %.2f %s\n",
			static_cast<const char*>(sheet.show->GetPointLabel(i).mb_str()), dot_x, dot_y,
			(sheet.pts[i].flags & PNT_LABEL) ? "donumber2" : "donumber"));
	}
	if (sheet.show->GetBoolDoCont())
	{
		CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", -field_x, -field_y));
		error = PrintCont(fp, sheet);
		if (error) return error;
	}
	return NULL;
}


const wxChar *PrintOverview(FILE *fp, const CC_sheet& sheet)
{
	unsigned short i;
	const wxChar *error;
	CC_coord fieldoff = sheet.show->GetMode().FieldOffset();
	CC_coord fieldsize = sheet.show->GetMode().FieldSize();
	float fieldx = COORD2FLOAT(fieldoff.x);
	float fieldy = COORD2FLOAT(fieldoff.y);
	float fieldwidth = COORD2FLOAT(fieldsize.x);
	float fieldheight = COORD2FLOAT(fieldsize.y);

	std::string namestr(sheet.name.utf8_str());

	CHECKPRINT(fprintf(fp, "%%%%Page: %s\n", namestr.c_str()));

	error = print_start_page(fp, sheet.show->GetBoolLandscape());
	if (error) return error;

	CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", field_x, field_y));
	CHECKPRINT(fprintf(fp, "drawfield\n"));
	CHECKPRINT(fprintf(fp, "/w %.2f def\n", width / fieldwidth * 2.0 / 3.0));
	for (i = 0; i < sheet.show->GetNumPoints(); i++)
	{
		CHECKPRINT(fprintf(fp, "%.2f %.2f dotbox\n",
			(COORD2FLOAT(sheet.pts[i].pos.x)-fieldx) / fieldwidth * width,
			(1.0 - (COORD2FLOAT(sheet.pts[i].pos.y)-
			fieldy)/fieldheight) * height));
	}
	return NULL;
}


bool copy_ps_file(const wxString& name, FILE *fp)
{
	char cpbuf[1024];
	FILE *infile;

	infile = OpenFileInDir(name, wxT("r"));
	if (infile == NULL) return false;
	while (fgets(cpbuf, 1024, infile))
	{
		if (fputs(cpbuf, fp) == EOF)
		{
/* Error copying file */
			fclose(infile);
			return false;
		}
	}
	fclose(infile);
	return true;
}
