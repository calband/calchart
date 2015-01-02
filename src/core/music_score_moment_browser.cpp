#include "music_score_jumps.h"
#include "music_score_moment_browser.h"

MusicScoreMomentBrowser::MusicScoreMomentBrowser(const TimeSignaturesCollection* timeSignatures, const MusicScoreJumpsCollection* jumps, MusicScoreMoment startTime)
: super(timeSignatures, startTime), mCurrentTime(startTime)
{
	mJumpBrowser.reset(new MusicScoreJumpBrowser(jumps, this));
}

void MusicScoreMomentBrowser::reset(MusicScoreMoment startTime) {
	resetIndependently(startTime);
	mJumpBrowser->resetIndependently(startTime);
}

MusicScoreMoment MusicScoreMomentBrowser::getCurrentTime() const {
	return mCurrentTime;
}

bool MusicScoreMomentBrowser::isValid() const {
	return isValidIndependently() && mJumpBrowser->isValidIndependently();
}

bool MusicScoreMomentBrowser::isValidIndependently() const {
	return super::isValid();
}

void MusicScoreMomentBrowser::resetIndependently(MusicScoreMoment startTime) {
	super::reset(startTime);
}

void MusicScoreMomentBrowser::setCurrentTime(MusicScoreMoment newTime) {
	mCurrentTime = newTime;
}

void MusicScoreMomentBrowser::pushForwardTime() {
	mJumpBrowser->pushForwardTime();
}

void MusicScoreMomentBrowser::pushForwardTimeIndependently() {
	mCurrentTime.beatAndBar.beat++;
	if (mCurrentTime.beatAndBar.beat >= getMostRecentEvent().beatsPerBar) {
		mCurrentTime.beatAndBar.beat = 0;
		mCurrentTime.beatAndBar.bar++;
	}
	transitionForwardUntilFinished();
}

void MusicScoreMomentBrowser::pushBackTime() {
	mJumpBrowser->pushBackTime();
}

void MusicScoreMomentBrowser::pushBackTimeIndependently() {
	mCurrentTime.beatAndBar.beat--;
	transitionBackUntilFinished();
	mCurrentTime.beatAndBar.beat = getMostRecentEvent().beatsPerBar - 1;
	mCurrentTime.beatAndBar.bar--;

}

void MusicScoreMomentBrowser::executeTransitionBack() {
	
}



