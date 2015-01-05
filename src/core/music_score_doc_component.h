#pragma once

#include "music_score_bar_labels.h"
#include "music_score_jumps.h"
#include "music_score_time_signatures.h"
#include "music_score_tempos.h"
#include "music_score_fragments.h"

/**
 * A component of the CalChartDoc which holds all information pertaining to the
 * music score for the show.
 */
class MusicScoreDocComponent {
public:
	/**
	 * Loads the Music Score data from a file.
	 * @param stream The file content.
	 * @return The Music Data structure encoded in the file.
	 */
	static MusicScoreDocComponent* loadFromFile(std::istream& stream);

	/**
	 * Serializes the music data so it can be written to a file.
	 * @return The serialized version of the music data.
	 */
	std::vector<uint8_t> serialize() const;

	/**
	 * Constructor.
	 */
	MusicScoreDocComponent();

	/**
	 * Returns the collection of all score jumps that occur in the show.
	 */
	MusicScoreJumpsCollection* getScoreJumps();

	/**
	 * Returns the collection of all bar labels in the show.
	 */
	MusicScoreBarLabelsCollection* getBarLabels();

	/**
	 * Returns the collection of all time signatures throughout the show score.
	 */
	TimeSignaturesCollection* getTimeSignatures();

	/**
	 * Returns the collection of all tempo changes throughout the show.
	 */
	MusicScoreTemposCollection* getTempos();

	/**
	 * Returns the collection of all score jumps that occur in the show.
	 */
	const MusicScoreJumpsCollection* getScoreJumps() const;

	/**
	 * Returns the collection of all bar labels in the show.
	 */
	const MusicScoreBarLabelsCollection* getBarLabels() const;

	/**
	 * Returns the collection of all time signatures throughout the show score.
	 */
	const TimeSignaturesCollection* getTimeSignatures() const;

	/**
	 * Returns the collection of all tempo changes throughout the show.
	 */
	const MusicScoreTemposCollection* getTempos() const;

	void addScoreFragment(std::shared_ptr<MusicScoreFragment> fragment);

	void removeScoreFragment(int fragmentIndex);

	void removeScoreFragment(MusicScoreFragment* fragment);

	unsigned getNumScoreFragments() const;
	
	std::shared_ptr<MusicScoreFragment> getScoreFragment(int fragmentIndex);

	std::vector<std::shared_ptr<MusicScoreFragment>> getScoreFragmentsByName(std::string name);

	std::shared_ptr<const MusicScoreFragment> getScoreFragment(int fragmentIndex) const;

	std::vector<std::shared_ptr<const MusicScoreFragment>> getScoreFragmentsByName(std::string name) const;




private:
	/**
	 * All score fragments in the show.
	 */
	std::vector<std::shared_ptr<MusicScoreFragment>> mFragments;

	/**
	 * Keeps track of the score jumps that occur throughout the show.
	 */
	std::unique_ptr<MusicScoreJumpsCollection> mJumps;

	/**
	 * Records the bar labels throughout the score.
	 */
	std::unique_ptr<MusicScoreBarLabelsCollection> mBarLabels;

	/**
	 * Records the time signatures throughout the show.
	 */
	std::unique_ptr<TimeSignaturesCollection> mTimeSignatures;

	/**
	 * Records the tempo changes that occur throughout the show.
	 */
	std::unique_ptr<MusicScoreTemposCollection> mTempos;
};
