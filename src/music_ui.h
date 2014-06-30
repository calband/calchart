#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/notebook.h>

#include "music_data.h"


/**
 * An exception thrown when a grid entry has invalid information (e.g. if the user has specifies a tempo change in bar 42.7 at beat "string").
 */
struct InvalidRowEntryException {
};

/**
 * Handles the information in the grids of the Music Data editor. 
 * This is responsible for making sure that the grid elements get sorted properly, that the grid's columns get set up properly, and that the grid's data is saved to the CalChartDoc.
 */
class GridHandler {
public:
	/**
	 * Sets up a grid so that it can hold the information that this GridHandler is sensative to.
	 * @param grid The grid to set up.
	 */
	virtual void setupGrid(wxGrid* grid) = 0;
	/**
	 * Sorts the elements in the grid according to the rules specific to this GridHandler.
	 * @param grid The grid to sort.
	 */
	virtual void sortGrid(wxGrid* grid) = 0;
	/**
	 * Returns whether or not the grid should be resorted if a change was made to the grid content at the given column and row.
	 * @param col The column at which a change was made.
	 * @param row The row at which a change was made.
	 * @return True if the grid should be resorted; false otherwise.
	 */
	virtual bool shouldResortGrid(int col, int row) = 0;
	/**
	 * Saves the content of the grid.
	 * The saved content general gets packed into the CalChartDoc.
	 * @param grid The grid to save.
	 */
	virtual void saveGrid(wxGrid* grid) = 0;
};


/**
 * Handles the grids in the "Music Data" editor.
 * @param MeasureEventType The type of event that is being recorded through grid entries.
 * @param EventData The data being stored in any particular event.
 */
template <typename MeasureEventType, typename EventData>
class MeasureDataGridHandler : public GridHandler {
public:
	/**
	 * Constructor.
	 * @param dataSource The source containing the events that are being manipulated through the grid.
	 * @param localData A source containing the events as they appear in the grid (they are not merged with those in dataSource unless explicitly saved by the user).
	 */
	MeasureDataGridHandler(MeasureEventData<MeasureEventType, EventData>* dataSource, MeasureEventData<MeasureEventType, EventData>* localData) : mSourceData(dataSource), mEditedData(localData), isSorting(false)
	{
		cloneInto(mSourceData, mEditedData);
	}
	/**
	 * Destructor.
	 */
	virtual ~MeasureDataGridHandler() { delete mEditedData; }

	virtual void setupGrid(wxGrid* grid) {
		update();
		grid->CreateGrid(mEditedData->getNumEvents(), getNumCols());
		writeToGrid(grid, mEditedData);
		setupColumnLabels(grid);
		grid->HideRowLabels();
	}

	virtual bool shouldResortGrid(int col, int row) {
		return !isSorting;
	}

	virtual void sortGrid(wxGrid* grid) {
		isSorting = true;
		std::vector<std::vector<wxString>> invalids = readFromGrid(grid, mEditedData);
		writeToGrid(grid, mEditedData);
		writeRowsDirectly(grid, mEditedData->getNumEvents(), invalids);
		isSorting = false;
	}

	virtual void saveGrid(wxGrid* grid) {
		std::vector<std::vector<wxString>> invalids = readFromGrid(grid, mEditedData);
		if (invalids.size() > 0) {
			//ERROR
		}
		cloneInto(mEditedData, mSourceData);
	}

protected:
	/**
	 * Writes the content of a particular MeasureEventData structure to a grid.
	 * @param targetGrid The grid to fill with the content of the MeasureEventData structure.
	 * @param source The data to fill the grid with.
	 */
	virtual void writeToGrid(wxGrid* targetGrid, MeasureEventData<MeasureEventType, EventData>* source) {
		targetGrid->ClearGrid();
		for (int index = 0; index < source->getNumEvents(); index++) {
			writeGridRow(targetGrid, index, source->getEvent(index));
		}
	}

	/**
	 * Clears the given MeasureEventData structure, and populates it with the events represented within the given grid.
	 * @param sourceGrid The grid containing the events to copy to the MeasureEventData structure.
	 * @param target The MeasureEventData structure to put the grid's events into.
	 * @return A list of the entries in the grid that could not be translated into measure events.
	 */
	virtual std::vector<std::vector<wxString>> readFromGrid(wxGrid* sourceGrid, MeasureEventData<MeasureEventType, EventData>* target) {
		std::vector<std::vector<wxString>> invalids;
		target->clearEvents();
		for (int index = 0; index < sourceGrid->GetNumberRows(); index++) {
			try {
				target->addEvent(readGridRow(sourceGrid, index));
			} catch (InvalidRowEntryException) {
				std::vector<wxString> currentRow;
				for (int col = 0; col < sourceGrid->GetNumberCols(); col++) {
					currentRow.push_back(sourceGrid->GetCellValue(wxGridCellCoords(index, col)));
				}
				invalids.push_back(currentRow);
			}
		}
		return invalids;
	}

