#ifndef _BEATINFO_H_
#define _BEATINFO_H_

#include <vector>
#include <iostream>

/**
 * Records the timing for each beat in the show, in milliseconds.
 */
class BeatInfo {
public:
	/**
	 * Adds a beat.
	 * @param time The time at which the beat occurs, in milliseconds.
	 */
	void addBeat(long time);

	/**
	 * Removes a beat.
	 * @param time The time from which to remove a beat.
	 */
	void removeBeat(long time);

	/** 
	 * Removes a beat.
	 * @param beatNum The index of the beat to remove. A show starts at the zeroth beat.
	 */
	void removeBeatByIndex(int beatNum);

	/**
	 * Clears all beats.
	 */
	void clearBeats();

	/** 
	 * Returns the index of the active beat for a particular time. 
	 * Example: if beat 4 occurs at 30 milliseconds, and beat 5 occurs at 50 milliseconds, then the active beat for all times between 30 and 50 milliseconds will be 4.
	 * @param time The time to check.
	 * @return The beat that is active at the given time.
	 */
	int getCurrentBeat(long time) const;

	/**
	 * Returns the time, in milliseconds, at which the specified beat occurs.
	 * @beatNum The index of the target beat. The show starts at the zeroth beat.
	 * @return The time, in milliseconds, at which the specified beat occurs.
	 */
	long getBeat(int beatNum) const;

	/** 
	 * Returns the total number of beats.
	 * @return The total number of beats.
	 */
	int getNumBeats() const;
private:
	/**
	 * A list containing all of the beats.
	 */
	std::vector<long> mBeatsList;
};

#endif