#ifndef _MUSIC_DATA_H_
#define _MUSIC_DATA_H_

#include "measure_event_data.h"
#include <string>

class MeasureBrowser;
class TempoBrowser;

/**
 * An event representing a change in the number of beats per bar.
 */
struct BeatsPerBarShiftEvent : BeatlessMeasureTime {
	using super = BeatlessMeasureTime;

	/**
	 * Constructor.
	 * @param bar The bar where the event occurs.
	 * @param beatsPerBar The number of beats in a bar.
	 */
	BeatsPerBarShiftEvent(int bar, int beatsPerBar) : super(bar), beatsPerBar(beatsPerBar) {};
	/**
	 * Converts this BeatlessMeasureTime into a regular MeasureTime.
	 * @return A MeasureTime representing this BeatlessMeasureTime.
	 */
	MeasureTime toMeasureTime() { return MeasureTime(bar, 0); };

	/**
	 * The number of beats per bar.
	 */
	int beatsPerBar;
};

/**
 * An event representing a change in tempo.
 */
struct TempoShiftEvent : MeasureTime {
	using super = MeasureTime;

	/**
	 * Constructor.
	 * @param bar The bar where the event occurs.
	 * @param beat The beat where the event occurs.
	 * @param beatsPerMinute The new tempo after this event, in beats per inute.
	 */
	TempoShiftEvent(int bar, int beat, int beatsPerMinute) : super(bar, beat), beatsPerMinute(beatsPerMinute) {};

	/**
	 * The new tempo after this event, in beats per minute.
	 */
	int beatsPerMinute;
};

/**
 * An event representing a change in song.
 */
struct SongChangeEvent : MeasureTime {
	using super = MeasureTime;

	/**
	 * Constructor.
	 * @param bar The bar where the event occurs.
	 * @param beat The beat where the event occurs.
	 * @param songName The name of the song that begins with this event.
	 */
	SongChangeEvent(int bar, int beat, std::string songName) : super(bar, beat), songName(songName) {};

	/**
	 * The name of the song that begins at this event.
	 */
	std::string songName;
};

/**
 * A MeasureEventData structure which records changes in the number of beats per bar.
 */
class MeasureData : public MeasureEventData<BeatsPerBarShiftEvent, int> {
private:
	class BeatCountSeeker : public PreviousPreferringSeeker {
	private:
		using super = PreviousPreferringSeeker;
		using base = SeekerBase;
	public:
		/**
		 * Constructor.
		 * @param seekingObject The MeasureEventData structure whose elements are being browsed.
		 */
		BeatCountSeeker(const MeasureData* seekingObject) : super(seekingObject), mBeatsElapsed(0) {};
	protected:
		/**
		 * Counts and records the number of beats that elapsed between the last seen event and the provided event.
		 * @param nextShift The next event, to count the number of beats to.
		 */
		virtual void countElapsedBeats(BeatsPerBarShiftEvent nextShift) {
			addElapsedBeats(getPrevious().beatsPerBar * (nextShift.bar - getPrevious().bar));
		}

		/**
		 * Adds the given number of beats to the total number of recorded elapsed beats.
		 * @param beats The number of additional elapsed beats to record.
		 */
		virtual void addElapsedBeats(int beats) {
			mBeatsElapsed += beats;
		}

		/**
		 * Returns the total number of elapsed beats since the start of the seek.
		 * @return The total number of elapsed beats since the start of the seek.
		 */
		virtual int getElapsedBeats() {
			return mBeatsElapsed;
		}
	private:
		/**
		 * The total number of elapsed beats since the start of the seek.
		 */
		int mBeatsElapsed;
	};

	/**
	 * A seeker designed to find the MeasureTime that occurs a certain number of beats after the start of the show.
	 */
	class GetBarSeeker : public BeatCountSeeker {
	private:
		using super = BeatCountSeeker;
	public:
		/**
		 * Constructor.
		 * @param seekingObject The MeasureEventData object whose events are being browsed.
		 * @param destinationBeat The beat of the MeasureTime that we are trying to find.
		 */
		GetBarSeeker(const MeasureData* seekingObject, int destinationBeat) : super(seekingObject), mDestination(destinationBeat) {};

		/**
		 * After seeking is finished, returns the MeasureTime that occurs at the target beat.
		 * @return The MeasureTime that occurs at the target beat.
		 */
		MeasureTime getFinalMeasureTime() {
			return makeFinalMeasureTime();
		}
	protected:
		virtual bool finishedSeeking(BeatsPerBarShiftEvent seekObj) {
			countElapsedBeats(seekObj);
			return getElapsedBeats() > mDestination;
		}

