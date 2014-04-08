#include "play_animation_controller.h"
#include "core\cc_sheet.h"

#include "animation_frame.h"
#include "animation_view.h"

PlayAnimationController::PlayAnimationController(AnimationView* view)
: mView(view), mSpeedScale(1.0)
{}

void PlayAnimationController::scaleSpeed(float scale) {
	mSpeedScale = scale;
}

float PlayAnimationController::getSpeedScale() {
	return mSpeedScale;
}

AnimationView* PlayAnimationController::getAnimationView() {
	return mView;
}

void PlayAnimationController::nextBeat() {
	if (!getAnimationView()->NextBeat()) {
		pause();
	}
}


TimedPlayAnimationController::AnimationTimer::AnimationTimer(TimedPlayAnimationController& notifyTarget) 
: mNotifyTarget(notifyTarget)
{}

void TimedPlayAnimationController::AnimationTimer::Notify() {
	mNotifyTarget.timerFired();
}

TimedPlayAnimationController::TimedPlayAnimationController(AnimationView* view)
: super(view)
{
	mTimer = makeTimer();
}

TimedPlayAnimationController::~TimedPlayAnimationController() {
	delete mTimer;
}

TimedPlayAnimationController::AnimationTimer* TimedPlayAnimationController::getTimer() {
	return mTimer;
}

TimedPlayAnimationController::AnimationTimer* TimedPlayAnimationController::makeTimer() {
	return new AnimationTimer(*this);
}

ClockedBeatPlayAnimationController::ClockedBeatPlayAnimationController(AnimationView* view)
: super(view)
{}

void ClockedBeatPlayAnimationController::play() {
	restartClock();
}

void ClockedBeatPlayAnimationController::pause() {
	getTimer()->Stop();
}

bool ClockedBeatPlayAnimationController::isPlaying() {
	return getTimer()->IsRunning();
}

void ClockedBeatPlayAnimationController::timerFired() {
	nextBeat();
}

void ClockedBeatPlayAnimationController::restartClock() {
	if (getTimer()->IsRunning()) {
		getTimer()->Stop();
	}
	getTimer()->Start(getStartTimerInterval() / getSpeedScale(), oneTick());
}

void ClockedBeatPlayAnimationController::restartAndContinueClock() {
	if (getTimer()->IsRunning()) {
		restartClock();
	}
}

bool ClockedBeatPlayAnimationController::oneTick() {
	return false;
}

void ClockedBeatPlayAnimationController::scaleSpeed(float scale) {
	super::scaleSpeed(scale);
	if (isPlaying()) {
		restartClock();
	}
}

ReclockBeatPlayAnimationController::ReclockBeatPlayAnimationController(AnimationView* view)
: super(view)
{}

void ReclockBeatPlayAnimationController::timerFired() {
	super::timerFired();
	long nextTimerInterval = getNextTimerInterval() / getSpeedScale();
	if (nextTimerInterval <= 0) {
		timerFired();
	} else {
		getTimer()->Start(nextTimerInterval, oneTick());
	}
}

bool ReclockBeatPlayAnimationController::oneTick() {
	return true;
}

long ReclockBeatPlayAnimationController::getStartTimerInterval() {
	return getNextTimerInterval();
}


BeatNumDependentAnimationController::BeatNumDependentAnimationController(AnimationView* view)
: super(view)
{}

void BeatNumDependentAnimationController::update() {
	mCurrBeat = 0;
	int targetSheet = getAnimationView()->GetCurrentSheet();
	const CalChartDoc* show = getAnimationView()->GetShow();
	for (int counter = 0; counter < targetSheet; counter++) {
		mCurrBeat += show->GetNthSheet(counter)->GetBeats();
	}
	mCurrBeat += getAnimationView()->GetCurrentBeat();
	restartClock();
}

int BeatNumDependentAnimationController::getCurrentBeatNum() {
	return mCurrBeat;
}

void BeatNumDependentAnimationController::nextBeat() {
	super::nextBeat();
	mCurrBeat++;
}



MusicPlayAnimationController::MusicPlayAnimationController(AnimationView* view, AnimationFrame* frame, BeatInfo* beats, std::string filePath, int musicPlayerId)
: super(view), mBeats(beats), mMusicPlayer(frame, musicPlayerId, filePath, wxDefaultPosition, wxSize(0, 0))
{}

void MusicPlayAnimationController::play() {
	mMusicPlayer.Play();
	super::play();
}

void MusicPlayAnimationController::pause() {
	mMusicPlayer.Pause();
	super::pause();
}

long MusicPlayAnimationController::getNextTimerInterval() {
	int nextBeatNum = getCurrentBeatNum() + 1;
	if (nextBeatNum >= mBeats->getNumBeats()) {
		getTimer()->Stop();
		return 0;
	}
	return mBeats->getBeat(getCurrentBeatNum() + 1) - ((wxLongLong)mMusicPlayer.Tell()).GetValue();
}

void MusicPlayAnimationController::update() {
	super::update();
	mMusicPlayer.Seek(mBeats->getBeat(getCurrentBeatNum()));
	restartAndContinueClock();
}

/*
TempoPlayAnimationController::TempoPlayAnimationController(AnimationView* view, TempoData* tempos)
: super(view), mTempos(tempos)
{}


long TempoPlayAnimationController::getNextTimerInterval() {
	//TODO: get tempo at current beat, get time to next one
}
*/

ConstantSpeedPlayAnimationController::ConstantSpeedPlayAnimationController(AnimationView* view, int beatsPerMinute)
: super(view), mBPM(beatsPerMinute)
{}

void ConstantSpeedPlayAnimationController::update() 
{}

void ConstantSpeedPlayAnimationController::setBPM(int beatsPerMinute) {
	mBPM = beatsPerMinute;
	restartAndContinueClock();
}

long ConstantSpeedPlayAnimationController::getStartTimerInterval() {
	long returnVal = 1000 * 60 / mBPM;
	return returnVal;
}