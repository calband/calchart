/* config.c
 * Basic configuration initialization for all systems
 * Config directory is pointed to by environment variable CALCHART_RT
 * This directory contains a file called "config" that sets basic options
 *
 * Modification history:
 * 1-7-95     Garrick Meeker              Created
 * 7-10-95    Garrick Meeker              Converted to C++
 *
 */

#include <wx_utils.h>
#include <wx_gdi.h>
#include "show.h"
#include <ctype.h>

#include "confgr.h"
#include "modes.h"

extern ShowModeList *modelist;

static char* ColorNames[COLOR_NUM] = {
  "FIELD",
  "FIELD DETAIL",
  "FIELD TEXT",
  "POINT",
  "POINT TEXT",
  "HILIT POINT",
  "HILIT POINT TEXT",
  "REF POINT",
  "REF POINT TEXT",
  "HILIT REF POINT",
  "HILIT REF POINT TEXT",
  "ANIM FRONT",
  "ANIM BACK",
  "ANIM SIDE",
  "HILIT ANIM FRONT",
  "HILIT ANIM BACK",
  "HILIT ANIM SIDE",
  "ANIM COLLISION",
  "SHAPES"
};

static char* DefaultColors[COLOR_NUM] = {
  "FOREST GREEN",
  "WHITE",
  "BLACK",
  "WHITE",
  "BLACK",
  "YELLOW",
  "BLACK",
  "PURPLE",
  "BLACK",
  "PURPLE",
  "BLACK",
  "WHITE",
  "YELLOW",
  "SKY BLUE",
  "RED",
  "RED",
  "RED",
  "PURPLE",
  "RED"
};

