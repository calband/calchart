/* print.cc
 * Handles all postscript printing to a file stream
 *
 * Modification history:
 * 1-7-95     Garrick Meeker              Created from previous CalPrint
 * 7-12-95    Garrick Meeker              Converted to C++
 *
 */

#include "show.h"
#include "confgr.h"
#include "modes.h"
#include <time.h>

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

static char *dot_routines[] = {
  "dotplain",
  "dotsolid",
  "dotbs",
  "dotsl",
  "dotx",
  "dotsolbs",
  "dotsolsl",
  "dotsolx",
};

static char *fontnames[] = {
  "CalChart",
  "contfont",
  "boldfont",
  "italfont",
  "bolditalfont",
};

static char nofile[] = "Unable to open print config files";
static char badfile[] = "Error writing to print file";

static char *gen_cont_line(CC_textline *line, PSFONT_TYPE *currfontnum,
			   float fontsize, FILE *fp);
static char *print_start_page(FILE *fp, Bool landscape);
static Bool copy_ps_file(const char *name, FILE *fp);

#define CHECKPRINT0(a) if ((a) < 0) { error = badfile; return 0; }
#define CHECKPRINT1(a) if ((a) < 0) { error = badfile; return; }
#define CHECKPRINT(a) if ((a) < 0) return badfile;

