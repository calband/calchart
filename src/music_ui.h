#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/notebook.h>

#include "core/music_data.h"


struct InvalidRowEntryException {
};

class GridHandler {
public:
	virtual void setupGrid(wxGrid* grid) = 0;
	virtual void sortGrid(wxGrid* grid) = 0;
	virtual bool shouldResortGrid(int col, int row) = 0;
	virtual void saveGrid(wxGrid* grid) = 0;
};

template <typename MeasureEventType, typename EventData>
class MeasureDataGridHandler : public GridHandler {
public:
	MeasureDataGridHandler(MeasureEventData<MeasureEventType, EventData>* dataSource, MeasureEventData<MeasureEventType, EventData>* localData) : mSourceData(dataSource), mEditedData(localData), isSorting(false)
	{
		cloneInto(mSourceData, mEditedData);
	}
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
	virtual void writeToGrid(wxGrid* targetGrid, MeasureEventData<MeasureEventType, EventData>* source) {
		targetGrid->ClearGrid();
		for (int index = 0; index < source->getNumEvents(); index++) {
			writeGridRow(targetGrid, index, source->getEvent(index));
		}
	}

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

	void writeRowsDirectly(wxGrid* target, int startRow, std::vector<std::vector<wxString>> rows) {
		for (int row = 0; row < rows.size(); row++) {
			std::vector<wxString> currRow = rows[row];
			for (int col = 0; col < currRow.size(); col++) {
				target->SetCellValue(wxGridCellCoords(startRow + row, col), currRow[col]);
			}
		}
	}

	virtual void setupColumnLabels(wxGrid* grid) = 0;

	virtual int getNumCols() = 0;

	virtual void writeGridRow(wxGrid* grid, int row, MeasureEventType mData) = 0;
	virtual MeasureEventType readGridRow(wxGrid* grid, int row) = 0;


	virtual void update() {
		cloneInto(mSourceData, mEditedData);
	}
	
	void cloneInto(MeasureEventData<MeasureEventType, EventData>* source, MeasureEventData<MeasureEventType, EventData>* dest) {
		dest->clearEvents();
		copyContents(source, dest);
	}

	void copyContents(MeasureEventData<MeasureEventType, EventData>* source, MeasureEventData<MeasureEventType, EventData>* dest) {
		for (int index = 0; index < source->getNumEvents(); index++) {
			dest->addEvent(source->getEvent(index));
		}
	}

	MeasureEventData<MeasureEventType, EventData>* mSourceData;
	MeasureEventData<MeasureEventType, EventData>* mEditedData;

	bool isSorting;

};


class BeatsPerBarGridHandler : public MeasureDataGridHandler<BeatsPerBarShiftEvent, int> {
private:
	using super = MeasureDataGridHandler;
public:
	BeatsPerBarGridHandler(MeasureEventData<BeatsPerBarShiftEvent, int>* source);
protected:
	virtual void setupColumnLabels(wxGrid* grid);

	virtual int getNumCols();

	virtual void writeGridRow(wxGrid* grid, int row, BeatsPerBarShiftEvent mData);
	virtual BeatsPerBarShiftEvent readGridRow(wxGrid* grid, int row);
};

class TempoGridHandler : public MeasureDataGridHandler<TempoShiftEvent, int> {
private:
	using super = MeasureDataGridHandler;
public:
	TempoGridHandler(MeasureEventData<TempoShiftEvent, int>* source);
protected:
	virtual void setupColumnLabels(wxGrid* grid);

	virtual int getNumCols();

	virtual void writeGridRow(wxGrid* grid, int row, TempoShiftEvent mData);
	virtual TempoShiftEvent readGridRow(wxGrid* grid, int row);
};


class SongGridHandler : public MeasureDataGridHandler<SongChangeEvent, std::string> {
private:
	using super = MeasureDataGridHandler;
public:
	SongGridHandler(MeasureEventData<SongChangeEvent, std::string>* source);
protected:
	virtual void setupColumnLabels(wxGrid* grid);

	virtual int getNumCols();

	virtual void writeGridRow(wxGrid* grid, int row, SongChangeEvent mData);
	virtual SongChangeEvent readGridRow(wxGrid* grid, int row);
};

class MeasureDataGridPage : public wxWindow {
private:
	using super = wxWindow;
public:
	MeasureDataGridPage(GridHandler* gridHandler, wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	~MeasureDataGridPage();

private:
	bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	void CreateControls();

	void OnElementChanged(wxGridEvent& eventObj);

	wxGrid* mGrid;
	GridHandler* mGridHandler;

	int mEmptyRow;

	bool mIsChanged;

	DECLARE_EVENT_TABLE()

};

class MeasureDataEditor : public wxDialog {
private:
	using super = wxDialog;
public:
	MeasureDataEditor(MusicData* musicData, wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Measure Data"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
	~MeasureDataEditor();

private:
	bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Measure Data"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

	void CreateControls();

	std::vector<GridHandler*> mGridHandlers;
	std::vector<wxString> mPageNames;

	DECLARE_EVENT_TABLE()

};




