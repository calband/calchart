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
	 * Loads a MusicScoreDocComponent from a stream with serialized data.
	 * @param streamStart The start of the MusicScoreDocComponent block in the stream.
	 * @param streamEnd The end of the MusicScoreDocComponent block in the stream.
	 * @return A MusicScoreDocComponent, loaded from the stream.
	 */
	static MusicScoreDocComponent* loadFromVersionedStream(std::istream_iterator<uint8_t> streamStart, std::istream_iterator<uint8_t> streamEnd);

	/**
	 * Loads a MusicScoreDocComponent from serialized data. The data must begin with
	 * a version string, so that the MusicScoreLoader can choose the appropriate
	 * loader to interpret it.
	 * @param data The serialized data to load from.
	 * @param size The number of elements in the data array.
	 * @return A MusicScoreDocComponent, loaded from the serialized data; nullptr if unsuccessful.
	 */
	static MusicScoreDocComponent* loadFromVersionedData(const uint8_t* data, size_t size);

	/**
	 * Serializes a MusicScoreDocComponent as if saving it in the most recent file format.
	 * The version is encoded at the beginning of the returned data.
	 * @param docComponent The doc component to serialize.
	 * @return A serialized representation of the MusicScoreDocComponent.
	 */
	static std::vector<uint8_t> serializeForLatestVersion(const MusicScoreDocComponent* docComponent);

	/**
	* Loads the Music Score data from serialized data.
	* @param data The serialized data to load from. This pointer will be incremented to the address where the
	*   function stops reading.
	* @param upperBound The address of the end of the data.
	* @return a MusicScoreDocComponent loaded from the serialized data.
	*/
	virtual MusicScoreDocComponent* loadFromData(const uint8_t*& data, const uint8_t* upperBound) const = 0;

	/**
	* Serializes Music Score data so that it can be written to a file or stream.
	* @param target The vector to write the serialized MusicScoreDocComponent to.
	* @param docComponent The doc component to serialize.
	*/
	virtual void serialize(std::vector<uint8_t>& target, const MusicScoreDocComponent* docComponent) const = 0;
