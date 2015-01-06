#include "music_score_loaders.h"
#include "cc_fileformat.h"

std::string MusicScoreLoader::LatestVersionString = "3.4.2";

std::unordered_map<std::string, std::shared_ptr<MusicScoreLoader>> MusicScoreLoader::VersionLoaders = {
	{ "3.4.2", std::shared_ptr<MusicScoreLoader>(new MusicScoreLoader__3_4_2()) }
};

MusicScoreDocComponent* MusicScoreLoader::loadFromVersionedData(const uint8_t* data, size_t size) {
	const uint8_t* upperBound = data + size;
	std::string versionString = getVersionStringFromData(data, upperBound);
	const MusicScoreLoader* loader = getLoaderForVersion(versionString);
	if (loader == nullptr) {
		return nullptr;
	}
	return loader->loadFromData(data, upperBound);
}

std::vector<uint8_t> MusicScoreLoader::serializeForLatestVersion(const MusicScoreDocComponent* docComponent) {
	std::vector<uint8_t> returnVal;
	const MusicScoreLoader* loader = getLoaderForVersion(LatestVersionString);
	if (loader == nullptr) {
		return returnVal;
	}
	serializeVersionString(returnVal, LatestVersionString);
	loader->serialize(returnVal, docComponent);
	return returnVal;
}

const MusicScoreLoader* MusicScoreLoader::getLoaderForVersion(std::string version) {
	if (VersionLoaders.find(version) != VersionLoaders.end()) {
		return VersionLoaders[version].get();
	}
	return nullptr;
}

std::string MusicScoreLoader::getVersionStringFromData(const uint8_t*& data, const uint8_t* upperBound) {
	return parseFirstStringWithinBounds(data, upperBound);
}

void MusicScoreLoader::serializeVersionString(std::vector<uint8_t>& data, std::string versionString) {
	using CalChart::Parser::AppendAndNullTerminate;
	AppendAndNullTerminate(data, versionString);
}

const char* MusicScoreLoader::findFirstNullCharacter(const char* buffer, const char* upperBound) {
	for (; buffer < upperBound; buffer++) {
		if ((*buffer) == '/0') {
			return buffer;
		}
	}
	return upperBound;
}

std::string MusicScoreLoader::parseFirstStringWithinBounds(const uint8_t*& buffer, const uint8_t* upperBound) {
	const uint8_t* firstNullCharacter = (const uint8_t*)findFirstNullCharacter((const char*)buffer, (const char*)upperBound);
	if (firstNullCharacter >= upperBound) {
		throw CC_FileException("Null-terminated string could not be found within specified bounds.");
	}
	std::string returnVal((char*)buffer, firstNullCharacter - buffer);
	buffer = firstNullCharacter + 1;
	return returnVal;
}

void MusicScoreLoader__3_4_2::serialize(std::vector<uint8_t>& target, const MusicScoreDocComponent* docComponent) const {
	using CalChart::Parser::Append;
	using CalChart::Parser::Construct_block;
	
	std::unordered_map<const MusicScoreFragment*, uint32_t> fragmentToIdMap = buildFragmentToIndexMap(docComponent);
	{
		std::vector<uint8_t> fragmentsBlock;
		serializeFragments(fragmentsBlock, docComponent);
		Append(target, Construct_block(INGL_MFRG, fragmentsBlock));
	}
	{
		std::vector<uint8_t> timeSignaturesBlock;
		serializeTimeSignatures(timeSignaturesBlock, docComponent->getTimeSignatures(), fragmentToIdMap);
		Append(target, Construct_block(INGL_TSIG, timeSignaturesBlock));
	}
	{
		std::vector<uint8_t> jumpsBlock;
		serializeJumps(jumpsBlock, docComponent->getScoreJumps(), fragmentToIdMap);
		Append(target, Construct_block(INGL_MJMP, jumpsBlock));
	}
	{
		std::vector<uint8_t> temposBlock;
		serializeTempos(temposBlock, docComponent->getTempos(), fragmentToIdMap);
		Append(target, Construct_block(INGL_MTMP, temposBlock));
	}
	{
		std::vector<uint8_t> barLabelsBlock;
		serializeBarLabels(barLabelsBlock, docComponent->getBarLabels(), fragmentToIdMap);
		Append(target, Construct_block(INGL_MLBL, barLabelsBlock));
	}
}

