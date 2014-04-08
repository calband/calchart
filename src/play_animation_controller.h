#ifndef _PLAY_ANIMATION_CONTROLLER_H_
#define _PLAY_ANIMATION_CONTROLLER_H_

#include <wx/timer.h>
#include <wx/mediactrl.h>

#include "core\BeatInfo.h"

class AnimationView;
class AnimationFrame;

class PlayAnimationController {
public:
	PlayAnimationController(AnimationView* view);

	virtual void play() = 0;
	virtual void pause() = 0;

	virtual bool isPlaying() = 0;

	virtual void scaleSpeed(float scale);
	virtual float getSpeedScale();

	virtual void update() = 0;
protected:
	AnimationView* getAnimationView();
	virtual void nextBeat();
private:
	AnimationView* mView;
	float mSpeedScale;
};


class TimedPlayAnimationController : public PlayAnimationController {
private:
	using super = PlayAnimationController;

	class AnimationTimer : public wxTimer {
	public:
		AnimationTimer(TimedPlayAnimationController& notifyTarget);
		void Notify();
	private:
		TimedPlayAnimationController& mNotifyTarget;
	};
public:

	TimedPlayAnimationController(AnimationView* view);
	virtual ~TimedPlayAnimationController();

protected:
	virtual void timerFired() = 0;

	AnimationTimer* getTimer();
private:
	AnimationTimer* makeTimer();

	AnimationTimer* mTimer;
};

class ClockedBeatPlayAnimationController : public TimedPlayAnimationController {
private:
	using super = TimedPlayAnimationController;
public:
	ClockedBeatPlayAnimationController(AnimationView* view);

	virtual void play();
	virtual void pause();

	virtual bool isPlaying();

	virtual void scaleSpeed(float scale);
protected:
	virtual void restartClock();
	virtual void restartAndContinueClock();

	virtual void timerFired();

	virtual long getStartTimerInterval() = 0;
	virtual bool oneTick();
};

class ReclockBeatPlayAnimationController : public ClockedBeatPlayAnimationController {
private:
	using super = ClockedBeatPlayAnimationController;
public:
	ReclockBeatPlayAnimationController(AnimationView* view);

protected:
	virtual void timerFired();

	virtual long getStartTimerInterval();
	virtual long getNextTimerInterval() = 0;

	virtual bool oneTick();
};

class BeatNumDependentAnimationController : public ReclockBeatPlayAnimationController {
private:
	using super = ReclockBeatPlayAnimationController;
public:
	BeatNumDependentAnimationController(AnimationView* view);

	virtual void update();
protected:
	int getCurrentBeatNum();

	virtual void nextBeat();
private:
	int mCurrBeat;
};


class MusicPlayAnimationController : public BeatNumDependentAnimationController {
private:
	using super = BeatNumDependentAnimationController;
public:
	MusicPlayAnimationController(AnimationView* view, AnimationFrame* frame, BeatInfo* beats, std::string filePath, int musicPlayerId = wxID_ANY);

	virtual void play();
	virtual void pause();

	void update();
protected:
	virtual long getNextTimerInterval();
private:
	wxMediaCtrl mMusicPlayer;
	BeatInfo* mBeats;
};

/*
class TempoPlayAnimationController : public BeatNumDependentAnimationController {
private:
	using super = BeatNumDependentAnimationController;
public:
	TempoPlayAnimationController(AnimationView* view, TempoData* tempos);
protected:
	virtual long getNextTimerInterval();
private:
	TempoData* mTempos;
};
*/

class ConstantSpeedPlayAnimationController : public ClockedBeatPlayAnimationController {
private:
	using super = ClockedBeatPlayAnimationController;
public:
	ConstantSpeedPlayAnimationController(AnimationView* view, int beatsPerMinute);

	virtual void update();

	void setBPM(int beatsPerMinute);
protected:
	virtual long getStartTimerInterval();
private:
	int mBPM;
};

#endif