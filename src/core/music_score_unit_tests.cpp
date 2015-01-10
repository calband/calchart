#include "music_score_unit_tests.h"
#include <random>
#include <cstdio>

std::map<ComparisonUnitTestHelpers::ComparisonRelationship, std::vector<ComparisonUnitTestHelpers::ComparisonRelationship>> ComparisonUnitTestHelpers::Implications = {
	{ ComparisonRelationship::Equal,				{ ComparisonRelationship::Equal, ComparisonRelationship::LessThanOrEqual, ComparisonRelationship::GreaterThanOrEqual } },
	{ ComparisonRelationship::LessThan,				{ ComparisonRelationship::LessThan, ComparisonRelationship::LessThanOrEqual, ComparisonRelationship::NotEqual } },
	{ ComparisonRelationship::LessThanOrEqual,		{ ComparisonRelationship::LessThanOrEqual } },
	{ ComparisonRelationship::GreaterThanOrEqual,	{ ComparisonRelationship::GreaterThanOrEqual } },
	{ ComparisonRelationship::GreaterThan,			{ ComparisonRelationship::GreaterThan, ComparisonRelationship::GreaterThanOrEqual, ComparisonRelationship::NotEqual } },
	{ ComparisonRelationship::NotEqual,				{ ComparisonRelationship::NotEqual } }
};

template <typename Type1, typename Type2>
bool ComparisonUnitTestHelpers::testRelationship(Type1 first, Type2 second, ComparisonRelationship relationship, bool expectedResult) {
	bool actualResult;
	switch (relationship) {
	case ComparisonRelationship::Equal:
		actualResult = first == second;
		break;
	case ComparisonRelationship::LessThan:
		actualResult = first < second;
		break;
	case ComparisonRelationship::LessThanOrEqual:
		actualResult = first <= second;
		break;
	case ComparisonRelationship::GreaterThanOrEqual:
		actualResult = first >= second;
		break;
	case ComparisonRelationship::GreaterThan:
		actualResult = first > second;
		break;
	case ComparisonRelationship::NotEqual:
		actualResult = first != second;
		break;
	default:
		actualResult = !expectedResult;
	}
	return (actualResult == expectedResult);
}

ComparisonUnitTestHelpers::ComparisonRelationship ComparisonUnitTestHelpers::getReverseRelationship(ComparisonRelationship relationship) {
	if (relationship == ComparisonRelationship::Equal || relationship == ComparisonRelationship::NotEqual) {
		return relationship;
	}
	return getOppositeRelationship(relationship);
}

ComparisonUnitTestHelpers::ComparisonRelationship ComparisonUnitTestHelpers::getOppositeRelationship(ComparisonRelationship relationship) {
	return (ComparisonRelationship)((int)ComparisonRelationship::__Last__ - ((int)relationship - (int)ComparisonRelationship::__First__));
}

template <typename Type1, typename Type2>
bool ComparisonUnitTestHelpers::testAllRelationships(Type1 first, Type2 second, ComparisonRelationship knownRelationship) {
	bool successful = true;
	for (ComparisonRelationship impliedRelationship : Implications.at(knownRelationship)) {
		successful = successful && testRelationship(first, second, impliedRelationship, true);
		successful = successful && testRelationship(second, first, getReverseRelationship(impliedRelationship), true);
	}
	ComparisonRelationship oppositeRelationship = getOppositeRelationship(knownRelationship);
	for (ComparisonRelationship impliedFalse : Implications.at(oppositeRelationship)) {
		bool skip = false;
		for (ComparisonRelationship impliedTrue : Implications.at(knownRelationship)) {
			if (impliedTrue == impliedFalse) {
				skip = true;
			}
		}
		if (skip) {
			continue;
		}
		successful = successful && testRelationship(first, second, impliedFalse, false);
		successful = successful && testRelationship(second, first, getReverseRelationship(impliedFalse), false);
	}
	return successful;
}

