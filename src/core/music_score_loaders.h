#pragma once

#include <unordered_map>
#include <functional>
#include "music_score_doc_component.h"


/**
 * The base class of all objects responsible for saving and loading the MusicScoreDocComponent.
 * The idea here is to separate the saving/loading of the MusicScoreDocComponent from
 * the real functions that it performs while it's being used by CalChart. Since we need to support
 * the saving/loading of old file versions, we can eliminate a lot of clutter from the MusicScoreDocComponent
 * by moving all of the saving/loading code for all file versions over here.
 * The MusicScoreLoader base class has static methods that are used to load from any file. Those static methods
 * figure out the version of the file being loaded, and then use that information to select the appropriate
 * MusicScoreLoader derived class to take over the loading process for that particular version. Each child class
 * of the MusicScoreLoader should only be concerned with loading a specific version of a file.
 * Usually, changes made to the file format over time are relatively small -- for this reason, loaders can be particularly
 * useful. Loaders for later versions can be derived from loaders for earlier versions, and can thus inherit most of
 * the same functionality. They can load most parts the same way, but override a few methods so that the differences
 * in the file format are accounted for. This allows the programmers to change only small amounts of code to support file
 * format changes while maintaining backward compatibility. 
 * When the file format is changed, the following steps should be made to register those changes with CalChart so that it
 * can properly load them:
 *     1. Derive a new MusicScoreLoader class from either the MusicScoreLoader base class or one of its child classes that can
 *        load and save the new file format.
 *     2. Add a mapping to the static variable called VersionLoaders in the MusicScoreLoader class. The mapping should map a unique version
 *        string to an instance of the new MusicScoreLoader class.
 *     3. Let the MusicScoreLoader that you have just changed the file format, so that when it saves new files, it will use the newest version
 *        instead of an older one. Do this by setting the static variable called LatestVersionString in the MusicScoreLoader class to the
 *        version string used in the mapping created in step 2.
 */
class MusicScoreLoader {
public:

	/**
	 * Loads a MusicScoreDocComponent from a stream. The stream must begin with
	 * a version string, so that the MusicScoreLoader can choose the appropriate
	 * loader to interpret it.
	 * @param stream The stream of data, beginning with a version string.
	 * @return A MusicScoreDocComponent, loaded from the stream; nullptr if unsuccessful.
	 */
	static MusicScoreDocComponent* loadFromVersionedStream(std::istream& stream);

	/**
	 * Serializes a MusicScoreDocComponent as if saving it in the most recent file format.
	 * The version is encoded at the beginning of the returned data.
	 * @param docComponent The doc component to serialize.
	 * @return A serialized representation of the MusicScoreDocComponent.
	 */
	static std::vector<uint8_t> serializeForLatestVersion(const MusicScoreDocComponent* docComponent);

	/**
	* Loads the Music Score data from a stream.
	* @param stream The stream, which does NOT come with a version string prepended.
	* @return a MusicScoreDocComponent loaded from the stream.
	*/
	virtual MusicScoreDocComponent* loadFromStream(std::istream& stream) const = 0;

	/**
	* Serializes Music Score data so that it can be written to a file or stream.
	* @param target The vector to write the serialized MusicScoreDocComponent to.
	* @param docComponent The doc component to serialize.
	*/
	virtual void serialize(std::vector<uint8_t>& target, const MusicScoreDocComponent* docComponent) const = 0;
protected:
	/**
	 * Returns the MusicScoreLoader associated with the given version string.
	 * @param version The version string.
	 * @return The MusicScoreLoader associated with the given version string; nullptr if no such
	 *   loader exists.
	 */
	static const MusicScoreLoader* getLoaderForVersion(std::string version);

	/**
	 * Returns the version string at the beginning of the stream.
	 * @param stream The stream with the version string.
	 * @return The version string at the beginning of the stream.
	 */
	static std::string getVersionStringFromStream(std::istream& stream);

	/**
	 * Serializes the version string.
	 * @param data The vector into which the serialized version string will be placed.
	 * @param versionString The version string to serialize.
	 */
	static void serializeVersionString(std::vector<uint8_t>& data, std::string versionString);

	/**
	 * A collection of mappings from version strings to the MusicScoreLoader objects responsible for
	 * saving and loading files of those versions.
	 */
	static std::unordered_map<std::string, std::unique_ptr<MusicScoreLoader>> VersionLoaders;

	/**
	 * The version string of the latest file format. This is the version that will be conformed to when
	 * the MusicScoreLoader serializes a MusicScoreDocComponent.
	 */
	static std::string LatestVersionString;
};

