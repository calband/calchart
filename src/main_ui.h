/* main_ui.h
 * Header for wxWindows interface
 *
 * Modification history:
 * 6-1-95     Garrick Meeker              Created
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

#ifndef _MAIN_UI_H_
#define _MAIN_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "basic_ui.h"
#include "show.h"
#include "cc_shapes.h"

#include <list>
#include <vector>

// Value of 10 translates to a canvas of 1760x1000.
#define FIELD_MAXZOOM 10

enum CC_DRAG_TYPES
{
	CC_DRAG_NONE, CC_DRAG_BOX, CC_DRAG_POLY,
	CC_DRAG_LASSO, CC_DRAG_LINE, CC_DRAG_CROSS
};
enum CC_MOVE_MODES
{
	CC_MOVE_NORMAL, CC_MOVE_LINE, CC_MOVE_ROTATE,
	CC_MOVE_SHEAR, CC_MOVE_REFL, CC_MOVE_SIZE,
	CC_MOVE_GENIUS
};

enum
{
	CALCHART__NEW_WINDOW = 1,
	CALCHART__APPEND_FILE,
	CALCHART__IMPORT_CONT_FILE,
	CALCHART__PRINT_EPS,
	CALCHART__INSERT_BEFORE,
	CALCHART__INSERT_AFTER,
	CALCHART__RELABEL,
	CALCHART__CLEAR_REF,
	CALCHART__EDIT_CONTINUITY,
	CALCHART__EDIT_PRINTCONT,
	CALCHART__SET_TITLE,
	CALCHART__SET_BEATS,
	CALCHART__SETUP,
	CALCHART__POINTS,
	CALCHART__ANIMATE,
	CALCHART__SELECTION,
	CALCHART__COLORS,
	CALCHART__ROWS,
	CALCHART__COLUMNS,
	CALCHART__NEAREST,

	CALCHART__prev_ss,
	CALCHART__next_ss,
	CALCHART__box,
	CALCHART__poly,
	CALCHART__lasso,
	CALCHART__move,
	CALCHART__line,
	CALCHART__rot,
	CALCHART__shear,
	CALCHART__reflect,
	CALCHART__size,
	CALCHART__genius,
	CALCHART__label_left,
	CALCHART__label_right,
	CALCHART__label_flip,
	CALCHART__setsym0,
	CALCHART__setsym1,
	CALCHART__setsym2,
	CALCHART__setsym3,
	CALCHART__setsym4,
	CALCHART__setsym5,
	CALCHART__setsym6,
	CALCHART__setsym7,
	CALCHART__slider_zoom,
	CALCHART__slider_sheet_callback,
	CALCHART__refnum_callback,
};
enum CC_SELECT_TYPES
{
	CC_SELECT_ROWS = CALCHART__ROWS,
	CC_SELECT_COLUMNS = CALCHART__COLUMNS,
	CC_SELECT_NEAREST = CALCHART__NEAREST,
};

class MainFrame;

class CC_WinNodeMain : public CC_WinNode
{
public:
	CC_WinNodeMain(CC_WinList *lst, MainFrame *frm);

	virtual void SetShow(CC_show *shw);
	virtual void ChangeName();
	virtual void UpdateSelections(wxWindow* win = NULL, int point = -1);
	virtual void UpdatePoints();
	virtual void UpdatePointsOnSheet(unsigned sht, int ref = -1);
	virtual void ChangeNumPoints(wxWindow *win);
	virtual void ChangePointLabels(wxWindow *win);
	virtual void ChangeShowMode(wxWindow *win);
	virtual void UpdateStatusBar();
	virtual void GotoContLocation(unsigned sht, unsigned contnum,
		int line = -1, int col = -1);
	virtual void AddSheet(unsigned sht);
	virtual void DeleteSheet(unsigned sht);
	virtual void AppendSheets();
	virtual void RemoveSheets(unsigned num);
	virtual void ChangeTitle(unsigned sht);

private:
	MainFrame *frame;
};

// Define a new application
class CalChartApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual void MacOpenFile(const wxString &fileName);
	int OnExit();
};

// Top-level frame
class TopFrame : public CC_MDIParentFrame
{
public:
	TopFrame(int width, int height);
	~TopFrame();
	void OnCloseWindow(wxCloseEvent& event);
	void OnCmdNew(wxCommandEvent& event);
	void OnCmdLoad(wxCommandEvent& event);
	void OnCmdExit(wxCommandEvent& event);
	void OnCmdAbout(wxCommandEvent& event);
	void OnCmdHelp(wxCommandEvent& event);
	void NewShow(CC_show *shw = NULL);
	void OpenShow(const wxString& filename = wxT(""));
	void Quit();
	void About();
	void Help();

	DECLARE_EVENT_TABLE()
};

class TopFrameDropTarget : public wxFileDropTarget
{
public:
	TopFrameDropTarget(TopFrame *f) : frame(f) {}
	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
private:
	TopFrame *frame;
};

class FieldCanvas;
// Define the main editing frame
class MainFrame : public CC_MDIChildFrame
{
public:
	MainFrame(wxMDIParentFrame *frame, int x, int y, int w, int h,
		CC_show *show = NULL, MainFrame *other_frame = NULL);
	~MainFrame();

	void OnCloseWindow(wxCloseEvent& event);
	void OnCmdNew(wxCommandEvent& event);
	void OnCmdNewWindow(wxCommandEvent& event);
	void OnCmdLoad(wxCommandEvent& event);
	void OnCmdAppend(wxCommandEvent& event);
	void OnCmdImportCont(wxCommandEvent& event);
	void OnCmdSave(wxCommandEvent& event);
	void OnCmdSaveAs(wxCommandEvent& event);
	void OnCmdPrint(wxCommandEvent& event);
	void OnCmdPrintEPS(wxCommandEvent& event);
	void OnCmdSelectColors(wxCommandEvent& event);
	void OnCmdClose(wxCommandEvent& event);
	void OnCmdExit(wxCommandEvent& event);
	void OnCmdUndo(wxCommandEvent& event);
	void OnCmdRedo(wxCommandEvent& event);
	void OnCmdInsertBefore(wxCommandEvent& event);
	void OnCmdInsertAfter(wxCommandEvent& event);
	void OnCmdDelete(wxCommandEvent& event);
	void OnCmdRelabel(wxCommandEvent& event);
	void OnCmdClearRef(wxCommandEvent& event);
	void OnCmdEditCont(wxCommandEvent& event);
	void OnCmdEditPrintCont(wxCommandEvent& event);
	void OnCmdSetTitle(wxCommandEvent& event);
	void OnCmdSetBeats(wxCommandEvent& event);
	void OnCmdSetup(wxCommandEvent& event);
	void OnCmdPoints(wxCommandEvent& event);
	void OnCmdAnimate(wxCommandEvent& event);
	void OnCmdSelect(int id);
	void OnCmdRows(wxCommandEvent& event);
	void OnCmdColumns(wxCommandEvent& event);
	void OnCmdNearest(wxCommandEvent& event);
	void OnCmdAbout(wxCommandEvent& event);
	void OnCmdHelp(wxCommandEvent& event);
	void OnMenuSelect(wxMenuEvent& event);

	void OnCmd_prev_ss(wxCommandEvent& event);
	void OnCmd_next_ss(wxCommandEvent& event);
	void OnCmd_box(wxCommandEvent& event);
	void OnCmd_poly(wxCommandEvent& event);
	void OnCmd_lasso(wxCommandEvent& event);
	void OnCmd_move(wxCommandEvent& event);
	void OnCmd_line(wxCommandEvent& event);
	void OnCmd_rot(wxCommandEvent& event);
	void OnCmd_shear(wxCommandEvent& event);
	void OnCmd_reflect(wxCommandEvent& event);
	void OnCmd_size(wxCommandEvent& event);
	void OnCmd_genius(wxCommandEvent& event);
	void OnCmd_label_left(wxCommandEvent& event);
	void OnCmd_label_right(wxCommandEvent& event);
	void OnCmd_label_flip(wxCommandEvent& event);
	void OnCmd_setsym0(wxCommandEvent& event);
	void OnCmd_setsym1(wxCommandEvent& event);
	void OnCmd_setsym2(wxCommandEvent& event);
	void OnCmd_setsym3(wxCommandEvent& event);
	void OnCmd_setsym4(wxCommandEvent& event);
	void OnCmd_setsym5(wxCommandEvent& event);
	void OnCmd_setsym6(wxCommandEvent& event);
	void OnCmd_setsym7(wxCommandEvent& event);

	bool OkayToClearShow();

	void LoadShow();
	void AppendShow();
	void ImportContFile();
	void SaveShow();
	void SaveShowAs();

	void SnapToGrid(CC_coord& c);
	void UpdatePanel();

	void SetCurrentLasso(CC_DRAG_TYPES type);
	void SetCurrentMove(CC_MOVE_MODES type);
	void slider_zoom_callback(wxScrollEvent &ev);
	void slider_sheet_callback(wxScrollEvent &);
	void refnum_callback(wxCommandEvent &);

	void Setup();

	wxChoice *grid_choice;
	wxChoice *ref_choice;
	wxSlider *zoom_slider;
	wxSlider *sheet_slider;

	FieldCanvas *field;
	CC_WinNodeMain *node;

	DECLARE_EVENT_TABLE()
};

class FieldCanvas : public AutoScrollCanvas
{
public:
// Basic functions
	FieldCanvas(CC_show *show, unsigned ss, MainFrame *frame,
		int def_zoom,
		FieldCanvas *from_canvas = NULL,
		int x = -1, int y = -1, int w = -1, int h = -1);
	~FieldCanvas(void);
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnScroll(wxScrollEvent& event);

// Misc show functions
	void RefreshShow(bool drawall = true, int point = -1);
	void UpdateBars();
	inline void UpdateSS() { RefreshShow(); ourframe->UpdatePanel(); }
	inline void GotoThisSS()
	{
		UpdateSS();
		ourframe->node->GotoSheet(show_descr.curr_ss);
	}
	inline void GotoSS(unsigned n)
	{
		show_descr.curr_ss = n; GotoThisSS();
	}
	inline void PrevSS()
	{
		if (show_descr.curr_ss > 0)
		{
			show_descr.curr_ss--; GotoThisSS();
		}
	}
	inline void NextSS()
	{
		if (show_descr.show)
		{
			if (++show_descr.curr_ss < show_descr.show->GetNumSheets())
			{
				GotoThisSS();
			} else --show_descr.curr_ss;
		}
	}
	inline bool SetZoomQuick(int factor)
	{
		if (factor != zoomf)
		{
			zoomf = factor;
			float f = factor * COORD2FLOAT(1);
			SetUserScale(f, f);
			return true;
		}
		return false;
	}
	inline void SetZoom(int factor)
	{
		if (SetZoomQuick(factor))
		{
			UpdateBars(); RefreshShow();
		}
	}

	void BeginDrag(CC_DRAG_TYPES type, CC_coord start);
	void BeginDrag(CC_DRAG_TYPES type, CC_shape *shape);
	void AddDrag(CC_DRAG_TYPES type, CC_shape *shape);
	void MoveDrag(CC_coord end);
	void EndDrag();

// Variables
	MainFrame *ourframe;
	CC_descr show_descr;
	CC_DRAG_TYPES curr_lasso;
	CC_MOVE_MODES curr_move;
	CC_SELECT_TYPES curr_select;
	unsigned curr_ref;

private:
	typedef std::vector<unsigned> PointList;
	void ClearShapes();
	void DrawDrag(bool on = true);
	void SelectOrdered(PointList& pointlist, const CC_coord& start);
	bool SelectWithLasso(const CC_lasso *lasso);
	bool SelectPointsInRect(const CC_coord& c1, const CC_coord& c2,
		unsigned ref = 0);

	CC_DRAG_TYPES drag;
	typedef std::vector<CC_shape*> ShapeList;
	ShapeList shape_list;
	CC_shape *curr_shape;
	bool dragon;
	int zoomf;

	DECLARE_EVENT_TABLE()
};

// we must use a list here so iterators aren't invalidated.
class MainFrameList: public std::list<MainFrame*>
{
public:
	bool CloseAllWindows();
};
#endif
