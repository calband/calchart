#include "music_score_jumps.h"
#include "music_score_moment_browser.h"

MusicScoreJumpsCollection::MusicScoreJumpsCollection(MusicScoreJump defaultEvent)
: super(defaultEvent)
{};

void MusicScoreJumpsCollection::addJump(MusicScoreMoment jumpTime, MusicScoreJump jumpDestination) {
	addEvent(jumpTime, jumpDestination);
}

MusicScoreJumpBrowser::MusicScoreJumpBrowser(const MusicScoreJumpsCollection* jumps, MusicScoreMomentBrowser* momentBrowser)
: super(jumps), mMomentBrowser(momentBrowser)
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
	jumpUntilSettled();
}

void MusicScoreJumpBrowser::setCurrentTime(MusicScoreMoment newTime) {
	//The current time should only be set through the moment browser -- all calls made through the jump browser should be ignored
}

void MusicScoreJumpBrowser::pushForwardTime() {
	mMomentBrowser->pushForwardTimeIndependently();
	jumpUntilSettled();
}

void MusicScoreJumpBrowser::jumpUntilSettled() {
	do {
		fixCurrentEvent(); //Make sure that theevent we are pointing to is correct
		if (getCurrentTime() == getMostRecentEventTime()) { //Jump, if it is time to jump
			mMomentBrowser->resetIndependently(getMostRecentEvent().jumpTo); //Jump to the new time
		}
	} while (getCurrentTime() != getLastFixedTime());
}