void MusicScoreMoment__UnitTests::test__compareInNullFragment() {
	MusicScoreMoment firstTypeA(nullptr, 0, 0);
	MusicScoreMoment secondTypeA = firstTypeA;
	MusicScoreMoment firstTypeB(nullptr, 0, 4);
	MusicScoreMoment firstTypeC(nullptr, 1, 0);

	assert(ComparisonUnitTestHelpers::testAllRelationships(firstTypeA, secondTypeA, ComparisonUnitTestHelpers::ComparisonRelationship::Equal));
	for (MusicScoreMoment currentTypeA : {firstTypeA, secondTypeA}) {
		assert(ComparisonUnitTestHelpers::testAllRelationships(currentTypeA, firstTypeB, ComparisonUnitTestHelpers::ComparisonRelationship::LessThan));
		assert(ComparisonUnitTestHelpers::testAllRelationships(currentTypeA, firstTypeC, ComparisonUnitTestHelpers::ComparisonRelationship::LessThan));
	}
	assert(ComparisonUnitTestHelpers::testAllRelationships(firstTypeB, firstTypeC, ComparisonUnitTestHelpers::ComparisonRelationship::LessThan));
}

void MusicScoreMoment__UnitTests::test__compareInSameFragment() {
	std::shared_ptr<MusicScoreFragment> fragment(new MusicScoreFragment("test"));
	MusicScoreMoment firstTypeA(fragment, 0, 0);
	MusicScoreMoment secondTypeA = firstTypeA;
	MusicScoreMoment firstTypeB(fragment, 0, 4);
	MusicScoreMoment firstTypeC(fragment, 1, 0);

	assert(ComparisonUnitTestHelpers::testAllRelationships(firstTypeA, secondTypeA, ComparisonUnitTestHelpers::ComparisonRelationship::Equal));
	for (MusicScoreMoment currentTypeA : {firstTypeA, secondTypeA}) {
		assert(ComparisonUnitTestHelpers::testAllRelationships(currentTypeA, firstTypeB, ComparisonUnitTestHelpers::ComparisonRelationship::LessThan));
		assert(ComparisonUnitTestHelpers::testAllRelationships(currentTypeA, firstTypeC, ComparisonUnitTestHelpers::ComparisonRelationship::LessThan));
	}
	assert(ComparisonUnitTestHelpers::testAllRelationships(firstTypeB, firstTypeC, ComparisonUnitTestHelpers::ComparisonRelationship::LessThan));
}

void MusicScoreMoment__UnitTests::test__compareInDifferentFragments() {
	std::shared_ptr<MusicScoreFragment> fragmentA(new MusicScoreFragment("fragA"));
	std::shared_ptr<MusicScoreFragment> fragmentB(new MusicScoreFragment("fragB"));
	std::shared_ptr<MusicScoreFragment> fragmentC(new MusicScoreFragment("fragC"));
	MusicScoreMoment firstTypeA(fragmentA, 0, 0);
	MusicScoreMoment secondTypeA = firstTypeA;
	MusicScoreMoment firstTypeB(fragmentB, 0, 4);
	MusicScoreMoment firstTypeC(fragmentC, 1, 0);

	std::vector<MusicScoreMoment> moments = { firstTypeA, secondTypeA, firstTypeB, firstTypeC };
	for (unsigned indexA = 0; indexA < moments.size(); indexA++) {
		MusicScoreMoment firstMoment = moments[indexA];
		for (unsigned indexB = 0; indexB < moments.size(); indexB++) {
			MusicScoreMoment secondMoment = moments[indexB];
			if (firstMoment != secondMoment) {
				for (unsigned whichRelationship = (int)ComparisonUnitTestHelpers::ComparisonRelationship::__First__ + 1; whichRelationship < (int)ComparisonUnitTestHelpers::ComparisonRelationship::__Last__; whichRelationship++) {
					if ((ComparisonUnitTestHelpers::ComparisonRelationship)(whichRelationship) == ComparisonUnitTestHelpers::ComparisonRelationship::NotEqual) {
						continue;
					}
					assert(ComparisonUnitTestHelpers::testRelationship(firstMoment, secondMoment, (ComparisonUnitTestHelpers::ComparisonRelationship)(whichRelationship), false));
				}
			}
		}
	}
}

