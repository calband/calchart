#ifndef _CONT_FILE_BUILDER_H_
#define _CONT_FILE_BUILDER_H_

#include "calchartdoc.h"
#include "cont.h"
#include "cont_file_formatter.h"
#include "cc_sheet.h"
#include <string>
#include <wx\wfstream.h>

#define OUTSTREAM wxOutputStream
#define writeToStream(stream, bytes, size) stream->Write(bytes, size)

extern int parsecontinuity();
extern const char *yyinputbuffer;
extern ContProcedure *ParsedContinuity;

/**
 * A collection of the compiled ContProcedures for a sheet.
 */
class ContCollection {
public:
	/**
	 * Constructor.
	 * @param sheet The sheet to make the ContProcedures for.
	 */
	ContCollection(const CC_sheet& sheet);
	/**
	 * Destructor.
	 */
	~ContCollection();

	/**
	 * Returns the number of dot types associated with the collection.
	 * @return The number of dot types associated with the collection.
	 */
	int getNumDotTypes() const;
	/**
	 * Returns the first continuity procedure associated with a particular dot type. The rest of the continuity procedures for that dot type are found chained to the first.
	 * @param index The index associated with the dot type whose continuities are being retrieved.
	 * @return The first continuity procedure associated with a particular dot type.
	 */
	ContProcedure* getContProcedures(int index) const;
	/**
	 * Returns the dot type associated with a particular index.
	 * @param index The index to check.
	 * @return The dot type associated with the specified index.
	 */
	SYMBOL_TYPE getDotType(int index) const;
private:
	/**
	 * A vector containing all dot types that have ContProcedures in this collection.
	 */
	std::vector<SYMBOL_TYPE> mDotTypes;
	/**
	 * A vector containing the ContProcedure chains associated with each dot type.
	 */
	std::vector<ContProcedure*> mProcedures;
};

/**
 * Used to print a representation of a sheet's continuities to a continuity file.
 * This class is used through the ContFileSheetWriter class (which is, in turn, used through the ContFileWriter class) - it generally should not be used directly.
 */
class ContFileContBlockWriter {
public:
	/**
	 * Writes the block of text in a continuity file that describes the continuities for a stuntsheet.
	 * @param output The stream to write the file output to.
	 * @param doc The CalChartDoc with the show that we're making a continuity file for.
	 * @param sheetIndex The index of the sheet whose continuities are being written to the file output.
	 * @param sheetNum A string that has the number of the sheet.
	 * @param continuities A collection of all of the continuities for the sheet.
	 * @param sheetStartBeat The beat at which the sheet whose continuities are being written starts, relative to the beginning of the show. 
	 */
	static void writeContBlock(OUTSTREAM* output, const CalChartDoc* doc, int sheetIndex, std::string sheetNum, ContCollection& continuities, int sheetStartBeat);
private:
	/**
	 * Collects all information about a continuity that is relevant to the way it is expressed in a continuity file.
	 */
	struct ContBlockData {
		/**
		 * Makes an empty structure, where all values are essentially made null or empty.
		 */
		ContBlockData();
		/**
		 * Constructor.
		 * @param dotType The dot type which uses this continuity.
		 * @param procedure The actual continuity procedure object which this entry represents.
		 * @param printCont A string representation of the continuity, as would appear in a continuity file.
		 * @param startTime The bar and beat at which this continuity starts, if known.
		 * @param realStartTimeIsKnown Indicates whether or not the bar and beat at which the continuity starts are known. If this is true, startTime must be a valid time.
		 * @param secondaryOrdering The 'secondary' ordering priority of this entry in the final continuity file. Continuities are sorted mostly by their start times and dot types, but when a single dot type has multiple dots without known start times, their order will be from least to greatest secondaryOrdering.
		 */
		ContBlockData(SYMBOL_TYPE dotType, ContProcedure* procedure, std::string printCont, MeasureTime startTime, bool realStartTimeIsKnown = true, int secondaryOrdering = 0);

		/**
		 * Destructor.
		 */
		~ContBlockData();

		/**
		 * The dot type which uses this continuity.
		 */
		SYMBOL_TYPE dotType;
		/**
		 * True if the dot type should read "ALL".
		 */
		bool encompassesAllDotTypes;
		
		/**
		 * The ContProcedure which this entry represents.
		 */
		ContProcedure* procedure;
		/**
		 * The string representation of the continuity, as would appear in a continuity file.
		 */
		std::string printCont;

		/**
		 * True if the time at which this continuity starts is known. If false, the startTime member of this structure is likely invalid.
		 */
		bool hasKnownStartTime;
		/**
		 * Indicates where this continuity should be sorted in the final output ordering when compared against other continuities having the same start time and dot type.
		 * Lower secondaryOrdering values put a continuity earlier in the output.
		 */
		int secondaryOrdering;
		/**
		 * The time at which the continuity starts, relative to the beginning of the show.
		 */
		MeasureTime startTime;
	};

