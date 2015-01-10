#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>

template <typename EventType> class MusicScoreEventBrowser;


typedef int32_t BarNumber;
typedef int32_t BeatNumber;

/**
 * Marks a specific beat of a specific bar of music.
 */
struct BeatAndBar {
	/**
	 * Constructor.
	 * @param bar The bar of the time that this object marks. Bar 0 is the first bar in the show.
	 * @param beat The beat of the time that this object marks (relative to the start of the bar that it marks). Beat 0 is the first beat in the bar.
	 */
	BeatAndBar(BarNumber bar, BeatNumber beat) : bar(bar), beat(beat) {};

	BarNumber bar;
	BeatNumber beat;

	/**
	 * Returns whether or not this object marks the same time as the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks the same time as the input; false otherwise.
	 */
	inline bool operator==(const BeatAndBar other) const {
		return (bar == other.bar
			&& beat == other.beat);
	};

	/**
	 * Returns whether or not this object marks an earlier time than the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks an earlier time than the input; false otherwise.
	 */
	inline bool operator<(const BeatAndBar other) const {
		if (bar < other.bar) {
			return true;
		}
		if (bar == other.bar) {
			return (beat < other.beat);
		}
		return false;
	};

	/**
	 * Returns whether or not this object marks a later time than the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks a later time than the input; false otherwise.
	 */
	inline bool operator>(const BeatAndBar other) const {
		if (bar > other.bar) {
			return true;
		}
		if (bar == other.bar) {
			return (beat > other.beat);
		}
		return false;
	};

	/**
	 * Returns whether or not this object marks an earlier time than or same time as the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks an earlier time than or same time as the input; false otherwise.
	 */
	inline bool operator<=(const BeatAndBar other) const {
		return ((*this) < other || (*this) == other);
	};

	/**
	 * Returns whether or not this object marks a later time than or the same time as the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks a later time than or same time as the input; false otherwise.
	 */
	inline bool operator>=(const BeatAndBar other) const {
		return ((*this) > other || (*this) == other);
	};

	/**
	 * Returns whether or not this object is not the same time as another.
	 * @param other The time to compare to this one.
	 * @return True if this object does not mark the same time as the other; false otherwise.
	 */
	inline bool operator!=(const BeatAndBar other) const {
		return !((*this) == other);
	};
};

/**
 * Represents a "fragment" of the music score. A music score can be composed
 * of pieces from other scores, and each of those pieces is a "fragment."
 */
struct MusicScoreFragment {

	/**
	 * Constructor.
	 * @param fragmentName The name of the fragment.
	 */
	MusicScoreFragment(std::string fragmentName) : name(fragmentName) {};

	/**
	 * The name of the fragment. This is convenient for users to differentiate
	 * between them.
	 */
	std::string name;
};

/**
 * Marks a particular moment in the music score.
 * A moment consists of a bar, a beat, and a fragment.
 * The "fragment" component of a moment may seem a little unintuitive at first, but here is the reason for it:
 * The score for a show might be the compilation of other music scores, and a user may need to identify some specific
 * bar and beat in one of the constituent scores. The fragment allows you to differentiate between the constituent scores and
 * mark a moment specific to one of them.
 */
struct MusicScoreMoment {
	/**
	 * Constructor.
	 * @param fragment The score fragment to which the object corresponds. The score can be a combination of completely independent music pieces,
	 *   where the bar numbers in the various pieces are absolutely separate from one another. Each of those independent music pieces counts as a score "fragment."
	 *   Fragment 0 is the first fragment in the show.
	 * @param bar The bar of the time that this object marks. Bar 0 is the first bar in the show.
	 * @param beat The beat of the time that this object marks (relative to the start of the bar that it marks). Beat 0 is the first beat in the bar.
	 */
	MusicScoreMoment(std::shared_ptr<const MusicScoreFragment> scoreFragment, BarNumber bar, BeatNumber beat) : beatAndBar(bar, beat), isValid(true), fragment(scoreFragment) {};

	std::shared_ptr<const MusicScoreFragment> fragment;
	BeatAndBar beatAndBar;
	bool isValid;

	/**
	 * Returns whether or not this moment occurs in the same fragment as another.
	 * @param other The other moment to check against this one.
	 * @return True if the moments occur in the same fragment; false otherwise.
	 */
	inline bool inSameFragment(const MusicScoreMoment other) const {
		return (fragment.get() == other.fragment.get());
	}