void MusicScoreMoment__UnitTests::runAllTests() {
	MusicScoreMoment__UnitTests tester;
	tester.test__compareInNullFragment();
	tester.test__compareInDifferentFragments();
	tester.test__compareInSameFragment();
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__basicAdd() {
	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(generateEvent()));
	EventType newEvent = generateEvent();
	MusicScoreMoment newTime(nullptr, 1, 0);
	addEventToTestTarget(testTarget.get(), newTime, newEvent);
	assert(testTarget->getEvent(nullptr, 0).first == newTime);
	assert(testTarget->getEvent(nullptr, 0).second == newEvent);
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__basicRemove() {
	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(generateEvent()));
	EventType trackedEvent = generateEvent();
	MusicScoreMoment newTime(nullptr, 1, 0);
	MusicScoreMoment laterTime(nullptr, 2, 0);
	addEventToTestTarget(testTarget.get(), newTime, generateEvent());
	addEventToTestTarget(testTarget.get(), laterTime, trackedEvent);
	testTarget->removeEvent(nullptr, 0);
	assert(testTarget->getEvent(nullptr, 0).first == laterTime);
	assert(testTarget->getEvent(nullptr, 0).second == trackedEvent);
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__numEventsForOneFragment() {
	const unsigned eventsToAdd = 5;
	const unsigned eventsToRemove = 4;

	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(generateEvent()));

	std::shared_ptr<MusicScoreFragment> fragment(new MusicScoreFragment("testFrag"));
	for (unsigned eventsAdded = 0; eventsAdded < eventsToAdd; eventsAdded++) {
		addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragment, eventsAdded, 0), generateEvent());
		assert(testTarget->getNumEvents(fragment.get()) == eventsAdded + 1);
	}
	for (unsigned eventsRemoved = 0; eventsRemoved < eventsToRemove; eventsRemoved++) {
		unsigned expectedNumEvents = eventsToAdd - eventsRemoved;
		testTarget->removeEvent(fragment.get(), rand() % expectedNumEvents);
		assert(testTarget->getNumEvents(fragment.get()) == expectedNumEvents - 1);
	}
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__numEventsForUnregisteredFragment() {
	const unsigned eventsToAdd = 5;
	
	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(generateEvent()));

	std::shared_ptr<MusicScoreFragment> fragment(new MusicScoreFragment("testFrag"));
	std::shared_ptr<MusicScoreFragment> unregistered(new MusicScoreFragment("unregistered"));

	for (unsigned eventsAdded = 0; eventsAdded < eventsToAdd; eventsAdded++) {
		addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragment, eventsAdded, 0), generateEvent());
	}

	assert(testTarget->getNumEvents(fragment.get()) == eventsToAdd);
	assert(testTarget->getNumEvents(nullptr) == 0);
	assert(testTarget->getNumEvents(unregistered.get()) == 0);
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__defaultEvent() {
	EventType defaultEvent = generateEvent();
	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(defaultEvent));
	EventType newEvent = generateEvent();
	MusicScoreMoment newTime(nullptr, 1, 0);
	addEventToTestTarget(testTarget.get(), newTime, newEvent);
	assert(testTarget->getDefaultEvent() == defaultEvent);
	assert(testTarget->getEvent(nullptr, 1).second == defaultEvent);
	assert(testTarget->getEvent(nullptr, -1).second == defaultEvent);
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__defaultEventOnUnregisteredFragment() {
	EventType defaultEvent = generateEvent();
	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(defaultEvent));
	std::shared_ptr<MusicScoreFragment> fragment(new MusicScoreFragment("testFrag"));
	std::shared_ptr<MusicScoreFragment> unregistered(new MusicScoreFragment("unregistered"));
	EventType newEvent = generateEvent();
	MusicScoreMoment newTime(fragment, 1, 0);
	addEventToTestTarget(testTarget.get(), newTime, newEvent);
	assert(testTarget->getEvent(unregistered.get(), 1).second == defaultEvent);
	assert(testTarget->getEvent(unregistered.get(), -1).second == defaultEvent);
	assert(testTarget->getEvent(unregistered.get(), 0).second == defaultEvent);
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__registeredFragments() {
	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(generateEvent()));
	std::shared_ptr<MusicScoreFragment> fragmentA(new MusicScoreFragment("fragA"));
	std::shared_ptr<MusicScoreFragment> fragmentB(new MusicScoreFragment("fragB"));
	std::shared_ptr<MusicScoreFragment> fragmentC(new MusicScoreFragment("fragC"));
	assert(testTarget->getAllFragmentsWithEvents().size() == 0);
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentA, 1, 0), generateEvent());
	assert(testTarget->getAllFragmentsWithEvents().size() == 1);
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentB, 1, 0), generateEvent());
	assert(testTarget->getAllFragmentsWithEvents().size() == 2);
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentC, 1, 0), generateEvent());
	assert(testTarget->getAllFragmentsWithEvents().size() == 3);
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentC, 2, 0), generateEvent());
	assert(testTarget->getAllFragmentsWithEvents().size() == 3);
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentB, 2, 0), generateEvent());
	assert(testTarget->getAllFragmentsWithEvents().size() == 3);
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentB, 3, 0), generateEvent());
	assert(testTarget->getAllFragmentsWithEvents().size() == 3);
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(nullptr, 1, 0), generateEvent());
	assert(testTarget->getAllFragmentsWithEvents().size() == 4);
	testTarget->removeEvent(nullptr, 0);
	assert(testTarget->getAllFragmentsWithEvents().size() == 3);
	testTarget->removeEvent(fragmentC.get(), 0);
	assert(testTarget->getAllFragmentsWithEvents().size() == 3);
	testTarget->removeEvent(fragmentC.get(), 0);
	assert(testTarget->getAllFragmentsWithEvents().size() == 2);
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__clearAll() {
	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(generateEvent()));
	std::shared_ptr<MusicScoreFragment> fragmentA(new MusicScoreFragment("fragA"));
	std::shared_ptr<MusicScoreFragment> fragmentB(new MusicScoreFragment("fragB"));
	std::shared_ptr<MusicScoreFragment> fragmentC(new MusicScoreFragment("fragC"));
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentA, 1, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentB, 1, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentB, 2, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentC, 1, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentC, 2, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentC, 3, 0), generateEvent());
	testTarget->clearEvents();
	assert(testTarget->getAllFragmentsWithEvents().size() == 0);
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__clearFragment() {
	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(generateEvent()));
	std::shared_ptr<MusicScoreFragment> fragmentA(new MusicScoreFragment("fragA"));
	std::shared_ptr<MusicScoreFragment> fragmentB(new MusicScoreFragment("fragB"));
	std::shared_ptr<MusicScoreFragment> fragmentC(new MusicScoreFragment("fragC"));
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentA, 1, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentB, 1, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentB, 2, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentC, 1, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentC, 2, 0), generateEvent());
	addEventToTestTarget(testTarget.get(), MusicScoreMoment(fragmentC, 3, 0), generateEvent());
	assert(testTarget->getAllFragmentsWithEvents().size() == 3);
	testTarget->clearEvents(fragmentA.get());
	assert(testTarget->getAllFragmentsWithEvents().size() == 2);
	testTarget->clearEvents(fragmentA.get());
	assert(testTarget->getAllFragmentsWithEvents().size() == 2);
	testTarget->clearEvents(nullptr);
	assert(testTarget->getAllFragmentsWithEvents().size() == 2);
	testTarget->clearEvents(fragmentC.get());
	assert(testTarget->getAllFragmentsWithEvents().size() == 1);
	testTarget->clearEvents(fragmentB.get());
	assert(testTarget->getAllFragmentsWithEvents().size() == 0);
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::test__resetDefaultEvent() {
	std::unique_ptr<CollectionOfMusicScoreEvents<EventType>> testTarget(makeTestTarget(generateEvent()));
	std::shared_ptr<MusicScoreFragment> fragment(new MusicScoreFragment("testFrag"));
	std::shared_ptr<MusicScoreFragment> unregistered(new MusicScoreFragment("unregistered"));
	EventType newDefault = generateEvent();
	EventType newEvent = generateEvent();
	MusicScoreMoment newTime(fragment, 1, 0);
	addEventToTestTarget(testTarget.get(), newTime, newEvent);
	testTarget->resetDefaultEvent(newDefault);
	assert(testTarget->getEvent(fragment.get(), 1).second == newDefault);
	assert(testTarget->getEvent(fragment.get(), -1).second == newDefault);
	assert(testTarget->getEvent(unregistered.get(), 0).second == newDefault);
}

