#ifndef _MEASURE_EVENT_DATA_H_
#define _MEASURE_EVENT_DATA_H_

#include <vector>
#include <algorithm>

template <typename MeasureEventType, typename EventData> class MeasureEventDataBrowser;

/**
 * A MeasureTime that falls on the beginning of a bar (it has not beat).
 * This is loosly considered to be a type of MeasureTime, though MeasureTime is technically a derivative of this class.
 * MeasureTime objects are usually used as template parameters, though, so both BeatlessMeasureTime and MeasureTime objects are
 * treated in essentially the same way.
 */
struct BeatlessMeasureTime {
	/**
	 * Constructor.
	 * @param bar The bar marked by this time. (Bar 0 is the first bar of the show).
	 */
	BeatlessMeasureTime(int bar) : bar(bar) {};

	/**
	 * The bar marked by this MeasureTime.
	 * Bar 0 marks the first bar in the show.
	 */
	int bar;
};

/**
 * A MeasureTime with both a beat and a bar.
 */
struct MeasureTime : BeatlessMeasureTime {
	using super = BeatlessMeasureTime;

	/**
	 * Constructor.
	 * @param bar The bar of the time that this object marks. Bar 0 is the first bar in the show.
	 * @param beat The beat of the time that this object marks (relative to the start of the bar that it marks). Beat 0 is the first beat in the bar.
	 */
	MeasureTime(int bar, int beat) : super(bar), beat(beat) {};

	int beat;
};

/**
 * A data structure designed to hold events that happen relative to timing in the music. That is, it essentially holds
 * data whose timing can be described using MeasureTime objects. 
 * @param MeasureEventType This is the type of event that will be stored in the structure. Usually, this will be a derivative of the MeasureTime struct.
 * In any case, from this object type, you must be able to generate an associated MeasureTime object identifying the timing of the event, and an associated EventData object (see second template parameter)
 * to represents the event itself.
 * @param EventData This is a representation of the event being put into the structure. 
 */
template <typename MeasureEventType, typename EventData>
class MeasureEventData {
public:
	/**
	 * Adds an event.
	 * @param eventObj The event to add.
	 */
	void addEvent(MeasureEventType eventObj);

	/**
	 * Returns the number of recorded events.
	 * @return The number of recorded events.
	 */
	int getNumEvents() const;
	/**
	 * Returns a particular event.
	 * @param index The index of the target event (the first event has an index of 0).
	 * @return The event having the given index.
	 */
	MeasureEventType getEvent(int index) const;
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
	 * This struct essentially acts as a comparator for MeasureEventType objects. This comparator is used to sort
	 * the order in which the MeasureEventType objects are recorded. "Events" that come earlier in sorted order will
	 * have smaller indices than those that come later.
	 */
	struct Sorter {
		/**
		 * Constructor.
		 * @param sortingObject The MeasureEventData whose items are being sorted.
		 */
		Sorter(const MeasureEventData* sortingObject) : mSorting(sortingObject) {};
		/**
		 * Compares one event to another, indicating which should come first in sorted order.
		 * @param first The first event to check.
		 * @param second The second event to check.
		 * @return True if the first event should come before the second in sorted order; false otherwise.
		 */
		bool operator() (MeasureEventType first, MeasureEventType second) { return mSorting->comesFirst(first, second); };
		
		/**
		 * The MeasureEventData object whose items are being sorted.
		 */
		const MeasureEventData* mSorting;
	};

	/**
	 * The base class for a "seeker" object. The seeker is an object used when browsing through the events of a MeasureEventData structure (in sorted order) to
	 * find an event that best matches some set of criteria.
	 */
	class SeekerBase {
	public:
		/**
		 * Constructor.
		 * @param seekingObject The MeasureEventData whose events are being browsed.
		 */
		SeekerBase(const MeasureEventData* seekingObject) : mSeeking(seekingObject) {};

