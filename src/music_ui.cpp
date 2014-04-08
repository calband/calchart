#include "music_ui.h"

BeatsPerBarGridHandler::BeatsPerBarGridHandler(MeasureEventData<BeatsPerBarShiftEvent, int>* source)
: super(source, new MeasureData())
{}

void BeatsPerBarGridHandler::setupColumnLabels(wxGrid* grid) {
	grid->SetColLabelValue(0, "Start Bar");
	grid->SetColLabelValue(1, "Beats/Bar");
}

int BeatsPerBarGridHandler::getNumCols() {
	return 2;
}

void BeatsPerBarGridHandler::writeGridRow(wxGrid* grid, int row, BeatsPerBarShiftEvent mData) {
	grid->SetCellValue(wxGridCellCoords(row, 0), wxString::Format(wxT("%i"), mData.bar));
	grid->SetCellValue(wxGridCellCoords(row, 1), wxString::Format(wxT("%i"), mData.beatsPerBar));
}

BeatsPerBarShiftEvent BeatsPerBarGridHandler::readGridRow(wxGrid* grid, int row) {
	long bar, beatsPerBar;
	if (!grid->GetCellValue(wxGridCellCoords(row, 0)).ToCLong(&bar) || !grid->GetCellValue(wxGridCellCoords(row, 1)).ToCLong(&beatsPerBar)) {
		throw InvalidRowEntryException();
	}
	return BeatsPerBarShiftEvent((int)bar, (int)beatsPerBar);
}


TempoGridHandler::TempoGridHandler(MeasureEventData<TempoShiftEvent, int>* source)
: super(source, new TempoData())
{}

void TempoGridHandler::setupColumnLabels(wxGrid* grid) {
	grid->SetColLabelValue(0, "Start Bar");
	grid->SetColLabelValue(1, "Start Beat");
	grid->SetColLabelValue(2, "BPM");
}

int TempoGridHandler::getNumCols() {
	return 3;
}

void TempoGridHandler::writeGridRow(wxGrid* grid, int row, TempoShiftEvent mData) {
	grid->SetCellValue(wxGridCellCoords(row, 0), wxString::Format(wxT("%i"), mData.bar));
	grid->SetCellValue(wxGridCellCoords(row, 1), wxString::Format(wxT("%i"), mData.beat));
	grid->SetCellValue(wxGridCellCoords(row, 2), wxString::Format(wxT("%i"), mData.beatsPerMinute));
}

TempoShiftEvent TempoGridHandler::readGridRow(wxGrid* grid, int row) {
	long bar, beat, tempo;
	if (!grid->GetCellValue(wxGridCellCoords(row, 0)).ToCLong(&bar) || !grid->GetCellValue(wxGridCellCoords(row, 1)).ToCLong(&beat) || !grid->GetCellValue(wxGridCellCoords(row, 2)).ToCLong(&tempo)) {
		throw InvalidRowEntryException();
	}
	return TempoShiftEvent((int)bar, (int)beat, (int)tempo);
}

SongGridHandler::SongGridHandler(MeasureEventData<SongChangeEvent, std::string>* source) 
: super(source, new SongData())
{}

void SongGridHandler::setupColumnLabels(wxGrid* grid) {
	grid->SetColLabelValue(0, "Start Bar");
	grid->SetColLabelValue(1, "Start Beat");
	grid->SetColLabelValue(2, "Song Title");
}

int SongGridHandler::getNumCols() {
	return 3;
}

void SongGridHandler::writeGridRow(wxGrid* grid, int row, SongChangeEvent mData) {
	grid->SetCellValue(wxGridCellCoords(row, 0), wxString::Format(wxT("%i"), mData.bar));
	grid->SetCellValue(wxGridCellCoords(row, 1), wxString::Format(wxT("%i"), mData.beat));
	grid->SetCellValue(wxGridCellCoords(row, 2), wxString(mData.songName.c_str()));
}

SongChangeEvent SongGridHandler::readGridRow(wxGrid* grid, int row) {
	long bar, beat;
	if (!grid->GetCellValue(wxGridCellCoords(row, 0)).ToCLong(&bar) || !grid->GetCellValue(wxGridCellCoords(row, 1)).ToCLong(&beat)) {
		throw InvalidRowEntryException();
	}
	return SongChangeEvent((int)bar, (int)beat, grid->GetCellValue(wxGridCellCoords(row, 2)).ToStdString());
}



