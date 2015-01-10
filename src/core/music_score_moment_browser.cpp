#include "music_score_jumps.h"
#include "music_score_moment_browser.h"

MusicScoreMomentBrowser::MusicScoreMomentBrowser(const TimeSignaturesCollection* timeSignatures, const MusicScoreJumpsCollection* jumps, MusicScoreMoment startTime)
: super(timeSignatures), mCurrentTime(startTime)
{
	mJumpBrowser.reset(new MusicScoreJumpBrowser(jumps, this));
	reset(startTime);
}

void MusicScoreMomentBrowser::reset(MusicScoreMoment startTime) {
	mJumpBrowser->reset(startTime);
}

MusicScoreMoment MusicScoreMomentBrowser::getCurrentTime() const {
	return mCurrentTime;
}

bool MusicScoreMomentBrowser::isValid() const {
	return mJumpBrowser->isValid();
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
	fixCurrentEvent();
}