		/**
		 * Calculates the final MeasureTime representation of the target beat.
		 * @return The final MeasureTime representation of the target beat.
		 */
		virtual MeasureTime makeFinalMeasureTime() {
			int spilloverBeats = mDestination - getElapsedBeats();
			int barSpillover = spilloverBeats / getPrevious().beatsPerBar;
			return MeasureTime(getPrevious().bar + barSpillover, spilloverBeats - getPrevious().beatsPerBar * barSpillover);
		}
	private:
		/**
		 * The target beat.
		 */
		int mDestination;
	};

	/**
	 * Finds the number of beats that elapse since the beginning of the show until a particular bar and beat.
	 */
	class GetBeatSeeker : public BeatCountSeeker {
	private:
		using super = BeatCountSeeker;
	public:
		/**
		 * Constructor.
		 * @param seekingObject The structure whose events are being browsed.
		 * @param bar The target bar.
		 * @param beat The target beat.
		 */
		GetBeatSeeker(const MeasureData* seekingObject, int bar, int beat) : BeatCountSeeker(seekingObject), mBeatSeeker(seekingObject, bar, beat) {};

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
		/**
		 * Used to find the first event that is beyond the bounds of the target time. That is the first event that should fail the test.
		 */
		BeatSeeker mBeatSeeker;
	};

public:
	/**
	 * Returns the bar which the given beat falls in.
	 * @param beat The target beat, relative to the start of the show.
	 * @return The bar in which the given beat falls.
	 */
	int getBar(int beat) const;
	/**
	 * Returns the number of beats elapsed before a particular bar and beat.
	 * @param bar The target bar.
	 * @param beat The target beat.
	 * @return The number of beats elapsed before a particular bar and beat.
	 */
	int getBeat(int bar, int beat = 0) const;

	/**
	 * Return the MeasureTime that marks the a target beat, relative to the beginning of the show.
	 * @param beat The target beat, relative to the beginning of the show.
	 * @return The MeasureTime marking the target beat.
	 */
	MeasureTime getMeasureTime(int beat) const;

	/**
	 * Makes a browser to step through the changes in the MeasureData beat by beat.
	 * @param beat The beat at which to start browsing, relative to the start of the show.
	 * @return A browser to step through the events of this object, beat by beat.
	 */
	MeasureBrowser* makeMeasureBrowser(int beat) const;
	/**
	 * Makes a browser to step through the changes in this MeasureData, beat by beat.
	 * @param bar The bar at which to start browsing.
	 * @param beat The beat at which to start browsing.
	 * @return A browser to step through the events of this object, beat by beat.
	 */
	MeasureBrowser* makeMeasureBrowser(int bar, int beat) const;
protected:
	virtual int extractEventData(BeatsPerBarShiftEvent eventObj) const;
	virtual MeasureTime convertToMeasureTime(BeatsPerBarShiftEvent eventObj) const;
};

/**
 * A browser to step through the events of  a MeasureData structure, beat by beat.
 */
class MeasureBrowser : public MeasureEventDataBrowser<BeatsPerBarShiftEvent, int> {
private:
	using super = MeasureEventDataBrowser;
public:
	/**
	 * Makes a new browser.
	 * @param source The MeasureData structure to step through.
	 * @param startTime The time at which to start browsing.
	 * @param startIndex The index of the event to start browsing from in the MeasureData structure.
	 * @return A new browser.
	 */
	static MeasureBrowser* make(const MeasureData* source, MeasureTime startTime, int startIndex);

	/**
	 * Returns the number of beats in the current bar that the browser is in.
	 * @return The number of beats in the bar that the browser is currently in.
	 */
	int getNumBeatsInCurrentBar() const;
protected:
	virtual void pushForwardTime();
	virtual void pushBackTime();
	virtual bool checkTransitionBack();

	/**
	 * Constructor.
	 * @param source The MeasureData structure to step through.
	 * @param startTime The time at which to start browsing.
	 * @param startIndex The index of the event to start browsing from in the MeasureData structure.
	 */
	MeasureBrowser(const MeasureData* source, MeasureTime startTime, int startIndex);
};

/**
 * A MeasureEventData structure that records the changes in tempo throughout the show.
 */
class TempoData : public MeasureEventData<TempoShiftEvent, int> {
public:
	/**
	 * Returns the tempo at a particular beat.
	 * @param beat The target beat, relative to the start of the show.
	 * @param measureData A structure containing information about how many beats are in each bar.
	 * @return The tempo at the particular beat.
	 */
	int getTempo(int beat, MeasureData* measureData) const;
	/**
	 * Returns the tempo at a particular time.
	 * @param bar The target bar.
	 * @param beat The target beat.
	 * @return The tempo at the given bar and beat.
	 */
	int getTempo(int bar, int beat) const;