BEGIN_EVENT_TABLE(MeasureDataGridPage, wxWindow)
EVT_GRID_CELL_CHANGED(MeasureDataGridPage::OnElementChanged)
END_EVENT_TABLE()

MeasureDataGridPage::MeasureDataGridPage(GridHandler* gridHandler, wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: super(parent, id, pos, size), mGridHandler(gridHandler), mIsChanged(false), mEmptyRow(0)
{
	Create(parent, id, pos, size);
}

MeasureDataGridPage::~MeasureDataGridPage()
{
	delete mGridHandler;
}


bool MeasureDataGridPage::Create(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size) {

	CreateControls();

	// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
	// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}

void MeasureDataGridPage::CreateControls() {
	wxBoxSizer *windowSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(windowSizer);

	mGrid = new wxGrid(this, wxID_ANY);
	mGridHandler->setupGrid(mGrid);
	mGrid->AppendRows(1);
	mEmptyRow = mGrid->GetNumberRows() - 1;
	
	windowSizer->Add(mGrid, wxSizerFlags().Border(wxALL, 5));

	Update();
}

void MeasureDataGridPage::OnElementChanged(wxGridEvent& eventObj) {
	int changedRow = eventObj.GetRow();
	int changedCol = eventObj.GetCol();
	bool emptyRowCreated = false;
	if (changedRow == mEmptyRow && !mGrid->GetCellValue(changedRow, changedCol).IsEmpty()) {
		mGrid->AppendRows(1);
		mEmptyRow = mGrid->GetNumberRows() - 1;
	} else {
		emptyRowCreated = true;
		for (int col = 0; col < mGrid->GetNumberCols(); col++) {
			if (!mGrid->GetCellValue(changedRow, changedCol).IsEmpty()) {
				emptyRowCreated = false;
				break;
			}
		}
		if (emptyRowCreated) {
			mGrid->DeleteRows(changedRow);
			mEmptyRow = mGrid->GetNumberRows() - 1;
		}
	}
	if (!emptyRowCreated && mGridHandler->shouldResortGrid(changedRow, changedCol)) {
		mGridHandler->sortGrid(mGrid);
	}
	mIsChanged = true;
}


BEGIN_EVENT_TABLE(MeasureDataEditor, wxDialog)
END_EVENT_TABLE()


MeasureDataEditor::MeasureDataEditor(MusicData* musicData, wxWindow *parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
: super(parent, id, caption, pos, size, style)
{
	mGridHandlers.push_back(new BeatsPerBarGridHandler(musicData->getBeatsPerBar()));
	mPageNames.push_back(wxString("Beats per Bar"));
	mGridHandlers.push_back(new TempoGridHandler(musicData->getTempos()));
	mPageNames.push_back(wxString("Tempos"));
	mGridHandlers.push_back(new SongGridHandler(musicData->getSongs()));
	mPageNames.push_back(wxString("Song Names"));
	Create(parent, id, caption, pos, size, style);
}

MeasureDataEditor::~MeasureDataEditor() {
	for (int index = 0; index < mGridHandlers.size(); index++) {
		delete mGridHandlers[index];
	}
}

bool MeasureDataEditor::Create(wxWindow *parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
	CreateControls();

	// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
	// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}


void MeasureDataEditor::CreateControls() {
	wxBoxSizer *windowSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(windowSizer);

	wxButton * mCloseButton = new wxButton(this, wxID_OK, wxT("&Close"));
	mCloseButton->SetDefault();

	wxNotebook* mGridNotebook = new wxNotebook(this, wxID_ANY);
	mGridNotebook->Hide();
	for (int index = 0; index < mGridHandlers.size(); index++) {
		mGridNotebook->AddPage(new MeasureDataGridPage(mGridHandlers[index], mGridNotebook, wxID_ANY), mPageNames[index]);
	}
	mGridNotebook->SetSelection(0);

	windowSizer->Add(mGridNotebook, wxSizerFlags().Expand().Border(wxALL, 5));
	windowSizer->Add(mCloseButton, wxSizerFlags().Border(wxALL, 5));
	windowSizer->Show(mGridNotebook);
	windowSizer->Layout();

	Update();
}