int CC_show::Print(FILE *fp, Bool eps, Bool overview, unsigned curr_ss,
		   int min_yards) {
  time_t t;
  CC_sheet *curr_sheet;
  CC_coord fullsize = mode->Size();
  CC_coord fieldsize = mode->FieldSize();
  float fullwidth = COORD2FLOAT(fullsize.x);
  float fullheight = COORD2FLOAT(fullsize.y);
  float fieldwidth = COORD2FLOAT(fieldsize.x);
  float fieldheight = COORD2FLOAT(fieldsize.y);

  num_pages = 0;
  /* first, calculate dimensions */
  if (!overview) {
    switch (mode->GetType()) {
    case SHOW_STANDARD:
      if (print_landscape) {
	width = page_height * DPI;
	if (print_do_cont) {
	  height = page_width * (1.0 - cont_ratio) * DPI;
	  field_y = page_width * cont_ratio * DPI;
	} else {
	  height = page_width * DPI;
	  field_y = 0;
	}
      } else {
	width = page_width * DPI;
	if (print_do_cont) {
	  height = page_height * (1.0 - cont_ratio) * DPI;
	  field_y = page_height * cont_ratio * DPI;
	} else {
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
      if (step_width > min_yards) {
	/* We can get the minimum size */
	step_width = (step_width / 8) * 8;
	field_w = step_width * (height / (fullheight+8.0));
	field_h = height * (fieldheight/(fullheight+8.0));
	if ((field_w/step_width*(step_width+4)) > width) {
	  /* Oops, we didn't made it big enough */
	  field_h *= width*step_width/(step_width+4)/field_w;
	  field_w = width*step_width/(step_width+4);
	}
      } else {
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
      if (print_landscape) {
	width = page_height * DPI;
	if (print_do_cont) {
	  height = page_width * (1.0 - cont_ratio) * DPI;
	  field_y = page_width * cont_ratio * DPI;
	} else {
	  height = page_width * DPI;
	  field_y = 0;
	}
      } else {
	width = page_width * DPI;
	if (print_do_cont) {
	  height = page_height * (1.0 - cont_ratio) * DPI;
	  field_y = page_height * cont_ratio * DPI;
	} else {
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
      if (field_w > width) {
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
  } else {
    if (print_landscape) {
      width = page_height * DPI;
      height = page_width * DPI;
    } else {
      width = page_width * DPI;
      height = page_height * DPI;
    }
    if ((width * (fullheight / fullwidth)) > height) {
      width = height * (fullwidth / fullheight);
    } else {
      height = width * (fullheight / fullwidth);
    }
    width--; height--;
    if (print_landscape) {
      real_width = height;
      real_height = width;
    } else {
      real_width = width;
      real_height = height;
    }
    field_x = 8.0 / fullwidth * width;
    field_y = 8.0 / fullheight * height;
    width *= fieldwidth/fullwidth;
    height *= fieldheight/fullheight;
  }

  /* Now write postscript header */
  if (eps) {
    CHECKPRINT0(fprintf(fp, "%%!PS-Adobe-3.0 EPSF-3.0\n"));
  } else {
    CHECKPRINT0(fprintf(fp, "%%!PS-Adobe-3.0\n"));
  }
  CHECKPRINT0(fprintf(fp, "%%%%BoundingBox: %.0f %.0f %.0f %.0f\n",
	  page_offset_x * DPI,
	  (paper_length - page_offset_y) * DPI - real_height,
	  page_offset_x * DPI + real_width,
	  (paper_length - page_offset_y) * DPI));
  time(&t);
  CHECKPRINT0(fprintf(fp, "%%%%CreationDate: %s", ctime(&t)));
  CHECKPRINT0(fprintf(fp, "%%%%Title: %s\n", (const char *)name));
  CHECKPRINT0(fprintf(fp, "%%%%Creator: CalChart\n"));
  CHECKPRINT0(fprintf(fp, "%%%%Pages: (atend)\n"));
  CHECKPRINT0(fprintf(fp, "%%%%PageOrder: Ascend\n"));
  if (!overview) {
    CHECKPRINT0(fprintf(fp,"%%%%DocumentNeededResources: font %s %s %s %s %s %s %s\n",
			head_font.Chars(), main_font.Chars(),
			number_font.Chars(), cont_font.Chars(),
			bold_font.Chars(), ital_font.Chars(),
			bold_ital_font.Chars()));
    CHECKPRINT0(fprintf(fp, "%%%%DocumentSuppliedResources: font CalChart\n"));
    CHECKPRINT0(fprintf(fp, "%%%%BeginDefaults\n"));
    CHECKPRINT0(fprintf(fp, "%%%%PageResources: font %s %s %s %s %s %s %s CalChart\n",
			head_font.Chars(), main_font.Chars(),
			number_font.Chars(), cont_font.Chars(),
			bold_font.Chars(), ital_font.Chars(),
			bold_ital_font.Chars()));
    CHECKPRINT0(fprintf(fp, "%%%%EndDefaults\n"));
  }
  CHECKPRINT0(fprintf(fp, "%%%%EndComments\n"));
  if (!overview) {
    switch (mode->GetType()) {
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
      if (!copy_ps_file("prolog0.ps", fp)) {
	error = nofile;
	return 0;
      }
      CHECKPRINT0(fprintf(fp, "%%%%EndProlog\n"));
      CHECKPRINT0(fprintf(fp, "%%%%BeginSetup\n"));
      CHECKPRINT0(fprintf(fp, "%%%%IncludeResources: font %s %s %s %s %s %s %s\n",
			  head_font.Chars(), main_font.Chars(),
			  number_font.Chars(), cont_font.Chars(),
			  bold_font.Chars(), ital_font.Chars(),
			  bold_ital_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/headfont0 /%s def\n", head_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/mainfont0 /%s def\n", main_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/numberfont0 /%s def\n", number_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/contfont0 /%s def\n", cont_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/boldfont0 /%s def\n", bold_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/italfont0 /%s def\n", ital_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/bolditalfont0 /%s def\n",
			  bold_ital_font.Chars()));
      if (!copy_ps_file("setup0.ps", fp)) {
	error = nofile;
	return 0;
      }
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
      if (!copy_ps_file("prolog1.ps", fp)) {
	error = nofile;
	return 0;
      }
      CHECKPRINT0(fprintf(fp, "%%%%EndProlog\n"));
      CHECKPRINT0(fprintf(fp, "%%%%BeginSetup\n"));
      CHECKPRINT0(fprintf(fp, "%%%%IncludeResources: font %s %s %s %s %s %s %s\n",
			  head_font.Chars(), main_font.Chars(),
			  number_font.Chars(), cont_font.Chars(),
			  bold_font.Chars(), ital_font.Chars(),
			  bold_ital_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/headfont0 /%s def\n", head_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/mainfont0 /%s def\n", main_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/numberfont0 /%s def\n", number_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/contfont0 /%s def\n", cont_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/boldfont0 /%s def\n", bold_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/italfont0 /%s def\n", ital_font.Chars()));
      CHECKPRINT0(fprintf(fp, "/bolditalfont0 /%s def\n",
			  bold_ital_font.Chars()));
      if (!copy_ps_file("setup1.ps", fp)) {
	error = nofile;
	return 0;
      }
      CHECKPRINT0(fprintf(fp, "%%%%EndSetup\n"));
      break;
    }
  } else {
    CHECKPRINT0(fprintf(fp, "%%%%BeginProlog\n"));
    CHECKPRINT0(fprintf(fp, "/whash %hd def\n",
			((ShowModeStandard *)mode)->HashW()));
    CHECKPRINT0(fprintf(fp, "/ehash %hd def\n", 
			((ShowModeStandard *)mode)->HashE()));
    CHECKPRINT0(fprintf(fp, "/fieldw %.2f def\n", width));
    CHECKPRINT0(fprintf(fp, "/fieldh %.2f def\n", height));
    if (!copy_ps_file("prolog2.ps", fp)) {
      error = nofile;
      return 0;
    }
    CHECKPRINT0(fprintf(fp, "%%%%EndProlog\n"));
    CHECKPRINT0(fprintf(fp, "%%%%BeginSetup\n"));
    if (!copy_ps_file("setup2.ps", fp)) {
      error = nofile;
      return 0;
    }
    CHECKPRINT0(fprintf(fp, "%%%%EndSetup\n"));
  }

  /* print continuity sheets first */
  if (print_do_cont_sheet && !eps && !overview) {
    PrintSheets(fp);
    if (!Ok()) return 0;
  }

  /* do stuntsheet pages now */
  split_sheet = FALSE;
  if (eps) {
    curr_sheet = GetNthSheet(curr_ss);
  } else {
    curr_sheet = sheets;
  }
  while (curr_sheet != NULL) {
    if (curr_sheet->picked || eps) {
      num_pages++;
      if (!overview) {
	switch (mode->GetType()) {
	case SHOW_STANDARD:
	  error = curr_sheet->PrintStandard(fp);
	  if (!Ok()) return 0;
	  break;
	case SHOW_SPRINGSHOW:
	  error = curr_sheet->PrintSpringshow(fp);
	  if (!Ok()) return 0;
	  break;
	}
      } else {
	error = curr_sheet->PrintOverview(fp);
	if (!Ok()) return 0;
      }
      if (eps) break;
      else {
	CHECKPRINT0(fprintf(fp, "showpage\n"));
      }
    }
    if (!split_sheet) {
      curr_sheet = curr_sheet->next;
    }
  }
  /* finally, write trailer */
  CHECKPRINT0(fprintf(fp, "%%%%Trailer\n"));
  CHECKPRINT0(fprintf(fp, "%%%%Pages: %hd\n", num_pages));
  CHECKPRINT0(fprintf(fp, "%%%%EOF\n"));

  return num_pages;
}

void CC_show::PrintSheets(FILE *fp) {
  wxNode *textnode;
  CC_textline *text;
  enum PSFONT_TYPE currfontnum = PSFONT_NORM;
  short lines_left = 0;
  short need_eject = FALSE;
  CC_sheet *sheet;

  for (sheet = sheets; sheet != NULL; sheet = sheet->next) {
    for (textnode = sheet->continuity.lines.First(); textnode != NULL;
	 textnode = textnode->Next()) {
      text = (CC_textline*)textnode->Data();
      if (!text->on_main) continue;
      if (lines_left <= 0) {
	if (num_pages > 0) {
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

      error = gen_cont_line(text, &currfontnum, text_size, fp);
      if (!Ok()) return;

      CHECKPRINT1(fprintf(fp, "/x lmargin def\n"));
      CHECKPRINT1(fprintf(fp, "/y y h sub def\n"));
      lines_left--;
      need_eject = TRUE;
    }
  }
  if (need_eject) {
    CHECKPRINT1(fprintf(fp, "showpage\n"));
  }
}

char *CC_sheet::PrintCont(FILE *fp) {
  wxNode *textnode;
  CC_textline *text;
  enum PSFONT_TYPE currfontnum = PSFONT_NORM;
  short cont_len = 0;
  float cont_height, this_size;
  char *error;

  cont_height = field_y - step_size*10;
  for (textnode = continuity.lines.First(); textnode != NULL;
       textnode = textnode->Next()) {
    text = (CC_textline*)textnode->Data();
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
  for (textnode = continuity.lines.First(); textnode != NULL;
       textnode = textnode->Next()) {
    text = (CC_textline*)textnode->Data();
    if (!text->on_sheet) continue;
    CHECKPRINT(fprintf(fp, "/x lmargin def\n"));

    error = gen_cont_line(text, &currfontnum, this_size, fp);
    if (error) return error;

    CHECKPRINT(fprintf(fp, "/y y h sub def\n"));
  }
  return NULL;
}

char *gen_cont_line(CC_textline *line, PSFONT_TYPE *currfontnum,
		    float fontsize, FILE *fp) {
  wxNode *textnode;
  CC_textchunk *part;
  const char *text;
  short tabstop;
  wxString temp_buf;

  tabstop = 0;
  for (textnode = line->chunks.First(); textnode != NULL;
       textnode = textnode->Next()) {
    part = (CC_textchunk*)textnode->Data();
    if (part->font == PSFONT_TAB) {
      if (++tabstop > 3) {
	CHECKPRINT(fprintf(fp, "space_over\n"));
      } else {
	CHECKPRINT(fprintf(fp, "tab%hd do_tab\n", tabstop));
      }
    } else {
      if (part->font != *currfontnum) {
	CHECKPRINT(fprintf(fp, "/%s findfont %.2f scalefont setfont\n",
			    fontnames[part->font], fontsize));
	*currfontnum = part->font;
      }
      text = part->text;
      while (*text != 0) {
	temp_buf = "";
	while (*text != 0) {
	  // Need backslash before parenthesis
	  if ((*text == '(') || (*text == ')')) {
	    temp_buf.Append('\\');
	  }
	  temp_buf.Append(*(text++));
	}
	
	if (!temp_buf.Empty()) {
	  if (line->center) {
	    CHECKPRINT(fprintf(fp, "(%s) centerText\n", temp_buf.GetData()));
	  } else {
	    CHECKPRINT(fprintf(fp, "(%s) leftText\n", temp_buf.GetData()));
	  }
	}
      }
    }
  }
  return NULL;
}

char *print_start_page(FILE *fp, Bool landscape) {
  CHECKPRINT(fprintf(fp, "0 setgray\n"));
  CHECKPRINT(fprintf(fp, "0.25 setlinewidth\n"));
  CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n",
		     page_offset_x * DPI,
		     (paper_length - page_offset_y) * DPI - real_height));
  if (landscape) {
    CHECKPRINT(fprintf(fp, "%.2f 0 translate 90 rotate\n", real_width));
  }
  return NULL;
}

char *CC_sheet::PrintStandard(FILE *fp) {
  float dot_x, dot_y, dot_w;
  short x_s, x_n;
  unsigned short i;
  short j;
  char *error;
  CC_coord fieldsize = show->mode->FieldSize();
  CC_coord fieldoff = show->mode->FieldOffset();
  Coord pmin = show->mode->MinPosition().x;
  Coord pmax = show->mode->MaxPosition().x;
  Coord fmin = show->mode->FieldOffset().x;
  Coord fmax = show->mode->FieldSize().x + fmin;
  float fieldwidth = COORD2FLOAT(fieldsize.x);
  float fieldheight = COORD2FLOAT(fieldsize.y);
  float fieldoffx = COORD2FLOAT(fieldoff.x);
  float fieldoffy = COORD2FLOAT(fieldoff.y);

  if (split_sheet) {
    CHECKPRINT(fprintf(fp, "%%%%Page: %s(N)\n", (const char *)name));
    if (number) {
      CHECKPRINT(fprintf(fp, "/pagenumtext (%sN) def\n",(const char *)number));
    }
    else {
      CHECKPRINT(fprintf(fp, "/pagenumtext () def\n"));
    }
    step_offset = COORD2INT(fieldsize.x) - step_width;
    /* south yardline */
    clip_s = INT2COORD(step_offset) + fmin;
    clip_n = pmax;
    split_sheet = FALSE;
  } else {
    /* find bounds */
    max_s = fmax;
    max_n = fmin;
    for (i=0; i < show->GetNumPoints(); i++) {
      if (pts[i].pos.x < max_s) max_s = pts[i].pos.x;
      if (pts[i].pos.x > max_n) max_n = pts[i].pos.x;
    }
    /* make sure bounds are on field */
    if (max_s < fmin) max_s = fmin;
    if (max_n > fmax) max_n = fmax;

    if (COORD2INT((max_n-max_s) +
		  ((max_s+fmin) % INT2COORD(8)))
	  > step_width) {
      /* Need to split into two pages */
      CHECKPRINT(fprintf(fp, "%%%%Page: %s(S)\n", (const char *)name));
      if (number) {
	CHECKPRINT(fprintf(fp, "/pagenumtext (%sS) def\n",
			   (const char *)number));
      } else {
	CHECKPRINT(fprintf(fp, "/pagenumtext () def\n"));
      }
      step_offset = 0;
      split_sheet = TRUE;
      clip_s = pmin;
      clip_n = INT2COORD(step_width) + fmin; /* north yardline */
    } else {
      CHECKPRINT(fprintf(fp, "%%%%Page: %s\n", (const char *)name));
      if (number) {
	CHECKPRINT(fprintf(fp, "/pagenumtext (%s) def\n",
			   (const char *)number));
      } else {
	CHECKPRINT(fprintf(fp, "/pagenumtext () def\n"));
      }
      step_offset = (COORD2INT(show->mode->FieldSize().x) - step_width) / 2;
      step_offset = (step_offset / 8) * 8;
      clip_s = pmin;
      clip_n = pmax;
      x_s = COORD2INT(max_s) - COORD2INT(fmin);
      x_n = COORD2INT(max_n) - COORD2INT(fmin);
      if ((x_s < step_offset) || (x_n > (step_offset + step_width))) {
	/* Recenter formation */
	step_offset = x_s - (step_width-(x_n-x_s))/2;
	if (step_offset < 0) step_offset = 0;
	else if ((step_offset + step_width) >
		 COORD2INT(show->mode->FieldSize().x))
	  step_offset = COORD2INT(show->mode->FieldSize().x) - step_width;
	step_offset = (step_offset / 8) * 8;
      }
    }
  }
  error = print_start_page(fp, show->GetBoolLandscape());
  if (error) return error;

  CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", field_x, field_y));
  
  /* Draw field */
  CHECKPRINT(fprintf(fp, "drawfield\n"));
  CHECKPRINT(fprintf(fp, "/mainfont findfont %.2f scalefont setfont\n",
		     step_size * yards_size));
  for (j=0; j <= step_width; j+=8) {
    CHECKPRINT(fprintf(fp, "/lmargin %.2f def /rmargin %.2f def\n",
		       step_size * j, step_size * j));
    CHECKPRINT(fprintf(fp, "/y %.2f def\n", field_h + (step_size / 2)));
    CHECKPRINT(fprintf(fp, "(%s) dup centerText\n",
		       yard_text[(step_offset +
				  (MAX_YARD_LINES-1)*4 +
				  COORD2INT(fieldoff.x) + j)/8].Chars()));
    CHECKPRINT(fprintf(fp, "/y %.2f def\n", -(step_size * 2)));
    CHECKPRINT(fprintf(fp, "centerText\n"));
  }

  dot_w = step_size / 2 * dot_ratio;
  CHECKPRINT(fprintf(fp, "/w %.4f def\n", dot_w));
  CHECKPRINT(fprintf(fp, "/plinew %.4f def\n", dot_w * pline_ratio));
  CHECKPRINT(fprintf(fp, "/slinew %.4f def\n", dot_w * sline_ratio));
  CHECKPRINT(fprintf(fp, "/numberfont findfont %.2f scalefont setfont\n",
		     dot_w * 2 * num_ratio));
  for (i = 0; i < show->GetNumPoints(); i++) {
    if ((pts[i].pos.x > clip_n) || (pts[i].pos.x < clip_s)) continue;
    dot_x = (COORD2FLOAT(pts[i].pos.x) - fieldoffx - step_offset) /
      step_width * field_w;
    dot_y = (1.0 - (COORD2FLOAT(pts[i].pos.y)-fieldoffy)/fieldheight)*field_h;
    CHECKPRINT(fprintf(fp, "%.2f %.2f %s\n",
		       dot_x, dot_y, dot_routines[pts[i].sym]));
    CHECKPRINT(fprintf(fp, "(%s) %.2f %.2f %s\n",
		       show->GetPointLabel(i), dot_x, dot_y,
		       pts[i].GetFlip() ? "donumber2" : "donumber"));
  }
  if (show->GetBoolDoCont()) {
    CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", -field_x, -field_y));
    error = PrintCont(fp);
    if (error) return error;
  }
  return NULL;
}

char *CC_sheet::PrintSpringshow(FILE *fp) {
  float dot_x, dot_y, dot_w;
  unsigned short i;
  short j;
  char *error;
  ShowModeSprShow *modesprshow = (ShowModeSprShow *)show->mode;

  CHECKPRINT(fprintf(fp, "%%%%Page: %s\n", (const char *)name));
  if (number) {
    CHECKPRINT(fprintf(fp, "/pagenumtext (%s) def\n", (const char *)number));
  } else {
    CHECKPRINT(fprintf(fp, "/pagenumtext () def\n"));
  }

  error = print_start_page(fp, show->GetBoolLandscape());
  if (error) return error;

  CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", field_x, field_y));
  
  /* Draw stage */
  CHECKPRINT(fprintf(fp, "   BeginEPSF\n"));
  CHECKPRINT(fprintf(fp, "   %.2f %.2f scale\n",
		     field_w / modesprshow->StageW(),
		     field_h / modesprshow->StageH()));
  CHECKPRINT(fprintf(fp, "   %hd %hd translate\n",
		     -modesprshow->StageX(), -modesprshow->StageY()));
  CHECKPRINT(fprintf(fp, "%%%%BeginDocument: %s\n", modesprshow->StageFile()));
  if (!copy_ps_file(modesprshow->StageFile(), fp)) {
    return nofile;
  }
  CHECKPRINT(fprintf(fp, "%%%%EndDocument\n"));
  CHECKPRINT(fprintf(fp, "EndEPSF\n"));
  /* Draw field */
  CHECKPRINT(fprintf(fp, "drawfield\n"));
  if (modesprshow->WhichYards()) {
    CHECKPRINT(fprintf(fp, "/mainfont findfont %.2f scalefont setfont\n",
		       step_size * yards_size));
    if (modesprshow->WhichYards() & (SPR_YARD_ABOVE | SPR_YARD_BELOW))
      for (j=0; j <= modesprshow->StepsW(); j+=8) {
	CHECKPRINT(fprintf(fp, "/lmargin %.2f def /rmargin %.2f def\n",
			   stage_field_x + step_size * j,
			   stage_field_x + step_size * j));
	if (modesprshow->WhichYards() & SPR_YARD_ABOVE) {
	  CHECKPRINT(fprintf(fp, "/y %.2f def\n",
			     field_h*
			     (modesprshow->TextTop()-modesprshow->StageX())/
			     modesprshow->StageH()));
	  CHECKPRINT(fprintf(fp, "(%s) centerText\n",
			     yard_text[(modesprshow->StepsX() +
					(MAX_YARD_LINES-1)*4 + j)/8].Chars()));
	}
	if (modesprshow->WhichYards() & SPR_YARD_BELOW) {
	  CHECKPRINT(fprintf(fp, "/y %.2f def\n",
			     field_h *
			     (modesprshow->TextBottom() -
			      modesprshow->StageX()) /
			     modesprshow->StageH() -(step_size*yards_size)));
	  CHECKPRINT(fprintf(fp, "(%s) centerText\n",
			     yard_text[(modesprshow->StepsX() +
					(MAX_YARD_LINES-1)*4 + j)/8].Chars()));
	}
      }
    if (modesprshow->WhichYards() & (SPR_YARD_LEFT | SPR_YARD_RIGHT))
      for (j=0; j <= modesprshow->StepsH(); j+=8) {
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
	if (modesprshow->WhichYards() & SPR_YARD_RIGHT) {
	  CHECKPRINT(fprintf(fp, "(%s) leftText\n", spr_line_text[j / 8].
			     Chars()));
	}
	if (modesprshow->WhichYards() & SPR_YARD_LEFT) {
	  CHECKPRINT(fprintf(fp, "(%s) rightText\n", spr_line_text[j / 8].
			     Chars()));
	}
      }
  }
  
  dot_w = step_size / 2 * dot_ratio;
  CHECKPRINT(fprintf(fp, "/w %.4f def\n", dot_w));
  CHECKPRINT(fprintf(fp, "/plinew %.4f def\n", dot_w * pline_ratio));
  CHECKPRINT(fprintf(fp, "/slinew %.4f def\n", dot_w * sline_ratio));
  CHECKPRINT(fprintf(fp, "/numberfont findfont %.2f scalefont setfont\n",
		     dot_w * 2 * num_ratio));
  for (i = 0; i < show->GetNumPoints(); i++) {
    dot_x = stage_field_x + 
      (COORD2FLOAT(pts[i].pos.x) - modesprshow->StepsX()) /
      modesprshow->StepsW() * stage_field_w;
    dot_y = stage_field_y + stage_field_h * (1.0 -
					     (COORD2FLOAT(pts[i].pos.y)-
					      modesprshow->StepsY())/
					     modesprshow->StepsH());
    CHECKPRINT(fprintf(fp, "%.2f %.2f %s\n",
		       dot_x, dot_y, dot_routines[pts[i].sym]));
    CHECKPRINT(fprintf(fp, "(%s) %.2f %.2f %s\n",
		       show->GetPointLabel(i), dot_x, dot_y,
		       (pts[i].flags & PNT_LABEL) ? "donumber2" : "donumber"));
  }
  if (show->GetBoolDoCont()) {
    CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", -field_x, -field_y));
    error = PrintCont(fp);
    if (error) return error;
  }
  return NULL;
}

char *CC_sheet::PrintOverview(FILE *fp) {
  unsigned short i;
  char *error;
  CC_coord fieldoff = show->mode->FieldOffset();
  CC_coord fieldsize = show->mode->FieldSize();
  float fieldx = COORD2FLOAT(fieldoff.x);
  float fieldy = COORD2FLOAT(fieldoff.y);
  float fieldwidth = COORD2FLOAT(fieldsize.x);
  float fieldheight = COORD2FLOAT(fieldsize.y);

  CHECKPRINT(fprintf(fp, "%%%%Page: %s\n", (const char *)name));

  error = print_start_page(fp, show->GetBoolLandscape());
  if (error) return error;

  CHECKPRINT(fprintf(fp, "%.2f %.2f translate\n", field_x, field_y));
  CHECKPRINT(fprintf(fp, "drawfield\n"));
  CHECKPRINT(fprintf(fp, "/w %.2f def\n", width / fieldwidth * 2.0 / 3.0));
  for (i = 0; i < show->GetNumPoints(); i++) {
    CHECKPRINT(fprintf(fp, "%.2f %.2f dotbox\n",
		       (COORD2FLOAT(pts[i].pos.x)-fieldx) / fieldwidth * width,
		       (1.0 - (COORD2FLOAT(pts[i].pos.y)-
			       fieldy)/fieldheight) * height));
  }
  return NULL;
}

Bool copy_ps_file(const char *name, FILE *fp) {
  char cpbuf[1024];
  FILE *infile;
 
  infile = OpenFileInDir(name, "r");
  if (infile == NULL) return FALSE;
  while (fgets(cpbuf, 1024, infile)) {
    if (fputs(cpbuf, fp) == EOF) {
      /* Error copying file */
      fclose(infile);
      return FALSE;
    }
  }
  fclose(infile);
  return TRUE;
}