protected:
	/**
	 * Returns the address of the first null character in the buffer.
	 * @param buffer The beginning of the buffer.
	 * @param upperBound The end of the buffer.
	 * @return The address of the first null character in the buffer, or the address of
	 *   the upper bound if no null character was found.
	 */
	static const char* findFirstNullCharacter(const char* buffer, const char* upperBound);

	/**
	 * Parses a null-terminated string from the beginning of a data buffer, if such a string
	 * can be constructed between the beginning of the buffer and the upper bound.
	 * If such a string can't be constructed, an error will be thrown.
	 * @param buffer The data buffer to construct a string from. After the function completes, this
	 *   value will point just beyond the end of the null character that terminated the generated string.
	 * @param upperBound The upper bound on the buffer -- no string can be created with characters
	 *   beyond or at this address.
	 * @return A string constructed from the beginning of the data buffer, if possible.
	 */
	static std::string parseFirstStringWithinBounds(const uint8_t*& buffer, const uint8_t* upperBound);

	/**
	 * Returns the MusicScoreLoader associated with the given version string.
	 * @param version The version string.
	 * @return The MusicScoreLoader associated with the given version string; nullptr if no such
	 *   loader exists.
	 */
	static const MusicScoreLoader* getLoaderForVersion(std::string version);

	/**
	 * Returns the version string at the beginning of the data.
	 * @param data The serialized data to read from. This pointer will be incremented so that
	 *   it points past the end of the version string, if the function is successful.
	 * @param upperBound The address of the end of the data array.
	 * @return The version string at the beginning of the data.
	 */
	static std::string getVersionStringFromData(const uint8_t*& data, const uint8_t* upperBound);

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
	static std::unordered_map<std::string, std::shared_ptr<MusicScoreLoader>> VersionLoaders;

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
 *		ONE_TIME_SIGNATURE					= MUSIC_SCORE_MOMENT_DATA(timeAtWhichTimeSigChangeOccurs) , ONE_TIME_SIGNATURE_DATA;
 *		ONE_TIME_SIGNATURE_DATA				= BigEndianInt32(beatsPerBar);
 *		TIME_SIGNATURES_END					= INGL_END , INGL_TSIG
 *
 *		JUMPS								= INGL_MJMP , JUMPS_DATA , JUMPS_END;
 *		JUMPS_DATA							= BigEndianUnsignedInt32(numJumps) , {ONE_JUMP}*
 *		ONE_JUMP							= MUSIC_SCORE_MOMENT_DATA(jumpFrom) , ONE_JUMP_DATA;
 *		ONE_JUMP_DATA						= MUSIC_SCORE_MOMENT_DATA(jumpTo);
 *		JUMPS_END							= INGL_END , INGL_MJMP
 *
 *		TEMPOS								= INGL_MTMP, TEMPOS_DATA , TEMPOS_END;
 *		TEMPOS_DATA							= ONE_TEMPO(defaultTempo), BigEndianUnsignedInt32(numTempoChanges) , {ONE_TEMPO}*
 *		ONE_TEMPO							= MUSIC_SCORE_MOMENT_DATA(timeAtWhichTempoChangeOccurs) ,  ONE_TEMPO_DATA;
 *		ONE_TEMPO_DATA						= BigEndianInt32(beatsPerMinute);
 *		TEMPOS_END							= INGL_END , INGL_MTMP
 *
 *		BAR_LABELS							= INGL_MLBL , BAR_LABELS_DATA , BAR_LABELS_END;
 *		BAR_LABELS_DATA						= BigEndianUnsignedInt32(numBarLabels) , {ONE_BAR_LABEL}* 
 *		ONE_BAR_LABEL						= MUSIC_SCORE_MOMENT_DATA(barThatIsLabelled) , ONE_BAR_LABEL_DATA;
 *		ONE_BAR_LABEL_DATA					= null-terminated char*(barLabel);
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
	virtual MusicScoreDocComponent* loadFromData(const uint8_t*& data, const uint8_t* upperBound) const;
	
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
	 * Serializes ONE_TIME_SIGNATURE_DATA (see class description for details).
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
	 * Serializes ONE_JUMP_DATA (see class description for details).
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
	 * Serializes ONE_TEMPO_DATA (see class description for details).
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
	 * Serializes ONE_BAR_LABEL_DATA (see class description for details).
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
	uint32_t countNumEventsToSerialize(const CollectionOfMusicScoreEvents<EventType>* eventCollection, const std::unordered_map<const MusicScoreFragment*, uint32_t>& fragmentToIdMap) const;

	/**
	 * Loads FRAGMENT_DATA into the buildTarget (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param blockToParse The address of the beginning of the data block to load from.
	 * @param blockSize the size of the block to load from.
	 */
	virtual void parseFragments(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const;
	/**
	 * Loads ONE_FRAGMENT and returns the result (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param dataToParse The address of the beginning of the data block to load from. After this
	 *   function is called, this pointer will be incremented to point at the end of the data that
	 *   was read.
	 * @param upperBound The address of the end of the data that can be read by this function.
	 * @return The result of loading one object from the data.
	 */
	virtual MusicScoreFragment parseIndividualFragment(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const;
	/**
	 * Loads TIME_SIGNATURES_DATA into the buildTarget (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param blockToParse The address of the beginning of the data block to load from.
	 * @param blockSize the size of the block to load from.
	 */
	virtual void parseTimeSignatures(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const;
	/**
	 * Loads ONE_TIME_SIGNATURE_DATA and returns the result (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param dataToParse The address of the beginning of the data block to load from. After this
	 *   function is called, this pointer will be incremented to point at the end of the data that
	 *   was read.
	 * @param upperBound The address of the end of the data that can be read by this function.
	 * @return The result of loading one object from the data.
	 */
	virtual TimeSignature parseIndividualTimeSignature(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const;
	/**
	 * Loads JUMPS_DATA into the buildTarget (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param blockToParse The address of the beginning of the data block to load from.
	 * @param blockSize the size of the block to load from.
	 */
	virtual void parseJumps(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const;
	/**
	 * Loads ONE_JUMP_DATA and returns the result (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param dataToParse The address of the beginning of the data block to load from. After this
	 *   function is called, this pointer will be incremented to point at the end of the data that
	 *   was read.
	 * @param upperBound The address of the end of the data that can be read by this function.
	 * @return The result of loading one object from the data.
	 */
	virtual MusicScoreJump parseIndividualJump(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const;
	/**
	 * Loads TEMPOS_DATA into the buildTarget (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param blockToParse The address of the beginning of the data block to load from.
	 * @param blockSize the size of the block to load from.
	 */
	virtual void parseTempos(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const;
	/**
	 * Loads ONE_TEMPO_DATA and returns the result (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param dataToParse The address of the beginning of the data block to load from. After this
	 *   function is called, this pointer will be incremented to point at the end of the data that
	 *   was read.
	 * @param upperBound The address of the end of the data that can be read by this function.
	 * @return The result of loading one object from the data.
	 */
	virtual MusicScoreTempo parseIndividualTempo(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const;
	/**
	 * Loads BAR_LABELS_DATA into the buildTarget (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param blockToParse The address of the beginning of the data block to load from.
	 * @param blockSize the size of the block to load from.
	 */
	virtual void parseBarLabels(MusicScoreDocComponent* buildTarget, const uint8_t* blockToParse, size_t blockSize) const;
	/**
	 * Loads ONE_BAR_LABEL_DATA and returns the result (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param dataToParse The address of the beginning of the data block to load from. After this
	 *   function is called, this pointer will be incremented to point at the end of the data that
	 *   was read.
	 * @param upperBound The address of the end of the data that can be read by this function.
	 * @return The result of loading one object from the data.
	 */
	virtual MusicScoreBarLabel parseIndividualBarLabel(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const;
	/**
	 * Loads MUSIC_SCORE_MOMENT_DATA and returns the result (see class description for details).
	 * @param buildTarget The MusicScoreDocComponent that is being loaded from the data.
	 * @param dataToParse The address of the beginning of the data block to load from. After this
	 *   function is called, this pointer will be incremented to point at the end of the data that
	 *   was read.
	 * @param upperBound The address of the end of the data that can be read by this function.
	 * @return The result of loading one object from the data.
	 */
	virtual MusicScoreMoment parseMusicScoreMoment(const MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound) const;
	/**
	 * Builds all music events associated with a particular CollectionOfMusicScoreEvents object,
	 * and loads them into that collection.
	 * @param buildTarget The MusicScoreDocComponent being build from the serialized data.
	 * @param upperBound The end of the data that can be used to build these events.
	 * @param eventBuildFunction The function that will be used to build events from the serialized data.
	 * @param eventLoadFunction The function that will be used to load the built events onto the build target.
	 */
	template <typename EventType>
	void parseMusicScoreEvents(MusicScoreDocComponent* buildTarget, const uint8_t*& dataToParse, const uint8_t* upperBound,
		std::function<EventType(const MusicScoreDocComponent*, const uint8_t*&, const uint8_t*)> eventBuildFunction,
		std::function<void(MusicScoreDocComponent*, MusicScoreMoment, EventType)> eventLoadFunction) const;

	/**
	 * Tests to see if the amount of data between two bounds is at least equal to a
	 * given number of bytes.
	 * @param lowerBound The lower bound for the data.
	 * @param upperBound The upper bound for the data.
	 * @param requiredBytes The number of bytes that should be between the lower and upper bounds.
	 * @return True if there are at least requiredBytes between the lower and upper bounds; false otherwise.
	 */
	bool testIfAdequateDataRemains(const uint8_t* lowerBound, const uint8_t* upperBound, size_t requiredBytes) const;

	/**
	 * Tests to see if there is any unprocessed data between the lower bound and upper bound.
	 * @param lowerBound The lower bound.
	 * @param upperBound The upper bound.
	 * @return True if there is any unprocessed data between the lower and upper bound; false otherwise.
	 */
	bool testIfAllDataUsed(const uint8_t* lowerBound, const uint8_t* upperBound) const;
};