void MusicScoreLoader__3_4_2::serializeFragments(std::vector<uint8_t>& data, const MusicScoreDocComponent* docComponent) const {
	using CalChart::Parser::Append;

	uint32_t numFragments = docComponent->getNumScoreFragments();
	Append(data, numFragments);
	for (uint32_t index = 0; index < numFragments; index++) {
		serializeIndividualFragment(data, *(docComponent->getScoreFragment(index).get()));
	}
}

void MusicScoreLoader__3_4_2::serializeIndividualFragment(std::vector<uint8_t>& data, const MusicScoreFragment& fragment) const {
	using CalChart::Parser::AppendAndNullTerminate;

	AppendAndNullTerminate(data, fragment.name);
}

void MusicScoreLoader__3_4_2::serializeTimeSignatures(std::vector<uint8_t>& data, const TimeSignaturesCollection* timeSignatures, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	const std::function<void(std::vector<uint8_t>&, const TimeSignature&, const std::unordered_map<const MusicScoreFragment*, uint32_t>&)> serializeFunc =
	[this](std::vector<uint8_t>& data, const TimeSignature& thisTimeSignature, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) {
		this->serializeIndividualTimeSignature(data, thisTimeSignature);
	};
	serializeCollectionOfMusicEvents(data, timeSignatures, fragmentToIdMap, true, serializeFunc);
}

void MusicScoreLoader__3_4_2::serializeIndividualTimeSignature(std::vector<uint8_t>& data, const TimeSignature& timeSignature) const {
	using CalChart::Parser::Append;

	Append(data, (uint32_t)timeSignature.beatsPerBar);
}

void MusicScoreLoader__3_4_2::serializeJumps(std::vector<uint8_t>& data, const MusicScoreJumpsCollection* jumps, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	const std::function<void(std::vector<uint8_t>&, const MusicScoreJump&, const std::unordered_map<const MusicScoreFragment*, uint32_t>&)> serializeFunc =
	[this](std::vector<uint8_t>& data, const MusicScoreJump& thisJump, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) {
		this->serializeIndividualJump(data, thisJump, fragmentToIdMap);
	};
	serializeCollectionOfMusicEvents(data, jumps, fragmentToIdMap, false, serializeFunc);
}

void MusicScoreLoader__3_4_2::serializeIndividualJump(std::vector<uint8_t>& data, const MusicScoreJump& jump, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	serializeMusicScoreMoment(data, jump.jumpTo, fragmentToIdMap);
}

void MusicScoreLoader__3_4_2::serializeTempos(std::vector<uint8_t>& data, const MusicScoreTemposCollection* tempos, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	const std::function<void(std::vector<uint8_t>&, const MusicScoreTempo&, const std::unordered_map<const MusicScoreFragment*, uint32_t>&)> serializeFunc =
	[this](std::vector<uint8_t>& data, const MusicScoreTempo& thisTempo, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) {
		this->serializeIndividualTempo(data, thisTempo);
	};
	serializeCollectionOfMusicEvents(data, tempos, fragmentToIdMap, true, serializeFunc);
}

void MusicScoreLoader__3_4_2::serializeIndividualTempo(std::vector<uint8_t>& data, const MusicScoreTempo& tempo) const {
	using CalChart::Parser::Append;

	Append(data, (uint32_t)tempo.beatsPerMinute);
}

void MusicScoreLoader__3_4_2::serializeBarLabels(std::vector<uint8_t>& data, const MusicScoreBarLabelsCollection* barLabels, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	const std::function<void(std::vector<uint8_t>&, const MusicScoreBarLabel&, const std::unordered_map<const MusicScoreFragment*, uint32_t>&)> serializeFunc =
	[this](std::vector<uint8_t>& data, const MusicScoreBarLabel& thisLabel, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) {
		this->serializeIndividualBarLabel(data, thisLabel);
	};
	serializeCollectionOfMusicEvents(data, barLabels, fragmentToIdMap, false, serializeFunc);
}

void MusicScoreLoader__3_4_2::serializeIndividualBarLabel(std::vector<uint8_t>& data, const MusicScoreBarLabel& barLabel) const {
	using CalChart::Parser::AppendAndNullTerminate;

	AppendAndNullTerminate(data, barLabel.label);
}

void MusicScoreLoader__3_4_2::serializeMusicScoreMoment(std::vector<uint8_t>& data, const MusicScoreMoment& moment, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	using CalChart::Parser::Append;

	Append(data, (uint32_t)(fragmentToIdMap.at(moment.fragment.get())));
	Append(data, (uint32_t)(moment.beatAndBar.bar));
	Append(data, (uint32_t)(moment.beatAndBar.beat));
}

