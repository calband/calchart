#ifndef _MEASURE_EVENT_DATA_H_
#define _MEASURE_EVENT_DATA_H_

#include <vector>
#include <algorithm>

template <typename MeasureEventType, typename EventData> class MeasureEventDataBrowser;

struct BeatlessMeasureTime {
	BeatlessMeasureTime(int bar) : bar(bar) {};

	int bar;
};

struct MeasureTime : BeatlessMeasureTime {
	using super = BeatlessMeasureTime;

	MeasureTime(int bar, int beat) : super(bar), beat(beat) {};

	int beat;
};

template <typename MeasureEventType, typename EventData>
class MeasureEventData {
public:
	void addEvent(MeasureEventType eventObj);

	int getNumEvents();
	MeasureEventType getEvent(int index);
	void removeEvent(int index);
	void clearEvents();
protected:
	struct Sorter {
		Sorter(MeasureEventData* sortingObject) : mSorting(sortingObject) {};
		bool operator() (MeasureEventType first, MeasureEventType second) { return mSorting->comesFirst(first, second); };
		
		MeasureEventData* mSorting;
	};

	class SeekerBase {
	public:
		SeekerBase(MeasureEventData* seekingObject) : mSeeking(seekingObject) {};

		virtual bool test(MeasureEventType seekObj) = 0;
		virtual MeasureEventType getPreferredEntry() = 0;
	protected:
		virtual EventData extractEventData(MeasureEventType eventObj) { return mSeeking->extractEventData(eventObj); };
		virtual MeasureTime convertToMeasureTime(MeasureEventType eventObj) { return mSeeking->convertToMeasureTime(eventObj); };
		bool comesFirst(MeasureTime event1, MeasureTime event2) { return mSeeking->comesFirst(event1, event2); };
		bool comesFirst(MeasureEventType event1, MeasureEventType event2) { return mSeeking->comesFirst(event1, event2); }
	private:
		MeasureEventData* mSeeking;
	};

	class PreviousPreferringSeeker : public SeekerBase {
	private:
		using super = SeekerBase;
	public:
		PreviousPreferringSeeker(MeasureEventData* seekingObject) : super(seekingObject), mPrevious(seekingObject->getEvent(0)) {};

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
		virtual bool finishedSeeking(MeasureEventType seekObj) = 0;

		virtual MeasureEventType getPrevious() {
			return mPrevious;
		}
	private:
		MeasureEventType mPrevious;
	};

	class BeatSeeker : public PreviousPreferringSeeker {
	private:
		using super = PreviousPreferringSeeker;
	public:
		BeatSeeker(MeasureEventData* seekingObject, int bar, int beat) : super(seekingObject), mSeekDestination(bar, beat) {};

		int getSeekBeat() { return mSeekDestination.beat; };
		int getSeekBar() { return mSeekDestination.bar; };
	protected:
		virtual bool finishedSeeking(MeasureEventType seekObj) { return comesFirst(convertToMeasureTime(seekObj), mSeekDestination); };
	private:
		MeasureTime mSeekDestination;
	};

	struct DeadSeeker : PreviousPreferringSeeker {
	private:
		using super = PreviousPreferringSeeker;
	public:
		DeadSeeker(MeasureEventData* seekingObject) : super(seekingObject) {};
	protected:
		virtual bool finishedSeeking(MeasureEventType seekObj) { return true; };
	};

	EventData getEvent(int bar, int beat);
	MeasureEventType getEncompassingEventMarker(int bar, int beat);

	MeasureEventType seekAndDeleteSeeker(SeekerBase* seeker);
	MeasureEventType seek(SeekerBase* seeker);

	virtual EventData extractEventData(MeasureEventType eventObj) = 0;
	virtual MeasureTime convertToMeasureTime(MeasureEventType eventObj) = 0;

	int getModCount();
private:
	int mModCount;

	bool comesFirst(MeasureTime event1, MeasureTime event2);
	bool comesFirst(MeasureEventType event1, MeasureEventType event2);

	std::vector<MeasureEventType> mEvents;

	friend class SeekerBase;
	friend struct Sorter;
	friend class MeasureEventDataBrowser<MeasureEventType, EventData>;
};

template <typename MeasureEventType, typename EventData>
class MeasureEventDataBrowser {
public:
	MeasureEventDataBrowser(MeasureEventData<MeasureEventType, EventData>* source, MeasureTime startTime, int startEventIndex);

