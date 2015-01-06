#pragma once

#include "music_score_doc_component.h"
#include <map>

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
	static bool testAllForwardRelationships(Type1 first, Type2 second, ComparisonRelationship knownRelationship);

	template <typename Type1, typename Type2>
	static bool testAllBackwardRelationships(Type1 first, Type2 second, ComparisonRelationship knownRelationship);

	template <typename Type1, typename Type2>
	static bool testAllRelationships(Type1 first, Type2 second, ComparisonRelationship knownRelationship);

	template <typename Type1, typename Type2>
	static bool testRelationship(Type1 first, Type2 second, ComparisonRelationship relationship, bool expectedResult);
private:
	ComparisonRelationship getReverseRelationship(ComparisonRelationship relationship);

	static std::map<ComparisonRelationship, std::vector<ComparisonRelationship>> Implications;
};


class MusicScoreMoment__UnitTests {
public:
	void test__compareInNullFragment();
	void test__compareInSameFragment();
	void test__compareInDifferentFragments();

	void runAllTests();
};

/**
class CollectionOfMusicEvents__UnitTests {
public:
	void test__numEventsForOneFragment();
	void test__numEventsForUnregisteredFragment();
	void test__defaultEvent();
	void test__defaultEventOnUnregisteredFragment();
};
*/



void runAllMusicScoreUnitTests();