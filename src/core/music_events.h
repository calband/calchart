#ifndef _MUSIC_EVENTS_H_
#define _MUSIC_EVENTS_H_

#include <vector>
#include <algorithm>

template <typename EventType> class MusicScoreEventBrowser;


typedef unsigned BarNumber;
typedef unsigned BeatNumber;
typedef unsigned FragmentId;

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
	MusicScoreMoment(FragmentId fragment, BarNumber bar, BeatNumber beat) : fragment(fragment), bar(bar), beat(beat) {};

	FragmentId fragment;
	BarNumber bar;
	BeatNumber beat;

	/**
	 * Returns whether or not this object marks the same time as the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks the same time as the input; false otherwise.
	 */
	inline bool operator==(MusicScoreMoment other) {
		return (fragment == other.fragment
			&& bar == other.bar
			&& beat == other.beat);
	};

	/**
	 * Returns whether or not this object marks an earlier time than the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks an earlier time than the input; false otherwise.
	 */
	inline bool operator<(MusicScoreMoment other) {
		if (fragment < other.fragment) {
			return true;
		}
		if (fragment == other.fragment) {
			if (bar < other.bar) {
				return true;
			}
			if (bar == other.bar) {
				return (beat < other.beat);
			}
		}
	};

	/**
	 * Returns whether or not this object marks a later time than the other.
	 * @param other The time to compare to this one.
	 * @return True if this object marks a later time than the input; false otherwise.
	 */
	inline bool operator>(MusicScoreMoment other) {
		if (fragment > other.fragment) {
			return true;
		}
		if (fragment == other.fragment) {
			if (bar > other.bar) {
				return true;
			}
			if (bar == other.bar) {
				return (beat > other.beat);
			}
		}
	};

	/**
	* Returns whether or not this object marks an earlier time than or same time as the other.
	* @param other The time to compare to this one.
	* @return True if this object marks an earlier time than or same time as the input; false otherwise.
	*/
	inline bool operator<=(MusicScoreMoment other) {
		return ((*this) < other || (*this) == other);
	};

	/**
	* Returns whether or not this object marks a later time than or the same time as the other.
	* @param other The time to compare to this one.
	* @return True if this object marks a later time than or same time as the input; false otherwise.
	*/
	inline bool operator>=(MusicScoreMoment other) {
		return ((*this) > other || (*this) == other);
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
	 * Returns the number of recorded events.
	 * @return The number of recorded events.
	 */
	int getNumEvents() const;

	/**
	 * Returns a particular event, as well as the time at which it occurs.
	 * @param index The index of the target event (the first event has an index of 0).
	 * @return The event having the given index, and the time at which it occurs.
	 */
	std::pair<MusicScoreMoment, EventType> getEvent(int index) const;

	/**
	 * Returns the event that is assumed to be active by default when no other events are active.
	 * @return The event that is assumed to be active by default when no other events are active.
	 */
	std::pair<MusicScoreMoment, EventType> getDefaultEvent() const;

	/**
	 * Removes a particular event.
	 * @param index The index of the target event (the first event has an index of 0).
	 */
	void removeEvent(int index);

	/**
	 * Clears all recorded events.
	 */
	void clearEvents();

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
	 * A sorted collection of all of the events in this structure.
	 */
	std::vector<std::pair<MusicScoreMoment, EventType>> mEvents;

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
	MusicScoreEventBrowser(const CollectionOfMusicScoreEvents<EventType, EventData>* source, MusicScoreMoment startTime);

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
	void nextBeat(unsigned amount = 1);

	/**
	 * Jumps back a beat.
	 * @param amount The number of beats to jump back.
	 */
	void previousBeat(unsigned amount = 1);

	/**
	 * Returns the current time that the browser is marking.
	 * @return The current time marked by the browser.
	 */
	MusicScoreMoment getCurrentTime() const;

	/**
	 * Returns the event that occurs most recently before the time marked by the browser.
	 * @return The event that occurs most recently before the time marked by the browser.
	 */
	EventType getMostRecentEvent() const;
protected:
	/**
	 * Advances to the next beat, without worrying about changes in the active event.
	 */
	virtual void pushForwardTime() = 0;

	/**
	 * Checks if the browser's new current time corresponds with the next active event.
	 * @return True if the browser's current time corresponds with the next event; false otherwise.
	 */
	virtual bool checkTransitionForward();
	/**
	 * Pushes to the next beat, without worrying about changes to the active event.
	 */
	virtual void pushBackTime() = 0;
	/**
	 * Checks if the browser's new current time corresponds with the previous active event.
	 * @return True if the browser's current time corresponds with the previous event; false otherwise.
	 */
	virtual bool checkTransitionBack();

	/**
	 * Sets the current time for the browser.
	 * @param newTime The new current time for the browser.
	 */
	void setCurrentTime(MusicScoreMoment newTime);
private:
	/**
	 * The mod count for the source MeasureEventData structure when the browser was created. If the mod count changes from the original, we know that the structure has changed,
	 * and thus know if the MeasureEventDataBrowser has become invalid.
	 */
	int mOriginalModCount;

	/**
	 * The current time.
	 */
	MusicScoreMoment mCurrentTime;

	/**
	 * The index of the event associated with the current time.
	 */
	int mCurrentEventIndex;

	/**
	 * The source data whose events are being browsed.
	 */
	const CollectionOfMusicScoreEvents<EventType>* mData;
};