	/**
	 * Populates the grid cells with the given content.
	 * @param target The grid to pupulate.
	 * @param startRow The index of the row where the writing will begin.
	 * @param rows The strings to populate the grid with.
	 */
	void writeRowsDirectly(wxGrid* target, int startRow, std::vector<std::vector<wxString>> rows) {
		for (int row = 0; row < rows.size(); row++) {
			std::vector<wxString> currRow = rows[row];
			for (int col = 0; col < currRow.size(); col++) {
				target->SetCellValue(wxGridCellCoords(startRow + row, col), currRow[col]);
			}
		}
	}

	/**
	 * Sets up the column labels in the grid.
	 * @param grid The grid whose column labels will be set up.
	 */
	virtual void setupColumnLabels(wxGrid* grid) = 0;

	/**
	 * Returns the number of columns in the grid.
	 * @return The number of columns in the grid.
	 */
	virtual int getNumCols() = 0;

	/**
	 * Writes an event to a row of the grid.
	 * @param grid The target grid.
	 * @param row The target row to receive the event.
	 * @param data The event to write to the row.
	 */
	virtual void writeGridRow(wxGrid* grid, int row, MeasureEventType data) = 0;
	/**
	 * Reads an event from a row of the grid. If the row does define a valid event, this should throw an error (an InvalidRowEntryException).
	 * @param grid The target grid.
	 * @param row The index of the row to read.
	 * @return The event represented by the indicated row of the grid.
	 */
	virtual MeasureEventType readGridRow(wxGrid* grid, int row) = 0;


	virtual void update() {
		cloneInto(mSourceData, mEditedData);
	}
	
	/**
	 * Makes the destination structure into a clone of the source structure, by first clearing it, and then populating it with the contents of the source.
	 * @param source The source MeasureEventData structure to copy.
	 * @param dest The destination MeasureEventData structure that will become a copy of the source.
	 */
	void cloneInto(MeasureEventData<MeasureEventType, EventData>* source, MeasureEventData<MeasureEventType, EventData>* dest) {
		dest->clearEvents();
		copyContents(source, dest);
	}

	/**
	 * Copies the contents of the source structure, and adds them to the destination structure.
	 * @param source The source MeasureEventData structure.
	 * @param dest The destination MeasureEventData structure that will recieve the contents of the source.
	 */
	void copyContents(MeasureEventData<MeasureEventType, EventData>* source, MeasureEventData<MeasureEventType, EventData>* dest) {
		for (int index = 0; index < source->getNumEvents(); index++) {
			dest->addEvent(source->getEvent(index));
		}
	}

	/**
	 * The data that is being edited through the grid.
	 */
	MeasureEventData<MeasureEventType, EventData>* mSourceData;
	/**
	 * The data that has been edited through the grid, but has not been saved into the original data yet.
	 */
	MeasureEventData<MeasureEventType, EventData>* mEditedData;

	/**
	 * True if the grid is currently being sorted. This is necessary to make sure that sorting doesn't initiate an infinite loop. The grid handler
	 * decides to sort the grid in response to some change made to the original grid; when the grid handler sorts the grid, it makes many changes to the
	 * grid that might prompt another resorting. But we don't want every sorting routine to prompt more sorting routines, because then we would never finish
	 * the sorting to begin with. In order to do this, we need to be able to check if the grid is being sorted by the handler, so that the handler can decide
	 * not to resort the grid if changes are made to it because it is being sorted.
	 */
	bool isSorting;

};

/**
 * A handler for the grid that is used to specify how many beats are in each bar of the show's music.
 */
class BeatsPerBarGridHandler : public MeasureDataGridHandler<BeatsPerBarShiftEvent, int> {
private:
	using super = MeasureDataGridHandler;
public:
	/**
	 * Constructor.
	 * @param source The place to save the changes made to the number of beats in each bar of the show.
	 */
	BeatsPerBarGridHandler(MeasureEventData<BeatsPerBarShiftEvent, int>* source);
protected:
	virtual void setupColumnLabels(wxGrid* grid);

	virtual int getNumCols();

	virtual void writeGridRow(wxGrid* grid, int row, BeatsPerBarShiftEvent mData);
	virtual BeatsPerBarShiftEvent readGridRow(wxGrid* grid, int row);
};

/**
 * A handler for the grid that is used to specify tempo changes in the show.
 */
