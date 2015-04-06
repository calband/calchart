#pragma once

#include "music_score_events.h"
#include "music_score_moment_browser.h"
#include <string>

class MusicScoreMomentBrowser;
class TimeSignaturesCollection;

/**
 * This can be used to jump between different parts of the score. This is particularly useful
 * when a score consists of pieces from other scores -- you can transition between different
 * fragments using these jump events.
 * To make use of these events, you enter them into a MusicScoreJumpsCollection. 
 * After you do so, any MusicScoreMomentBrowser object that is connected to that collection
 * will properly jump around the music score according to the events placed in the MusicScoreJumpsCollection. 
 */
struct MusicScoreJump {

	/**
	* Constructor.
	* @param fragmentJumpToTime When you jump to a new time, this marks the beat, bar, and
	*   fragment to jump to.
	*/
	MusicScoreJump(MusicScoreMoment fragmentJumpToTime) : jumpTo(fragmentJumpToTime) {};

	/**
	 * The time to jump to when this event is hit.
	 */
	MusicScoreMoment jumpTo;

	bool operator==(const MusicScoreJump other) const {
		return other.jumpTo == jumpTo;
	}
};

/**
 * Keeps track of all of the 'jumps' that occur in a music score. This is useful
 * when a score is made up of pieces of other scores. Then you can jump between the
 * different scores by entering MusicScoreJump events into this collection.
 */
class MusicScoreJumpsCollection : public CollectionOfMusicScoreEvents<MusicScoreJump> {
private:
	using super = CollectionOfMusicScoreEvents<MusicScoreJump>;
public:
	MusicScoreJumpsCollection(MusicScoreJump defaultEvent);

	/**
	 * Adds a jump event to the collection.
	 * @param jumpTime The time at which to make the jump.
	 * @param jumpDestination The time to jump to when the event is hit.
	 */
	void addJump(MusicScoreMoment jumpTime, MusicScoreJump jumpDestination);
};

/**
 * A browser that works in conjunction with a MusicScoreMomentBrowser to keep track of the
 * current time as you step through a score. Keeping track of the current time can be tricky
 * because of jump events. This browser makes sure that jumps are taken, while the MusicScoreMomentBrowser
 * makes sure that time transitions between different bars. This browser should not be accessed directly --
 * it should be used only by a MusicScoreMomentBrowser.
 */
class MusicScoreJumpBrowser : public MusicScoreEventBrowser<MusicScoreJump> {
private:
	using super = MusicScoreEventBrowser<MusicScoreJump>;
public:
	virtual void reset(MusicScoreMoment startTime);

	virtual MusicScoreMoment getCurrentTime() const;

	virtual bool isValid() const;
protected:
	bool isValidIndependently() const;

	void resetIndependently(MusicScoreMoment startTime);

	virtual void setCurrentTime(MusicScoreMoment newTime);

	virtual void pushForwardTime();

	void jumpUntilSettled();

	/**
	 * The MusicScoreMomentBrowser that works in conjunction with this jump browser.
	 */
	MusicScoreMomentBrowser* mMomentBrowser;

	/**
	 * Constructor.
	 * @param jumps The collection that tracks all of the jumps in the music score.
	 * @param momentBrowser The MusicScoreFragmentBrowser will work in conjunction with this MusicScoreMomentBrowser
	 *   to accurately keep track of the bar, beat, and fragment that the browsers are currently looking at.
	 *   This browser will always look at the same time as the provided moment browser.
	 */
	MusicScoreJumpBrowser(const MusicScoreJumpsCollection* jumps, MusicScoreMomentBrowser* momentBrowser);

	friend class MusicScoreMomentBrowser;
};