/**
 * A loader for the MusicScoreDocComponent for CalChart version 3.4.2.
 *
 * The format for saving the MusicScoreDocComponent in version 3.4.2 is as follows:
 *
 *		MUSIC_SCORE_DATA					= FRAGMENTS , TIME_SIGNATURES , JUMPS , TEMPOS , BAR_LABELS;
 *
 *		FRAGMENTS							= INGL_MFRG , {FRAGMENTS_DATA}* , FRAGMENTS_END;
 *		FRAGMENTS_DATA						= BigEndianUnsignedInt32(numFragments) {ONE_FRAGMENT}*;
 *		ONE_FRAGMENT						= null-terminated char*
 *		FRAGMENTS_END						= INGL_END , INGL_MFRG
 *
 *		TIME_SIGNATURES						= INGL_TSIG , TIME_SIGNATURES_DATA , TIME_SIGNATURES_END;
 *		TIME_SIGNATURES_DATA				= ONE_TIME_SIGNATURE(defaultTimeSignature) , BigEndianUnsignedInt32(numTimeSignatures) , {ONE_TIME_SIGNATURE}* ;
 *		ONE_TIME_SIGNATURE					= MUSIC_SCORE_MOMENT_DATA(timeAtWhichTimeSigChangeOccurs) , BigEndianInt32(beatsPerBar);
 *		TIME_SIGNATURES_END					= INGL_END , INGL_TSIG
 *
 *		JUMPS								= INGL_MJMP , JUMPS_DATA , JUMPS_END;
 *		JUMPS_DATA							= BigEndianUnsignedInt32(numJumps) , {ONE_JUMP}*
 *		ONE_JUMP							= MUSIC_SCORE_MOMENT_DATA(jumpFrom) , MUSIC_SCORE_MOMENT_DATA(jumpTo);
 *		JUMPS_END							= INGL_END , INGL_MJMP
 *
 *		TEMPOS								= INGL_MTMP, TEMPOS_DATA , TEMPOS_END;
 *		TEMPOS_DATA							= ONE_TEMPO(defaultTempo), BigEndianUnsignedInt32(numTempoChanges) , {ONE_TEMPO}*
 *		ONE_TEMPO							= MUSIC_SCORE_MOMENT_DATA(timeAtWhichTempoChangeOccurs) ,  BigEndianInt32(beatsPerMinute);
 *		TEMPOS_END							= INGL_END , INGL_MTMP
 *
 *		BAR_LABELS							= INGL_MLBL , BAR_LABELS_DATA , BAR_LABELS_END;
 *		BAR_LABELS_DATA						= BigEndianUnsignedInt32(numBarLabels) , {ONE_BAR_LABEL}* 
 *		ONE_BAR_LABEL						= MUSIC_SCORE_MOMENT_DATA(barThatIsLabelled) , null-terminated char*(barLabel);
 *		BAR_LABELS_END						= INGL_END , INGL_MLBL
 *
 *		MUSIC_SCORE_MOMENT_DATA				= FRAGMENT_ID , BAR_NUMBER , BEAT_NUMBER;
 *		FRAGMENT_ID							= BigEndianUnsignedInt32(indexOfFragment_fragmentZeroIsFirstFragmentSaved);
 *		BAR_NUMBER							= BigEndianInt32(barNumber)
 *		BEAT_NUMBER							= BigEndianInt32(beatNumber)
 *
 *		INGL_MFRG							= 'M','F','R','G';
 *		INGL_TSIG							= 'M','T','S','G';
 *		INGL_MJMP							= 'M','J','M','P';
 *		INGL_MTMP							= 'M','T','M','P';
 *		INGL_MLBL							= 'M','L','B','L';
 */
class MusicScoreLoader__3_4_2 : public MusicScoreLoader {
public:
	virtual MusicScoreDocComponent* loadFromStream(std::istream& stream) const;
	
	virtual void serialize(std::vector<uint8_t>& target, const MusicScoreDocComponent* docComponent) const;
protected:
	/**
	 * Serializes FRAGMENT_DATA (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param docComponent The MusicScoreDocComponent that is being serialized.
	 */
	virtual void serializeFragments(std::vector<uint8_t>& data, const MusicScoreDocComponent* docComponent) const;
	/**
	 * Serializes ONE_FRAGMENT (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param fragment The fragment to serialize.
	 */
	virtual void serializeIndividualFragment(std::vector<uint8_t>& data, const MusicScoreFragment& fragment) const;

