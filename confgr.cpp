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
#include "show.h"
#include <ctype.h>

#include "confgr.h"
#include "modes.h"

extern ShowModeList *modelist;

unsigned int window_default_width = 600;
unsigned int window_default_height = 450;
unsigned int undo_buffer_size = 50000;
char print_file[MAX_FNAME_LEN] = "LPT1";
char print_cmd[MAX_FNAME_LEN] = "lpr";
char print_opts[MAX_FNAME_LEN] = "";
char print_view_cmd[MAX_FNAME_LEN] = "ghostview";
char print_view_opts[MAX_FNAME_LEN] = "";
char head_font[MAX_FONT_LEN] = "Helvetica-Bold";
char main_font[MAX_FONT_LEN] = "Helvetica";
char number_font[MAX_FONT_LEN] = "Helvetica";
char cont_font[MAX_FONT_LEN] = "Courier";
char bold_font[MAX_FONT_LEN] = "Courier-Bold";
char ital_font[MAX_FONT_LEN] = "Courier-Italic";
char bold_ital_font[MAX_FONT_LEN] = "Courier-BoldItalic";
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
char yard_text[21][8] = {
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
char spr_line_text[MAX_SPR_LINES][8] = {
  "A",
  "B",
  "C",
  "D",
  "E",
};

static wxPathList configdirs;
static char runtimedir[MAX_PATH_LEN];

char *ReadConfig(void) {
  FILE *fp;
  char com_buf[1024];
  char stage_eps_file[MAX_FNAME_LEN];
  CC_coord bord1(INT2COORD(8),INT2COORD(8)), bord2(INT2COORD(8),INT2COORD(8));
  char *retmsg = NULL;
  unsigned short whash, ehash;
  short eps_stage_x, eps_stage_y, eps_stage_w, eps_stage_h;
  short eps_field_x, eps_field_y, eps_field_w, eps_field_h;
  short eps_steps_x, eps_steps_y, eps_steps_w, eps_steps_h;
  short eps_text_left, eps_text_right, eps_text_top, eps_text_bottom;
  unsigned char which_spr_yards;

  // Add './runtime'
  wxGetWorkingDirectory(runtimedir, MAX_PATH_LEN);
  strncat(runtimedir, PATH_SEPARATOR, MAX_PATH_LEN);
  strncat(runtimedir, "runtime", MAX_PATH_LEN);
  configdirs.Add(runtimedir);
  configdirs.AddEnvList("CALCHART_RT");

  fp = OpenFileInDir("config", "r");
  if (fp == NULL) {
    retmsg = "Unable to open config file.  Using default values.\n";
  } else {
    while (my_fgets(com_buf, 1024, fp) != NULL) {
      /* check for comment */
      if (com_buf[0] == '#') continue;
      if (com_buf[0] == 0) continue;
      if (strcmp("WINDOW_WIDTH", com_buf) == 0) {
	fscanf(fp, " %d \n", &window_default_width);
	continue;
      }
      if (strcmp("WINDOW_HEIGHT", com_buf) == 0) {
	fscanf(fp, " %d \n", &window_default_height);
	continue;
      }
      if (strcmp("UNDO_BUF_SIZE", com_buf) == 0) {
	fscanf(fp, " %d \n", &undo_buffer_size);
	continue;
      }
      if (strcmp("PRINT_FILE", com_buf) == 0) {
	my_fgets(print_file, MAX_FNAME_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_CMD", com_buf) == 0) {
	my_fgets(print_cmd, MAX_FNAME_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_OPTS", com_buf) == 0) {
	my_fgets(print_opts, MAX_FNAME_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_VIEW_CMD", com_buf) == 0) {
	my_fgets(print_view_cmd, MAX_FNAME_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_VIEW_OPTS", com_buf) == 0) {
	my_fgets(print_view_opts, MAX_FNAME_LEN, fp);
	continue;
      }
      if (strcmp("DEFINE_STANDARD_MODE", com_buf) == 0) {
	my_fgets(com_buf, 1024, fp);
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
	my_fgets(com_buf, 1024, fp);
	my_fgets(stage_eps_file, MAX_FNAME_LEN, fp);
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
	modelist->Add(new ShowModeSprShow(com_buf, bord1, bord2,
					  which_spr_yards, stage_eps_file,
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
	my_fgets(head_font, MAX_FONT_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_NUMBER_FONT", com_buf) == 0) {
	my_fgets(number_font, MAX_FONT_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_ANNO_FONT", com_buf) == 0) {
	my_fgets(main_font, MAX_FONT_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_FONT", com_buf) == 0) {
	my_fgets(cont_font, MAX_FONT_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_BOLD_FONT", com_buf) == 0) {
	my_fgets(bold_font, MAX_FONT_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_ITALIC_FONT", com_buf) == 0) {
	my_fgets(ital_font, MAX_FONT_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_BOLD_ITALIC_FONT", com_buf) == 0) {
	my_fgets(bold_ital_font, MAX_FONT_LEN, fp);
	continue;
      }
      if (strcmp("PRINT_YARDS", com_buf) == 0) {
	fscanf(fp, " %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s ",
	       yard_text[0], yard_text[1], yard_text[2],
	       yard_text[3], yard_text[4], yard_text[5], yard_text[6],
	       yard_text[7], yard_text[8], yard_text[9], yard_text[10],
	       yard_text[11], yard_text[12], yard_text[13], yard_text[14],
	       yard_text[15], yard_text[16], yard_text[17], yard_text[18],
	       yard_text[19], yard_text[20]);
	continue;
      }
      if (strcmp("PRINT_SPR_LINES", com_buf) == 0) {
	fscanf(fp, " %s %s %s %s %s ",
	       spr_line_text[0], spr_line_text[1], spr_line_text[2],
	       spr_line_text[3], spr_line_text[4]);
	continue;
      }
      if (!retmsg) {
	char tmpbuf[256];
	sprintf(tmpbuf, "Warning: '%s' is not recognized in config file.\n",
		com_buf);
	retmsg = copystring(tmpbuf);
      }
    }
    fclose(fp);
  }

  if (modelist->First() == NULL) {
    // No modes were defined.  Add a default
    modelist->Add(new ShowModeStandard("Standard", bord1, bord2,
				       DEF_HASH_W, DEF_HASH_E));
  }

  return retmsg;
}

// Open a file in the specified dir.
FILE *OpenFileInDir(const char *name, const char *modes) {
  FILE *fp;
  char *fullpath;

  fullpath = configdirs.FindValidPath((char *)name);
  if (fullpath == NULL) return NULL;

  fp = fopen(fullpath, modes);
  return fp;
}

// Like fgets, but strips off the newline
char *my_fgets(char *buffer, int length, FILE *fp) {
  char *p;
  int i;

  p = fgets(buffer, length, fp);
  if (p != NULL) {
    i = strlen(buffer);
    if (--i >= 0) {
      if (buffer[i] == '\n') buffer[i] = 0;
    }
    if (--i >= 0) {
      if (buffer[i] == '\r') buffer[i] = 0;
    }
  }
  return p;
}