std::unordered_map<const MusicScoreFragment*, uint32_t> MusicScoreLoader__3_4_2::buildFragmentToIndexMap(const MusicScoreDocComponent* docComponent) const {
	std::unordered_map<const MusicScoreFragment*, uint32_t> returnVal;
	for (unsigned index = 0; index < docComponent->getNumScoreFragments(); index++) {
		returnVal.emplace(docComponent->getScoreFragment(index).get(), index);
	}
	return returnVal;
}

template <typename EventType>
void MusicScoreLoader__3_4_2::serializeCollectionOfMusicEvents(std::vector<uint8_t>& data, const CollectionOfMusicScoreEvents<EventType>* eventCollection, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap, bool serializeDefaultEvent,
	const std::function<void(std::vector<uint8_t>&, const EventType&, const std::unordered_map<const MusicScoreFragment*, uint32_t>&)>& eventSerializationFunction) const {
	using CalChart::Parser::Append;

	if (serializeDefaultEvent) {
		eventSerializationFunction(data, eventCollection->getDefaultEvent(), fragmentToIdMap);
	}
	Append(data, countNumEventsToSerialize<EventType>(eventCollection, fragmentToIdMap));
	for (auto validFragmentIterator = fragmentToIdMap.cbegin(); validFragmentIterator != fragmentToIdMap.cend(); validFragmentIterator++) {
		const MusicScoreFragment* fragment = (*validFragmentIterator).first;
		for (unsigned index = 0; index < eventCollection->getNumEvents(fragment); index++) {
			auto eventPair = eventCollection->getEvent(fragment, index);
			serializeMusicScoreMoment(data, eventPair.first, fragmentToIdMap);
			eventSerializationFunction(data, eventPair.second, fragmentToIdMap);
		}
	}
}

template <typename EventType>
uint32_t MusicScoreLoader__3_4_2::countNumEventsToSerialize(const CollectionOfMusicScoreEvents<EventType>* eventCollection, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	uint32_t totalEvents = 0;
	for (auto iterator = fragmentToIdMap.cbegin(); iterator != fragmentToIdMap.cend(); iterator++) {
		totalEvents += eventCollection->getNumEvents((*iterator).first);
	}
	return totalEvents;
}

MusicScoreDocComponent* MusicScoreLoader__3_4_2::loadFromData(const uint8_t*& data, const uint8_t* upperBound) const {
	std::unique_ptr<MusicScoreDocComponent> newMusicScore;
	newMusicScore.reset(new MusicScoreDocComponent());

	auto blocksTable = CalChart::Parser::ParseOutLabels(data, upperBound);

	auto makeParseFunction = [this](void (MusicScoreLoader__3_4_2::*funcPtr)(MusicScoreDocComponent*, const uint8_t*, size_t) const) {
		std::function<void(MusicScoreDocComponent*, const uint8_t*, size_t)> returnVal = [this, funcPtr](MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) {
			(this->*funcPtr)(buildTarget, blockToParse, blockSize);
		};
		return returnVal;
	};

	std::vector<std::pair<uint32_t, std::function<void(MusicScoreDocComponent*, const uint8_t*, size_t)>>> blocksToProcess = {
		{ INGL_MFRG, makeParseFunction(&MusicScoreLoader__3_4_2::parseFragments) },
		{ INGL_TSIG, makeParseFunction(&MusicScoreLoader__3_4_2::parseTimeSignatures) },
		{ INGL_MJMP, makeParseFunction(&MusicScoreLoader__3_4_2::parseJumps) },
		{ INGL_MTMP, makeParseFunction(&MusicScoreLoader__3_4_2::parseTempos) },
		{ INGL_MLBL, makeParseFunction(&MusicScoreLoader__3_4_2::parseBarLabels) }
	};

	for (unsigned blockIndex = 0; blockIndex < blocksToProcess.size(); blockIndex++) {
		int32_t blockLabelToProcess = blocksToProcess[blockIndex].first;
		bool blockProcessed = false;
		for (auto dataBlockInfo : blocksTable) {
			if (std::get<0>(dataBlockInfo) == blockLabelToProcess) {
				blockProcessed = true;
				const uint8_t* blockStart = std::get<1>(dataBlockInfo);
				size_t blockSize = std::get<2>(dataBlockInfo);
				blocksToProcess[blockIndex].second(newMusicScore.get(), blockStart, blockSize);
			}
		}
		if (!blockProcessed) {
			throw CC_FileException("Did not find required data block for MusicScoreDocComponent.");
		}
	}

	MusicScoreDocComponent* returnVal = newMusicScore.get();
	newMusicScore.release();
	return returnVal;
}