	/**
	 * Returns whether or not this object marks the same time as the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks the same time as the input; false otherwise.
	 */
	inline bool operator==(const MusicScoreMoment other) const {
		return inSameFragment(other) && beatAndBar == other.beatAndBar;
	};

	/**
	 * Returns whether or not this object marks an earlier time than the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks an earlier time than the input; false otherwise.
	 */
	inline bool operator<(const MusicScoreMoment other) const {
		return inSameFragment(other) && beatAndBar < other.beatAndBar;
	};

	/**
	 * Returns whether or not this object marks a later time than the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks a later time than the input; false otherwise.
	 */
	inline bool operator>(const MusicScoreMoment other) const {
		return inSameFragment(other) && beatAndBar > other.beatAndBar;
	};

	/**
	* Returns whether or not this object marks an earlier time than or same time as the other.
	* @param other The time to compare to this one.
	* @return True if this object marks an earlier time than or same time as the input; false otherwise.
	*/
	inline bool operator<=(const MusicScoreMoment other) const {
		return ((*this) < other || (*this) == other);
	};

	/**
	* Returns whether or not this object marks a later time than or the same time as the other.
	* @param other The time to compare to this one.
	* @return True if this object marks a later time than or same time as the input; false otherwise.
	*/
	inline bool operator>=(const MusicScoreMoment other) const {
		return ((*this) > other || (*this) == other);
	};

	/**
	 * Returns whether or not this object is not the same time as another.
	 * @param other The time to compare to this one.
	 * @return True if this object does not mark the same time as the other; false otherwise.
	 */
	inline bool operator!=(const MusicScoreMoment other) const {
		return !((*this) == other);
	};
};

/**
 * A data structure designed to hold "events" that happen relative to timing in the music. This includes things
 * like tempo changes, time signature changes, etc.
 * @param EventType The type of event that is occurring with the music.
 */
template <typename EventType>
class CollectionOfMusicScoreEvents {
public:
	/**
	 * Constructor.
	 * @param defaultEvent If an event is requested at a particular time, and either no events have been registered or
	 *   no events have occurred before that time, this event will be returned by default.
	 */
	CollectionOfMusicScoreEvents(EventType defaultEvent);

	/**
	 * Makes this collection into a copy of the given one.
	 * @param other The collection to copy from.
	 */
	void copyContentFrom(const CollectionOfMusicScoreEvents<EventType>* other);

	/**
	 * Returns the number of recorded events.
	 * @param fragment The fragment with which the events are associated.
	 * @return The number of recorded events associated with the given fragment.
	 */
	int getNumEvents(const MusicScoreFragment* fragment) const;

	/**
	 * Returns a particular event, as well as the time at which it occurs.
	 * @param fragment The fragment whose events will be searched.
	 * @param index The index of the target event (the first event has an index of 0).
	 * @return The event having the given index, and the time at which it occurs.
	 */
	std::pair<MusicScoreMoment, EventType> getEvent(const MusicScoreFragment* fragment, int index) const;

	/**
	 * Removes a particular event.
	 * @param fragment The fragment with which the event is associated.
	 * @param index The index of the target event (the first event has an index of 0).
	 */
	void removeEvent(const MusicScoreFragment* fragment, int index);

	/**
	 * Clears all recorded events.
	 */
	void clearEvents();

	/**
	 * Clears all events associated with a particular fragment.
	 * @param fragment The fragment whose events should be cleared.
	 */
	void clearEvents(const MusicScoreFragment* fragment);

	/**
	 * Returns a list of all fragments that have events associated with them in this collection.
	 * @return A vector containing all fragments with events.
	 */
	std::vector<const MusicScoreFragment*> getAllFragmentsWithEvents() const;

	/**
	 * Returns the default event used by this collection.
	 * @return The default event.
	 */
	const EventType getDefaultEvent() const;


	/**
	 * Resets the default event to the specified one.
	 * @param newDefault The new default event.
	 */
	void resetDefaultEvent(EventType newDefault);

protected:
	/**
	 * Acts as a comparator for entries in the event list. It is used to make sure that the event
	 * list stays sorted at all times.
	 */
	struct EventSorter {
		/**
		 * Compares one event to another, indicating which should come first in sorted order.
		 * @param first The first event to check.
		 * @param second The second event to check.
		 * @return True if the first event should come before the second in sorted order; false otherwise.
		 */
		bool operator() (std::pair<MusicScoreMoment, EventType> first, std::pair<MusicScoreMoment, EventType> second) { return first.first < second.first; };
	};

