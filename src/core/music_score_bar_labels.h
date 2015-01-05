#pragma once

#include "music_score_events.h"
#include <string>
/**
 * Represents a label for a bar of the score.
 */
struct MusicScoreBarLabel {

	/**
	 * Constructor.
	 * @param label The label for the bar.
	 */
	MusicScoreBarLabel(std::string label) : label(label) {};

	/**
	 * The label for the bar.
	 */
	std::string label;
};

/**
 * Keeps track of all of the bar labels for a music score. These bar labels
 * are used in generating show printouts.
 */
class MusicScoreBarLabelsCollection : public CollectionOfMusicScoreEvents<MusicScoreBarLabel> {
private:
	using super = CollectionOfMusicScoreEvents<MusicScoreBarLabel>;
public:
	MusicScoreBarLabelsCollection(MusicScoreBarLabel defaultEvent);

	/**
	 * Adds a bar label to the score.
	 * @param fragment The score fragment in which the bar label is located.
	 * @param bar The bar that is being labeled.
	 * @param label The label for the bar.
	 */
	void addBarLabel(const MusicScoreFragment* fragment, BarNumber bar, MusicScoreBarLabel label);

	/**
	 * Returns the name of the bar at the given time.
	 * If the bar is labeled, then its label will be returned; otherwise, its
	 * bar number will be returned as a string.
	 * @param time The time to get a bar label for.
	 * @return The bar label at the given time.
	 */
	std::string getBarName(MusicScoreMoment time);
};
