#include "beats_ui.h"
#include "core\fileio\json_file_formatter.h"

#include <wx/tglbtn.h>
#include <wx/wfstream.h>

enum
{
	BeatsExport_Load = 1100,
	BeatsExport_Export,
	BeatsExport_Stop,
	BeatsExport_AutoFirstBeat,
	BeatsExport_AutoLastBeat,
	BeatsExport_AddBeat,
	BeatsExport_BeatBuilder
};

BEGIN_EVENT_TABLE(BeatsEditor, wxDialog)
EVT_BUTTON(BeatsExport_Load, BeatsEditor::OnLoad)
EVT_BUTTON(BeatsExport_Export, BeatsEditor::OnExport)
EVT_BUTTON(BeatsExport_Stop, BeatsEditor::OnStop)
EVT_BUTTON(BeatsExport_AddBeat, BeatsEditor::OnAddBeat)
EVT_TOGGLEBUTTON(BeatsExport_AutoFirstBeat, BeatsEditor::OnAutoFirstBeatToggle)
EVT_TOGGLEBUTTON(BeatsExport_AutoLastBeat, BeatsEditor::OnAutoLastBeatToggle)
END_EVENT_TABLE()

const std::string BeatBuilder::invalidTime = "--";

BeatBuilder::BeatBuilder(wxWindow* parentWindow, int playerId)
: mIsReady(false), mPlayer(parentWindow, playerId, wxEmptyString, wxDefaultPosition, wxSize(0, 0)), mBeatInfo(new BeatInfo())
{
	mPlayLength = invalidTime;
}

BeatBuilder::~BeatBuilder() {
	delete mBeatInfo;
}

void BeatBuilder::selectSong(std::string filePath) {
	mIsReady = mPlayer.Load(filePath);
	if (mIsReady) {
		mPlayLength = timeToString(((wxLongLong)mPlayer.Length()).GetValue());
	} else {
		mPlayLength = invalidTime;
	}
}

void BeatBuilder::start(bool addBeatToStart) {
	if (isReadyToPlay()) {
		resetBeats();
		mPlayer.Play();
		if (addBeatToStart) {
			addBeat(0);
		}
	}
}

void BeatBuilder::stop(bool addBeatToEnd) {
	if (isPlaying()) {
		mPlayer.Pause();
		mPlayer.Seek(0);
		if (addBeatToEnd) {
			addBeat(mPlayer.Length());
		}
	}
}

bool BeatBuilder::isPlaying() {
	return (mPlayer.GetState() == wxMEDIASTATE_PLAYING);
}

bool BeatBuilder::isReadyToPlay() {
	return mIsReady && !isPlaying();
}

void BeatBuilder::addBeat() {
	if (isPlaying()) {
		addBeat(mPlayer.Tell());
	}
}

void BeatBuilder::addBeat(long time) {
	mBeatInfo->addBeat(time);
}

void BeatBuilder::resetBeats() {
	mBeatInfo->clearBeats();
}

std::string BeatBuilder::getCurrentPlayTime() {
	if (isPlaying()) {
		return timeToString(((wxLongLong)mPlayer.Tell()).GetValue());
	} else {
		return invalidTime;
	}
}

std::string BeatBuilder::getPlayLength() {
	return mPlayLength;
}

BeatInfo* BeatBuilder::getBeatInfo() {
	return mBeatInfo;
}

std::string BeatBuilder::timeToString(long time) {
	long seconds = (time / 1000);
	int hours = seconds / (60 * 60);
	seconds -= hours * (60) * (60);
	int minutes = seconds / 60;
	seconds -= minutes * 60;
	char formatted[9];
	sprintf(formatted, "%2i:%02i:%02i", hours, minutes, seconds);
	return std::string(formatted);
}

