#pragma once

#include "music_score_events.h"
#include "music_score_time_signatures.h"
#include <memory>

class MusicScoreJumpBrowser;
class MusicScoreJumpsCollection;

/**
 * This browser steps through the show and keep track of the current time, which is composed of a beat, bar,
 * and fragment.
 * Keeping track of the current time is tricky because the time signature can change throughout,
 * and also the score can be broken into fragments and we need to track when you transition from
 * one to another.
 * In order to track the current beat, bar, and fragment, this browser must work in close conjunction
 * with a jump browser. This browser gets access to the changes in time signature, while the jump
 * browser gets access to when the score jumps from one piece to another.
 * When browsing one beat forward/back, the music score moment browser pushes forward a beat first. 
 * The music score moment browser knows how many beats are in each bar, so it can properly report what 
 * beat/bar the current time *would be* if no jump was taken. Then the jump browser takes
 * this time and checks if a jump should be taken. If it should, then it informs the music moment
 * browser.
 */
class MusicScoreMomentBrowser : public MusicScoreEventBrowser<TimeSignature> {
private:
	using super = MusicScoreEventBrowser<TimeSignature>;
public:
	virtual void reset(MusicScoreMoment startTime);

	virtual MusicScoreMoment getCurrentTime() const;

	virtual bool isValid() const;
protected:
	bool isValidIndependently() const;

	void resetIndependently(MusicScoreMoment startTime);

	virtual void setCurrentTime(MusicScoreMoment newTime);

	virtual void pushForwardTime();
	void pushForwardTimeIndependently();

	/**
	 * The MusicScoreJumpBrowser that works in conjunction with this moment browser.
	 * The jump browser makes sure that this browser is informed about when a jump needs to be taken.
	 */
	std::unique_ptr<MusicScoreJumpBrowser> mJumpBrowser;

	/**
	 * The current time being marked by this browser.
	 */
	MusicScoreMoment mCurrentTime;

	/**
	* Constructor.
	* @param timeSignatures The collection that tracks all of the changes to the time signature in the music score.
	* @param jumps The collection that tracks all of the jumps in the music score.
	* @param startTime The time to begin browsing from.
	*/
	MusicScoreMomentBrowser(const TimeSignaturesCollection* timeSignatures, const MusicScoreJumpsCollection* jumps, MusicScoreMoment startTime);

	friend class MusicScoreJumpBrowser;
	friend class TimeSignaturesCollection;
};
