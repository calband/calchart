#ifndef _BEATINFO_H_
#define _BEATINFO_H_

#include <vector>
#include <iostream>
class BeatInfo {
public:
	int beatCount = 0;
	std::vector<long> beatsList;

	/*Adds a beat.
	time = the time at which the beat occurs (that is, the number of milliseconds since the beginning of the song to the beat)  */
	void addBeat(long time);

	/*Removes the beat that occurs at a particular time. */
	void removeBeat(long time);

	/* Removes a particular beat. (e.g. remove the 5th beat translates to removeBeat(5), and remove the 0th beat translates to removeBeat(0)) */
	void removeBeatByIndex(int beatNum);

	/*Clears all beats */
	void clearBeats();

	/* Returns the beatNum of the beat occurring at a particular time. E.g. if the 1st beat occurs at 20ms, and the second beat occurs at 30 ms, then getCurrentBeat(25) returns 1, because 25 ms occurs during the first beat, and before the second. */
	int getCurrentBeat(long time);

	/* Returns the time, in milliseconds, at which a particular beat occurs. */
	long getBeat(int beatNum);

	/* Returns the total number of beats in the music. */
	int getNumBeats();
};

#endif