	virtual bool isValid();

	void nextBeat();
	void previousBeat();

	MeasureTime getCurrentTime();
	MeasureEventType getEncompassingEventMarker();
protected:
	EventData getCurrentEvent();
	virtual void pushForwardTime() = 0;
	virtual bool checkTransitionForward();
	virtual void pushBackTime() = 0;
	virtual bool checkTransitionBack();
	void setCurrentTime(MeasureTime newTime);
private:
	int mOriginalModCount;

	MeasureTime mCurrentTime;

	int mCurrentEventIndex;

	MeasureEventData<MeasureEventType, EventData>* mData;
};








template <typename MeasureEventType, typename EventData>
void MeasureEventData<MeasureEventType, EventData>::addEvent(MeasureEventType event) {
	mEvents.push_back(event);
	std::sort(mEvents.begin(), mEvents.end(), Sorter(this));
	mModCount++;
}

template <typename MeasureEventType, typename EventData>
EventData MeasureEventData<MeasureEventType, EventData>::getEvent(int bar, int beat) {
	return extractEventData(getEncompassingEventMarker(bar, beat));
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventData<MeasureEventType, EventData>::getEncompassingEventMarker(int bar, int beat) {
	return seekAndDeleteSeeker(new BeatSeeker(this, bar, beat));
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventData<MeasureEventType, EventData>::seekAndDeleteSeeker(SeekerBase* seeker) {
	MeasureEventType returnVal = seek(seeker);
	delete seeker;
	return returnVal;
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventData<MeasureEventType, EventData>::seek(SeekerBase* seeker) {
	for (std::vector<MeasureEventType>::iterator iteratorObj = mEvents.begin(); iteratorObj != mEvents.end(); iteratorObj++) {
		if (seeker->test(*iteratorObj)) {
			break;
		}
	}
	return seeker->getPreferredEntry();
}

template <typename MeasureEventType, typename EventData>
int MeasureEventData<MeasureEventType, EventData>::getNumEvents() {
	return mEvents.size();
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventData<MeasureEventType, EventData>::getEvent(int index) {
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
int MeasureEventData<MeasureEventType, EventData>::getModCount() {
	return mModCount;
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventData<MeasureEventType, EventData>::comesFirst(MeasureTime event1, MeasureTime event2) {
	if (event1.bar == event2.bar) {
		return event1.beat < event2.beat;
	}
	return event1.bar < event2.bar;
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventData<MeasureEventType, EventData>::comesFirst(MeasureEventType event1, MeasureEventType event2) {
	return comesFirst(convertToMeasureTime(event1), convertToMeasureTime(event2));
}

template <typename MeasureEventType, typename EventData>
MeasureEventDataBrowser<MeasureEventType, EventData>::MeasureEventDataBrowser(MeasureEventData<MeasureEventType, EventData>* source, MeasureTime startTime, int startEventIndex)
: mData(source), mOriginalModCount(source->getModCount()), mCurrentTime(startTime), mCurrentEventIndex(startEventIndex)
{
}

template <typename MeasureEventType, typename EventData>
bool MeasureEventDataBrowser<MeasureEventType, EventData>::isValid() {
	return mData->getModCount() == mOriginalModCount;
}

template <typename MeasureEventType, typename EventData>
void MeasureEventDataBrowser<MeasureEventType, EventData>::nextBeat() {
	pushForwardTime();
	checkTransitionForward();
}

template <typename MeasureEventType, typename EventData>
void MeasureEventDataBrowser<MeasureEventType, EventData>::previousBeat() {
	pushBackTime();
	checkTransitionBack();
}

template <typename MeasureEventType, typename EventData>
MeasureTime MeasureEventDataBrowser<MeasureEventType, EventData>::getCurrentTime() {
	return mCurrentTime;
}

template <typename MeasureEventType, typename EventData>
MeasureEventType MeasureEventDataBrowser<MeasureEventType, EventData>::getEncompassingEventMarker() {
	return mData->getEvent(mCurrentEventIndex);
}

template <typename MeasureEventType, typename EventData>
EventData MeasureEventDataBrowser<MeasureEventType, EventData>::getCurrentEvent() {
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
	if (mCurrentEventIndex < mData->getNumEvents() - 1 && mData->comesFirst(mData->convertToMeasureTime(mData->getEvent(mCurrentEventIndex)), getCurrentTime())) {
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