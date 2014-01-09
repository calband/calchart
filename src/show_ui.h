/*
 * show_ui.h
 * Classes for interacting with shows
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

#ifndef _SHOW_UI_H_
#define _SHOW_UI_H_

#include "calchartdoc.h"
#include <wx/wizard.h>
#include <wx/docview.h>

#include <vector>

/**
 * A view of the CalChart doc that is used to make sure that the PointPicker
 * is updated when the doc is updated.
 * The PointPicker creates this view and links it to a CalChart doc.
 * Because the PointPicker needs to update when the active selection of
 * points changes, the PointPicker needs to recieve updates from the CalChart
 * doc. Because the CalChart doc only updates views that are attached to it,
 * the PointPicker creates this view and attaches it to the doc. Whenever this
 * view recieves updates, it informs the PointPicker.
 * The view's containing frame is its associated PointPicker.
 */
class PointPickerView : public wxView
{
public:
	/**
	 * Makes the view.
	 */
	PointPickerView();
	/**
	 * Cleanup.
	 */
	~PointPickerView();
	/**
	 * Does nothing. The view only exists as a means of collecting updates for
	 * the PointPicker.
	 * @param dc Unused.
	 */
    virtual void OnDraw(wxDC *dc);
	/**
	 * Called when the view recieves an update. This view simply forwards
	 * the update to its associated PointPicker.
	 * @param sender The view that sent the update.
	 * @param hint The message associated with the update, which provides
	 * the reason for the update.
	 */
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
};

/**
 * The dialog through which the user can select/deselect points.
 * This dialog appears when the user selects the "Point Selections"
 * option in the edit menu of the field frame.
 */
class PointPicker : public wxDialog
{
public:
	/**
	 * Makes the point picker.
	 * @param shw The show which this point picker will modify the
	 * selection of.
	 * @param parent The parent for this dialog. If the parent is
	 * closed, this dialog will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the dialog box.
	 * @param pos The position of the dialog.
	 * @param size The size of the dialog.
	 * @param style The style in which the dialog will be drawn.
	 */
	PointPicker(CalChartDoc& shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Select Points"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~PointPicker();

	/**
	 * Called when the CalChartDoc updates its views. This method
	 * is called by a PointPickerView, and makes sure that the
	 * selection in the PointPicker matches the selection in the
	 * show itself.
	 */
	void Update();

private:
	/**
	 * The show whose active selection is being modified.
	 */
	CalChartDoc& mShow;
	/**
	 * The view that is collecting updates from the CalChartDoc
	 * for this object.
	 */
	PointPickerView *mView;
	/**
	 * The list of points that is displayed to the user when this
	 * dialogue is open, from which the user can select/deselect points.
	 */
	wxListBox *mList;
	/**
	 * A list of all point labels (regardless of whether or not they
	 * are selected).
	 */
	std::vector<wxString> mCachedLabels;
	/**
	 * A list of all of the points that are currently selected.
	 */
	SelectionList mCachedSelection;

	/**
	* Called while setting up the dialog.
	* @param parent The parent for this dialog. If the parent is
	* closed, this dialog will be closed too.
	* @param id The identity of the window.
	* @param caption The title of the dialog box.
	* @param pos The position of the dialog.
	* @param size The size of the dialog.
	* @param style The style in which the dialog will be drawn.
	*/
	bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Select Points"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	/**
	* Sets up the GUI components through which the user provides
	* input.
	*/
	void CreateControls();

	/**
	 * Selects all points in the show. The change in the show's selection will
	 * cause the show to update all of its views, which will cause the update()
	 * method of this object to be called, thereby making sure that the
	 * selection in this object matches the selection in the show.
	 */
	void PointPickerAll(wxCommandEvent&);
	/**
	 * Deselects all points in the show. The change in the show's selection will
	 * cause the show to update all of its views, which will cause the update()
	 * method of this object to be called, thereby making sure that the
	 * selection in this object matches the selection in the show.
	 */
	void PointPickerNone(wxCommandEvent&);
	/**
	 * Makes sure that the selection which the user defined in the PointPicker
	 * becomes the active selection in the show itself. This is used to
	 * select/deselect individual points. The change in the show's selection will
	 * cause the show to update all of its views, which will cause the update()
	 * method of this object to be called, thereby making sure that the
	 * selection in this object matches the selection in the show.
	 */
	void PointPickerSelect(wxCommandEvent&);

	DECLARE_EVENT_TABLE()
};

/**
 * A frame that appears when the "Set Up Marchers.." option is selected from
 * the 'Edit' menu of CalChart. This frame gives the user control of properties
 * of the show, such as the labels for the dots, and the number of dots in
 * the show. This is the first page of the setup wizard.
 */
class ShowInfoReq : public wxDialog
{
	DECLARE_CLASS( ShowInfoReq )
	DECLARE_EVENT_TABLE()

public:
	/**
	 * Creates the dialog.
	 * @param shw The document which this dialog will edit.
	 * @param parent The parent for this dialog. If the parent is
	 * closed, this dialog will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the dialog box.
	 * @param pos The position of the dialog.
	 * @param size The size of the dialog.
	 * @param style The style in which the dialog will be drawn.
	 */
	ShowInfoReq(CalChartDoc& shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Show Info"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	/**
	 * Cleanup.
	 */
	~ShowInfoReq( );

private:
	/**
	 * Called while setting up the dialog.
	 * @param parent The parent for this dialog. If the parent is
	 * closed, this dialog will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the dialog box.
	 * @param pos The position of the dialog.
	 * @param size The size of the dialog.
	 * @param style The style in which the dialog will be drawn.
	 */
	bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Show Info"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	/**
	 * Sets up the GUI components through which the user provides
	 * input.
	 */
	void CreateControls();

	/**
	 * Checks to make sure that the user's input is valid
	 * before accepting and applying the changes.
	 * @return True if the user's input is valid and can be
	 * applied; false otherwise.
	 */
    virtual bool Validate();
	
	/**
	 * Syncs the page's GUI components with the data stored in
	 * this objects. That is, makes sure that the various components
	 * are aware of the values of mNumberPoints, mNumberColumns, and
	 * mLabels.
	 * @return True if the transfer was successful; false otherwise.
	 */
    virtual bool TransferDataToWindow();
	/**
	 * Records the settings stored in the GUI components of the
	 * page to the variables of this object. That is, this fills
	 * mNumberPoints, mNumberColumns, and mLabels with the values
	 * specified by the user.
	 * @return True if transfer was successful; false otherwise.
	 */
    virtual bool TransferDataFromWindow();

	// The data this dialog sets for the user
private:
	/**
	 * The number of the points that the show should have, after
	 * the dialogue is closed.
	 */
	unsigned mNumberPoints;
	/**
	 * The number of columns into which the initial points should
	 * be arranged, after the dialog is closed.
	 */
	unsigned mNumberColumns;
	/**
	 * The labels that the points should have, after the
	 * dialog is closed.
	 */
	std::vector<wxString> mLabels;

public:
	/**
	 * Returns the number of points that should be used in
	 * the show, as indicated by the user.
	 */
	unsigned GetNumberPoints() const { return mNumberPoints; }

	/**
	 * Returns the number of columns in which the points should be
	 * initially arranged for the show, as indicated by the user.
	 */
	unsigned GetNumberColumns() const { return mNumberColumns; }

	/**
	 * Returns the labels for the points in the show, as indicated
	 * by the user.
	 */
	std::vector<wxString> GetLabels() { return mLabels; }

private:
	CalChartDoc& mShow;
	void OnReset(wxCommandEvent&);
};

/**
 * A version of the Show Info dialog (which appears when the user selects the
 * "Set Up Marchers" option) that can be inserted into the show creation wizard.
 * It builds its layout using the same function as ShowInfoReq:
 * LayoutShowInfo(...). 
 */
class ShowInfoReqWizard : public wxWizardPageSimple
{
	DECLARE_CLASS( ShowInfoReqWizard )
public:

