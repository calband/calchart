#ifndef _PLAY_ANIMATION_CONTROLLER_H_
#define _PLAY_ANIMATION_CONTROLLER_H_

#include <wx/timer.h>
#include <wx/mediactrl.h>

#include "beat_info.h"
#include "music_data.h"

class AnimationView;
class AnimationFrame;

/**
 * Automatically advances an Animation object through the beats of a show.
 */
class PlayAnimationController {
public:
	/**
	 * Constructor.
	 * @param view The AnimationView whose Animation will be controlled through this player.
	 */
	PlayAnimationController(AnimationView* view);

	/**
	 * Activates the player, so that it will begin advancing the Animation through the beats of a show.
	 */
	virtual void play() = 0;
	/*
	 * Pauses the player, so that it will stop advancing an Animation through the beats of a show.
	 */
	virtual void pause() = 0;

	/**
	 * Returns true if the player is active.
	 * @return True if the player is currently active; false otherwise.
	 */
	virtual bool isPlaying() = 0;

	/**
	 * Scales the speed of the animation.
	 * @param scale The new scale for the speed of the animation.
	 */
	virtual void scaleSpeed(float scale);
	/**
	 * Returns the current scale for the speed of the animation.
	 * @return The current scale for the speed of the animation.
	 */
	virtual float getSpeedScale();

	/**
	 * Updates the player to a change made to the animation. This should be called anytime a change is made to the
	 * Animation through some source other than the player itself.
	 */
	virtual void update() = 0;
protected:
	/**
	 * Returns the AnimationView that is used by this player.
	 * @return The AnimationView that is used by this player.
	 */
	AnimationView* getAnimationView();
	/**
	 * Advances the show to the next beat.
	 * @return True if the animation was stepped to the next beat; false otherwise.
	 */
	virtual bool nextBeat();
	/**
	 * Returns whether or not there are any more beats in the show.
	 * @return True if there is at least one more beat in the show; false otherwise.
	 */
	virtual bool hasNextBeat();
private:
	/**
	 * The AnimationView that will be used to advance the Animation through the beats of the show.
	 */
	AnimationView* mView;
	/**
	 * The scale for the speed of the Animation.
	 */
	float mSpeedScale;
};



/**
 * An AnimationController that responds to a timer. The way in which it responds to the timer is unspecified.
 */
class TimedPlayAnimationController : public PlayAnimationController {
private:
	using super = PlayAnimationController;

	/**
	 * The timer that is associated with an animation controller. When it fires, it calls the timerFired() method of the TimedPlayAnimationController.
	 */
	class AnimationTimer : public wxTimer {
	public:
		/**
		 * Constructor.
		 * @param notifyTarget The animation controller that should be alerted whenever the timer fires.
		 */
		AnimationTimer(TimedPlayAnimationController& notifyTarget);
		/**
		 * Called when the timer fires.
		 */
		void Notify();
	private:
		/**
		 * The animation controller to alert anytime the timer fires.
		 */
		TimedPlayAnimationController& mNotifyTarget;
	};
public:
	/**
	 * Constructor.
	 * @param view The AnimationView whose Animation will be controlled by this animation controller.
	 */
	TimedPlayAnimationController(AnimationView* view);
	/**
	 * Destructor.
	 */
	virtual ~TimedPlayAnimationController();

protected:
	/**
	 * This method is caled when the timer associated with this animation controller fires.
	 */
	virtual void timerFired() = 0;

	/**
	 * Returns the timer associated with this animation controller.
	 * @return The timer associated with this animation controller.
	 */
	AnimationTimer* getTimer();
private:
	/**
	 * Makes a timer to associate with this animation controller.
	 * @return A new timer to associate with this animation controller.
	 */
	AnimationTimer* makeTimer();

	/**
	 * The timer associated with this animation controller.
	 */
	AnimationTimer* mTimer;
};



/**
 * An AnimationController that moves to the next beat of an animation each time its timer fires.
 */
class ClockedBeatPlayAnimationController : public TimedPlayAnimationController {
private:
	using super = TimedPlayAnimationController;
public:
	/**
	 * Constructor.
	 * @param view The AnimationView whose animation will be controlled with this Animation Controller.
	 */
	ClockedBeatPlayAnimationController(AnimationView* view);

	virtual void play();
	virtual void pause();

	virtual bool isPlaying();

	virtual void scaleSpeed(float scale);
protected:
	/**
	 * Restarts the timer associated with this Animation Controller.
	 */
	virtual void restartClock();
	/**
	 * Makes the timer for this Animation Controller continue. If the clock is running already, nothing will happen; otherwise, the clock will start running.
	 */
	virtual void continueClock();

	virtual void timerFired();

	/**
	 * Given that the timer is being started, returns the number of milliseconds that should ellapse before the next time the timer fires.
	 * @return The number of milliseconds that should ellapse before the next time the timer fires.
	 */
	virtual long getStartTimerInterval() = 0;
	/**
	 * Returns whether or not the timer should continue after it fires.
	 * @return True if the timer should be set up to stop after firing once; false if it should be set up to continue indefinitely.
	 */
	virtual bool oneTick();
private:
	/**
	 * True if the Animation Controller is paused; false otherwise.
	 */
	bool mPaused;
};