template <typename EventType>
void CollectionOfMusicEvents__UnitTests<EventType>::runTests() {
	test__basicAdd();
	test__basicRemove();
	test__numEventsForOneFragment();
	test__numEventsForUnregisteredFragment();
	test__defaultEvent();
	test__defaultEventOnUnregisteredFragment();
	test__registeredFragments();
	test__clearFragment();
	test__clearAll();
	test__resetDefaultEvent();
}



void TimeSignaturesCollection__UnitTests::runAllTests() {
	TimeSignaturesCollection__UnitTests tester;
	tester.runTests();
}

TimeSignature TimeSignaturesCollection__UnitTests::generateEvent() {
	mCounter++;
	return TimeSignature(mCounter);
}

void TimeSignaturesCollection__UnitTests::addEventToTestTarget(CollectionOfMusicScoreEvents<TimeSignature>* target, MusicScoreMoment eventTime, TimeSignature eventObj) {
	((TimeSignaturesCollection*)target)->addTimeSignatureChange(eventTime.fragment, eventTime.beatAndBar.bar, eventObj);
}

CollectionOfMusicScoreEvents<TimeSignature>* TimeSignaturesCollection__UnitTests::makeTestTarget(TimeSignature defaultEvent) {
	return new TimeSignaturesCollection(defaultEvent);
}

