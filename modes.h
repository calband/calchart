/* modes.h
 * Definitions for the show mode classes
 *
 * Modification history:
 * 9-4-95     Garrick Meeker              Created
 *
 */

#ifndef _MODES_H_
#define _MODES_H_

#include "show.h"

enum SHOW_TYPE { SHOW_STANDARD, SHOW_SPRINGSHOW };

#define SPR_YARD_LEFT 8
#define SPR_YARD_RIGHT 4
#define SPR_YARD_ABOVE 2
#define SPR_YARD_BELOW 1

class ShowMode
{
public:
  ShowMode(const char *nam, CC_coord siz, CC_coord off,
	   CC_coord bord1, CC_coord bord2);
  ShowMode(const char *nam, CC_coord siz, CC_coord bord1, CC_coord bord2);
  virtual ~ShowMode();

  virtual SHOW_TYPE GetType() = 0;
  virtual void Draw(wxDC *dc) = 0;
  virtual void DrawAnim(wxDC *dc) = 0;
  inline CC_coord& Offset() { return offset; };
  inline CC_coord FieldOffset() { return -(offset-border1); }
  inline CC_coord& Size() { return size; };
  inline CC_coord FieldSize() { return size-border1-border2; }
  inline CC_coord MinPosition() { return -offset; }
  inline CC_coord MaxPosition() { return size-offset; }
  inline const char *GetName() { return name; };
  CC_coord ClipPosition(const CC_coord& pos);

  ShowMode *next;

protected:
  CC_coord offset, size;
  CC_coord border1, border2;
private:
  char *name;
};

class ShowModeStandard : public ShowMode
{
public:
  ShowModeStandard(const char *nam, CC_coord bord1, CC_coord bord2,
		   unsigned short whash, unsigned short ehash);
  ~ShowModeStandard();

  SHOW_TYPE GetType();
  void Draw(wxDC *dc);
  void DrawAnim(wxDC *dc);
  inline unsigned short HashW() { return hashw; }
  inline unsigned short HashE() { return hashe; }

private:
  unsigned short hashw, hashe;
};

class ShowModeSprShow : public ShowMode
{
public:
  // Look at calchart.cfg for description of arguments
  ShowModeSprShow(const char *nam, CC_coord bord1, CC_coord bord2,
		  unsigned char which, const char *file,
		  short stg_x, short stg_y,
		  short stg_w, short stg_h,
		  short fld_x, short fld_y,
		  short fld_w, short fld_h,
		  short stps_x, short stps_y,
		  short stps_w, short stps_h,
		  short txt_l, short txt_r,
		  short txt_tp, short txt_bm);
  ~ShowModeSprShow();

  SHOW_TYPE GetType();
  void Draw(wxDC *dc);
  void DrawAnim(wxDC *dc);
  inline const char *StageFile() { return stagefile; }
  inline unsigned char WhichYards() { return which_yards; }
  inline short StageX() { return stage_x; }
  inline short StageY() { return stage_y; }
  inline short StageW() { return stage_w; }
  inline short StageH() { return stage_h; }
  inline short FieldX() { return field_x; }
  inline short FieldY() { return field_y; }
  inline short FieldW() { return field_w; }
  inline short FieldH() { return field_h; }
  inline short StepsX() { return steps_x; }
  inline short StepsY() { return steps_y; }
  inline short StepsW() { return steps_w; }
  inline short StepsH() { return steps_h; }
  inline short TextLeft() { return text_left; }
  inline short TextRight() { return text_right; }
  inline short TextTop() { return text_top; }
  inline short TextBottom() { return text_bottom; }

private:
  char *stagefile;
  unsigned char which_yards;
  short stage_x, stage_y, stage_w, stage_h;
  short field_x, field_y, field_w, field_h;
  short steps_x, steps_y, steps_w, steps_h;
  short text_left, text_right, text_top, text_bottom;
};

class ShowModeList
{
public:
  ShowModeList();
  ~ShowModeList();

  void Add(ShowMode *mode);
  ShowMode *Find(char *name);
  ShowMode *Default();
  inline ShowMode *First() { return list; }

private:
  ShowMode *list;
};

#endif