		/**
		 * Tests an object from the MeasureEventData structure, and returns whether or not the seeker has found the object it is looking for.
		 * @param seekObj The event to test.
		 * @return True if the seeker has found the object that it is looking for; false otherwise.
		 */
		virtual bool test(MeasureEventType seekObj) = 0;
		/**
		 * Returns the event that best matched the criteria of this seeker.
		 * @return The event that best matched the criteria for this seeker.
		 */
		virtual MeasureEventType getPreferredEntry() = 0;
	protected:
		/**
		 * Extracts the event data from an event.
		 * @param eventObj The event to extract event data from.
		 * @return The event data for the given event.
		 */
		EventData extractEventData(MeasureEventType eventObj) { return mSeeking->extractEventData(eventObj); };
		/**
		 * Extracts the timing of an event object.
		 * @param eventObj The event to extract timing for.
		 * @return The timing of the provided event object.
		 */
		MeasureTime convertToMeasureTime(MeasureEventType eventObj) { return mSeeking->convertToMeasureTime(eventObj); };
		/**
		 * Checks whether or not a particular time comes before another chronologically.
		 * @param event1 The first time.
		 * @param event2 The second time.
		 * @return True if the first time comes before another chronologically.
		 */
		bool comesFirst(MeasureTime event1, MeasureTime event2) { return mSeeking->comesFirst(event1, event2); };
		/**
		 * Checks whether or not an event comes before another in sorted order.
		 * @param event1 The first event.
		 * @param event2 The second event.
		 * @return True if the first event comes before the other in sorted oreder.
		 */
		bool comesFirst(MeasureEventType event1, MeasureEventType event2) { return mSeeking->comesFirst(event1, event2); }
	private:
		/**
		 * The MeasureEventData structure whose events are being browsed.
		 */
		const MeasureEventData* mSeeking;
	};

	/**
	 * A type of seeker which stops looking when it finds the first event that does NOT fit its criteria. Since it stops AFTER seeing
	 * the first event that does not fit its criteria, it prefers the last event which DID fit its criteria.
	 * That is, the seeker prefers the last event that it tested (the most recent event to pass its test).
	 */
	class PreviousPreferringSeeker : public SeekerBase {
	private:
		using super = SeekerBase;
	public:
		/**
		 * Constructor.
		 * @param seekingObject The MeasureEventData structure whose events are being browsed.
		 */
		PreviousPreferringSeeker(const MeasureEventData* seekingObject) : super(seekingObject), mPrevious(seekingObject->getEvent(0)) {};

		virtual bool test(MeasureEventType seekObj) {
			if (finishedSeeking(seekObj)) {
				return true;
			}
			mPrevious = seekObj;
			return false;
		}

		virtual MeasureEventType getPreferredEntry() {
			return getPrevious();
		}
	protected:
		/**
		 * Returns true if the given event is the first to fail the seeker's criteria. 
		 * @param seekObj The event to check.
		 * @return True if the given event fails the seeker's search criteria; false otherwise.
		 */
		virtual bool finishedSeeking(MeasureEventType seekObj) = 0;

		/**
		 * Returns the last event to pass the seeker's test.
		 * @return The last event to pass the seeker's test.
		 */
		virtual MeasureEventType getPrevious() {
			return mPrevious;
		}
	private:
		/**
		 * The last event to pass the seeker's test.
		 */
		MeasureEventType mPrevious;
	};

	/**
	 * A seeker which finds the latest event occuring before or at a particular bar and beat.
	 */
	class BeatSeeker : public PreviousPreferringSeeker {
	private:
		using super = PreviousPreferringSeeker;
	public:
		/**
		 * Constructor.
		 * @param seekingObject The MeasureEventData object whose events are being browsed by this seeker.
		 * @param bar The target bar.
		 * @param beat The target beat.
		 */
		BeatSeeker(const MeasureEventData* seekingObject, int bar, int beat) : super(seekingObject), mSeekDestination(bar, beat) {};

