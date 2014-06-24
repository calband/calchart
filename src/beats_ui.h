#include <wx\mediactrl.h>
#include <wx\dialog.h>
#include <wx\button.h>

#include "animation_view.h"
#include "core\BeatInfo.h"
#include "beats_file.h"

/**
 * An object used by the BeatsEditor to play music and add beats to it.
 */
class BeatBuilder {
public:
	/**
	 * Constructor.
	 * @param parentWindow The parent window (required to make sure that when the window is closed, the music player will close also, and will not continue playing music).
	 * @param playerId The identity of the music player.
	 */
	BeatBuilder(wxWindow* parentWindow, int playerId);
	/**
	 * Destructor.
	 */
	~BeatBuilder();

	/**
	 * Loads a song given a particular file path.
	 * @param filePath The filepath for the song file to load.
	 */
	void selectSong(std::string filePath);
	
	/**
	 * Starts the music, and resets where the beats are. 
	 * @param addBeatToStart If true, the player will add a beat immediately when the music starts playing.
	 */
	void start(bool addBeatToStart = false);
	/**
	 * Stops the music.
	 * @param addBeatToEnd If true, the player will add a beat immediately when the music stops playing.
	 */
	void stop(bool addBeatToEnd = false);

	/**
	 * Returns whether or not the music is playing.
	 * @return True if the music is playing; false otherwise.
	 */
	bool isPlaying();
	/**
	 * Returns whether or not the music can play.
	 * @return True if the music is ready to start playing; false otherwise.
	 */
	bool isReadyToPlay();

	/**
	 * Adds a beat to the current spot in the music.
	 */
	void addBeat();

	/**
	 * Returns the current time in the music being played.
	 * @return The current time in the music.
	 */
	std::string getCurrentPlayTime();
	/**
	 * Returns the duration of the music being played.
	 * @return The duration of the music being played.
	 */
	std::string getPlayLength();

	/**
	 * Returns the BeatInfo object that is collecting the beats for the music.
	 * @return The BeatInfo object that is collecting the beats for the music.
	 */
	BeatInfo* getBeatInfo();
private:
	/**
	 * Adds a beat at the following time.
	 * @param time The time at which the beat occurs, in milliseconds, relative to the beginning of the music.
	 */
	void addBeat(long time);
	/**
	 * Clears all of the beats.
	 */
	void resetBeats();

	/**
	 * Converts a time, in milliseconds, to a string.
	 * @param time The time to convert, in milliseconds.
	 * @return A string representation of the given time, in the format hh:mm:ss.
	 */
	std::string timeToString(long time);

	/**
	 * The music player.
	 */
	wxMediaCtrl mPlayer;
	/**
	 * True if this object is ready to play music; false otherwise. 
	 */
	bool mIsReady;
	/**
	 * A string representing the duration of the music that is being played.
	 */
	std::string mPlayLength;

	/**
	 * A string representing an "invalid time." This could be used, for example, to display the duration of the music, when the music has not been loaded yet.
	 */
	static const std::string invalidTime;

	/**
	 * The beats in the music.
	 */
	BeatInfo* mBeatInfo;
};

/**
 * The dialogue used to define where the beats fall in a music file.
 */
class BeatsEditor : public wxDialog {
private:
	using super = wxDialog;

	DECLARE_EVENT_TABLE()
public:
	/**
	 * Constructor.
	 * @param targetAnimation The view containing the Animation that will be animated with the music.
	 * @param id The id of the window.
	 * @param caption The title of the window.
	 * @param pos The position of the window.
	 * @param size The size of the window.
	 * @param style The style of the window.
	 */
	BeatsEditor(AnimationView* targetAnimation, wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Music Beats"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
	/**
	 * Destructor.
	 */
	~BeatsEditor();

private:
	/**
	 * Stops the music and finishes planting beats into the music.
	 */
	void Stop();

	/**
	 * Updates the controls of the window.
	 */
	void Update();

	/**
	 * Called when the "Load" button is pressed. This prompts the user to choose a music file to load.
	 * @param event The event that prompted this function.
	 */
	void OnLoad(wxCommandEvent& event);
	/**
	 * Called when the "Export" button is pressed. This prompts the user to name an output file to record the beats of the music.
	 * @param event The event that prompted this action.
	 */
	void OnExport(wxCommandEvent& event);
	/**
	 * Called when the "Stop" button is pressed. This stops the music and finishes placing beats.
	 * @param event The event that prompted this action.
	 */
	void OnStop(wxCommandEvent& event);
	/**
	 * Called when the music reaches its end.
	 * @param event The event that prompted the action.
	 */
	void OnFinished(wxMediaEvent& event);
	/**
	 * Called when the user adds a beat to the music.
	 * @param event The event that prompted the action.
	 */
	void OnAddBeat(wxCommandEvent& event);
	/**
	 * Called when the button is pressed that toggles whether or not a beat will be placed when the user starts the music.
	 * @param event The event that prompted the action.
	 */
	void OnAutoFirstBeatToggle(wxCommandEvent& event);
	/**
	 * Called when the button is pressed that toggles whether or not a beat will be placed when the music stops (either automatically or because the user manually stopped it).
	 * @param event The event that prompted the action.
	 */
	void OnAutoLastBeatToggle(wxCommandEvent& event);

	/**
	 * Sets up the window.
	 * @param parent The parent window.
	 * @param id The id of the window.
	 * @param caption The name of the window.
	 * @param pos The position of the window.
	 * @param size The size of the window.
	 * @param style The style of the window.
	 */
	bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Music Beats"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

	/**
	 * Makes the controls of the window.
	 */
	void CreateControls();

	/**
	 * The AnimationView whose animation will be animated according to the beats of the music.
	 */
	AnimationView* mAnimationView;
	/**
	 * The object used to place beats in the music.
	 */
	BeatBuilder mBeatBuilder;

	/**
	 * True if a beat should be added automatically when the music is started; false otherwise.
	 */
	bool mShouldAddAutoFirstBeat;
	/**
	 * True if a beat should be automatically added when the music stops; false otherwise.
	 */
	bool mShouldAddAutoLastBeat;

	/**
	 * The file that is going to be played.
	 */
	std::string mTargetFile;

	/**
	 * Displays the name of the file that is currently being played.
	 */
	wxStaticText* mFileNameDisplay;
	/**
	 * Displays the number of beats that have been placed in the music.
	 */
	wxStaticText* mNumBeatsDisplay;
	/**
	 * Displays errors.
	 */
	wxStaticText* mErrorDisplay;
	/**
	 * The "Load" button.
	 */
	wxButton* mLoadButton;
	/**
	 * The "Export" button.
	 */
	wxButton* mExportButton;
	/**
	 * The "Stop" button.
	 */
	wxButton* mStopButton;
	/**
	 * The "Add New Beat" button.
	 */
	wxButton* mNewBeatButton;
	/**
	 * The "Close" button.
	 */
	wxButton* mCloseButton;
};

