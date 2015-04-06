#pragma once

#include "music_score_events.h"

class MusicScoreJumpsCollection;
class MusicScoreMomentBrowser;

/**
 * Represents a time signature in a music score.
 * As of now, the only important attribute of a time signature is the number of beats per bar.
 */
struct TimeSignature {

	/**
	 * Constructor.
	 * @param beatsPerBar The number of beats in a bar.
	 */
	TimeSignature(int32_t beatsPerBar) : beatsPerBar(beatsPerBar) {};

	/**
	 * The number of beats per bar.
	 */
	int32_t beatsPerBar;

	bool operator==(const TimeSignature other) const {
		return other.beatsPerBar == beatsPerBar;
	}
};

/**
 * A CollectionOfMusicScoreEvents structure which records changes in the time signature.
 */
class TimeSignaturesCollection : public CollectionOfMusicScoreEvents<TimeSignature> {
private:
	using super = CollectionOfMusicScoreEvents<TimeSignature>;
public:
	TimeSignaturesCollection(TimeSignature defaultEvent);

	/**
	 * Finds the time (bar and beat number) where a particular beat falls in the score.
	 * @param beat The number of beats elapsed since the start of the show.
	 * @return The time where the given beat falls in the score.
	 */
	//MusicScoreMoment getTimeFromBeatNumber(const Fragment* fragment, int beat) const;

	/**
	 * Adds a time signature change to the specified time.
	 * @param fragment The fragment during which the change occurs.
	 * @param bar The bar on which the change occurs.
	 * @param newTimeSignature The new time signature after the change.
	 */
	void addTimeSignatureChange(std::shared_ptr<const MusicScoreFragment> fragment, BarNumber bar, TimeSignature newTimeSignature);

	/**
	 * Makes a browser to step through the show, beat by beat, and provide the current time.
	 * @param jumps A collection that keeps track of when the score jumps from one place to another.
	 * @param startTime The time at which to start browsing.
	 * @return A browser to step through the show beat by beat and keep track of the current time.
	 */
	MusicScoreMomentBrowser* makeMusicScoreMomentBrowser(const MusicScoreJumpsCollection* jumps, MusicScoreMoment startTime) const;
};

