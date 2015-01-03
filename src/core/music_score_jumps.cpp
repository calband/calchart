#include "music_score_jumps.h"
#include "music_score_moment_browser.h"

MusicScoreJumpsCollection::MusicScoreJumpsCollection(MusicScoreJump defaultEvent)
: super(defaultEvent)
{};

void MusicScoreJumpsCollection::addJump(MusicScoreMoment jumpTime, MusicScoreJump jumpDestination) {
	addEvent(jumpTime, jumpDestination);
}

MusicScoreJumpBrowser::MusicScoreJumpBrowser(const MusicScoreJumpsCollection* jumps, MusicScoreMomentBrowser* momentBrowser)
: mMomentBrowser(momentBrowser), super(jumps, momentBrowser->getCurrentTime())
{}

void MusicScoreJumpBrowser::reset(MusicScoreMoment startTime) {
	mMomentBrowser->resetIndependently(startTime);
	resetIndependently(startTime);
}

MusicScoreMoment MusicScoreJumpBrowser::getCurrentTime() const {
	return mMomentBrowser->getCurrentTime();
}

bool MusicScoreJumpBrowser::isValid() const {
	return isValidIndependently() && mMomentBrowser->isValidIndependently();
}

bool MusicScoreJumpBrowser::isValidIndependently() const {
	return super::isValid();
}

void MusicScoreJumpBrowser::resetIndependently(MusicScoreMoment startTime) {
	super::reset(startTime);
}

void MusicScoreJumpBrowser::setCurrentTime(MusicScoreMoment newTime) {
	mMomentBrowser->setCurrentTime(newTime);
}

void MusicScoreJumpBrowser::pushForwardTime() {
	mMomentBrowser->pushForwardTimeIndependently();
	transitionForwardUntilFinished();
}

void MusicScoreJumpBrowser::executeTransitionForward() {
	super::executeTransitionForward();
	MusicScoreMoment newCurrentTime = getMostRecentEvent().jumpTo;
	reset(newCurrentTime);
}

void MusicScoreJumpBrowser::pushBackTime() {
	mMomentBrowser->pushBackTimeIndependently();
	transitionBackUntilFinished();
}

void MusicScoreJumpBrowser::executeTransitionBack() {
	MusicScoreMoment newCurrentTime = getMostRecentEventTime();
	reset(newCurrentTime);
	pushBackTime();
}



