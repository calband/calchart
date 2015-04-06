#pragma once

#include "music_score_doc_component.h"
#include "calchartdoc.h"
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/propgrid/propgrid.h>

/**
 * Each category in the property grid used to edit fragments has
 * a "handler". The handler is in charge of adding appropriate
 * properties for the category, and for saving those properties
 * to the MusicScoreDocComponent.
 */
class FragmentEditor__CategoryHandler {
public:
	/**
	 * Loads a category into the grid from the music score
	 * that will be handled by this handler.
	 * @param musicScore The music score to load from.
	 * @param fragment The fragment whose properties are being edited.
	 * @param propertyGrid The grid to load into.
	 * @return The category created by this handler.
	 */
	virtual wxPropertyCategory* loadGridFromMusicScore(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPropertyGrid* propertyGrid) = 0;

	/**
	 * Called when one of this handler's properties is edited.
	 * The handler needs to save the change to the music score.
	 * @param musicScore The score to save the change to.
	 * @param fragment The fragment whose properties are being edited.
	 * @param prop The property that was changed.
	 * @param propertyGrid The grid of all properties.
	 * @param category The category that this handler is in charge of.
	 */
	virtual void onPropertyModified(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) = 0;

	/**
	 * Builds an empty property with the correct format to be
	 * used by this handler, and adds it to the grid.
	 * @return The new property.
	 */
	virtual wxPGProperty* insertEmptyProperty(MusicScoreDocComponent* musicScore, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) = 0;

	/**
	 * Returns whether or not the property is "empty".
	 * @param prop The property to check.
	 * @return True if the property is "empty", false otherwise.
	 */
	virtual bool propertyIsEmpty(wxPGProperty* prop) = 0;

	/**
	 * Reacts to the deletion fo a property handled by this object.
	 * @param musicScore The score to save changes to.
	 * @param fragment The fragment whose properties are being edited.
	 * @param prop The property that was changed.
	 * @param propertyGrid The grid of all properties.
	 * @param category The category that this handler is in charge of.
	 */
	virtual void onPropertyDeleted(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) = 0;

	/**
	 * Reacts to the addition fo a property handled by this object.
	 * @param musicScore The score to save changes to.
	 * @param fragment The fragment whose properties are being edited.
	 * @param prop The property that was added.
	 * @param propertyGrid The grid of all properties.
	 * @param category The category that this handler is in charge of.
	 */
	virtual void onPropertyAdded(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) = 0;
};



/**
 * The frame through which the Music Score is edited.
 */
class MusicScoreEditFrame : public wxFrame {
private:
	using super = wxFrame;
public:
	/**
	 * Constructor.
	 * @param musicScore The music score that will be edited through this window.
	 * @param parent The parent for this window.
	 * @param id The id of this window.
	 * @param caption The title for this window.
	 * @param pos The position of the window.
	 * @param size The size of the window.
	 * @param style The style of the window.
	 */
	MusicScoreEditFrame(CalChartDoc& doc, wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Music Score Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER);
	

	/**
	 * Called when the close button is selected.
	 */
	void onClose(wxCommandEvent& evt);
	
	/**
	 * Called when the save button is selected.
	 */
	void onSave(wxCommandEvent& evt);

	/**
	 * Called when the "Add Fragment" button is selected.
	 */
	void onAddFragment(wxCommandEvent& evt);

	/**
	 * Called when the "rename" option is selected from the
	 * fragment right-click menu.
	 */
	void onPopupRenameFragment(wxCommandEvent& evt);

	/**
	 * Called when the "delete" option is selected from the
	 * fragment right-click menu.
	 */
	void onPopupDeleteFragment(wxCommandEvent& evt);

	/**
	 * Called when the "delete" option is selected from the popup menu
	 * that appears when you right click a fragment property.
	 */
	void onPopupDeleteFragmentProperty(wxCommandEvent& evt);

	/**
	 * Called when a fragment in the fragment listctrl is right-clicked.
	 */
	void onFragmentRightClick(wxListEvent& evt);

	/**
	 * Called when a fragment is selected in the fragment list.
	 */
	void onFragmentSelected(wxListEvent& evt);

	/**
	 * Called when a fragment is deselected from the fragment list.
	 */
	void onFragmentDeselected(wxListEvent& evt);