static Bool DefaultMonoColors[COLOR_NUM] = {
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

static int DefaultMonoPenHollow[COLOR_NUM] = {
  0,
  0,
  0,
  0,
  0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  0
};

static int DefaultMonoPenWidth[COLOR_NUM] = {
  1,
  1,
  1,
  1,
  1,
  3,
  1,
  1,
  1,
  3,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

static int DefaultMonoPenDotted[COLOR_NUM] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

wxColourMap *CalChartColorMap;
wxPen *CalChartPens[COLOR_NUM];
wxBrush *CalChartBrushes[COLOR_NUM];

wxString program_dir;
wxString shows_dir;
wxString autosave_dir;
wxString autosave_dirname = AUTOSAVE_VAR;
unsigned int window_default_width = 600;
unsigned int window_default_height = 450;
unsigned int undo_buffer_size = 50000;
unsigned int autosave_interval = 300;
unsigned int default_zoom = 5;
wxString print_file = "LPT1";
wxString print_cmd = "lpr";
wxString print_opts = "";
wxString print_view_cmd = "ghostview";
wxString print_view_opts = "";
wxString head_font = "Helvetica-Bold";
wxString main_font = "Helvetica";
wxString number_font = "Helvetica";
wxString cont_font = "Courier";
wxString bold_font = "Courier-Bold";
wxString ital_font = "Courier-Italic";
wxString bold_ital_font = "Courier-BoldItalic";
float page_width = 7.5;
float page_height = 10.0;
float paper_length = 11.0;
float page_offset_x = 0.5;
float page_offset_y = 0.5;
float header_size = 3.0;
float yards_size = 1.5;
float text_size = 10.0;
float dot_ratio = 1.0;
float num_ratio = 1.2;
float pline_ratio = 1.0;
float sline_ratio = 1.0;
float cont_ratio = 0.25;
wxString yard_text[21] = {
  "0",
  "5",
  "10",
  "15",
  "20",
  "25",
  "30",
  "35",
  "40",
  "45",
  "50",
  "45",
  "40",
  "35",
  "30",
  "25",
  "20",
  "15",
  "10",
  "5",
  "0",
};
wxString spr_line_text[MAX_SPR_LINES] = {
  "A",
  "B",
  "C",
  "D",
  "E",
};

static wxPathList configdirs;
static wxString runtime_dir;

char *ReadConfig(const char *path) {
  FILE *fp;
  int i;
  wxString com_buf;
  CC_coord bord1(INT2COORD(8),INT2COORD(8)), bord2(INT2COORD(8),INT2COORD(8));
  char *retmsg = NULL;
  unsigned short whash, ehash;
  short eps_stage_x, eps_stage_y, eps_stage_w, eps_stage_h;
  short eps_field_x, eps_field_y, eps_field_w, eps_field_h;
  short eps_steps_x, eps_steps_y, eps_steps_w, eps_steps_h;
  short eps_text_left, eps_text_right, eps_text_top, eps_text_bottom;
  unsigned char which_spr_yards;
  wxPathList tmp_configdirs;

  char *tmpstr = wxGetWorkingDirectory();
  program_dir = tmpstr;
  delete [] tmpstr;

  // Get default path
  tmpstr = FullPath(path);
  runtime_dir = tmpstr;
  delete [] tmpstr;

  // Set search path for files
  tmp_configdirs.AddEnvList("CALCHART_RT");
  tmp_configdirs.Add(runtime_dir.GetData());

  fp = OpenFileInDir("config", "r", &tmp_configdirs);
  if (fp == NULL) {
    retmsg = "Unable to open config file.  Using default values.\n";
  } else {
    while (!feof(fp)) {
      ReadDOSline(fp, com_buf);

      if (com_buf.Empty()) continue;
      /* check for comment */
      if (com_buf.Elem(0) == '#') continue;
      if (strcmp("PROGRAM_DIR", com_buf) == 0) {
	ReadDOSline(fp, program_dir);
	continue;
      }
      if (strcmp("SHOWS_DIR", com_buf) == 0) {
	ReadDOSline(fp, shows_dir);
	continue;
      }
      if (strcmp("AUTOSAVE_DIR", com_buf) == 0) {
	ReadDOSline(fp, autosave_dirname);
	continue;
      }
      if (strcmp("RUNTIME_DIR", com_buf) == 0) {
	ReadDOSline(fp, runtime_dir);
	continue;
      }
      if (strcmp("WINDOW_WIDTH", com_buf) == 0) {
	fscanf(fp, " %d \n", &window_default_width);
	continue;
      }
      if (strcmp("WINDOW_HEIGHT", com_buf) == 0) {
	fscanf(fp, " %d \n", &window_default_height);
	continue;
      }
      if (strcmp("DEFAULT_ZOOM", com_buf) == 0) {
	fscanf(fp, " %d \n", &default_zoom);
	continue;
      }
      if (strcmp("UNDO_BUF_SIZE", com_buf) == 0) {
	fscanf(fp, " %d \n", &undo_buffer_size);
	continue;
      }
      if (strcmp("AUTOSAVE_INTERVAL", com_buf) == 0) {
	fscanf(fp, " %d \n", &autosave_interval);
	continue;
      }
      if (strcmp("PRINT_FILE", com_buf) == 0) {
	ReadDOSline(fp, print_file);
	continue;
      }
      if (strcmp("PRINT_CMD", com_buf) == 0) {
	ReadDOSline(fp, print_cmd);
	continue;
      }
      if (strcmp("PRINT_OPTS", com_buf) == 0) {
	ReadDOSline(fp, print_opts);
	continue;
      }
      if (strcmp("PRINT_VIEW_CMD", com_buf) == 0) {
	ReadDOSline(fp, print_view_cmd);
	continue;
      }
      if (strcmp("PRINT_VIEW_OPTS", com_buf) == 0) {
	ReadDOSline(fp, print_view_opts);
	continue;
      }
      if (strcmp("DEFINE_STANDARD_MODE", com_buf) == 0) {
	ReadDOSline(fp, com_buf);
	fscanf(fp, " %hu %hu \n", &whash, &ehash);
	fscanf(fp, " %hu %hu %hu %hu \n",
	       &bord1.x, &bord1.y, &bord2.x, &bord2.y);
	bord1.x = INT2COORD(bord1.x);
	bord1.y = INT2COORD(bord1.y);
	bord2.x = INT2COORD(bord2.x);
	bord2.y = INT2COORD(bord2.y);
	modelist->Add(new ShowModeStandard(com_buf, bord1, bord2,
					   whash, ehash));
	continue;
      }
      if (strcmp("DEFINE_SPRSHOW_MODE", com_buf) == 0) {
	wxString mode_name;
	ReadDOSline(fp, mode_name);
	ReadDOSline(fp, com_buf);
	fscanf(fp, " %c \n", &which_spr_yards);
	if (which_spr_yards <= '9') {
	  which_spr_yards -= '0';
	} else {
	  which_spr_yards = toupper(which_spr_yards) - 'A' + 10;
	}
	fscanf(fp, " %hu %hu %hu %hu \n",
	       &bord1.x, &bord1.y, &bord2.x, &bord2.y);
	bord1.x = INT2COORD(bord1.x);
	bord1.y = INT2COORD(bord1.y);
	bord2.x = INT2COORD(bord2.x);
	bord2.y = INT2COORD(bord2.y);
	fscanf(fp, " %hd %hd %hd %hd \n", &eps_stage_x, &eps_stage_y,
	       &eps_stage_w, &eps_stage_h);
	fscanf(fp, " %hd %hd %hd %hd \n", &eps_field_x, &eps_field_y,
	       &eps_field_w, &eps_field_h);
	fscanf(fp, " %hd %hd %hd %hd \n", &eps_steps_x, &eps_steps_y,
	       &eps_steps_w, &eps_steps_h);
	fscanf(fp, " %hd %hd %hd %hd \n", &eps_text_left, &eps_text_right,
	       &eps_text_top, &eps_text_bottom);
	modelist->Add(new ShowModeSprShow(mode_name.Chars(), bord1, bord2,
					  which_spr_yards, com_buf,
					  eps_stage_x, eps_stage_y,
					  eps_stage_w, eps_stage_h,
					  eps_field_x, eps_field_y,
					  eps_field_w, eps_field_h,
					  eps_steps_x, eps_steps_y,
					  eps_steps_w, eps_steps_h,
					  eps_text_left, eps_text_right,
					  eps_text_top, eps_text_bottom));
	continue;
      }
      if (strcmp("PRINT_PAGE_WIDTH", com_buf) == 0) {
	fscanf(fp, " %f \n", &page_width);
	continue;
      }
      if (strcmp("PRINT_PAGE_HEIGHT", com_buf) == 0) {
	fscanf(fp, " %f \n", &page_height);
	continue;
      }
      if (strcmp("PRINT_PAPER_LENGTH", com_buf) == 0) {
	fscanf(fp, " %f \n", &paper_length);
	continue;
      }
      if (strcmp("PRINT_LEFT_MARGIN", com_buf) == 0) {
	fscanf(fp, " %f \n", &page_offset_x);
	continue;
      }
      if (strcmp("PRINT_TOP_MARGIN", com_buf) == 0) {
	fscanf(fp, " %f \n", &page_offset_y);
	continue;
      }
      if (strcmp("PRINT_HEADER_SIZE", com_buf) == 0) {
	fscanf(fp, " %f \n", &header_size);
	continue;
      }
      if (strcmp("PRINT_YARDS_SIZE", com_buf) == 0) {
	fscanf(fp, " %f \n", &yards_size);
	continue;
      }
      if (strcmp("PRINT_TEXT_SIZE", com_buf) == 0) {
	fscanf(fp, " %f \n", &text_size);
	continue;
      }
      if (strcmp("PRINT_DOT_RATIO", com_buf) == 0) {
	fscanf(fp, " %f \n", &dot_ratio);
	continue;
      }
      if (strcmp("PRINT_NUM_RATIO", com_buf) == 0) {
	fscanf(fp, " %f \n", &num_ratio);
	continue;
      }
      if (strcmp("PRINT_PLAIN_LINE", com_buf) == 0) {
	fscanf(fp, " %f \n", &pline_ratio);
	continue;
      }
      if (strcmp("PRINT_SOLID_LINE", com_buf) == 0) {
	fscanf(fp, " %f \n", &sline_ratio);
	continue;
      }
      if (strcmp("PRINT_CONT_RATIO", com_buf) == 0) {
	fscanf(fp, " %f \n", &cont_ratio);
	continue;
      }
      if (strcmp("PRINT_HEADER_FONT", com_buf) == 0) {
	ReadDOSline(fp, head_font);
	continue;
      }
      if (strcmp("PRINT_NUMBER_FONT", com_buf) == 0) {
	ReadDOSline(fp, number_font);
	continue;
      }
      if (strcmp("PRINT_ANNO_FONT", com_buf) == 0) {
	ReadDOSline(fp, main_font);
	continue;
      }
      if (strcmp("PRINT_FONT", com_buf) == 0) {
	ReadDOSline(fp, cont_font);
	continue;
      }
      if (strcmp("PRINT_BOLD_FONT", com_buf) == 0) {
	ReadDOSline(fp, bold_font);
	continue;
      }
      if (strcmp("PRINT_ITALIC_FONT", com_buf) == 0) {
	ReadDOSline(fp, ital_font);
	continue;
      }
      if (strcmp("PRINT_BOLD_ITALIC_FONT", com_buf) == 0) {
	ReadDOSline(fp, bold_ital_font);
	continue;
      }
      if (strcmp("PRINT_YARDS", com_buf) == 0) {
	char yardbuf[16];
	for (i=0; i<21; i++) {
	  fscanf(fp, " %s ", yardbuf);
	  yard_text[i] = yardbuf;
	}
	continue;
      }
      if (strcmp("PRINT_SPR_LINES", com_buf) == 0) {
	char yardbuf[16];
	for (i=0; i<MAX_SPR_LINES; i++) {
	  fscanf(fp, " %s ", yardbuf);
	  spr_line_text[i] = yardbuf;
	}
	continue;
      }
      if (strcmp("COLOR", com_buf) == 0) {
	int mono, hollow, width, dotted;
	char coltype_buf[256];
	char colname_buf[256];

	fscanf(fp, " \"%[^\"]\" \"%[^\"]\" %d %d %d %d ",
	       coltype_buf, colname_buf, &mono, &hollow, &width, &dotted);
	for (i=0; i<COLOR_NUM; i++) {
	  if (strcmp(ColorNames[i], coltype_buf) == 0) {
	    if (wxColourDisplay()) {
	      unsigned r, g, b;
	      wxColour *c;
	      
	      if (sscanf(colname_buf, " %u %u %u ", &r, &g, &b) == 3) {
		c = new wxColour(r, g, b);
	      } else {
		c = new wxColour(colname_buf);
	      }
	      CalChartPens[i] = wxThePenList->FindOrCreatePen(c, 1, wxSOLID);
	      CalChartBrushes[i] = wxTheBrushList->FindOrCreateBrush(c,
								     wxSOLID);
	      delete c;
	    } else {
	      CalChartPens[i] =
		wxThePenList->FindOrCreatePen(mono ? wxWHITE:wxBLACK, width,
					      dotted ? wxDOT:wxSOLID);
	      CalChartBrushes[i] = hollow ? wxTRANSPARENT_BRUSH :
		(mono ? wxWHITE_BRUSH:wxBLACK_BRUSH);
	    }
	    break;
	  }
	}
	if (i == COLOR_NUM) {
	  wxString tmpbuf;
	  tmpbuf.sprintf("Warning: color '%s' in config file is not recognized.\n",
			 coltype_buf);
	  retmsg = copystring(tmpbuf.GetData());
	}
	continue;
      }
      if (!retmsg) {
	wxString tmpbuf;
	tmpbuf.sprintf("Warning: '%s' is not recognized in config file.\n",
		       com_buf.GetData());
	retmsg = copystring(tmpbuf.GetData());
      }
    }
    fclose(fp);
  }

  if (autosave_dirname.Firstchar() == '$') {
    const char *d;
    if ((d = getenv(autosave_dirname.GetData()+1)) != NULL) {
      autosave_dir = d;
    } else {
      autosave_dir = AUTOSAVE_DIR;
    }
  } else {
    autosave_dir = autosave_dirname;
  }

  if (modelist->First() == NULL) {
    // No modes were defined.  Add a default
    modelist->Add(new ShowModeStandard("Standard", bord1, bord2,
				       DEF_HASH_W, DEF_HASH_E));
  }

  if (wxColourDisplay()) {
    unsigned char *rd, *gd, *bd;
    int j;
    int n = 0;
    wxColour *c1, *c2;

    for (i=0; i<COLOR_NUM; i++) {
      unsigned r, g, b;
      wxColour *c;
      
      if (sscanf(DefaultColors[i], " %u %u %u ", &r, &g, &b) == 3) {
	c = new wxColour(r, g, b);
      } else {
	c = new wxColour(DefaultColors[i]);
      }
      if (CalChartPens[i] == NULL) {
	CalChartPens[i] = wxThePenList->FindOrCreatePen(c, 1, wxSOLID);
      }
      if (CalChartBrushes[i] == NULL) {
	CalChartBrushes[i] = wxTheBrushList->FindOrCreateBrush(c, wxSOLID);
      }
      delete c;
    }
    rd = new unsigned char[COLOR_NUM];
    gd = new unsigned char[COLOR_NUM];
    bd = new unsigned char[COLOR_NUM];
    CalChartColorMap = new wxColourMap();
    for (i = 0; i < COLOR_NUM; i++) {
      /* old code to prevent white and black from being allocated
      for (j = -2; j < i; j++) {
	c1 = &CalChartPens[i]->GetColour();
	if (j >= 0) {
	  c2 = &CalChartPens[j]->GetColour();
	} else {
	  if (j < -1) {
	    c2 = wxBLACK;
	  } else {
	    c2 = wxWHITE;
	  }
	}
	*/
      for (j = 0; j < i; j++) {
	c1 = &CalChartPens[i]->GetColour();
	if (j >= 0) {
	  c2 = &CalChartPens[j]->GetColour();
	} else {
	  if (j < -1) {
	    c2 = wxBLACK;
	  } else {
	    c2 = wxWHITE;
	  }
	}
	if ((c1->Red() == c2->Red()) &&
	    (c1->Green() == c2->Green()) &&
	    (c1->Blue() == c2->Blue())) {
	  break;
	}
      }
      if (i == j) {
	CalChartPens[i]->GetColour().Get(&rd[n], &gd[n], &bd[n]);
	n++;
      }
    }
    CalChartColorMap->Create(n, rd, gd, bd);
    delete [] rd;
    delete [] gd;
    delete [] bd;
  } else {
    for (i=0; i<COLOR_NUM; i++) {
      if (CalChartPens[i] == NULL) {
	CalChartPens[i] =
	  wxThePenList->FindOrCreatePen(DefaultMonoColors[i] ? wxWHITE:wxBLACK,
					DefaultMonoPenWidth[i],
					DefaultMonoPenDotted[i]?wxDOT:wxSOLID);
      }
      if (CalChartBrushes[i] == NULL) {
	CalChartBrushes[i] =
	  DefaultMonoPenHollow[i] ? wxTRANSPARENT_BRUSH :
	  (DefaultMonoColors[i] ? wxWHITE_BRUSH:wxBLACK_BRUSH);
      }
    }
  }
  // Set search path for files
  configdirs.AddEnvList("CALCHART_RT");
  configdirs.Add(runtime_dir.GetData());

  return retmsg;
}

// Open a file in the specified dir.
FILE *OpenFileInDir(const char *name, const char *modes, wxPathList *list) {
  FILE *fp;
  char *fullpath;

  if (!list) list = &configdirs;
  fullpath = list->FindValidPath((char *)name);
  if (fullpath == NULL) return NULL;

  fp = fopen(fullpath, modes);
  return fp;
}

char *FullPath(const char *path) {
  if (wxIsAbsolutePath(path)) {
    return copystring(path);
  } else {
    // make into a full path
    wxString newpath;
    char *cdw = wxGetWorkingDirectory();
    newpath = cdw;
    delete [] cdw;
    newpath.Append(PATH_SEPARATOR);
    newpath.Append(path);
    return copystring(newpath.GetData());
  }
}

int ReadDOSline(FILE *fp, wxString& str) {
  int c = Readline(fp, str);
  if (c > 0) {
    if (str[str.Length()-1] == '\r') {
      str.RemoveLast();
      return c-1;
    }
  }
  return c;
}
