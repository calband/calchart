/* config.h
 * Definitions for the configuration routines
 *
 * Modification history:
 * 1-7-95     Garrick Meeker              Created
 * 7-10-95    Garrick Meeker              Converted to C++
 *
 */

#ifndef _CONFGR_H_
#define _CONFGR_H_

#define MAX_SPR_LINES 5

#define MAX_PATH_LEN 512
#define MAX_FNAME_LEN 128
#define MAX_FONT_LEN 100

extern unsigned int window_default_width;
extern unsigned int window_default_height;
extern unsigned int undo_buffer_size;
extern char print_file[MAX_FNAME_LEN];
extern char print_cmd[MAX_FNAME_LEN];
extern char print_opts[MAX_FNAME_LEN];
extern char print_view_cmd[MAX_FNAME_LEN];
extern char print_view_opts[MAX_FNAME_LEN];
extern char head_font[MAX_FONT_LEN];
extern char main_font[MAX_FONT_LEN];
extern char number_font[MAX_FONT_LEN];
extern char cont_font[MAX_FONT_LEN];
extern char bold_font[MAX_FONT_LEN];
extern char ital_font[MAX_FONT_LEN];
extern char bold_ital_font[MAX_FONT_LEN];
extern float page_width;
extern float page_height;
extern float paper_length;
extern float page_offset_x;
extern float page_offset_y;
extern float header_size;
extern float yards_size;
extern float text_size;
extern float dot_ratio;
extern float num_ratio;
extern float pline_ratio;
extern float sline_ratio;
extern float cont_ratio;
extern char yard_text[21][8];
extern char spr_line_text[MAX_SPR_LINES][8];

extern char *ReadConfig(void);
extern FILE *OpenFileInDir(const char *name, const char *modes);
extern char *my_fgets(char *buffer, int length, FILE *fp);

#endif