	/**
	 * Serializes TIME_SIGNATURES_DATA (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param timeSignatures The collection of all time signatures to serialize.
	 * @param fragmentToIdMap A mapping from fragment pointer to id. The 'id' is the index of the fragment in
	 *   the MusicScoreDocComponent.
	 */
	virtual void serializeTimeSignatures(std::vector<uint8_t>& data, const TimeSignaturesCollection* timeSignatures, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const;
	/**
	 * Serializes ONE_TIME_SIGNATURE (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param timeSignature The time signature to serialize.
	 */
	virtual void serializeIndividualTimeSignature(std::vector<uint8_t>& data, const TimeSignature& timeSignature) const;

	/**
	 * Serializes JUMPS_DATA (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param jumps The collection of score jumps to serialize.
	 * @param fragmentToIdMap A mapping from fragment pointer to id. The 'id' is the index of the fragment in
	 *   the MusicScoreDocComponent.
	 */
	virtual void serializeJumps(std::vector<uint8_t>& data, const MusicScoreJumpsCollection* jumps, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const;
	/**
	 * Serializes ONE_JUMP (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param jump The jump to serialize.
	 * @param fragmentToIdMap A mapping from fragment pointer to id. The 'id' is the index of the fragment in
	 *   the MusicScoreDocComponent.
	 */
	virtual void serializeIndividualJump(std::vector<uint8_t>& data, const MusicScoreJump& jump, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const;

	/**
	 * Serializes TEMPOS_DATA (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param tempos The collection of tempo changes to serialize.
	 * @param fragmentToIdMap A mapping from fragment pointer to id. The 'id' is the index of the fragment in
	 *   the MusicScoreDocComponent.
	 */
	virtual void serializeTempos(std::vector<uint8_t>& data, const MusicScoreTemposCollection* tempos, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const;
	/**
	 * Serializes ONE_TEMPO (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param tempo The tempo to serialize.
	 */
	virtual void serializeIndividualTempo(std::vector<uint8_t>& data, const MusicScoreTempo& tempo) const;

	/**
	 * Serializes BAR_LABELS_DATA (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param barLabels The collection of bar labels to serialize.
	 * @param fragmentToIdMap A mapping from fragment pointer to id. The 'id' is the index of the fragment in
	 *   the MusicScoreDocComponent.
	 */
	virtual void serializeBarLabels(std::vector<uint8_t>& data, const MusicScoreBarLabelsCollection* barLabels, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const;
	/**
	 * Serializes ONE_BAR_LABEL (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param barLabel The bar label to serialize.
	 */
	virtual void serializeIndividualBarLabel(std::vector<uint8_t>& data, const MusicScoreBarLabel& barLabel) const;

	/**
	 * Serializes MUSIC_SCORE_MOMENT_DATA (see class description for details).
	 * @param data The vector to which the serialized data should be written.
	 * @param moment The moment to serialize.
	 * @param fragmentToIdMap A mapping from fragment pointer to id. The 'id' is the index of the fragment in
	 *   the MusicScoreDocComponent.
	 */
	virtual void serializeMusicScoreMoment(std::vector<uint8_t>& data, const MusicScoreMoment& moment, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const;

	/**
	 * Builds a map from score fragment to its index.
	 * @param docComponent The MusicScoreDocComponent containing the fragments.
	 * @return A map from each score fragment in the MusicScoreDocComponent to its index.
	 */
	std::unordered_map<const MusicScoreFragment*, uint32_t> buildFragmentToIndexMap(const MusicScoreDocComponent* docComponent) const;

	/**
	 * Serializes the events in a CollectionOfMusicScoreEvents. Only events associated with valid fragments
	 * are serialized. Valid fragments are those that appear in the mapping from fragment to fragment id.
	 * The collection in serialized in the following order:
	 *		eventSerializationFunction(defaultEvent)   (if serializing default event is enabled)
	 *		BigEndianUnsignedInt32(number of events)
	 *		for each event in collection:
	 *			eventSerializationFunction(event in collection)
	 * @param <EventType> The type of event held in the CollectionOfMusicEvents.
	 * @param data The vector to which the serialized data should be written.
	 * @param eventCollection The collection of events to serialize.
	 * @param fragmentToIdMap A mapping from fragment pointer to id. The 'id' is the index of the fragment in
	 *   the MusicScoreDocComponent.
	 * @param serializeDefaultEvent True if the default event should be serialized; false otherwise.
	 * @param eventSerializationFunction The function used to serialize an individual event in the
	 *   CollectionOfMusicScoreEvents.
	 */
	template <typename EventType>
	void serializeCollectionOfMusicEvents(std::vector<uint8_t>& data, const CollectionOfMusicScoreEvents<EventType>* eventCollection, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap, bool serializeDefaultEvent,
		const std::function<void(std::vector<uint8_t>&, const EventType&, const std::unordered_map<const MusicScoreFragment*, uint32_t>&)>& eventSerializationFunction) const;

	/**
	 * Counts the number of events in a CollectionOfMusicScoreEvents that will be serialized.
	 * Events are only serialized if they are associated with a 'valid fragment', which is a fragment
	 * that is a key in the map from fragments to fragment ids.
	 * @param eventCollection The collection of events to serialize.
	 * @param fragmentToIdMap A mapping from fragment pointer to id. The 'id' is the index of the fragment in
	 *   the MusicScoreDocComponent.
	 * @return The number of events in the CollectionOfMusicScoreEvents that will be serialized.
	 */
	template <typename EventType>
	uint32_t countNumEventsToSerialize<EventType>(const CollectionOfMusicScoreEvents<EventType>* eventCollection, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const;
};