		/**
		 * Returns the target bar.
		 * @return The target bar.
		 */
		int getSeekBeat() { return mSeekDestination.beat; };
		/**
		 * Returns the target beat.
		 * @return The target beat.
		 */
		int getSeekBar() { return mSeekDestination.bar; };
	protected:
		virtual bool finishedSeeking(MeasureEventType seekObj) { return comesFirst(mSeekDestination, convertToMeasureTime(seekObj)); };
	private:
		/**
		 * The target bar and beat.
		 */
		MeasureTime mSeekDestination;
	};

	/**
	 * A seeker which does nothing.
	 */
	class DeadSeeker : PreviousPreferringSeeker {
	private:
		using super = PreviousPreferringSeeker;
	public:
		/**
		 * Constructor.
		 * @param seekingObject The MeasureEventData object whose events are browsed using this seeker.
		 */
		DeadSeeker(const MeasureEventData* seekingObject) : super(seekingObject) {};
	protected:
		virtual bool finishedSeeking(MeasureEventType seekObj) { return true; };
	};

	/**
	 * Returns the event data for the event that is active at the given bar and beat.
	 * @param bar The target bar.
	 * @param beat The target beat.
	 */
	EventData getEvent(int bar, int beat) const;
	/**
	 * Returns the event that is active at the given bar and beat.
	 * @param bar The target bar.
	 * @param beat The target beat.
	 */
	MeasureEventType getEncompassingEventMarker(int bar, int beat) const;

	/**
	 * Uses the given seeker to find an event, and then deletes the seeker when finished.
	 * @param seeker The seeker to use to find an event.
	 * @return The event found by the seeker.
	 */
	MeasureEventType seekAndDeleteSeeker(SeekerBase* seeker) const;
	/**
	 * Uses the given seeker to find an event.
	 * @param seeker The seeker to use to find an event.
	 * @return The event found by the seeker.
	 */
	MeasureEventType seek(SeekerBase* seeker) const;

	/**
	 * Extracts the data from an event.
	 * @param eventObj The event whose data will be extracted.
	 * @return The data for the given event.
	 */
	virtual EventData extractEventData(MeasureEventType eventObj) const = 0;
	/**
	 * Gets the timing for a particular event.
	 * @param eventObj The event to get the timing for.
	 * @return The timing for the given event.
	 */
	virtual MeasureTime convertToMeasureTime(MeasureEventType eventObj) const = 0;

	/**
	 * Returns the counter which reflects the number of changes that have been make to the MeasureEventData structure since its creation.
	 * @return The mod counter for the MeasureEventData structure.
	 */
	int getModCount() const;
private:
	/**
	 * A counter to identify the number of modifications that have been made to this object since its creation.
	 * This will not necessarily be the same as the number of modifications that have been made - what is important is simply that when modifications are made,
	 * this counter changes.
	 */
	int mModCount;

	/**
	 * Returns whether or not the first time occurs before the second when in sorted order.
	 * @param event1 The first time.
	 * @param event2 The second time.
	 * @return True if the first time comes before the second in sorted order; false otherwise.
	 */
	bool comesFirst(MeasureTime event1, MeasureTime event2) const;
	/**
	 * Returns whether or not the first event occurs before the second when in sorted order.
	 * @param event1 The first event.
	 * @param event2 The second event.
	 * @return True if the first event comes before the second in sorted order; false otherwise.
	 */
	bool comesFirst(MeasureEventType event1, MeasureEventType event2) const;
	/**
	 * Returns whether or not the first time occurs before or at the same spot as the second when in sorted order.
	 * @param event1 The first time.
	 * @param event2 The second time.
	 * @return True if the first time comes before or at the same time as the second in sorted order; false otherwise.
	 */
	bool comesFirstOrSimultaneously(MeasureTime event1, MeasureTime event2) const;
	/**
	 * Returns whether or not the first event occurs before or at the same spot as the second when in sorted order.
	 * @param event1 The first event.
	 * @param event2 The second event.
	 * @return True if the first event comes before or at the same time as the second in sorted order; false otherwise.
	 */
	bool comesFirstOrSimultaneously(MeasureEventType event1, MeasureEventType event2) const;

