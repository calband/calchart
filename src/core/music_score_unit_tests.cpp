#include "music_score_unit_tests.h"

std::map<ComparisonUnitTestHelpers::ComparisonRelationship, std::vector<ComparisonUnitTestHelpers::ComparisonRelationship>> ComparisonUnitTestHelpers::Implications{
	{ ComparisonRelationship::Equal,				{ ComparisonRelationship::Equal, ComparisonRelationship::LessThanOrEqual, ComparisonRelationship::GreaterThanOrEqual } },
	{ ComparisonRelationship::LessThan,				{ ComparisonRelationship::LessThan, ComparisonRelationship::LessThanOrEqual, ComparisonRelationship::NotEqual } },
	{ ComparisonRelationship::LessThanOrEqual,		{ ComparisonRelationship::LessThanOrEqual } },
	{ ComparisonRelationship::GreaterThanOrEqual,	{ ComparisonRelationship::GreaterThanOrEqual } },
	{ ComparisonRelationship::GreaterThan,			{ ComparisonRelationship::GreaterThan, ComparisonRelationship::GreaterThanOrEqual, ComparisonRelationship::NotEqual } },
	{ ComparisonRelationship::NotEqual,				{ ComparisonRelationship::NotEqual } },
};

template <typename Type1, typename Type2>
bool ComparisonUnitTestHelpers::testRelationship(Type1 first, Type2 second, ComparisonRelationship relationship, bool expectedResult) {
	bool actualResult;
	switch (relationship) {
	case Equal:
		actualResult = first == second;
		break
	case LessThan:
		actualResult = first < second;
		break;
	case LessThanOrEqual:
		actualResult = first <= second;
		break;
	case GreaterThanOrEqual:
		actualResult = first >= second;
		break;
	case GreaterThan:
		actualResult = first > second;
		break;
	case NotEqual:
		actualResult = first != second;
		break;
	default:
		actualResult = !expectedResult;
	}
	return (actualResult == expectedResult);
}

ComparisonUnitTestHelpers::ComparisonRelationship ComparisonUnitTestHelpers::getReverseRelationship(ComparisonRelationship relationship) {
	return (ComparisonRelationship)((int)ComparisonRelationship::__Last__ - ((int)relationship - (int)ComparisonRelationship::__First__));
}

template <typename Type1, typename Type2>
bool ComparisonUnitTestHelpers::testAllForwardRelationships(Type1 first, Type2 second, ComparisonRelationship knownRelationship) {
	for (ComparisonRelationship impliedRelationship : Implications.at(knownRelationship)) {
		testRelationship(first, second, impliedRelationship, true);
	}
	//Also test contrapositives
	ComparisonRelationship reverseRelationship = getReverseRelationship();
	for (ComparisonRelationship impliedFalse : Implications.at(reverseRelationship)) {
		testRelationship(first, second, impliedFalse, false);
	}
}

template <typename Type1, typename Type2>
bool ComparisonUnitTestHelpers::testAllBackwardRelationships(Type1 first, Type2 second, ComparisonRelationship knownRelationship) {
	ComparisonRelationship reverseRelationship = getReverseRelationship(); 
	return testAllForwardRelationships(second, first, reverseRelationship);
}

template <typename Type1, typename Type2>
bool ComparisonUnitTestHelpers::testAllRelationships(Type1 first, Type2 second, ComparisonRelationship knownRelationship) {
	testAllForwardRelationships(first, second, knownRelationship);
	testAllBackwardRelationships(first, second, knownRelationship);
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
	MusicScoreFragment fragment("test");
	MusicScoreMoment firstTypeA(&fragment, 0, 0);
	MusicScoreMoment secondTypeA = firstTypeA;
	MusicScoreMoment firstTypeB(&fragment, 0, 4);
	MusicScoreMoment firstTypeC(&fragment, 1, 0);

	assert(ComparisonUnitTestHelpers::testAllRelationships(firstTypeA, secondTypeA, ComparisonUnitTestHelpers::ComparisonRelationship::Equal));
	for (MusicScoreMoment currentTypeA : {firstTypeA, secondTypeA}) {
		assert(ComparisonUnitTestHelpers::testAllRelationships(currentTypeA, firstTypeB, ComparisonUnitTestHelpers::ComparisonRelationship::LessThan));
		assert(ComparisonUnitTestHelpers::testAllRelationships(currentTypeA, firstTypeC, ComparisonUnitTestHelpers::ComparisonRelationship::LessThan));
	}
	assert(ComparisonUnitTestHelpers::testAllRelationships(firstTypeB, firstTypeC, ComparisonUnitTestHelpers::ComparisonRelationship::LessThan));
}

void MusicScoreMoment__UnitTests::test__compareInDifferentFragments() {
	MusicScoreFragment fragmentA("fragA");
	MusicScoreFragment fragmentB("fragB");
	MusicScoreFragment fragmentC("fragC");
	MusicScoreMoment firstTypeA(&fragmentA, 0, 0);
	MusicScoreMoment secondTypeA = firstTypeA;
	MusicScoreMoment firstTypeB(&fragmentB, 0, 4);
	MusicScoreMoment firstTypeC(&fragmentC, 1, 0);

	std::vector<MusicScoreMoment> allTypes = { firstTypeA, secondTypeA, firstTypeB, firstTypeC };
	for (MusicScoreMoment firstMoment : allTypes) {
		for (MusicScoreMoment secondMoment : allTypes) {
			if (firstMoment != secondMoment) {
				for (unsigned whichRelationship = (int)ComparisonUnitTestHelpers::ComparisonRelationship::__First__ + 1; whichRelationship < (int)ComparisonUnitTestHelpers::ComparisonRelationship::__Last__; whichRelationship++) {
					assert(ComparisonUnitTestHelpers::testRelationship(firstMoment, secondMoment, (ComparisonUnitTestHelpers::ComparisonRelationship)(whichRelationship), false));
				}
			}
		}
	}
}

void MusicScoreMoment__UnitTests::runAllTests() {
	test__compareInNullFragment();
	test__compareInDifferentFragments();
	test__compareInSameFragment();
}
