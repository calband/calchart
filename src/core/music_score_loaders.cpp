#include "music_score_loaders.h"
#include "cc_fileformat.h"

std::string MusicScoreLoader::LatestVersionString = "3.4.2";

std::unordered_map<std::string, std::unique_ptr<MusicScoreLoader>> MusicScoreLoader::VersionLoaders = {
	{ "3.4.2", std::unique_ptr<MusicScoreLoader>(new MusicScoreLoader__3_4_2()) }
};

MusicScoreDocComponent* MusicScoreLoader::loadFromVersionedStream(std::istream& stream) {
	std::string versionString = getVersionStringFromStream(stream);
	const MusicScoreLoader* loader = getLoaderForVersion(versionString);
	if (loader == nullptr) {
		return nullptr;
	}
	return loader->loadFromStream(stream);
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

std::string MusicScoreLoader::getVersionStringFromStream(std::istream& stream) {

}

void MusicScoreLoader::serializeVersionString(std::vector<uint8_t>& data, std::string versionString) {
	using CalChart::Parser::AppendAndNullTerminate;
	AppendAndNullTerminate(data, versionString);
}



MusicScoreDocComponent* MusicScoreLoader__3_4_2::loadFromStream(std::istream& stream) const;

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
	serializeCollectionOfMusicEvents(data, jumps, fragmentToIdMap, false,
		[this](std::vector<uint8_t>& data, const MusicScoreJump& thisJump, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) {
			this->serializeIndividualJump(data, thisJump, fragmentToIdMap);
		}
	);
}

void MusicScoreLoader__3_4_2::serializeIndividualJump(std::vector<uint8_t>& data, const MusicScoreJump& jump, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	serializeMusicScoreMoment(data, jump.jumpTo, fragmentToIdMap);
}

void MusicScoreLoader__3_4_2::serializeTempos(std::vector<uint8_t>& data, const MusicScoreTemposCollection* tempos, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	const std::function<void(std::vector<uint8_t>&, const MusicScoreTempo&, const std::unordered_map<const MusicScoreFragment*, uint32_t>&)> serializeFunc =
	[this](std::vector<uint8_t>& data, const MusicScoreTempo& thisTempo, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) {
		this->serializeIndividualTempo(data, thisTempo);
	};
	serializeCollectionOfMusicEvents(data, tempos, fragmentToIdMap, true, serializefunc;
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
	Append(data, (int32_t)(moment.beatAndBar.bar));
	Append(data, (int32_t)(moment.beatAndBar.beat));
}

std::unordered_map<const MusicScoreFragment*, uint32_t> MusicScoreLoader__3_4_2::buildFragmentToIndexMap(const MusicScoreDocComponent* docComponent) {
	std::unordered_map<const MusicScoreFragment*, uint32_t> returnVal;
	for (unsigned index = 0; index < docComponent->getNumScoreFragments(); index++) {
		returnVal.emplace(docComponent->getScoreFragment(index), index);
	}
	return returnVal;
}

template <typename EventType>
void MusicScoreLoader__3_4_2::serializeCollectionOfMusicEvents<EventType>(std::vector<uint8_t>& data, const CollectionOfMusicScoreEvents<EventType>* eventCollection, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap, bool serializeDefaultEvent,
	const std::function<void(std::vector<uint8_t>&, const EventType&, const std::unordered_map<const MusicScoreFragment*, uint32_t>&)>& eventSerializationFunction) const {
	using CalChart::Parser::Append;

	if (serializeDefaultEvent) {
		eventSerializationFunction(data, eventCollection->getDefaultEvent(), fragmentToIdMap);
	}
	Append(data, countNumEventsToSerialize<EventType>(eventCollection, fragmentToIdMap));
	for (auto validFragmentIterator = fragmentToIdMap.cbegin(), validFragmentIterator < fragmentToIdMap.cend(), validFragmentIterator++) {
		const MusicScoreFragment* fragment = (*validFragmentIterator).first;
		for (unsigned index = 0; index < eventCollection->getNumEvents(fragment); index++) {
			auto eventPair = eventCollection->getEvent(fragment, index);
			serializeMusicScoreMoment(data, eventPair.first, fragmentToIdMap);
			eventSerializationFunction(data, eventPair.second, fragmentToIdMap);
		}
	}
}

template <typename EventType>
uint32_t MusicScoreLoader__3_4_2::countNumEventsToSerialize<EventType>(const CollectionOfMusicScoreEvents<EventType>* eventCollection, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const {
	uint32_t totalEvents = 0;
	for (auto iterator = fragmentToIdMap.cbegin(), iterator < fragmentToIdMap.cend(); iterator++) {
		totalEvents += eventCollection->getNumEvents((*iterator).first);
	}
	return totalEvents;
}