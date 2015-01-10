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
	 * Constructor.
	 */
	MusicScoreDocComponent();

	/**
	 * Copy constructor.
	 * @param other The doc component to copy.
	 */
	MusicScoreDocComponent(const MusicScoreDocComponent& other);

	/**
	 * Changes this doc component so that it becomes a copy of the
	 * given one.
	 * @param other The doc to copy from.
	 */
	void copyContentFrom(const MusicScoreDocComponent& other);

	/**
	 * Returns the collection of all score jumps that occur in the show.
	 * @return The collection of all score jumps.
	 */
	MusicScoreJumpsCollection* getScoreJumps();

	/**
	 * Returns the collection of all bar labels in the show.
	 * @return The collection of all bar labels.
	 */
	MusicScoreBarLabelsCollection* getBarLabels();

	/**
	 * Returns the collection of all time signatures throughout the show score.
	 * @return The collection of all time signatures.
	 */
	TimeSignaturesCollection* getTimeSignatures();

	/**
	 * Returns the collection of all tempo changes that occur throughout the show.
	 * @return The collection of all tempo changes.
	 */
	MusicScoreTemposCollection* getTempos();

	/**
	 * Returns the collection of all score jumps that occur in the show.
	 * @return The collection of all score jumps.
	 */
	const MusicScoreJumpsCollection* getScoreJumps() const;

	/**
	 * Returns the collection of all bar labels in the show.
	 * @return The collection of all bar labels.
	 */
	const MusicScoreBarLabelsCollection* getBarLabels() const;

	/**
	 * Returns the collection of all time signatures throughout the show score.
	 * @return The collection of all time signatures.
	 */
	const TimeSignaturesCollection* getTimeSignatures() const;

	/**
	 * Returns the collection of all tempo changes that occur throughout the show.
	 * @return The collection of all tempo changes.
	 */
	const MusicScoreTemposCollection* getTempos() const;

	/**
	 * Adds a score fragment to the show.
	 * @param fragment The fragment to register with the show.
	 */
	void addScoreFragment(std::shared_ptr<MusicScoreFragment> fragment);

	/**
	 * Removes a fragment from the show. All fragments are stored as shared pointers, so it will not be deleted
	 * until all shared pointers referencing it are destroyed.
	 * If the removeAssociatedEvents field is true, then all 'events' associated with the fragment are removed
	 * from all of the subcomponents of this object. 
	 * (Subcomponents include the tempo changes collection, the bar labels collection, etc.)
	 * @param fragmentIndex The index of the fragment to remove.
	 * @param removeAssociatedEvents True if the events associated with the fragment should be removed from the
	 *   subcomponents of this object; false if they should be conserved.
	 */
	void removeScoreFragment(int fragmentIndex, bool removeAssociatedEvents = true);

	/**
	 * Checks if the given fragment has been registered with the doc component.
	 * @param fragment The fragment to check.
	 * @return True if the fragment has been registered with the doc component; false otherwise.
	 */
	bool scoreFragmentIsRegistered(const MusicScoreFragment* fragment) const;

	/**
	 * Returns the number of fragments in the show.
	 * @return The number of fragments in the show.
	 */
	unsigned getNumScoreFragments() const;
	
	/**
	 * Returns the score fragment with the given index.
	 * @param fragmentIndex The index of the fragment to retrieve. The first fragment is at index 0.
	 * @return A shared pointer to the fragment at the given index; null if the index is invalid.
	 */
	std::shared_ptr<MusicScoreFragment> getScoreFragment(int fragmentIndex);

	/**
	 * Returns the score fragment with the given index.
	 * @param fragmentIndex The index of the fragment to retrieve. The first fragment is at index 0.
	 * @return A shared pointer to the fragment at the given index; null if the index is invalid.
	 */
	std::shared_ptr<const MusicScoreFragment> getScoreFragment(int fragmentIndex) const;
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