	/**
	 * A sorted collection of all of the events in this structure.
	 */
	std::vector<MeasureEventType> mEvents;

	//friend class SeekerBase;
	//friend struct Sorter;
	friend class MeasureEventDataBrowser<MeasureEventType, EventData>;
};

/**
 * Browses the events of a MeasureEventData structure, beat by beat. This is very effective for detecting changes to the music as you animate through
 * a show.
 */
template <typename MeasureEventType, typename EventData>
class MeasureEventDataBrowser {
public:
	/**
	 * Constructor.
	 * @param source The structure whose events are being browsed.
	 * @param startTime The time to start browsing from.
	 * @param startEventIndex The index of the event associated with the start time.
	 */
	MeasureEventDataBrowser(const MeasureEventData<MeasureEventType, EventData>* source, MeasureTime startTime, int startEventIndex);

	/**
	 * Returns whether or not the browser is still valid. It becomes invalid when the source MeasureEventData is changed while the Browser seeks through it.
	 * @return True if the browser is valid; false otherwise.
	 */
	virtual bool isValid() const;

	/**
	 * Advances to the next beat.
	 */
	void nextBeat();
	/**
	 * Jumps back a beat.
	 */
	void previousBeat();

	/**
	 * Returns the current time that the browser is marking.
	 * @return The current time marked by the browser.
	 */
	MeasureTime getCurrentTime() const;
	/**
	 * Returns the event which is active during the current time.
	 * @return The event which is active at the time marked by the browser.
	 */
	MeasureEventType getEncompassingEventMarker() const;
protected:
	/**
	 * Returns the event that is active at the browser's current time.
	 * @return The event that is currently active.
	 */
	EventData getCurrentEvent() const;
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
	void setCurrentTime(MeasureTime newTime);
private:
	/**
	 * The mod count for the source MeasureEventData structure when the browser was created. If the mod count changes from the original, we know that the structure has changed,
	 * and thus know if the MeasureEventDataBrowser has become invalid.
	 */
	int mOriginalModCount;

	/**
	 * The current time.
	 */
	MeasureTime mCurrentTime;

	/**
	 * The index of the event associated with the current time.
	 */
	int mCurrentEventIndex;

	/**
	 * The source data whose events are being browsed.
	 */
	const MeasureEventData<MeasureEventType, EventData>* mData;
};








template <typename MeasureEventType, typename EventData>
void MeasureEventData<MeasureEventType, EventData>::addEvent(MeasureEventType event) {
	mEvents.push_back(event);
	std::sort(mEvents.begin(), mEvents.end(), Sorter(this));
	mModCount++;
}