void MusicScoreJumpsCollection__UnitTests::runAllTests() {
	MusicScoreJumpsCollection__UnitTests tester;
	tester.runTests();
}

MusicScoreJump MusicScoreJumpsCollection__UnitTests::generateEvent() {
	mCounter++;
	return MusicScoreJump(MusicScoreMoment(nullptr, mCounter, 0));
}

void MusicScoreJumpsCollection__UnitTests::addEventToTestTarget(CollectionOfMusicScoreEvents<MusicScoreJump>* target, MusicScoreMoment eventTime, MusicScoreJump eventObj) {
	((MusicScoreJumpsCollection*)target)->addJump(eventTime, eventObj);
}

CollectionOfMusicScoreEvents<MusicScoreJump>* MusicScoreJumpsCollection__UnitTests::makeTestTarget(MusicScoreJump defaultEvent) {
	return new MusicScoreJumpsCollection(defaultEvent);
}

void MusicScoreTemposCollection__UnitTests::runAllTests() {
	MusicScoreTemposCollection__UnitTests tester;
	tester.runTests();
}

MusicScoreTempo MusicScoreTemposCollection__UnitTests::generateEvent() {
	mCounter++;
	return MusicScoreTempo(mCounter);
}

void MusicScoreTemposCollection__UnitTests::addEventToTestTarget(CollectionOfMusicScoreEvents<MusicScoreTempo>* target, MusicScoreMoment eventTime, MusicScoreTempo eventObj) {
	((MusicScoreTemposCollection*)target)->addTempoChange(eventTime, eventObj);
}

CollectionOfMusicScoreEvents<MusicScoreTempo>* MusicScoreTemposCollection__UnitTests::makeTestTarget(MusicScoreTempo defaultEvent) {
	return new MusicScoreTemposCollection(defaultEvent);
}

void MusicScoreBarLabelsCollection__UnitTests::runAllTests() {
	MusicScoreBarLabelsCollection__UnitTests tester;
	tester.runTests();
}

MusicScoreBarLabel MusicScoreBarLabelsCollection__UnitTests::generateEvent() {
	mCounter++;
	char buffer[100];
	sprintf(buffer, "%i", mCounter);
	return MusicScoreBarLabel(std::string(buffer));
}

void MusicScoreBarLabelsCollection__UnitTests::addEventToTestTarget(CollectionOfMusicScoreEvents<MusicScoreBarLabel>* target, MusicScoreMoment eventTime, MusicScoreBarLabel eventObj) {
	((MusicScoreBarLabelsCollection*)target)->addBarLabel(eventTime.fragment, eventTime.beatAndBar.bar, eventObj);
}

CollectionOfMusicScoreEvents<MusicScoreBarLabel>* MusicScoreBarLabelsCollection__UnitTests::makeTestTarget(MusicScoreBarLabel defaultEvent) {
	return new MusicScoreBarLabelsCollection(defaultEvent);
}

void MusicScoreMomentBrowser__UnitTests::test__defaultEventInGoodFragment() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	timeSigs.addTimeSignatureChange(nullptr, 4, TimeSignature(8));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));
	jumps.addJump(MusicScoreMoment(nullptr, 8, 2), MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 0, 0));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 0));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 1));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 2));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 3));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 1, 0));
	browser->nextBeat(4);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 2, 0));
};

void MusicScoreMomentBrowser__UnitTests::test__defaultEventInUnregisteredFragment() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 0, 0));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 0));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 1));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 2));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 3));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 1, 0));
	browser->nextBeat(4);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 2, 0));
};