template <typename EventType>
CollectionOfMusicScoreEvents<EventType>::CollectionOfMusicScoreEvents(EventType defaultEvent)
: mDefaultEvent(defaultEvent), mModCount(0)
{}

template <typename EventType>
void CollectionOfMusicScoreEvents<EventType>::addEvent(MusicScoreMoment eventTime, EventType eventObj) {
	mEvents.push_back(std::pair<MusicScoreMoment, EventType>(eventTime, eventObj));
	std::sort(mEvents.begin(), mEvents.end(), EventSorter());
	mModCount++;
}

template <typename EventType>
std::pair<MusicScoreMoment, EventType> CollectionOfMusicScoreEvents<EventType>::getMostRecentEvent(MusicScoreMoment time) const {
	return getEvent(getMostRecentEventIndex(time));
}

template <typename EventType>
int CollectionOfMusicScoreEvents<EventType>::getMostRecentEventIndex(MusicScoreMoment time) const {
	for (unsigned index = 0; index < mEvents.size(); index++) {
		if (mEvents[index].second > time) {
			if (index > 0) {
				return index - 1;
			}
			else {
				return -1;
			}
		}
	}
	return mEvents.size() - 1;
}

template <typename EventType>
int CollectionOfMusicScoreEvents<EventType>::getNumEvents() const {
	return mEvents.size();
}

template <typename EventType>
std::pair<MusicScoreMoment, EventType> CollectionOfMusicScoreEvents<EventType>::getEvent(int index) const {
	if (index < 0) {
		return getDefaultEvent();
	}
	return mEvents[index];
}

template <typename EventType>
std::pair<MusicScoreMoment, EventType> CollectionOfMusicScoreEvents<EventType>::getDefaultEvent() const {
	MusicScoreMoment time(-1, -1, -1);
	return std::pair<MusicScoreMoment, EventType>(time, mDefaultEvent);
}

template <typename EventType>
void CollectionOfMusicScoreEvents<EventType>::removeEvent(int index) {
	mEvents.erase(mEvents.begin() + index);
	mModCount++;
}

template <typename EventType>
void CollectionOfMusicScoreEvents<EventType>::clearEvents() {
	mEvents.clear();
	mModCount++;
}

template <typename EventType>
int CollectionOfMusicScoreEvents<EventType>::getModCount() const {
	return mModCount;
}

template <typename EventType>
MusicScoreEventBrowser<EventType>::MusicScoreEventBrowser(const CollectionOfMusicScoreEvents<EventType>* source, MusicScoreMoment startTime)
: mData(source), mCurrentTime(startTime)
{
	reset(startTime);
}

template <typename EventType>
MusicScoreEventBrowser<EventType>::reset(MusicScoreMoment startTime)
{
	mOriginalModCount = mData->getModCount();
	setCurrentTime(startTime);
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
		while (checkTransitionForward());
	}
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::previousBeat(unsigned amount) {
	if (isValid()) {
		for (unsigned pushed = 0; pushed < amount; pushed++) {
			pushBackTime();
		}
		while (checkTransitionBack());
	}
}

template <typename EventType>
MusicScoreMoment MusicScoreEventBrowser<EventType>::getCurrentTime() const {
	return mCurrentTime;
}

template <typename EventType>
EventType MusicScoreEventBrowser<EventType>::getMostRecentEvent() const {
	if (isValid()) {
		return mData->getEvent(mCurrentEventIndex).second;
	} else {
		return mData->getDefaultEvent().second;
	}
}

template <typename EventType>
bool MusicScoreEventBrowser<EventType>::checkTransitionBack() {
	if (mCurrentEventIndex > -1 && getCurrentTime() < getMostRecentEvent()) {
		mCurrentEventIndex -= 1;
		return true;
	}
	return false;
}

template <typename EventType>
bool MusicScoreEventBrowser<EventType>::checkTransitionForward() {
	if (mCurrentEventIndex < mData->getNumEvents() - 1 && getCurrentTime() >= mData->getEvent(mCurrentEventIndex + 1).first) {
		mCurrentEventIndex += 1;
		return true;
	}
	return false;
}

template <typename EventType>
void MusicScoreEventBrowser<EventType>::setCurrentTime(MusicScoreMoment newTime) {
	if (isValid()) {
		mCurrentTime = newTime;
		mCurrentEventIndex = mData->getMostRecentEventIndex(startTime);
	}
}

#endif