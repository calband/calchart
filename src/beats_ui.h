#include <wx\mediactrl.h>
#include <wx\dialog.h>
#include <wx\button.h>

#include "animation_view.h"
#include "core\BeatInfo.h"
#include "beats_file.h"

/**
class BeatsFileBuilder {
public:
	static void buildFile(wxOutputStream& output, BeatInfo* beats);
private:
	static void addFormatting(JSONObjectValue& mainObject);
	static void JSONObjectValue* makeBeatsObject(BeatInfo* beats);
};
**/

class BeatBuilder {
public:
	BeatBuilder(wxWindow* parentWindow, int playerId);
	~BeatBuilder();

	void selectSong(std::string filePath);
	
	void start(bool addBeatToStart = false);
	void stop(bool addBeatToEnd = false);

	bool isPlaying();
	bool isReadyToPlay();

	void addBeat();

	std::string getCurrentPlayTime();
	std::string getPlayLength();

	BeatInfo* getBeatInfo();
private:
	void addBeat(long time);
	void resetBeats();

	std::string timeToString(long time);

	wxMediaCtrl mPlayer;
	bool mIsReady;
	std::string mPlayLength;

	static const std::string invalidTime;

	BeatInfo* mBeatInfo;
};


class BeatsEditor : public wxDialog {
private:
	using super = wxDialog;

	DECLARE_EVENT_TABLE()
public:
	BeatsEditor(AnimationView* targetAnimation, wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Music Beats"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
	~BeatsEditor();

private:
	
	void Stop();

	void Update();

	void OnLoad(wxCommandEvent& event);
	void OnExport(wxCommandEvent& event);
	void OnStop(wxCommandEvent& event);
	void OnFinished(wxMediaEvent& event);
	void OnAddBeat(wxCommandEvent& event);
	void OnAutoFirstBeatToggle(wxCommandEvent& event);
	void OnAutoLastBeatToggle(wxCommandEvent& event);

	bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Music Beats"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

	void CreateControls();

	AnimationView* mAnimationView;
	BeatBuilder mBeatBuilder;

	bool mShouldAddAutoFirstBeat;
	bool mShouldAddAutoLastBeat;

	std::string mTargetFile;

	wxStaticText* mFileNameDisplay;
	wxStaticText* mNumBeatsDisplay;
	wxStaticText* mErrorDisplay;
	wxButton* mLoadButton;
	wxButton* mExportButton;
	wxButton* mStopButton;
	wxButton* mNewBeatButton;
	wxButton* mCloseButton;
};

