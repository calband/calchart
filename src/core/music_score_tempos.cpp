#include "music_score_tempos.h"

MusicScoreTemposCollection::MusicScoreTemposCollection(MusicScoreTempo defaultEvent)
: super(defaultEvent)
{}

void MusicScoreTemposCollection::addTempoChange(MusicScoreMoment eventTime, MusicScoreTempo tempo) {
	addEvent(eventTime, tempo);
}

MusicScoreTempoBrowser* MusicScoreTemposCollection::makeTempoBrowser(const MusicScoreJumpsCollection* jumps, const TimeSignaturesCollection* timeSignatures, MusicScoreMoment startTime) const {
	if (jumps == nullptr || timeSignatures == nullptr) {
		return nullptr;
	}
	return new MusicScoreTempoBrowser(this, jumps, timeSignatures, startTime);
}

MusicScoreMoment MusicScoreTempoBrowser::getCurrentTime() const {
	return mMomentBrowser->getCurrentTime();
}

bool MusicScoreTempoBrowser::isValid() const {
	return super::isValid() && mMomentBrowser->isValid();
}

void MusicScoreTempoBrowser::setCurrentTime(MusicScoreMoment newTime) {
	//Do nothing -- this should never be called
}

void MusicScoreTempoBrowser::pushForwardTime() {
	mMomentBrowser->nextBeat();
}

MusicScoreTempoBrowser::MusicScoreTempoBrowser(const MusicScoreTemposCollection* tempos, const MusicScoreJumpsCollection* jumps, const TimeSignaturesCollection* timeSignatures, MusicScoreMoment startTime)
: super(tempos), mMomentBrowser(timeSignatures->makeMusicScoreMomentBrowser(jumps, startTime))
{
	reset(mMomentBrowser->getCurrentTime());
}