	/**
	 * Makes the wizard page.
	 * @param parent The wizard which this page is a part of.
	 */
	ShowInfoReqWizard(wxWizard *parent);

	/**
	 * Syncs the page's GUI components with the data stored in
	 * this objects. That is, makes sure that the various components
	 * are aware of the values of mNumberPoints, mNumberColumns, and
	 * mLabels.
	 * @return True if the transfer was successful; false otherwise.
	 */
	virtual bool TransferDataToWindow();

	/**
	 * Records the settings stored in the GUI components of the
	 * page to the variables of this object. That is, this fills
	 * mNumberPoints, mNumberColumns, and mLabels with the values
	 * specified by the user.
	 * @return True if transfer was successful; false otherwise.
	 */
	virtual bool TransferDataFromWindow();

	/**
	 * Checks to make sure that the user's input is valid
	 * before accepting and applying the changes.
	 * @return True if the user's input is valid and can be
	 * applied; false otherwise.
	 */
	virtual bool Validate();

	// The data this dialog sets for the user
private:
	/**
	 * Records whether or not TransferDataToWindow() has been called
	 * yet on this object since its creation. Will be true if
	 * TransferDataToWindow() has NOT been called yet; false
	 * otherwised. The page performs some setup on the first time
	 * that TransferDataToWindow() is called, which is the reason
	 * for recording whether or not the method has been called.
	 */
	bool mTransferDataToWindowFirstTime;
	/**
	 * The number of the points that the show should have, after
	 * the setup wizard is finished.
	 */
	unsigned mNumberPoints;
	/**
	 * The number of columns into which the initial points should
	 * be arranged, after the setup wizard is finished.
	 */
	unsigned mNumberColumns;
	/**
	 * The labels that the points should have, after the
	 * setup wizard is finished.
	 */
	std::vector<wxString> mLabels;

public:
	/**
	 * Returns the number of points that should be used in
	 * the show, as indicated by the user.
	 */
	unsigned GetNumberPoints() const { return mNumberPoints; }

	/**
	 * Returns the number of columns in which the points should be
	 * initially arranged for the show, as indicated by the user.
	 */
	unsigned GetNumberColumns() const { return mNumberColumns; }

	/**
	 * Returns the labels for the points in the show, as indicated
	 * by the user.
	 */
	std::vector<wxString> GetLabels() { return mLabels; }
};

#endif