class TempoGridHandler : public MeasureDataGridHandler<TempoShiftEvent, int> {
private:
	using super = MeasureDataGridHandler;
public:
	/** 
	 * Constructor.
	 * @param source The place to save the changes made to the tempos of the show.
	 */
	TempoGridHandler(MeasureEventData<TempoShiftEvent, int>* source);
protected:
	virtual void setupColumnLabels(wxGrid* grid);

	virtual int getNumCols();

	virtual void writeGridRow(wxGrid* grid, int row, TempoShiftEvent mData);
	virtual TempoShiftEvent readGridRow(wxGrid* grid, int row);
};

/**
 * A handler for the grid that is used to specify song names for the show.
 */
class SongGridHandler : public MeasureDataGridHandler<SongChangeEvent, std::string> {
private:
	using super = MeasureDataGridHandler;
public:
	/**
	 * Constructor.
	 * @param source The place to save the changes made to the show's song names.
	 */
	SongGridHandler(MeasureEventData<SongChangeEvent, std::string>* source);
protected:
	virtual void setupColumnLabels(wxGrid* grid);

	virtual int getNumCols();

	virtual void writeGridRow(wxGrid* grid, int row, SongChangeEvent mData);
	virtual SongChangeEvent readGridRow(wxGrid* grid, int row);
};

/**
 * A notebook page in the Music Data editor that is used to edit some aspect of the show's music through a grid.
 */
class MeasureDataGridPage : public wxWindow {
private:
	using super = wxWindow;
public:
	/**
	 * Constructor.
	 * @param gridHandler The grid handler that will set up and manage the grid for this page.
	 * @param parent The parent window for the page.
	 * @param id The identity of the window.
	 * @param pos The position of the window.
	 * @param size The size of the window.
	 */
	MeasureDataGridPage(GridHandler* gridHandler, wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	/**
	 * Destructor.
	 */
	~MeasureDataGridPage();

	/**
	 * Returns whether or not the page has been modified since the last save.
	 * @return True if the page has been modified since its last save; false otherwise.
	 */
	bool isModified();
	/**
	 * Saves the contents of the page to the CalChartDoc.
	 */
	void save();
private:
	/**
	 * Sets up the window.
	 * @param parent The parent window.
	 * @param id The identity of the window.
	 * @param pos The position of the window.
	 * @param size The size of the window.
	 */
	bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	/**
	 * Sets up the controls for the window.
	 */
	void CreateControls();

	/**
	 * Called when the user modifies the content of a grid cell.
	 * @param eventObj An event carrying information about the grid modification.
	 */
	void OnElementChanged(wxGridEvent& eventObj);

	/**
	 * The grid that appears on the page.
	 */
	wxGrid* mGrid;
	/**
	 * The object responsible for managing the grid's content.
	 */
	GridHandler* mGridHandler;

	/**
	 * The index of the first empty row in the grid. Exactly one empty row is always maintained at the end of the grid.
	 */
	int mEmptyRow;

	/**
	 * Records whether or not the content of the page has been modified since the last save.
	 */
	bool mIsChanged;

	DECLARE_EVENT_TABLE()

};

/**
 * The dialogue through which the Music Data is edited.
 */
class MeasureDataEditor : public wxFrame {
private:
	using super = wxFrame;
public:
	/**
	 * Constructor.
	 * @param musicData The music data that will be edited through this window.
	 * @param parent The parent for this window.
	 * @param id The id of this window.
	 * @param caption The title for this window.
	 * @param pos The position of the window.
	 * @param size The size of the window.
	 * @param style The style of the window.
	 */
	MeasureDataEditor(MusicData* musicData, wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Measure Data"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
	/**
	 * Destructor.
	 */
	~MeasureDataEditor();

	/**
	 * Called when the dialogue is closed by the user.
	 * @param event The event which prompted this function to be called.
	 */
	void OnClose(wxCommandEvent& event);
	/**
	 * Called when the user saves the content of the dialogue.
	 * @param event The event which prompted this function to be called.
	 */
	void OnSave(wxCommandEvent& event);

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
		const wxString& caption = wxT("Measure Data"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

	/**
	 * Creates the controls that appear in the dialogue.
	 */
	void CreateControls();

	/**
	 * Saves the content of the window.
	 */
	void save();

	/**
	 * A list of the grid handlers associated with each of the pages.
	 */
	std::vector<GridHandler*> mGridHandlers;
	/**
	 * A list of the pages through which the music data can be edited.
	 */
	std::vector<MeasureDataGridPage*> mPages;
	/**
	 * The names of the pages through which the music data is edited.
	 */
	std::vector<wxString> mPageNames;

	DECLARE_EVENT_TABLE()

};