BeatsEditor::BeatsEditor(AnimationView* targetAnimation, wxWindow *parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
: super(parent, id, caption, pos, size, style), mBeatBuilder(this, BeatsExport_BeatBuilder), mShouldAddAutoFirstBeat(false), mShouldAddAutoLastBeat(false)
{
	Create(parent, id, caption, pos, size, style);
}

BeatsEditor::~BeatsEditor() {

}

bool BeatsEditor::Create(wxWindow *parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {

	CreateControls();

	// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
	// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}

void BeatsEditor::CreateControls() {
	this->Connect(BeatsExport_BeatBuilder, wxEVT_MEDIA_FINISHED,
		wxMediaEventHandler(BeatsEditor::OnFinished));

	wxBoxSizer *windowSizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(windowSizer);

	wxBoxSizer *buttonColumn = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *infoColumn = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *beatTappingPreferences = new wxBoxSizer(wxHORIZONTAL);

	mCloseButton = new wxButton(this, wxID_OK, wxT("&Close"));
	mCloseButton->SetDefault();
	mLoadButton = new wxButton(this, BeatsExport_Load, wxT("&Load"));
	mExportButton = new wxButton(this, BeatsExport_Export, wxT("&Export"));
	mStopButton = new wxButton(this, BeatsExport_Stop, wxT("&Stop"));
	mNewBeatButton = new wxButton(this, BeatsExport_AddBeat, wxT("Click to add a beat."));
	wxToggleButton* autoFirstBeatButton = new wxToggleButton(this, BeatsExport_AutoFirstBeat, wxT("Put Beat At Start"));
	wxToggleButton* autoLastBeatButton = new wxToggleButton(this, BeatsExport_AutoLastBeat, wxT("Put Beat At End"));
	mFileNameDisplay = new wxStaticText(this, wxID_ANY, "");
	mNumBeatsDisplay = new wxStaticText(this, wxID_ANY, "");
	mErrorDisplay = new wxStaticText(this, wxID_ANY, "");


	buttonColumn->Add(mLoadButton, wxSizerFlags().Border(wxALL, 5));
	buttonColumn->Add(mExportButton, wxSizerFlags().Border(wxALL, 5));
	buttonColumn->Add(mStopButton, wxSizerFlags().Border(wxALL, 5));
	buttonColumn->Add(mCloseButton, wxSizerFlags().Border(wxALL, 5));


	beatTappingPreferences->Add(autoFirstBeatButton, wxSizerFlags().Border(wxALL, 5));
	beatTappingPreferences->Add(autoLastBeatButton, wxSizerFlags().Border(wxALL, 5));

	infoColumn->Add(mNumBeatsDisplay);
	infoColumn->Add(beatTappingPreferences, wxSizerFlags().Center());
	infoColumn->Add(mNewBeatButton, wxSizerFlags().Center());
	infoColumn->Add(mErrorDisplay, wxSizerFlags().Center());

	windowSizer->Add(buttonColumn);
	windowSizer->Add(infoColumn);
	
	Update();
}

void BeatsEditor::Stop() {
	mBeatBuilder.stop(mShouldAddAutoLastBeat);
	Update();
}

void BeatsEditor::OnLoad(wxCommandEvent& event) {
	wxString targetFilename = wxFileSelector(wxT("Select a music file"));
	if (targetFilename.IsEmpty()) {
		return;
	}
	mBeatBuilder.selectSong(targetFilename.ToStdString());
	if (!mBeatBuilder.isReadyToPlay()) {
		mErrorDisplay->SetLabel("ERROR! Could not open file.");
	} else {
		mErrorDisplay->SetLabel("");
	}
	Update();
}

void BeatsEditor::OnExport(wxCommandEvent& event) {
	wxFileDialog saveFileDialog(this, wxString("Export Beats File"), wxEmptyString, wxEmptyString,
		"JSON files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveFileDialog.ShowModal() == wxID_CANCEL) {
		return;
	}
	wxFileOutputStream outputStream(saveFileDialog.GetPath());
	BeatsFileHandler::exportBeatsFile(&outputStream, mBeatBuilder.getBeatInfo());
}

void BeatsEditor::OnStop(wxCommandEvent& event) {
	Stop();
}

void BeatsEditor::OnFinished(wxMediaEvent& event) {
	Stop();
}

void BeatsEditor::OnAddBeat(wxCommandEvent& event) {
	if (!mBeatBuilder.isPlaying()) {
		mBeatBuilder.start(mShouldAddAutoFirstBeat);
	} else {
		mBeatBuilder.addBeat();
	}
	Update();
}

void BeatsEditor::OnAutoFirstBeatToggle(wxCommandEvent& event) {
	mShouldAddAutoFirstBeat = event.IsChecked();
}

void BeatsEditor::OnAutoLastBeatToggle(wxCommandEvent& event) {
	mShouldAddAutoLastBeat = event.IsChecked();
}

void BeatsEditor::Update() {
	if (mBeatBuilder.isReadyToPlay() || mBeatBuilder.isPlaying()) {
		mNewBeatButton->Enable(true);
	} else {
		mNewBeatButton->Enable(false);
	}
	if (mBeatBuilder.isPlaying()) {
		mLoadButton->Enable(false);
		mExportButton->Enable(false);
		mStopButton->Enable(true);
		mNewBeatButton->SetLabel("Add a beat.");
		mCloseButton->Enable(false);
	} else {
		mLoadButton->Enable(true);
		mExportButton->Enable(true);
		mStopButton->Enable(false);
		mNewBeatButton->SetLabel("Click to start!");
		mCloseButton->Enable(true);
	}
	//mFileNameDisplay->SetLabel("Music File: " + mTargetFile);
	mNumBeatsDisplay->SetLabel(wxString::Format("Number of Beats: %i", mBeatBuilder.getBeatInfo()->getNumBeats()));
}