	/**
	 * Returns the event that is assumed to be active by default when no other events are active.
	 * @return The event that is assumed to be active by default when no other events are active.
	 */
	std::pair<MusicScoreMoment, EventType> getDefaultEventWithTime() const;

	/**
	 * Returns the most recent event that occurred before a given time.
	 * @param time The target time.
	 * @return The event that occurred most recently before the given time, as well as the time at which it occurred.
	 */
	std::pair<MusicScoreMoment, EventType> getMostRecentEvent(MusicScoreMoment time) const;

	/**
	 * Returns the index of the most recent event that occurred before a given time.
	 * @param time The target time.
	 * @return The index of the event that occurred most recently before the given time.
	 */
	int getMostRecentEventIndex(MusicScoreMoment time) const;

	/**
	 * Returns a counter which reflects the number of changes that have been make to the MeasureEventData structure since its creation.
	 * @return The mod counter for the MeasureEventData structure.
	 */
	int getModCount() const;

	/**
	 * Adds an event.
	 * @param eventTime The time at which the event occurs.
	 * @param eventObj The event to add.
	 */
	void addEvent(MusicScoreMoment eventTime, EventType eventObj);

private:
	/**
	 * A counter that changes every time a modification is made to the EventCollection.
	 */
	int mModCount;

	/**
	 * For each fragment, a sorted collection of all of the events in that fragment.
	 */
	std::unordered_map<const MusicScoreFragment*, std::vector<std::pair<MusicScoreMoment, EventType>>> mEvents;

	/**
	 * When no other events are active, this one is assumed to be active by default.
	 */
	EventType mDefaultEvent;

	friend class MusicScoreEventBrowser<EventType>;
};

/**
 * Browses the events of a MeasureEventCollection structure, beat by beat. This is very useful for detecting changes in the music as you animate through
 * a show.
 */
template <typename EventType>
class MusicScoreEventBrowser {
public:
	/**
	 * Constructor.
	 * @param source The structure whose events are being browsed.
	 * @param startTime The time to start browsing from.
	 */
	MusicScoreEventBrowser(const CollectionOfMusicScoreEvents<EventType>* source, MusicScoreMoment startTime);

	/**
	 * If the browser has become invalid, it can be made valid again through this function.
	 * @param startTime The time at which the browser should begin after resetting.
	 */
	virtual void reset(MusicScoreMoment startTime);

	/**
	 * Returns whether or not the browser is still valid. It becomes invalid when the source (a MeasureEventCollection) is changed while the Browser is seeking through it.
	 * @return True if the browser is valid; false otherwise.
	 */
	virtual bool isValid() const;

	/**
	 * Advances to the next beat.
	 * @param amount The number of beats to advance by.
	 */
	virtual void nextBeat(unsigned amount = 1);

	/**
	 * Jumps back a beat.
	 * @param amount The number of beats to jump back.
	 */
	virtual void previousBeat(unsigned amount = 1);

	/**
	 * Returns the current time that the browser is marking.
	 * @return The current time marked by the browser.
	 */
	virtual MusicScoreMoment getCurrentTime() const = 0;

	/**
	 * Returns the event that occurs most recently before the time marked by the browser.
	 * @return The event that occurs most recently before the time marked by the browser.
	 */
	EventType getMostRecentEvent() const;

	/**
	 * Returns the timing of the event that occurs most recently before the time marked by the browser.
	 * @return The timing of the event that occurs most recently before the time marked by the browser.
	 */
	MusicScoreMoment getMostRecentEventTime() const;
protected:
	/**
	 * Advances to the next beat, without worrying about changes in the active event.
	 */
	virtual void pushForwardTime() = 0;

	/**
	 * Checks if the browser's current event properly corresponds with the time that it is tracking.
	 * @return True if the browser's current time corresponds with the next event; false otherwise.
	 */
	inline bool checkTransitionForward();

	/**
	 * Pushes to the next beat, without worrying about changes to the active event.
	 */
	virtual void pushBackTime() = 0;

	/**
	 * Causes the browser to track the next event.
	 */
	virtual void executeTransitionForward();

	/**
	* If the browser should be tracking an event that occurred after the one it is currently tracking, this will cause the browser
	* to track that event.
	*/
	inline void transitionForwardUntilFinished();

