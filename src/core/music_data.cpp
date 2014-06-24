#include "music_data.h"

int MeasureData::getBar(int beat) const {
	return getMeasureTime(beat).bar;
}

int MeasureData::getBeat(int bar, int beat) const {
	GetBeatSeeker* seeker = new GetBeatSeeker(this, bar, beat);
	seek(seeker);
	int returnVal = seeker->getElapsedBeats();
	delete seeker;
	return returnVal;
}

MeasureTime MeasureData::getMeasureTime(int beat) const {
	GetBarSeeker* seeker = new GetBarSeeker(this, beat);
	seek(seeker);
	MeasureTime returnVal = seeker->getFinalMeasureTime();
	delete seeker;
	return returnVal;
}

int MeasureData::extractEventData(BeatsPerBarShiftEvent eventObj) const {
	return eventObj.beatsPerBar;
}

MeasureTime MeasureData::convertToMeasureTime(BeatsPerBarShiftEvent eventObj) const {
	return eventObj.toMeasureTime();
}

MeasureBrowser* MeasureData::makeMeasureBrowser(int beat) const {
	if (getNumEvents() > 0) {
		MeasureTime time = getMeasureTime(beat);
		return makeMeasureBrowser(time.bar, time.beat);
	} else {
		return nullptr;
	}
}

MeasureBrowser* MeasureData::makeMeasureBrowser(int bar, int beat) const {
	BeatsPerBarShiftEvent targetEncompassingEvent = getEncompassingEventMarker(bar, beat);
	int index;
	for (index = 0; index < getNumEvents(); index++) {
		if (getEvent(index).bar == targetEncompassingEvent.bar) {
			break;
		}
	}
	return MeasureBrowser::make(this, MeasureTime(bar, beat), index);
}

MeasureBrowser* MeasureBrowser::make(const MeasureData* source, MeasureTime startTime, int startIndex) {
	if (source->getNumEvents() > 0) {
		return new MeasureBrowser(source, startTime, startIndex);
	}
	return nullptr;
}

MeasureBrowser::MeasureBrowser(const MeasureData* source, MeasureTime startTime, int startIndex)
: super(source, startTime, startIndex)
{}

int MeasureBrowser::getNumBeatsInCurrentBar() const {
	return getCurrentEvent();
}

void MeasureBrowser::pushForwardTime() {
	MeasureTime current = getCurrentTime();
	current.beat += 1;
	if (current.beat >= getCurrentEvent()) {
		current.bar += 1;
		current.beat = 0;
	}
	setCurrentTime(current);
}


void MeasureBrowser::pushBackTime() {
	MeasureTime current = getCurrentTime();
	current.beat -= 1;
	if (current.beat < 0) {
		current.bar -= 1;
		current.beat = getCurrentEvent();
	}
	setCurrentTime(current);
}


bool MeasureBrowser::checkTransitionBack() {
	if (super::checkTransitionBack()) {
		MeasureTime current = getCurrentTime();
		current.beat = getCurrentEvent();
		setCurrentTime(current);
		return true;
	}
	return false;
}

int TempoData::getTempo(int beat, MeasureData* measureData) const {
	MeasureTime time = measureData->getMeasureTime(beat);
	return getTempo(time.bar, time.beat);
}

int TempoData::getTempo(int bar, int beat) const {
	return getEvent(bar, beat);
}

TempoBrowser* TempoData::makeTempoBrowser(int beat, MeasureData* measureData) const {
	if (measureData->getNumEvents() > 0) {
		MeasureTime time = measureData->getMeasureTime(beat);
		return makeTempoBrowser(time.bar, time.beat, measureData);
	} else {
		return nullptr;
	}
}

TempoBrowser* TempoData::makeTempoBrowser(int bar, int beat, MeasureData* measureData) const {
	TempoShiftEvent targetEncompassingEvent = getEncompassingEventMarker(bar, beat);
	int index;
	for (index = 0; index < getNumEvents(); index++) {
		TempoShiftEvent otherEvent = getEvent(index);
		if (otherEvent.bar == targetEncompassingEvent.bar && otherEvent.beat == targetEncompassingEvent.beat) {
			break;
		}
	}
	return TempoBrowser::make(this, measureData, MeasureTime(bar, beat), index);
}

int TempoData::extractEventData(TempoShiftEvent eventObj) const {
	return eventObj.beatsPerMinute;
}
MeasureTime TempoData::convertToMeasureTime(TempoShiftEvent eventObj) const {
	return eventObj;
}

TempoBrowser* TempoBrowser::make(const TempoData* source, const MeasureData* measureSource, MeasureTime startTime, int startIndex) {
	if (source->getNumEvents() > 0 && measureSource->getNumEvents() > 0) {
		return new TempoBrowser(source, measureSource, startTime, startIndex);
	}
	return nullptr;
}

TempoBrowser::TempoBrowser(const TempoData* source, const MeasureData* measureSource, MeasureTime startTime, int startIndex)
: super(source, startTime, startIndex), mMeasures(measureSource->makeMeasureBrowser(startTime.bar, startTime.beat))
{}

TempoBrowser::~TempoBrowser() {
	delete mMeasures;
}

bool TempoBrowser::isValid() const {
	return super::isValid() && mMeasures->isValid();
}

int TempoBrowser::getCurrentTempo() const {
	return getCurrentEvent();
}

void TempoBrowser::pushForwardTime() {
	mMeasures->nextBeat();
	setCurrentTime(mMeasures->getCurrentTime());
}

void TempoBrowser::pushBackTime() {
	mMeasures->previousBeat();
	setCurrentTime(mMeasures->getCurrentTime());
}

std::string SongData::getSong(int beat, MeasureData* measureData) const {
	MeasureTime time = measureData->getMeasureTime(beat);
	return getSong(time.bar, time.beat);
}

std::string SongData::getSong(int bar, int beat) const {
	return getEvent(bar, beat);
}

std::string SongData::extractEventData(SongChangeEvent eventObj) const {
	return eventObj.songName;
}

MeasureTime SongData::convertToMeasureTime(SongChangeEvent eventObj) const {
	return eventObj;
}


MusicData::MusicData()
: mBeatsPerBar(new MeasureData()), mTempos(new TempoData()), mSongData(new SongData())
{}

MusicData::~MusicData() {
	delete mBeatsPerBar;
	delete mTempos;
	delete mSongData;
}

MeasureData* MusicData::getBeatsPerBar() {
	return mBeatsPerBar;
}

TempoData* MusicData::getTempos() {
	return mTempos;
}

SongData* MusicData::getSongs() {
	return mSongData;
}

const MeasureData* MusicData::getBeatsPerBar() const {
	return mBeatsPerBar;
}

const TempoData* MusicData::getTempos() const {
	return mTempos;
}

const SongData* MusicData::getSongs() const {
	return mSongData;
}