	/**
	 * Used as a function that compares two entries in the continuity block to tell which should come before the other in the final output.
	 */
	struct ContBlockDataComparator {
		/**
		 * Compares two entries in the continuity block and returns whether or not the first one should be expressed before the second in the final output.
		 * @param element1 The first entry.
		 * @param element2 The second entry.
		 * @return True if element1 should be expressed before element2 in the final output; false otherwise.
		 */
		bool operator() (ContBlockData element1, ContBlockData element2);
	};

	/**
	 * Transforms the original continuities of the sheet into a list of ContBlockData entries that describe them.
	 * @param dataDestination The list to place the ContBlockData entries into.
	 * @param doc The CalChartDoc containing the show that we're writing to the continuity file.
	 * @param sheetIndex The index of the sheet whose continuities are being written to the continuity file.
	 * @param sheetStartBeat The beat on which the sheet starts, relative to the start of the show.
	 * @param measureData All music-related information (used to figure out timing of the continuities with respect to the music).
	 */
	static void makeContBlockData(std::vector<ContBlockData>& dataDestination, const CalChartDoc* doc, int sheetIndex, ContCollection& continuities, int sheetStartBeat, const MeasureData* measureData);
	/**
	 * Writes a list of ContBlockData entries to the continuity file.
	 * @param output The stream to write the file output to.
	 * @param data A collection of all of the ContBlockData entries.
	 * @param sheetNum The sheet number to lead the continuity section with in the output file.
	 */
	static void writeContBlockData(OUTSTREAM* output, std::vector<ContBlockData> data, std::string sheetNum);

	/**
	 * Returns a string that will represent the timing of a continuity in the final output.
	 * @param data The data describing the target continuity.
	 */
	static std::string getTimeString(ContBlockData& data);
	/**
	 * Returns the string that will give the dot type associated with the continuity in the final output.
	 * @param data The data describing the target continuity.
	 */
	static std::string getSymbolString(ContBlockData& data);
	/**
	 * Returns the string that will describe the continuity in the final output.
	 * @param data The data describing the target continuity.
	 */
	static std::string getContString(ContBlockData& data);

	/**
	 * Returns true if the string representation of a continuity should be written to the end of the last continuity; false if it should start its own line.
	 * @param procedure The procedure to check. 
	 * @return True if the continuity should be written to the end of the previous line of the continuity file; false if it should start on a new line.
	 */
	static bool shouldAppendContinuityTextToLastEntry(ContProcedure* procedure);
	/**
	 * Returns the description of a continuity that will represent the continuity in the final file.
	 * @param procedure The procedure to describe.
	 * @param compiler A compiler that is required for calculating the continuity's description.
	 * @param newLine True if the print continuity for this procedure will start a new line; false if it will be appended to the end of the last line.
	 */
	static std::string getPrintContinuity(ContProcedure* procedure, AnimateCompile* compiler, bool newLine = true);
	/**
	 * Returns whether or not the duration of a particular procedure is known for all dots.
	 * @param procedure The procedure to check.
	 * @return True if the duration of the given procedure is known; false otherwise.
	 */
	static bool hasExplicitDuration(ContProcedure* procedure);
	/**
	 * Returns the duration of a particular procedure, if known.
	 * @param procedure The procedure to get the duration of.
	 * @param compiler A compiler, necessary to find the duration of a procedure.
	 * @return The duration of a particular procedure, in beats.
	 */
	static int getExplicitDuration(ContProcedure* procedure, AnimateCompile* compiler);

	/**
	 * Sorts the ContBlockData entries so that they will occur in the same order that they should in the output file.
	 * @param data The ContBlockData entries to sort.
	 */
	static void sortBlockData(std::vector<ContBlockData>& data);

	/**
	 * Returns true if there are no dots corresponding to the specified dot type.
	 * @param dotType The dot type to check.
	 * @param sheet The sheet to check.
	 * @return True if there are no dots corresponding to the specified dot type; false otherwise.
	 */
	static bool symbolTypeUnused(SYMBOL_TYPE dotType, const CC_sheet& sheet);

	/**
	 * Merges instructions into an "All" dot type, if all dot types share the same behavior at the same time.
	 * @param continuities The compiled continuities.
	 * @param data The ContBlockData entries that will be written to the output file.
	 */
	static void mergeSimilarInstructions(ContCollection& continuities, std::vector<ContBlockData>& data);
	/**
	 * Makes sure that only the first continuity for each dot type will have its timing specified in the output file (as opposed to all of them).
	 * @param data The ContBlockData entries that will be written to the output file.
	 */
	static void removeRedundantTiming(std::vector<ContBlockData>& data);
};

/**
 * Used to translate the contents of a particular sheet to its continuity file representation.
 * This should really be accessed through the ContFileWriter class - it should generally not be used on its own.
 */
