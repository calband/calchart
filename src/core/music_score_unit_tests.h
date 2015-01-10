#pragma once

#include "music_score_doc_component.h"
#include <map>
#include <functional>

class ComparisonUnitTestHelpers {
public:
	enum class ComparisonRelationship {
		__First__ = 0,
		Equal,
		LessThan,
		LessThanOrEqual,
		GreaterThanOrEqual,
		GreaterThan,
		NotEqual,
		__Last__
	};

	template <typename Type1, typename Type2>
	static bool testAllRelationships(Type1 first, Type2 second, ComparisonRelationship knownRelationship);

	template <typename Type1, typename Type2>
	static bool testRelationship(Type1 first, Type2 second, ComparisonRelationship relationship, bool expectedResult);
private:
	static ComparisonRelationship getReverseRelationship(ComparisonRelationship relationship);
	static ComparisonRelationship getOppositeRelationship(ComparisonRelationship relationship);

	static std::map<ComparisonRelationship, std::vector<ComparisonRelationship>> Implications;
};

class MusicScoreMoment__UnitTests {
public:
	void test__compareInNullFragment();
	void test__compareInSameFragment();
	void test__compareInDifferentFragments();

	static void runAllTests();
};

template <typename EventType>
class CollectionOfMusicEvents__UnitTests {
public:
	void test__basicAdd();

	void test__basicRemove();

	void test__numEventsForOneFragment();

	void test__numEventsForUnregisteredFragment();

	void test__defaultEvent();

	void test__defaultEventOnUnregisteredFragment();

	void test__registeredFragments();

	void test__clearAll();

	void test__clearFragment();

	void test__resetDefaultEvent();

	void runTests();
protected:
	virtual EventType generateEvent() = 0;
	
	virtual void addEventToTestTarget(CollectionOfMusicScoreEvents<EventType>* target, MusicScoreMoment eventTime, EventType eventObj) = 0;
	virtual CollectionOfMusicScoreEvents<EventType>* makeTestTarget(EventType defaultEvent) = 0;
};


class TimeSignaturesCollection__UnitTests : public CollectionOfMusicEvents__UnitTests<TimeSignature> {
private:
	using super = CollectionOfMusicEvents__UnitTests<TimeSignature>;
public:
	static void runAllTests();
protected:
	virtual TimeSignature generateEvent();

	virtual void addEventToTestTarget(CollectionOfMusicScoreEvents<TimeSignature>* target, MusicScoreMoment eventTime, TimeSignature eventObj);
	virtual CollectionOfMusicScoreEvents<TimeSignature>* makeTestTarget(TimeSignature defaultEvent);

	uint32_t mCounter = 0;
};

class MusicScoreJumpsCollection__UnitTests : public CollectionOfMusicEvents__UnitTests<MusicScoreJump> {
private:
	using super = CollectionOfMusicEvents__UnitTests<MusicScoreJump>;
public:
	static void runAllTests();
protected:
	virtual MusicScoreJump generateEvent();

	virtual void addEventToTestTarget(CollectionOfMusicScoreEvents<MusicScoreJump>* target, MusicScoreMoment eventTime, MusicScoreJump eventObj);
	virtual CollectionOfMusicScoreEvents<MusicScoreJump>* makeTestTarget(MusicScoreJump defaultEvent);

	uint32_t mCounter = 0;
};

class MusicScoreTemposCollection__UnitTests : public CollectionOfMusicEvents__UnitTests<MusicScoreTempo> {
private:
	using super = CollectionOfMusicEvents__UnitTests<MusicScoreTempo>;
public:
	static void runAllTests();
protected:
	virtual MusicScoreTempo generateEvent();

	virtual void addEventToTestTarget(CollectionOfMusicScoreEvents<MusicScoreTempo>* target, MusicScoreMoment eventTime, MusicScoreTempo eventObj);
	virtual CollectionOfMusicScoreEvents<MusicScoreTempo>* makeTestTarget(MusicScoreTempo defaultEvent);

	uint32_t mCounter = 0;
};

class MusicScoreBarLabelsCollection__UnitTests : public CollectionOfMusicEvents__UnitTests<MusicScoreBarLabel> {
private:
	using super = CollectionOfMusicEvents__UnitTests<MusicScoreBarLabel>;
public:
	static void runAllTests();
protected:
	virtual MusicScoreBarLabel generateEvent();

	virtual void addEventToTestTarget(CollectionOfMusicScoreEvents<MusicScoreBarLabel>* target, MusicScoreMoment eventTime, MusicScoreBarLabel eventObj);
	virtual CollectionOfMusicScoreEvents<MusicScoreBarLabel>* makeTestTarget(MusicScoreBarLabel defaultEvent);

	uint32_t mCounter = 0;
};


class MusicScoreMomentBrowser__UnitTests {
public:
	static void test__defaultEventInGoodFragment();
	static void test__defaultEventInUnregisteredFragment();
	static void test__startAtNonzeroTimeInGoodFragment();
	static void test__startATNonzeroTimeInUnregisteredFragment();
	static void test__stepOneTimeSigBound();
	static void test__stepOneJump();
	static void test__startOnJump();
	static void test__jumpSeries();
	static void test__startOnJumpSeries();

	static void runAllTests();
};



void runAllMusicScoreUnitTests();