	/**
	 * Called when the fragment starts being renamed.
	 */
	void onFragmentBeginRename(wxListEvent& evt);

	/**
	 * Called when a fragment is renamed.
	 */
	void onFragmentRenamed(wxListEvent& evt);

	/**
	 * Called when a property is changed.
	 */
	void onFragmentPropertyChanged(wxPropertyGridEvent& evt);

	/**
	 * Called when a property is about to be changed. The change
	 * can be vetoed.
	 */
	void onFragmentPropertyChanging(wxPropertyGridEvent& evt);

	/**
	 * Called when a property is right-clicked.
	 */
	void onFragmentPropertyRightClicked(wxPropertyGridEvent& evt);

	/**
	 * Called when a start fragment is selected from the choice dropdown.
	 */
	void onStartFragmentSelected(wxCommandEvent& evt);
private:
	/**
	 * Creates the window.
	 * @param parent The parent window.
	 * @param id The id of the window.
	 * @param caption The title of the window.
	 * @param pos The position of the window.
	 * @param size The size of the window.
	 * @param style The style of the window.
	 */
	bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Music Score Editor"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

	/**
	 * Creates the controls that appear in the window.
	 */
	void createControls();

	/**
	 * Builds a popup menu at the given position to control the fragment
	 * with the given index in the listctrl.
	 * @param position The position to build the menu at.
	 * @param index The index of the fragment that is being edited with 
	 *   the popup menu. The index is the index of the fragment in the listctrl,
	 *   not in the MusicScoreDocComponent.
	 */
	void makeFragmentRightClickMenu(const wxPoint& position, long index);

	/**
	 * Builds a popup menu when a fragment property is right-clicked.
	 * @param position The position at which to build the menu.
	 * @param clickedProp The property that was right-clicked.
	 */
	void makeFragmentPropertyRightClickMenu(const wxPoint& position, wxPGProperty* clickedProp);

	/**
	 * Syncs the window with the content of the Music Score.
	 */
	void loadFrameContentFromMusicScore();

	/**
	 * Syncs the fragment editor with the currently selected fragment.
	 */
	void resetFragmentEditWindow();

	/**
	 * Syncs the start fragment choice with the fragments in the music score.
	 */
	void resetStartFragmentChoice();

	/**
	 * Deselects the given fragment if it is currently selected.
	 * @param fragment The fragment to deselect.
	 */
	void deselectFragmentIfSelected(std::shared_ptr<MusicScoreFragment> fragment);

	/**
	 * Saves the content of the window to the Music Score Doc Component.
	 */
	void saveToMusicScoreDocComponent();

	/**
	 * Iterates through a category of the fragment editor property grid,
	 * and refreshes it. When it is refreshed, only one 'empty' entry
	 * is kept. Any others are deleted.
	 * @param category The category to refresh.
	 */
	void refreshEditorCategory(wxPropertyCategory* category);

	/**
	 * The calchart doc containing the music score to edit.
	 */
	CalChartDoc& mDoc;

	/**
	 * A copy of the music score being edited. Changes are made to this one as the
	 * user makes changes in the window. When the window is saved, the content of
	 * this music score is coppied to the one that is really being edited.
	 */
	MusicScoreDocComponent mLocalMusicScore;

	/**
	 * The list of fragments, as it appears in the window.
	 */
	wxListCtrl* mFragmentList;

	/** 
	 * The property grid used to edit the properties of a fragment.
	 */
	wxPropertyGrid* mFragmentEditor;

	/**
	 * The choice control used to select the start fragment.
	 */
	wxChoice* mStartFragmentChoice;

	/**
	 * The music score fragment that is selected from the fragments list.
	 */
	std::shared_ptr<MusicScoreFragment> mSelectedFragment;

	/**
	 * The handlers for each of the categories that appear in the property
	 * grid for editing fragments.
	 */
	std::vector<std::unique_ptr<FragmentEditor__CategoryHandler>> mFragCategoryHandlers;

	/**
	 * A map from category in the fragment editor property grid
	 * to the handler that controls it.
	 */
	std::unordered_map<wxPropertyCategory*, FragmentEditor__CategoryHandler*> mFragCategoryToHandlerMap;

	/**
	 * Keeps track of whether or not changes have been made since the
	 * last save. True if yes; false otherwise.
	 */
	bool mModified;

	DECLARE_EVENT_TABLE()

};