class ContFileSheetWriter {
public:
	/**
	 * Writes the representation of a particular stuntsheet to a continuity file.
	 * @param output The stream that to write the file output to.
	 * @param doc The document containing the show with the target stuntsheet.
	 * @param sheetNum The index of the sheet to write to the file.
	 * @param header The message to add to the beginning of the page for the stuntsheet.
	 */
	static void writeContSheet(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum, std::string header);
private:
	/**
	 * Writes the tag to the output that marks the beginning of a new sheet.
	 * @param output The stream that we're writing the file output to.
	 * @param doc The CalChartDoc that we're making a continuity file for.
	 * @param sheetNum The index of the sheet that is being printed to the continuity file.
	 */
	static void writeSheetTag(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum);
	/**
	 * Writes a sheet's 'header' to the output, consisting of a message specified in the parameters and, if the sheet marks the start of a new song, that song's title.
	 * @param output The stream to write the file output to.
	 * @param doc The CalChartDoc that we're making a continuity file for.
	 * @param sheetNum The index of the sheet whose header is being written to the continuity file.
	 * @param header A message to begin the header with.
	 * @param sheetStartTime The starting beat and bar for the stunt sheet, relative to the start of the show.
	 */
	static void writeHeader(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum, std::string header, MeasureTime sheetStartTime);
	/**
	 * If the specified sheet markes the beginning of a song, writes the name of that song to the output.
	 * @param output The stream to write the file output to.
	 * @param doc The CalChartDoc that we're making a continuity file for.
	 * @param sheetNum The index of the sheet that might mark the beginning of a song.
	 * @param sheetStartTime The time at which the target stuntsheet starts, relative to the start of the show.
	 */
	static void writeSongTitle(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum, MeasureTime sheetStartTime);
	/**
	 * Writes the continuities for a sheet to the continuity file.
	 * @param output The stream to write the file output to.
	 * @param doc The CalChartDoc that we're making a continuity file for.
	 * @param sheetNum The index of the sheet whose continuities will be printed.
	 * @param sheetStartBeat The beat on which this sheet starts (relative to the start of the show).
	 */
	static void writeContBlock(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum, int sheetStartBeat);
	/**
	 * Writes a line to the continuity file describing the number of total beats in a stuntsheet.
	 * @param output The stream to write the file output to.
	 * @param doc The CalChartDoc that we're making a continuity file for.
	 * @param sheetNum The index of the sheet whose total number of beats will be printed.
	 */
	static void writeTotalBeats(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum);
	/**
	* Writes an empty line to the output file.
	* @param output The stream to write the file output to.
	*/
	static void writeEmptyLine(OUTSTREAM* output);

	/**
	 * If the specified time marks the start of a song, returns the name of that song.
	 * @param doc The CalChartDoc to get the song names from.
	 * @param sheetStartTime The time that might mark the start of a song.
	 * @return If the specified time marks the start of the song, then the name of the song; otherwise, an empty string.
	 */
	static std::string getSongTitle(const CalChartDoc* doc, MeasureTime sheetStartTime);
	/**
	 * Returns the name of a particular stuntsheet.
	 * @param doc The CalChartDoc that the sheet is from.
	 * @param sheetIndex The index of the target sheet.
	 * @return The name of the specified sheet.
	 */
	static std::string getSheetName(const CalChartDoc* doc, int sheetIndex);
	/**
	 * Returns a particular sheet's 'sheet number', in string form.
	 * @param doc The CalChartdoc that the sheet is from.
	 * @param sheetIndex the index of the target sheet.
	 * @return A string representation of a sheet's 'sheet number'.
	 */
	static std::string getSheetNumber(const CalChartDoc* doc, int sheetIndex);
	/**
	 * Returns the length of a particular sheet, in beats.
	 * @param doc The CalChartDoc that the sheet is from.
	 * @param sheetIndex The index of the target sheet.
	 * @return The length of the specified sheet, in beats.
	 */
	static int getNumBeats(const CalChartDoc* doc, int sheetIndex);

	/**
	 * Returns the beat and bar at which a particular sheet starts, relative to the start of the show..
	 * @param doc The CalChartDoc that the sheet is from.
	 * @param sheetIndex The index of the target sheet.
	 * @return The beat and bar at which the specified sheet starts.
	 */
	static MeasureTime calculateSheetStartTime(const CalChartDoc* doc, int sheetIndex);
	/**
	 * Returns the beat at which a sheet starts, relative to the start of the show.
	 * @param doc The CalChartDoc that the sheet is from.
	 * @param sheetIndex The index of the target sheet.
	 * @return The beat at which the sheet starts, relatie to the start of the show.
	 */
	static int calculateSheetStartBeat(const CalChartDoc* doc, int sheetIndex);
};

/**
 * This class is used to write a Continuity File.
 */
class ContFileWriter {
public:
	/**
	 * Generates a continuity file for a show.
	 * @param output The stream where the output file will be written.
	 * @param doc The show to generate a continuity file for.
	 */
	static void writeContFile(OUTSTREAM* output, const CalChartDoc* doc);
protected:
	/**
	 * Given a particular sheet, returns a message that should be printed at the top of its page.
	 * The first sheet, for example, begins with "THE UNIVERSITY OF CALIFORNIA MARCHING BAND."
	 * The last sheet says "GO BEARS! BEAT THE ___!."
	 * @param doc The document that we're making a continuity file for.
	 * @param sheetIndex The index of the sheet that we should give a header for.
	 * @return A message to print at the top of the page for a particular sheet.
	 */
	static std::string getSheetHeader(const CalChartDoc* doc, int sheetIndex);
};

#endif