	/**
	 * Makes a browser to step through the tempo changes, beat by beat.
	 * @param beat The beat to start browsing from, relative to the start of the show.
	 * @param measureData A structure that tracks how many beats are in each bar.
	 * @return A new browser to step through tempo changes.
	 */
	TempoBrowser* makeTempoBrowser(int beat, MeasureData* measureData) const;
	/**
	 * Makes a browser to step through the tempo changes, beat by beat.
	 * @param bar The bar to start browsing from.
	 * @param beat The beat to start browsing from, relative to the start of the bar.
	 * @param measureData A structure that tracks how many beats are in each bar.
	 * @return A new browser to step through tempo changes.
	 */
	TempoBrowser* makeTempoBrowser(int bar, int beat, MeasureData* measureData) const;
protected:
	virtual int extractEventData(TempoShiftEvent eventObj) const;
	virtual MeasureTime convertToMeasureTime(TempoShiftEvent eventObj) const;
};

/**
 * A browser to step through tempo changes, beat by beat.
 */
class TempoBrowser : public MeasureEventDataBrowser<TempoShiftEvent, int> {
private:
	using super = MeasureEventDataBrowser;
public:
	/**
	 * Makes a new TempoBrowser.
	 * @param source The TempoData structure to browse through.
	 * @param measureSource A structure that tracks how many beats are in each bar.
	 * @param startTime The time to start browsing from.
	 * @param startIndex The index of the event to start browsing from.
	 * @return A new browser.
	 */
	static TempoBrowser* make(const TempoData* source, const MeasureData* measureSource, MeasureTime startTime, int startIndex);
	/**
	 * Destructor.
	 */
	~TempoBrowser();

	virtual bool isValid() const;

	/**
	 * Returns the current tempo.
	 * @return The current tempo.
	 */
	int getCurrentTempo() const;
protected:
	virtual void pushForwardTime();
	virtual void pushBackTime();

	/**
	 * Constructor.
	 * @param source The TempoData structure to browse through.
	 * @param measureSource A structure that tracks how many beats are in each bar.
	 * @param startTime The time to start browsing from.
	 * @param startIndex The index of the event to start browsing from.
	 */
	TempoBrowser(const TempoData* source, const MeasureData* measureSource, MeasureTime startTime, int startIndex);
private:
	/**
	 * A browser that is used to track the number of beats in the current bar.
	 */
	MeasureBrowser* mMeasures;
};

/**
 * A MeasureEventData structure designed to record song changes in the music.
 */
class SongData : public MeasureEventData<SongChangeEvent, std::string> {
public:
	/**
	 * Returns the name of the song occuring at a target beat.
	 * @param beat The target beat.
	 * @param measureData A structure recording the number of beats per bar throughout the show.
	 * @return The name of the song occuring at the target time.
	 */
	std::string getSong(int beat, MeasureData* measureData) const;
	/**
	 * Returns the name of the song occuring at the target time.
	 * @param bar The target bar.
	 * @param beat The target beat.
	 * @return The name of the song occuring at the target time.
	 */
	std::string getSong(int bar, int beat) const;
protected:
	virtual std::string extractEventData(SongChangeEvent eventObj) const;
	virtual MeasureTime convertToMeasureTime(SongChangeEvent eventObj) const;
};

/**
 * A structure to record information about the music behind a CalChart Show.
 * This includes the number of beats per bar, the tempo changes, and the song names.
 */
class MusicData {
public:
	/**
	 * Constructor.
	 */
	MusicData();
	/**
	 * Destructor.
	 */
	~MusicData();

	/**
	 * Returns a MeasureData structure which records the number of beats per bar in the music throughout the show.
	 */
	MeasureData* getBeatsPerBar();
	/**
	 * Returns a TempoData structure which records the tempo changes that occur throughout the show.
	 */
	TempoData* getTempos();
	/*
	 * Returns a SongData structure which records the song changes that occur throughout the show.
	 */
	SongData* getSongs();
	/**
	 * Returns a MeasureData structure which records the number of beats per bar in the music throughout the show.
	 */
	const MeasureData* getBeatsPerBar() const;
	/**
	 * Returns a TempoData structure which records the tempo changes that occur throughout the show.
	 */
	const TempoData* getTempos() const;
	/*
	 * Returns a SongData structure which records the song changes that occur throughout the show.
	 */
	const SongData* getSongs() const;

private:
	/**
	 * Records the song changes that occur throughout the show.
	 */
	SongData* mSongData;
	/**
	 * Returns the number of beats per bar throughout the show.
	 */
	MeasureData* mBeatsPerBar;
	/**
	 * Records the tempo changes that occur throughout the show.
	 */
	TempoData* mTempos;
};

#endif