#pragma once

#include "music_score_events.h"
#include "music_score_jumps.h"
#include "music_score_time_signatures.h"
#include <string>

class MusicScoreTempoBrowser;

/**
 * Represents the tempo at a particular time in the music score.
 */
struct MusicScoreTempo {

	/**
	 * Constructor.
	 * @param bpm The tempo in the score, in beats per minute.
	 */
	MusicScoreTempo(int32_t bpm) : beatsPerMinute(bpm) {};

	/**
	 * The tempo, in beats per minute.
	 */
	int32_t beatsPerMinute;

	bool operator==(const MusicScoreTempo other) const {
		return other.beatsPerMinute == beatsPerMinute;
	}
};

/**
 * Keeps track of all of the changes that occur in the tempo of the music score.
 */
class MusicScoreTemposCollection : public CollectionOfMusicScoreEvents<MusicScoreTempo> {
private:
	using super = CollectionOfMusicScoreEvents<MusicScoreTempo>;
public:
	MusicScoreTemposCollection(MusicScoreTempo defaultEvent);

	/**
	 * Adds a tempo change to the score.
	 * @param eventTime The time at which the tempo change occurs.
	 * @param tempo The new tempo to add, in beats per minute.
	 */
	void addTempoChange(MusicScoreMoment eventTime, MusicScoreTempo tempo);

	/**
	 * Constructor.
	 * @param jumps The collection that tracks all of the jumps in the music score.
	 * @param timeSignatures The collection that tracks changes to time signature.
	 * @param startTime The time to start browsing from.
	 */
	MusicScoreTempoBrowser* makeTempoBrowser(const MusicScoreJumpsCollection* jumps, const TimeSignaturesCollection* timeSignatures, MusicScoreMoment startTime) const;
};

/**
 * A browser that can step through the show, beat by beat, and track the current tempo.
 */
class MusicScoreTempoBrowser : public MusicScoreEventBrowser<MusicScoreTempo> {
private:
	using super = MusicScoreEventBrowser<MusicScoreTempo>;
public:
	virtual MusicScoreMoment getCurrentTime() const;

	virtual bool isValid() const;
protected:
	virtual void setCurrentTime(MusicScoreMoment newTime);

	virtual void pushForwardTime();
	virtual void pushBackTime();

	/**
	 * The MusicScoreMomentBrowser that keeps track of the current time.
	 */
	MusicScoreMomentBrowser* mMomentBrowser;

	/**
	 * Constructor.
	 * @param tempos The collection that tracks all of the tempo changes.
	 * @param jumps The collection that tracks all of the jumps in the music score.
	 * @param timeSignatures The collection that tracks changes to time signature.
	 * @param startTime The time to start browsing from.
	 */
	MusicScoreTempoBrowser(const MusicScoreTemposCollection* tempos, const MusicScoreJumpsCollection* jumps, const TimeSignaturesCollection* timeSignatures, MusicScoreMoment startTime);

	friend class MusicScoreTemposCollection;
};
