/*
 * main_ui.h
 * Header for wxWindows interface
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

#ifndef _MAIN_UI_H_
#define _MAIN_UI_H_

#include "basic_ui.h"
#include "cc_shapes.h"
#include "cc_show.h"
#include "background_image.h"

#include <wx/mdi.h>
#include <wx/docview.h>
#include <wx/docmdi.h>

#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>
class Animation;

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
	CALCHART__APPEND_FILE = wxID_HIGHEST,
	// the wxView print is wrong, doesn't do landscape.  rolling our own
	CALCHART__wxID_PRINT,
	CALCHART__wxID_PREVIEW,
	CALCHART__IMPORT_CONT_FILE,
	CALCHART__LEGACY_PRINT,
	CALCHART__LEGACY_PRINT_EPS,
	CALCHART__INSERT_BEFORE,
	CALCHART__INSERT_AFTER,
	CALCHART__RELABEL,
	CALCHART__EDIT_CONTINUITY,
	CALCHART__SET_SHEET_TITLE,
	CALCHART__SET_BEATS,
	CALCHART__SETUP,
	CALCHART__SETDESCRIPTION,
	CALCHART__SETMODE,
	CALCHART__POINTS,
	CALCHART__ANIMATE,
	CALCHART__SELECTION,

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
	CALCHART__draw_paths,
	CALCHART__AddBackgroundImage,
	CALCHART__AdjustBackgroundImage,
	CALCHART__RemoveBackgroundImage,
};

class FieldCanvas;
class MainFrameView;

// Define the main editing frame
class MainFrame : public wxDocMDIChildFrame
{
public:
	// MainFrame will own the show that is passed in
	MainFrame(wxDocument* doc, wxView* view, wxDocMDIParentFrame *frame, const wxPoint& pos, const wxSize& size);
	~MainFrame();

	void OnCmdAppend(wxCommandEvent& event);
	void OnCmdImportCont(wxCommandEvent& event);
	void OnCmdSave(wxCommandEvent& event);
	void OnCmdSaveAs(wxCommandEvent& event);
	void OnCmdPrint(wxCommandEvent& event);
	void OnCmdPrintPreview(wxCommandEvent& event);
	void OnCmdLegacyPrint(wxCommandEvent& event);
	void OnCmdLegacyPrintEPS(wxCommandEvent& event);
	void OnCmdPageSetup(wxCommandEvent& event);
	void OnCmdPreferences(wxCommandEvent& event);
	void OnCmdClose(wxCommandEvent& event);
	void OnCmdRedo(wxCommandEvent& event);
	void OnCmdInsertBefore(wxCommandEvent& event);
	void OnCmdInsertAfter(wxCommandEvent& event);
	void OnCmdDelete(wxCommandEvent& event);
	void OnCmdRelabel(wxCommandEvent& event);
	void OnCmdEditCont(wxCommandEvent& event);
	void OnCmdSetSheetTitle(wxCommandEvent& event);
	void OnCmdSetBeats(wxCommandEvent& event);
	void OnCmdSetup(wxCommandEvent& event);
	void OnCmdSetDescription(wxCommandEvent& event);
	void OnCmdSetMode(wxCommandEvent& event);
	void OnCmdPoints(wxCommandEvent& event);
	void OnCmdAnimate(wxCommandEvent& event);
	void OnCmdAbout(wxCommandEvent& event);
	void OnCmdHelp(wxCommandEvent& event);

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
	void OnChar(wxKeyEvent& event);

	void OnCmd_AddBackgroundImage(wxCommandEvent& event);
	void OnCmd_AdjustBackgroundImage(wxCommandEvent& event);
	void OnCmd_RemoveBackgroundImage(wxCommandEvent& event);

	void OnSize(wxSizeEvent& event);

	void AppendShow();
	void ImportContFile();

	void SnapToGrid(CC_coord& c);
	void UpdatePanel();

	void SetCurrentLasso(CC_DRAG_TYPES type);
	void SetCurrentMove(CC_MOVE_MODES type);
	void zoom_callback(wxCommandEvent &);
	void zoom_callback_textenter(wxCommandEvent &);
	void slider_sheet_callback(wxScrollEvent &);
	void refnum_callback(wxCommandEvent &);
	void OnEnableDrawPaths(wxCommandEvent &);

	void Setup();
	void SetDescription();
	void SetMode();

	const FieldCanvas * GetCanvas() const { return field; }
	FieldCanvas * GetCanvas() { return field; }

	wxChoice *grid_choice;
	wxChoice *ref_choice;
	wxComboBox *zoom_box;
	wxSlider *sheet_slider;

	FieldCanvas *field;
	
	DECLARE_EVENT_TABLE()
};

class FieldCanvas : public CtrlScrollCanvas
{
public:
// Basic functions
	FieldCanvas(wxView *view, MainFrame *frame, float def_zoom);
	virtual ~FieldCanvas(void);
	void OnPaint(wxPaintEvent& event);
	virtual void OnMouseLeftDown(wxMouseEvent& event);
	virtual void OnMouseLeftUp(wxMouseEvent& event);
	virtual void OnMouseLeftDoubleClick(wxMouseEvent& event);
	virtual void OnMouseRightDown(wxMouseEvent& event);
	virtual void OnMouseMove(wxMouseEvent& event);
	void OnChar(wxKeyEvent& event);

// Misc show functions
	void SetZoom(float factor);

	void BeginDrag(CC_DRAG_TYPES type, CC_coord start);
	void AddDrag(CC_DRAG_TYPES type, CC_shape *shape);
	void MoveDrag(CC_coord end);
	void EndDrag();

	// return true on success
	bool SetBackgroundImage(const wxImage& image);
	void AdjustBackgroundImage(bool enable);
	void RemoveBackgroundImage();
	
// Variables
	MainFrame *ourframe;
	CC_show* mShow;
	MainFrameView* mView;
	CC_DRAG_TYPES curr_lasso;
	CC_MOVE_MODES curr_move;

private:
	void ClearShapes();

	CC_DRAG_TYPES drag;
	typedef std::vector<CC_shape*> ShapeList;
	ShapeList shape_list;
	CC_shape *curr_shape;

	// Background Picture
	boost::shared_ptr<BackgroundImage> mBackgroundImage;
	
	DECLARE_EVENT_TABLE()
};

class MainFrameView : public wxView
{
public:
    MainFrame *mFrame;
  
    MainFrameView();
    ~MainFrameView();

    bool OnCreate(wxDocument *doc, long flags);
    void OnDraw(wxDC *dc);
    void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
    bool OnClose(bool deleteWindow = true);

	void OnWizardSetup(CC_show& show);

	///// Modify the show /////
	bool DoTranslatePoints(const CC_coord& pos);
	bool DoTransformPoints(const Matrix& transmat);
	bool DoMovePointsInLine(const CC_coord& start, const CC_coord& second);
	bool DoSetPointsSymbol(SYMBOL_TYPE sym);
	bool DoSetDescription(const wxString& descr);
	void DoSetMode(const wxString& mode);
	void DoSetShowInfo(unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels);
	bool DoSetSheetTitle(const wxString& descr);
	bool DoSetSheetBeats(unsigned short beats);
	bool DoSetPointsLabel(bool right);
	bool DoSetPointsLabelFlip();
	bool DoInsertSheets(const CC_show::CC_sheet_container_t& sht, unsigned where);
	bool DoDeleteSheet(unsigned where);

	int FindPoint(CC_coord pos) const;
	CC_coord PointPosition(int which) const;

	///// Change show attributes /////
	void GoToSheet(size_t which);
	void GoToNextSheet();
	void GoToPrevSheet();

	void SetReferencePoint(unsigned which);

	///// Select /////
	void SelectWithLasso(const CC_lasso *lasso, bool toggleSelected);
	void SelectPointsInRect(const CC_coord& c1, const CC_coord& c2, bool toggleSelected);
private:
	typedef std::vector<unsigned> PointList;
	void SelectOrdered(PointList& pointlist, bool toggleSelected);

	///// Drawing marcher's paths /////
public:
	// call this when we need to generate the marcher's paths.
	void OnEnableDrawPaths(bool enable);
private:
	void DrawPaths(wxDC& dc, const CC_sheet& sheet);
	void GeneratePaths();
	boost::shared_ptr<Animation> mAnimation;
	bool mDrawPaths;
	
private:
	CC_show* mShow;
	unsigned mCurrentReferencePoint;

    DECLARE_DYNAMIC_CLASS(MainFrameView)
};

#endif