/**
 * An AnimationController that not only advances to the next beat when a timer fires, but might also reclock the speed of its timer. 
 */
class ReclockBeatPlayAnimationController : public ClockedBeatPlayAnimationController {
private:
	using super = ClockedBeatPlayAnimationController;
public:
	/**
	 * Constructor.
	 * @param view The AnimationView whose Animation will be played by this Animation Controller.
	 */
	ReclockBeatPlayAnimationController(AnimationView* view);

protected:
	virtual void timerFired();

	virtual long getStartTimerInterval();
	/**
	 * Given that the clock is already running, returns how many milliseconds until the next time it should fire.
	 * @return The number of milliseconds until the next time that the timer should fire.
	 */
	virtual long getNextTimerInterval() = 0;

	/**
	 * Returns whether or not the interval between timer firings should be adjusted.
	 * @return True if the interval between timer ticks should be adjusted; false otherwise.
	 */
	virtual bool shouldReclockThisTick();

	virtual bool oneTick();
};



/**
 * A ReclockBeatPlayAnimationcontroller that is sensitive to the current beat of the animation. 
 */
class BeatNumDependentAnimationController : public ReclockBeatPlayAnimationController {
private:
	using super = ReclockBeatPlayAnimationController;
public:
	/**
	 * Constructor.
	 * @param view The AnimationView whose Animation will be played by this Animation Controller.
	 */
	BeatNumDependentAnimationController(AnimationView* view);

	virtual void update();
protected:
	/**
	 * Returns the current beat of the animation (relative to the start of the show).
	 * @return The current beat of the animation (relative to the start of the show).
	 */
	int getCurrentBeatNum();

	virtual bool nextBeat();

	virtual bool hasNextBeat();
private:
	/**
	 * The current beat of the animation (relative to the start of the show).
	 */
	int mCurrBeat;
};

/**
 * An AnimationController which plays an animation in sync with music.
 */
class MusicPlayAnimationController : public BeatNumDependentAnimationController {
private:
	using super = BeatNumDependentAnimationController;
public:
	MusicPlayAnimationController(AnimationView* view, AnimationFrame* frame, BeatInfo* beats, std::string filePath, int musicPlayerId = wxID_ANY);

	virtual void play();
	virtual void pause();

	void update();

	virtual void scaleSpeed(float scale);
protected:
	virtual long getNextTimerInterval();
	virtual bool hasNextBeat();
private:
	/**
	 * The music player.
	 */
	wxMediaCtrl mMusicPlayer;
	/**
	 * A record of where ever beat is in the music file being played.
	 */
	BeatInfo* mBeats;
};

/**
 * An AnimationController which plays a show animation at varying speeds throughout, depending on the show's varying tempos. 
 */
class TempoPlayAnimationController : public BeatNumDependentAnimationController {
private:
	using super = BeatNumDependentAnimationController;
public:
	/**
	 * Constructor.
	 * @param view The AnimationView whose Animation will be played by this controller.
	 * @param tempos A collection of the tempos for the animation.
	 * @param bars A structure that defines where the bars fall in the music.
	 */
	TempoPlayAnimationController(AnimationView* view, TempoData* tempos, MeasureData* bars);
	/**
	 * Destructor.
	 */
	~TempoPlayAnimationController();

	virtual void update();
protected:
	/**
	 * Makes sure that the browser is valid. If the browser is not valid, it attempts to replace it with a new valid one.
	 * @return True if the browser was successfully made valid; false if it was invalid and could not be repaired.
	 */
	virtual bool validateBrowser();
	/**
	 * Replaces the old browser with a new one.
	 */
	virtual void rebuildBrowser();
	virtual long getStartTimerInterval();
	virtual long getNextTimerInterval();
	virtual bool nextBeat();
	virtual bool hasNextBeat();

	virtual bool shouldReclockThisTick();
	virtual bool oneTick();
private:
	/**
	 * The collection of tempo changes that define the speed of the animation.
	 */
	TempoData* mTempos;
	/**
	 * Describes where bars fall in the music.
	 */
	MeasureData* mMeasures;
	/**
	 * Used to detect tempo changes as you browse through the beats of the animation.
	 */
	TempoBrowser* mBrowser;

	/**
	 * The current tempo of the animation, in beats per minute.
	 */
	int mCurrentTempo;
};

/**
 * An AnimationController which plays an animation at a constant tempo. 
 */
class ConstantSpeedPlayAnimationController : public ClockedBeatPlayAnimationController {
private:
	using super = ClockedBeatPlayAnimationController;
public:
	/**
	 * Constructor.
	 * @param view The AnimationView whose Animation will be played by this object.
	 * @param beatsPerMinute The initial tempo for the animation, in beats per minute.
	 */
	ConstantSpeedPlayAnimationController(AnimationView* view, int beatsPerMinute);

	virtual void update();

	/**
	 * Changes the tempo of the animation.
	 * @param beatsPerMinute The new tempo for the Animation, in beats per minute.
	 */
	void setBPM(int beatsPerMinute);
protected:
	virtual long getStartTimerInterval();
	virtual bool nextBeat();
private:
	/**
	 * The current tempo of the animation, in beats per minute.
	 */
	int mBPM;
};

#endif