	/**
	 * Checks if the browser's current event properly corresponds with the time that it is tracking.
	 * @return True if the browser's current time corresponds with the previous event; false otherwise.
	 */
	inline bool checkTransitionBack();
	
	/**
	 * Causes the browser to track the previous event.
	 */
	virtual void executeTransitionBack();

	/**
	 * If the browser should be tracking an event that occurred earlier than the one it is currently tracking, this will cause the browser
	 * to track that event.
	 */
	inline void transitionBackUntilFinished();

	/**
	 * Make sure that the browser tracks the correct event.
	 */
	inline void fixCurrentEvent();

	/**
	 * Sets the current time for the browser.
	 * @param newTime The new current time for the browser.
	 */
	virtual void setCurrentTime(MusicScoreMoment newTime) = 0;
protected:
	/**
	 * The source data whose events are being browsed.
	 */
	const CollectionOfMusicScoreEvents<EventType>* mData;
private:
	/**
	 * The mod count for the source MeasureEventData structure when the browser was created. If the mod count changes from the original, we know that the structure has changed,
	 * and thus know if the MeasureEventDataBrowser has become invalid.
	 */
	int mOriginalModCount;

	/**
	 * The index of the event associated with the current time.
	 */
	int mCurrentEventIndex;
};





template <typename EventType>
CollectionOfMusicScoreEvents<EventType>::CollectionOfMusicScoreEvents(EventType defaultEvent)
: mDefaultEvent(defaultEvent), mModCount(0)
{}

template <typename EventType>
void CollectionOfMusicScoreEvents<EventType>::copyContentFrom(const CollectionOfMusicScoreEvents<EventType>* other) {
	mModCount++;
	mEvents = other->mEvents;
	mDefaultEvent = other->mDefaultEvent;
}

template <typename EventType>
void CollectionOfMusicScoreEvents<EventType>::addEvent(MusicScoreMoment eventTime, EventType eventObj) {
	const MusicScoreFragment* eventFragment = eventTime.fragment.get();
	if (getNumEvents(eventFragment) == 0) {
		mEvents.emplace(eventFragment, std::vector<std::pair<MusicScoreMoment, EventType>>());
	}
	mEvents[eventFragment].push_back(std::pair<MusicScoreMoment, EventType>(eventTime, eventObj));
	std::sort(mEvents[eventFragment].begin(), mEvents[eventFragment].end(), EventSorter());
	mModCount++;
}

template <typename EventType>
std::pair<MusicScoreMoment, EventType> CollectionOfMusicScoreEvents<EventType>::getMostRecentEvent(MusicScoreMoment time) const {
	return getEvent(time.fragment.get(), getMostRecentEventIndex(time));
}

template <typename EventType>
int CollectionOfMusicScoreEvents<EventType>::getMostRecentEventIndex(MusicScoreMoment time) const {
	if (getNumEvents(time.fragment.get()) == 0) {
		return -1;
	}
	const std::vector<std::pair<MusicScoreMoment, EventType>>& fragmentEvents = mEvents.at(time.fragment.get());
	for (unsigned index = 0; index < fragmentEvents.size(); index++) {
		if (fragmentEvents.at(index).first > time) {
			if (index > 0) {
				return index - 1;
			}
			else {
				return -1;
			}
		}
	}
	return fragmentEvents.size() - 1;
}

template <typename EventType>
int CollectionOfMusicScoreEvents<EventType>::getNumEvents(const MusicScoreFragment* fragment) const {
	if (mEvents.find(fragment) == mEvents.end()) {
		return 0;
	}
	return mEvents.at(fragment).size();
}

template <typename EventType>
std::pair<MusicScoreMoment, EventType> CollectionOfMusicScoreEvents<EventType>::getEvent(const MusicScoreFragment* fragment, int index) const {
	if (index < 0 || index >= getNumEvents(fragment)) {
		return getDefaultEventWithTime();
	}
	return mEvents.at(fragment)[index];
}

template <typename EventType>
std::pair<MusicScoreMoment, EventType> CollectionOfMusicScoreEvents<EventType>::getDefaultEventWithTime() const {
	MusicScoreMoment time(nullptr, -1, -1);
	return std::pair<MusicScoreMoment, EventType>(time, getDefaultEvent());
}

template <typename EventType>
std::vector<const MusicScoreFragment*> CollectionOfMusicScoreEvents<EventType>::getAllFragmentsWithEvents() const {
	std::vector<const MusicScoreFragment*> returnVal;
	for (auto iterator = mEvents.cbegin(); iterator != mEvents.cend(); iterator++) {
		returnVal.push_back((*iterator).first);
	}
	return returnVal;
}