void MusicScoreMomentBrowser__UnitTests::test__startAtNonzeroTimeInGoodFragment() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	timeSigs.addTimeSignatureChange(nullptr, 10, TimeSignature(8));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));
	jumps.addJump(MusicScoreMoment(nullptr, 20, 2), MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 4, 2));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 2));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 3));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 5, 0));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 5, 1));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 5, 2));
	browser->nextBeat(4);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 6, 2));
};

void MusicScoreMomentBrowser__UnitTests::test__startATNonzeroTimeInUnregisteredFragment() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 4, 2));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 2));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 3));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 5, 0));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 5, 1));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 5, 2));
	browser->nextBeat(4);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 6, 2));
};

void MusicScoreMomentBrowser__UnitTests::test__stepBackInGoodFragment() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	timeSigs.addTimeSignatureChange(nullptr, 10, TimeSignature(8));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));
	jumps.addJump(MusicScoreMoment(nullptr, 20, 2), MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 4, 2));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 2));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 1));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 0));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 3, 3));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 3, 2));
	browser->previousBeat(4);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 2, 2));
};

void MusicScoreMomentBrowser__UnitTests::test__stepBackInUnregisteredFragment() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 4, 2));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 2));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 1));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 4, 0));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 3, 3));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 3, 2));
	browser->previousBeat(4);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 2, 2));
};

void MusicScoreMomentBrowser__UnitTests::test__stepOneTimeSigBound() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	timeSigs.addTimeSignatureChange(nullptr, 10, TimeSignature(8));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));
	jumps.addJump(MusicScoreMoment(nullptr, 20, 2), MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 9, 2));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 9, 2));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 9, 3));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 10, 0));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 10, 1));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 10, 2));
	browser->nextBeat(8);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 11, 2));
};

void MusicScoreMomentBrowser__UnitTests::test__backstepOneTimeSigBound() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	timeSigs.addTimeSignatureChange(nullptr, 10, TimeSignature(8));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));
	jumps.addJump(MusicScoreMoment(nullptr, 20, 2), MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 11, 2));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 11, 2));
	browser->previousBeat(8);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 10, 2));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 10, 1));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 10, 0));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 9, 3));
	browser->previousBeat(4);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 8, 3));
};

void MusicScoreMomentBrowser__UnitTests::test__backstepOneJump() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	timeSigs.addTimeSignatureChange(nullptr, 10, TimeSignature(2));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));
	jumps.addJump(MusicScoreMoment(nullptr, 20, 0), MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 0, 2));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 2));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 1));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 0));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 19, 1));
	browser->previousBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 19, 0));
	browser->previousBeat(2);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 18, 0));
};

void MusicScoreMomentBrowser__UnitTests::test__stepOneJump() {
	TimeSignaturesCollection timeSigs(TimeSignature(4));
	timeSigs.addTimeSignatureChange(nullptr, 10, TimeSignature(2));
	MusicScoreJumpsCollection jumps(MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));
	jumps.addJump(MusicScoreMoment(nullptr, 20, 0), MusicScoreJump(MusicScoreMoment(nullptr, 0, 0)));

	MusicScoreMomentBrowser* browser = timeSigs.makeMusicScoreMomentBrowser(&jumps, MusicScoreMoment(nullptr, 19, 0));
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 19, 0));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 19, 1));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 0));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 1));
	browser->nextBeat();
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 0, 2));
	browser->nextBeat(4);
	assert(browser->getCurrentTime() == MusicScoreMoment(nullptr, 1, 2));
};


void MusicScoreMomentBrowser__UnitTests::runAllTests() {
	test__defaultEventInGoodFragment();
	test__defaultEventInUnregisteredFragment();
	test__startAtNonzeroTimeInGoodFragment();
	test__startATNonzeroTimeInUnregisteredFragment();
	test__stepBackInGoodFragment();
	test__stepBackInUnregisteredFragment();
	test__stepOneTimeSigBound();
	test__backstepOneTimeSigBound();
	test__stepOneJump();
	test__backstepOneJump();
}


void runAllMusicScoreUnitTests() {
	MusicScoreMoment__UnitTests::runAllTests();
	TimeSignaturesCollection__UnitTests::runAllTests();
	MusicScoreJumpsCollection__UnitTests::runAllTests();
	MusicScoreTemposCollection__UnitTests::runAllTests();
	MusicScoreBarLabelsCollection__UnitTests::runAllTests();
	MusicScoreMomentBrowser__UnitTests::runAllTests();
}
