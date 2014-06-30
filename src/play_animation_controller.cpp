#include "play_animation_controller.h"
#include "cc_sheet.h"

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

bool PlayAnimationController::nextBeat() {
	if (hasNextBeat()) {
		return getAnimationView()->NextBeat();
	} else{
		pause();
		return false;
	}
}

bool PlayAnimationController::hasNextBeat() {
	return true;
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
: super(view), mPaused(true)
{}

void ClockedBeatPlayAnimationController::play() {
	mPaused = false;
	restartClock();
}

void ClockedBeatPlayAnimationController::pause() {
	mPaused = true;
	getTimer()->Stop();
}

bool ClockedBeatPlayAnimationController::isPlaying() {
	return !mPaused;
}

void ClockedBeatPlayAnimationController::timerFired() {
	nextBeat();
	if (!hasNextBeat()) {
		pause();
	}
}

void ClockedBeatPlayAnimationController::restartClock() {
	getTimer()->Stop();
	if (hasNextBeat()) {
		getTimer()->Start(getStartTimerInterval() / getSpeedScale(), oneTick());
	} else {
		pause();
	}
}

void ClockedBeatPlayAnimationController::continueClock() {
	if (getTimer()->IsRunning()) {
		restartClock();
	}
}

bool ClockedBeatPlayAnimationController::oneTick() {
	return false;
}

void ClockedBeatPlayAnimationController::scaleSpeed(float scale) {
	super::scaleSpeed(scale);
	continueClock();
}

ReclockBeatPlayAnimationController::ReclockBeatPlayAnimationController(AnimationView* view)
: super(view)
{}

void ReclockBeatPlayAnimationController::timerFired() {
	super::timerFired();
	if (isPlaying() && hasNextBeat() && shouldReclockThisTick()) {
		long nextTimerInterval = getNextTimerInterval() / getSpeedScale();
		if (isPlaying()) {
			if (nextTimerInterval <= 0) {
				timerFired();
			}
			else {
				getTimer()->Start(nextTimerInterval, oneTick());
			}
		}
	}
}

bool ReclockBeatPlayAnimationController::shouldReclockThisTick() {
	return oneTick();
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
	continueClock();
}

int BeatNumDependentAnimationController::getCurrentBeatNum() {
	return mCurrBeat;
}

bool BeatNumDependentAnimationController::nextBeat() {
	bool returnVal = super::nextBeat();
	mCurrBeat++;
	return returnVal;
}

bool BeatNumDependentAnimationController::hasNextBeat() {
	return getAnimationView()->GetCurrentSheet() < getAnimationView()->GetNumberSheets() - 1 || getAnimationView()->GetCurrentBeat() < getAnimationView()->GetNumberBeats() - 1;
}

MusicPlayAnimationController::MusicPlayAnimationController(AnimationView* view, AnimationFrame* frame, BeatInfo* beats, std::string filePath, int musicPlayerId)
: super(view), mBeats(beats), mMusicPlayer(frame, musicPlayerId, filePath, wxDefaultPosition, wxSize(0, 0))
{}

void MusicPlayAnimationController::play() {
	super::play();
	if (isPlaying()) {
		mMusicPlayer.Play();
	}
}

void MusicPlayAnimationController::pause() {
	mMusicPlayer.Pause();
	super::pause();
}

void MusicPlayAnimationController::scaleSpeed(float scale) {
	super::scaleSpeed(scale);
	mMusicPlayer.SetPlaybackRate(getSpeedScale());
}

long MusicPlayAnimationController::getNextTimerInterval() {
	return mBeats->getBeat(getCurrentBeatNum() + 1) - ((wxLongLong)mMusicPlayer.Tell()).GetValue();
}

void MusicPlayAnimationController::update() {
	super::update();
	if (hasNextBeat()) {
		mMusicPlayer.Seek(mBeats->getBeat(getCurrentBeatNum()));
	}
	continueClock();
}

bool MusicPlayAnimationController::hasNextBeat() {
	return super::hasNextBeat() && getCurrentBeatNum() < mBeats->getNumBeats() - 1;
}

TempoPlayAnimationController::TempoPlayAnimationController(AnimationView* view, TempoData* tempos, MeasureData* bars)
: super(view), mTempos(tempos), mMeasures(bars), mBrowser(nullptr)
{}

TempoPlayAnimationController::~TempoPlayAnimationController() {
	if (mBrowser != nullptr) {
		delete mBrowser;
	}
}

void TempoPlayAnimationController::update() {
	super::update();
	rebuildBrowser();
	continueClock();
}

void TempoPlayAnimationController::rebuildBrowser() {
	if (mBrowser != nullptr) {
		delete mBrowser;
	}
	mBrowser = mTempos->makeTempoBrowser(getCurrentBeatNum(), mMeasures);
}

bool TempoPlayAnimationController::validateBrowser() {
	if (mBrowser == nullptr || !mBrowser->isValid()) {
		rebuildBrowser();
	}
	return mBrowser->isValid();
}

bool TempoPlayAnimationController::nextBeat() {
	if (!validateBrowser()) {
		pause();
		return false;
	} else {
		mBrowser->nextBeat();
		return super::nextBeat();
	}
}

bool TempoPlayAnimationController::hasNextBeat() {
	return super::hasNextBeat() && mTempos->getNumEvents() > 0 && mMeasures->getNumEvents() > 0;
}

long TempoPlayAnimationController::getStartTimerInterval() {
	mCurrentTempo = mBrowser->getCurrentTempo();
	return super::getStartTimerInterval();
}

long TempoPlayAnimationController::getNextTimerInterval() {
	return 1000 * 60 / mCurrentTempo;
}

bool TempoPlayAnimationController::shouldReclockThisTick() {
	int newTempo = mBrowser->getCurrentTempo();
	bool returnVal = newTempo != mCurrentTempo;
	mCurrentTempo = newTempo;
	return returnVal;
}

bool TempoPlayAnimationController::oneTick() {
	return false;
}

ConstantSpeedPlayAnimationController::ConstantSpeedPlayAnimationController(AnimationView* view, int beatsPerMinute)
: super(view), mBPM(beatsPerMinute)
{}

void ConstantSpeedPlayAnimationController::update() 
{}

void ConstantSpeedPlayAnimationController::setBPM(int beatsPerMinute) {
	mBPM = beatsPerMinute;
	continueClock();
}

long ConstantSpeedPlayAnimationController::getStartTimerInterval() {
	long returnVal = 1000 * 60 / mBPM;
	return returnVal;
}

bool ConstantSpeedPlayAnimationController::nextBeat() {
	if (!super::nextBeat()) {
		pause();
		return false;
	}
	return true;
}