template <typename EventType>
void CollectionOfMusicScoreEvents<EventType>::removeEvent(const MusicScoreFragment* fragment, int index) {
	if (index < 0 || index >= getNumEvents(fragment)) {
		return;
	}
	mEvents[fragment].erase(mEvents[fragment].begin() + index);
	mModCount++;
	if (getNumEvents(fragment) == 0) {
		mEvents.erase(fragment);
	}
}

template <typename EventType>
void CollectionOfMusicScoreEvents<EventType>::clearEvents() {
	mEvents.clear();
	mModCount++;
}

template <typename EventType>
void CollectionOfMusicScoreEvents<EventType>::clearEvents(const MusicScoreFragment* fragment) {
	mEvents[fragment].clear();
	mEvents.erase(fragment);
	mModCount++;
}

template <typename EventType>
const EventType CollectionOfMusicScoreEvents<EventType>::getDefaultEvent() const {
	return mDefaultEvent;
}

template <typename EventType>
void CollectionOfMusicScoreEvents<EventType>::resetDefaultEvent(EventType newDefault) {
	mDefaultEvent = newDefault;
}

template <typename EventType>
int CollectionOfMusicScoreEvents<EventType>::getModCount() const {
	return mModCount;
}

template <typename EventType>
MusicScoreEventBrowser<EventType>::MusicScoreEventBrowser(const CollectionOfMusicScoreEvents<EventType>* source, MusicScoreMoment startTime)
: mData(source)
{
	reset(startTime);
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::reset(MusicScoreMoment startTime)
{
	mOriginalModCount = mData->getModCount();
	setCurrentTime(startTime);
	fixCurrentEvent();
}

template <typename EventType>
bool MusicScoreEventBrowser<EventType>::isValid() const {
	return mData->getModCount() == mOriginalModCount;
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::nextBeat(unsigned amount) {
	if (isValid()) {
		for (unsigned pushed = 0; pushed < amount; pushed++) {
			pushForwardTime();
		}
		fixCurrentEvent();
	}
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::executeTransitionForward() {
	mCurrentEventIndex++;
}

template <typename EventType>
bool MusicScoreEventBrowser<EventType>::checkTransitionForward() {
	MusicScoreMoment currentTime = getCurrentTime();
	return (mCurrentEventIndex < mData->getNumEvents(currentTime.fragment.get()) - 1) && mData->getEvent(getCurrentTime().fragment.get(), mCurrentEventIndex + 1).first <= currentTime;
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::transitionForwardUntilFinished() {
	while (checkTransitionForward()) {
		executeTransitionForward();
	}
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::previousBeat(unsigned amount) {
	if (isValid()) {
		for (unsigned pushed = 0; pushed < amount; pushed++) {
			pushBackTime();
		}
		fixCurrentEvent();
	}
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::executeTransitionBack() {
	mCurrentEventIndex--;
}

template <typename EventType>
bool MusicScoreEventBrowser<EventType>::checkTransitionBack() {
	MusicScoreMoment currentTime = getCurrentTime();
	return (mCurrentEventIndex > 0) && mData->getEvent(getCurrentTime().fragment.get(), mCurrentEventIndex).first > getCurrentTime();
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::transitionBackUntilFinished() {
	while (checkTransitionBack()) {
		executeTransitionBack();
	}
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::fixCurrentEvent() {
	if (mCurrentEventIndex < 0 || mCurrentEventIndex >= mData->getNumEvents(getCurrentTime().fragment.get())) {
		mCurrentEventIndex = mData->getMostRecentEventIndex(getCurrentTime());
	} else {
		transitionForwardUntilFinished();
		transitionBackUntilFinished();
	}
}

template <typename EventType>
EventType MusicScoreEventBrowser<EventType>::getMostRecentEvent() const {
	if (isValid()) {
		return mData->getEvent(getCurrentTime().fragment.get(), mCurrentEventIndex).second;
	} else {
		return mData->getDefaultEvent();
	}
}

template <typename EventType>
MusicScoreMoment MusicScoreEventBrowser<EventType>::getMostRecentEventTime() const {
	if (isValid()) {
		return mData->getEvent(getCurrentTime().fragment.get(), mCurrentEventIndex).first;
	}
	else {
		return getCurrentTime();
	}
}