template <typename MeasureEventType, typename EventData>
EventData MeasureEventData<MeasureEventType, EventData>::getEvent(int bar, int beat) const {
	return extractEventData(getEncompassingEventMarker(bar, beat));
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventData<MeasureEventType, EventData>::getEncompassingEventMarker(int bar, int beat) const {
	return seekAndDeleteSeeker(new BeatSeeker(this, bar, beat));
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventData<MeasureEventType, EventData>::seekAndDeleteSeeker(SeekerBase* seeker) const {
	MeasureEventType returnVal = seek(seeker);
	delete seeker;
	return returnVal;
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventData<MeasureEventType, EventData>::seek(SeekerBase* seeker) const {
	for (std::vector<MeasureEventType>::const_iterator iteratorObj = mEvents.begin(); iteratorObj != mEvents.end(); iteratorObj++) {
		if (seeker->test(*iteratorObj)) {
			break;
		}
	}
	return seeker->getPreferredEntry();
}

template <typename MeasureEventType, typename EventData>
int MeasureEventData<MeasureEventType, EventData>::getNumEvents() const {
	return mEvents.size();
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventData<MeasureEventType, EventData>::getEvent(int index) const {
	return mEvents[index];
}

template <typename MeasureEventType, typename EventData>
void MeasureEventData<MeasureEventType, EventData>::removeEvent(int index) {
	mEvents.erase(mEvents.begin() + index);
	mModCount++;
}

template <typename MeasureEventType, typename EventData>
void MeasureEventData<MeasureEventType, EventData>::clearEvents() {
	mEvents.clear();
	mModCount++;
}

template <typename MeasureEventType, typename EventData>
int MeasureEventData<MeasureEventType, EventData>::getModCount() const {
	return mModCount;
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventData<MeasureEventType, EventData>::comesFirst(MeasureTime event1, MeasureTime event2) const {
	if (event1.bar == event2.bar) {
		return event1.beat < event2.beat;
	}
	return event1.bar < event2.bar;
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventData<MeasureEventType, EventData>::comesFirst(MeasureEventType event1, MeasureEventType event2) const {
	return comesFirst(convertToMeasureTime(event1), convertToMeasureTime(event2));
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventData<MeasureEventType, EventData>::comesFirstOrSimultaneously(MeasureTime event1, MeasureTime event2) const {
	return (comesFirst(event1, event2) || (event1.bar == event2.bar && event1.beat == event2.beat));
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventData<MeasureEventType, EventData>::comesFirstOrSimultaneously(MeasureEventType event1, MeasureEventType event2) const {
	return comesFirstOrSimultaneously(convertToMeasureTime(event1), convertToMeasureTime(event2));
}

template <typename MeasureEventType, typename EventData>
MeasureEventDataBrowser<MeasureEventType, EventData>::MeasureEventDataBrowser(const MeasureEventData<MeasureEventType, EventData>* source, MeasureTime startTime, int startEventIndex)
: mData(source), mOriginalModCount(source->getModCount()), mCurrentTime(startTime), mCurrentEventIndex(startEventIndex)
{
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventDataBrowser<MeasureEventType, EventData>::isValid() const {
	return mData->getModCount() == mOriginalModCount && mData->getNumEvents() > 0;
}

template <typename MeasureEventType, typename EventData>
void MeasureEventDataBrowser<MeasureEventType, EventData>::nextBeat() {
	pushForwardTime();
	while (checkTransitionForward());
}

template <typename MeasureEventType, typename EventData>
void MeasureEventDataBrowser<MeasureEventType, EventData>::previousBeat() {
	pushBackTime();
	while (checkTransitionBack());
}

template <typename MeasureEventType, typename EventData>
MeasureTime MeasureEventDataBrowser<MeasureEventType, EventData>::getCurrentTime() const {
	return mCurrentTime;
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventDataBrowser<MeasureEventType, EventData>::getEncompassingEventMarker() const {
	return mData->getEvent(mCurrentEventIndex);
}

template <typename MeasureEventType, typename EventData>
EventData MeasureEventDataBrowser<MeasureEventType, EventData>::getCurrentEvent() const {
	return mData->extractEventData(getEncompassingEventMarker());
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventDataBrowser<MeasureEventType, EventData>::checkTransitionBack() {
	if (mCurrentEventIndex > 0 && mData->comesFirst(getCurrentTime(), mData->convertToMeasureTime(getEncompassingEventMarker()))) {
		mCurrentEventIndex -= 1;
		return true;
	}
	return false;
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventDataBrowser<MeasureEventType, EventData>::checkTransitionForward() {
	if (mCurrentEventIndex < mData->getNumEvents() - 1 && mData->comesFirstOrSimultaneously(mData->convertToMeasureTime(mData->getEvent(mCurrentEventIndex + 1)), getCurrentTime())) {
		mCurrentEventIndex += 1;
		return true;
	}
	return false;
}

template <typename MeasureEventType, typename EventData>
void MeasureEventDataBrowser<MeasureEventType, EventData>::setCurrentTime(MeasureTime newTime) {
	mCurrentTime = newTime;
}

#endif