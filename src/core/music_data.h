#ifndef _MUSIC_DATA_H_
#define _MUSIC_DATA_H_

#include "measure_event_data.h"
#include <string>

class MeasureBrowser;
class TempoBrowser;


struct BeatsPerBarShiftEvent : BeatlessMeasureTime {
	using super = BeatlessMeasureTime;

	BeatsPerBarShiftEvent(int bar, int beatsPerBar) : super(bar), beatsPerBar(beatsPerBar) {};
	MeasureTime toMeasureTime() { return MeasureTime(bar, 0); };

	int beatsPerBar;
};

struct TempoShiftEvent : MeasureTime {
	using super = MeasureTime;

	TempoShiftEvent(int bar, int beat, int beatsPerMinute) : super(bar, beat), beatsPerMinute(beatsPerMinute) {};

	int beatsPerMinute;
};

struct SongChangeEvent : MeasureTime {
	using super = MeasureTime;

	SongChangeEvent(int bar, int beat, std::string songName) : super(bar, beat), songName(songName) {};

	std::string songName;
};


class MeasureData : public MeasureEventData<BeatsPerBarShiftEvent, int> {
private:
	class BeatCountSeeker : public PreviousPreferringSeeker {
	private:
		using super = PreviousPreferringSeeker;
		using base = SeekerBase;
	public:
		BeatCountSeeker(MeasureData* seekingObject) : super(seekingObject) {};
	protected:
		virtual void countElapsedBeats(BeatsPerBarShiftEvent nextShift) {
			addElapsedBeats(getPrevious().beatsPerBar * (nextShift.bar - getPrevious().bar));
		}

		virtual void addElapsedBeats(int beats) {
			mBeatsElapsed += beats;
		}

		virtual int getElapsedBeats() {
			return mBeatsElapsed;
		}
	private:
		int mBeatsElapsed;
	};

	class GetBarSeeker : public BeatCountSeeker {
	private:
		using super = BeatCountSeeker;
	public:
		GetBarSeeker(MeasureData* seekingObject, int destinationBeat) : super(seekingObject), mDestination(destinationBeat) {};

		MeasureTime getFinalMeasureTime() {
			return makeFinalMeasureTime();
		}
	protected:
		virtual bool finishedSeeking(BeatsPerBarShiftEvent seekObj) {
			countElapsedBeats(seekObj);
			return getElapsedBeats() > mDestination;
		}

		virtual MeasureTime makeFinalMeasureTime() {
			int spilloverBeats = mDestination - getElapsedBeats();
			int barSpillover = spilloverBeats / getPrevious().beatsPerBar;
			return MeasureTime(getPrevious().bar + barSpillover, spilloverBeats - getPrevious().beatsPerBar * barSpillover);
		}
	private:
		int mDestination;
	};

	class GetBeatSeeker : public BeatCountSeeker {
	private:
		using super = BeatCountSeeker;
	public:
		GetBeatSeeker(MeasureData* seekingObject, int bar, int beat) : BeatCountSeeker(seekingObject), mBeatSeeker(seekingObject, bar, beat) {};

		int getElapsedBeats() {
			return super::getElapsedBeats();
		}
	protected:
		virtual bool finishedSeeking(BeatsPerBarShiftEvent seekObj) {
			if (!mBeatSeeker.test(seekObj)) {
				countElapsedBeats(seekObj);
				return false;
			} else {
				addElapsedBeats(mBeatSeeker.getSeekBeat());
				return true;
			}
		}
	private:
		BeatSeeker mBeatSeeker;
	};

public:
	int getBar(int beat);
	int getBeat(int bar, int beat = 0);

	MeasureTime getMeasureTime(int beat);

	MeasureBrowser* makeMeasureBrowser(int beat);
	MeasureBrowser* makeMeasureBrowser(int bar, int beat);
protected:
	virtual int extractEventData(BeatsPerBarShiftEvent eventObj);
	virtual MeasureTime convertToMeasureTime(BeatsPerBarShiftEvent eventObj);
};

class MeasureBrowser : public MeasureEventDataBrowser<BeatsPerBarShiftEvent, int> {
private:
	using super = MeasureEventDataBrowser;
public:
	MeasureBrowser(MeasureData* source, MeasureTime startTime, int startIndex);

	int getNumBeatsInCurrentBar();
protected:
	virtual void pushForwardTime();
	virtual void pushBackTime();
	virtual bool checkTransitionBack();
};

class TempoData : public MeasureEventData<TempoShiftEvent, int> {
public:
	int getTempo(int beat, MeasureData* measureData);
	int getTempo(int bar, int beat);

	TempoBrowser* makeTempoBrowser(int beat, MeasureData* measureData);
	TempoBrowser* makeTempoBrowser(int bar, int beat, MeasureData* measureData);
protected:
	virtual int extractEventData(TempoShiftEvent eventObj);
	virtual MeasureTime convertToMeasureTime(TempoShiftEvent eventObj);
};

class TempoBrowser : public MeasureEventDataBrowser<TempoShiftEvent, int> {
private:
	using super = MeasureEventDataBrowser;
public:
	TempoBrowser(TempoData* source, MeasureData* measureSource, MeasureTime startTime, int startIndex);
	~TempoBrowser();

	virtual bool isValid();

	int getCurrentTempo();
protected:
	virtual void pushForwardTime();
	virtual void pushBackTime();
private:
	MeasureBrowser* mMeasures;
};

class SongData : public MeasureEventData<SongChangeEvent, std::string> {
public:
	std::string getSong(int beat, MeasureData* measureData);
	std::string getSong(int bar, int beat);
protected:
	virtual std::string extractEventData(SongChangeEvent eventObj);
	virtual MeasureTime convertToMeasureTime(SongChangeEvent eventObj);
};


class MusicData {
public:
	MusicData();
	~MusicData();

	MeasureData* getBeatsPerBar();
	TempoData* getTempos();
	SongData* getSongs();

private:
	SongData* mSongData;
	MeasureData* mBeatsPerBar;
	TempoData* mTempos;
};

#endif