void MusicScoreLoader__3_4_2::parseFragments(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const {

}

MusicScoreFragment MusicScoreLoader__3_4_2::parseIndividualFragment(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const {
	std::string fragmentName;
	try {
		fragmentName = parseFirstStringWithinBounds(dataToParse, upperBound);
	}
	catch (CC_FileException) {
		throw CC_FileException("Could not parse Fragment.");
	}
	return MusicScoreFragment(fragmentName);
}

void MusicScoreLoader__3_4_2::parseTimeSignatures(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const {
	const std::function<TimeSignature(const MusicScoreDocComponent*, const uint8_t*&, const uint8_t*)> eventBuildFunction =
	[this](const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) {
		return parseIndividualTimeSignature(buildTarget, dataToParse, upperBound);
	};
	const std::function<void(MusicScoreDocComponent*, MusicScoreMoment, TimeSignature)> eventLoadFunction =
	[](MusicScoreDocComponent* buildTarget, MusicScoreMoment eventTime, TimeSignature eventObj) {
		buildTarget->getTimeSignatures()->addTimeSignatureChange(eventTime.fragment.get(), eventTime.beatAndBar.bar, eventObj);
	};
	const uint8_t* upperBound = blockToParse + blockSize;
	buildTarget->getTimeSignatures()->resetDefaultEvent(parseIndividualTimeSignature(buildTarget, blockToParse, upperBound));
	parseMusicScoreEvents(buildTarget, blockToParse, upperBound, eventBuildFunction, eventLoadFunction);
}

TimeSignature MusicScoreLoader__3_4_2::parseIndividualTimeSignature(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const {
	const size_t sizeInData = sizeof(uint32_t) / sizeof(uint8_t);
	if (!testIfAdequateDataRemains(dataToParse, upperBound, sizeInData)) {
		throw CC_FileException("Could not parse Time Signature.");
	}
	TimeSignature returnVal(get_big_long(dataToParse));
	dataToParse += sizeInData;
	return returnVal;
}

void MusicScoreLoader__3_4_2::parseJumps(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const {
	const std::function<MusicScoreJump(const MusicScoreDocComponent*, const uint8_t*&, const uint8_t*)> eventBuildFunction =
	[this](const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) {
		return parseIndividualJump(buildTarget, dataToParse, upperBound);
	};
	const std::function<void(MusicScoreDocComponent*, MusicScoreMoment, MusicScoreJump)> eventLoadFunction =
	[](MusicScoreDocComponent* buildTarget, MusicScoreMoment eventTime, MusicScoreJump eventObj) {
		buildTarget->getScoreJumps()->addJump(eventTime, eventObj);
	};
	const uint8_t* upperBound = blockToParse + blockSize;
	parseMusicScoreEvents(buildTarget, blockToParse, upperBound, eventBuildFunction, eventLoadFunction);
}

MusicScoreJump MusicScoreLoader__3_4_2::parseIndividualJump(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const {
	try {
		return MusicScoreJump(parseMusicScoreMoment(buildTarget, dataToParse, upperBound));
	} catch (CC_FileException) {
		throw CC_FileException("Could not parse Jump.");
	}
}

void MusicScoreLoader__3_4_2::parseTempos(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const {
	const std::function<MusicScoreTempo(const MusicScoreDocComponent*, const uint8_t*&, const uint8_t*)> eventBuildFunction =
	[this](const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) {
		return parseIndividualTempo(buildTarget, dataToParse, upperBound);
	};
	const std::function<void(MusicScoreDocComponent*, MusicScoreMoment, MusicScoreTempo)> eventLoadFunction =
	[](MusicScoreDocComponent* buildTarget, MusicScoreMoment eventTime, MusicScoreTempo eventObj) {
		buildTarget->getTempos()->addTempoChange(eventTime, eventObj);
	};
	const uint8_t* upperBound = blockToParse + blockSize;
	buildTarget->getTempos()->resetDefaultEvent(parseIndividualTempo(buildTarget, blockToParse, upperBound));
	parseMusicScoreEvents(buildTarget, blockToParse, upperBound, eventBuildFunction, eventLoadFunction);
}

MusicScoreTempo MusicScoreLoader__3_4_2::parseIndividualTempo(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const {
	const size_t sizeInData = sizeof(uint32_t) / sizeof(uint8_t);
	if (!testIfAdequateDataRemains(dataToParse, upperBound, sizeInData)) {
		throw CC_FileException("Could not parse Tempo.");
	}
	MusicScoreTempo returnVal(get_big_long(dataToParse));
	dataToParse += sizeInData;
	return returnVal;
}

void MusicScoreLoader__3_4_2::parseBarLabels(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const {
	const std::function<MusicScoreBarLabel(const MusicScoreDocComponent*, const uint8_t*&, const uint8_t*)> eventBuildFunction =
	[this](const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) {
		return parseIndividualBarLabel(buildTarget, dataToParse, upperBound);
	};
	const std::function<void(MusicScoreDocComponent*, MusicScoreMoment, MusicScoreBarLabel)> eventLoadFunction =
	[](MusicScoreDocComponent* buildTarget, MusicScoreMoment eventTime, MusicScoreBarLabel eventObj) {
		buildTarget->getBarLabels()->addBarLabel(eventTime.fragment.get(), eventTime.beatAndBar.bar, eventObj);
	};
	const uint8_t* upperBound = blockToParse + blockSize;
	parseMusicScoreEvents(buildTarget, blockToParse, upperBound, eventBuildFunction, eventLoadFunction);
}

MusicScoreBarLabel MusicScoreLoader__3_4_2::parseIndividualBarLabel(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const {
	std::string barLabel;
	try {
		barLabel = parseFirstStringWithinBounds(dataToParse, upperBound);
	} catch (CC_FileException) {
		throw CC_FileException("Could not parse Bar Label.");
	}
	return MusicScoreBarLabel(barLabel);
}

MusicScoreMoment MusicScoreLoader__3_4_2::parseMusicScoreMoment(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const {
	const size_t sizeInData = (sizeof(uint32_t)+sizeof(int32_t)* 2) / sizeof(uint8_t);
	if (!testIfAdequateDataRemains(dataToParse, upperBound, sizeInData)) {
		throw CC_FileException("Could not parse Music Score Moment.");
	}
	uint32_t fragmentId = get_big_long(dataToParse);
	dataToParse += sizeof(uint32_t) / sizeof(uint8_t);
	int32_t barNumber = get_big_long(dataToParse);
	dataToParse += sizeof(int32_t) / sizeof(uint8_t);
	int32_t beatNumber = get_big_long(dataToParse);
	dataToParse += sizeof(int32_t) / sizeof(uint8_t);
	return MusicScoreMoment(buildTarget->getScoreFragment(fragmentId).get(), barNumber, beatNumber);
}

template <typename EventType>
void MusicScoreLoader__3_4_2::parseMusicScoreEvents(MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound,
	std::function<EventType(const MusicScoreDocComponent*, const uint8_t*&, const uint8_t*)> eventBuildFunction,
	std::function<void(MusicScoreDocComponent*, MusicScoreMoment, EventType)> eventLoadFunction) const {

	if (!testIfAdequateDataRemains(dataToParse, upperBound, sizeof(uint32_t))) {
		throw CC_FileException("Could not determine how many events to parse.");
	}
	uint32_t numEventsToParse = get_big_long(dataToParse);
	dataToParse += sizeof(uint32_t) / sizeof(uint8_t);
	for (uint32_t parsedEvents = 0; parsedEvents < numEventsToParse; parsedEvents++) {
		MusicScoreMoment eventTime = parseMusicScoreMoment(buildTarget, dataToParse, upperBound);
		EventType newEvent = eventBuildFunction(buildTarget, dataToParse, upperBound);
		eventLoadFunction(buildTarget, eventTime, newEvent);
	}
	if (!testIfAllDataUsed(dataToParse, upperBound)) {
		throw CC_FileException("Unexpected data.");
	}
}

bool MusicScoreLoader__3_4_2::testIfAdequateDataRemains(const uint8_t* lowerBound, const uint8_t* upperBound, size_t requiredBytes) const {
	return (upperBound >= lowerBound && upperBound - lowerBound >= requiredBytes);
}

bool MusicScoreLoader__3_4_2::testIfAllDataUsed(const uint8_t* lowerBound, const uint8_t* upperBound) const {
	return lowerBound == upperBound;
}