/*
 * field_frame.h
 * Frame for the field window
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#pragma once

#include "cc_types.h"

#include <wx/docview.h>

class CC_coord;
class wxComboBox;
class wxSlider;
class CC_show;

class FieldCanvas;
class FieldView;
class CalChartDoc;
class CalChartConfiguration;

// Define the main editing frame
class FieldFrame : public wxDocChildFrame
{
	typedef wxDocChildFrame super;
public:
	// FieldFrame will own the show that is passed in
	FieldFrame(wxDocument* doc, wxView* view, CalChartConfiguration& config_, wxDocParentFrame *frame, const wxPoint& pos, const wxSize& size);
	~FieldFrame();

	void OnCmdAppend(wxCommandEvent& event);
	void OnCmdImportCont(wxCommandEvent& event);
	void OnCmdSave(wxCommandEvent& event);
	void OnCmdSaveAs(wxCommandEvent& event);
	void OnCmdPrint(wxCommandEvent& event);
	void OnCmdPrintPreview(wxCommandEvent& event);
	void OnCmdLegacyPrint(wxCommandEvent& event);
	void OnCmdLegacyPrintEPS(wxCommandEvent& event);
	void OnCmdPageSetup(wxCommandEvent& event);
    void OnCmdExportViewerFile(wxCommandEvent& event);
	void OnCmdPreferences(wxCommandEvent& event);
	void OnCmdClose(wxCommandEvent& event);
	void OnCmdRedo(wxCommandEvent& event);
	void OnCmdInsertBefore(wxCommandEvent& event);
	void OnCmdInsertAfter(wxCommandEvent& event);
    void OnCmdInsertFromOtherShow(wxCommandEvent& event);
	void OnCmdCopySheet(wxCommandEvent& event);
	void OnCmdPasteSheet(wxCommandEvent& event);
	void OnCmdDelete(wxCommandEvent& event);
	void OnCmdRelabel(wxCommandEvent& event);
	void OnCmdEditCont(wxCommandEvent& event);
	void OnCmdEditPrintCont(wxCommandEvent& event);
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
	void OnCmd_swap(wxCommandEvent& event);
	void OnCmd_line(wxCommandEvent& event);
	void OnCmd_rot(wxCommandEvent& event);
	void OnCmd_shear(wxCommandEvent& event);
	void OnCmd_reflect(wxCommandEvent& event);
	void OnCmd_size(wxCommandEvent& event);
	void OnCmd_genius(wxCommandEvent& event);
	void OnCmd_label_left(wxCommandEvent& event);
	void OnCmd_label_right(wxCommandEvent& event);
	void OnCmd_label_flip(wxCommandEvent& event);
	void OnCmd_label_hide(wxCommandEvent& event);
	void OnCmd_label_show(wxCommandEvent& event);
	void OnCmd_label_visibility_toggle(wxCommandEvent& event);
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

	void OnCmd_GhostOption(wxCommandEvent& event);

	void OnCmd_ResetReferencePoint(wxCommandEvent& event);

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

	const FieldCanvas * GetCanvas() const { return mCanvas; }
	FieldCanvas * GetCanvas() { return mCanvas; }

	const FieldView * GetFieldView() const;
	FieldView * GetFieldView();

	const CalChartDoc * GetShow() const;
	CalChartDoc * GetShow();
	
	wxChoice *mGridChoice;
	wxChoice *mRefChoice;
	wxComboBox *mZoomBox;
	wxSlider *mSheetSlider;

	FieldCanvas *mCanvas;
	
	wxWindow *mAnimationFrame;
	CalChartConfiguration& config;
	void ClearAnimationFrame();
	
	DECLARE_EVENT_TABLE()
private:
	void refreshGhostOptionStates();
};