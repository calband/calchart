#ifndef _CONT_FILE_FORMATTER_H_
#define _CONT_FILE_FORMATTER_H_

#include <iostream>
#include <vector>
#include <wx\wfstream.h>

#define OUTSTREAM wxOutputStream
#define writeToStream(stream, bytes, size) stream->Write(bytes, size)

/**
 * An enumeration of the formatting that can be tagged to a message in the continuity file.
 */
enum ContFileFormatTag {
	FIRST_TAG = 0,
	BOLD = FIRST_TAG,
	ITALIC,
	CENTERED,
	CONT_SHEET_ONLY,
	STUNT_SHEET_ONLY,
	LAST_TAG
};

/**
 * A collection of very basic functions used for writing a continuity file. This class provides methods for adding formatting to a message,
 * and for writing lines to the output in pieces, which will be separated by tabs.
 */
class ContFileStreamer {
public:
	/**
	 * Writes a line to the ouput file, in pieces. The pieces are separated from one another by tabs.
	 * @param outputStream The stream to write the output to.
	 * @param pieces A collection of pieces to write to the output, which will be separated by tabs.
	 */
	static void addLine(OUTSTREAM* outputStream, std::vector<std::string> pieces);
	/**
	 * Writes a line to the output file.
	 * @param outputStream The stream to write the output to.
	 * @param line The line to write.
	 */
	static void addLine(OUTSTREAM* outputStream, std::string line);
	/**
	 * Formats a message by applying the given formatting options.
	 * @param originalMessage The message to format.
	 * @param formattingTags The formatting to apply.
	 * @return The formatted message.
	 */
	static std::string formatMessage(std::string originalMessage, std::vector<ContFileFormatTag> formattingTags);
	/**
	 * Formats a message by applying the given formatting option.
	 * @param originalMessage The message to format.
	 * @param formattingTag The formatting option to apply.
	 * @return The formatted message.
	 */
	static std::string formatMessage(std::string originalMessage, ContFileFormatTag formattingTag);
protected:
	/**
	 * Formats a piece of the line before adding it to the output.
	 * @param piece The piece to format.
	 * @param pieceNum The index of the piece.
	 * @return The formatted line fragment.
	 */
	static std::string formatLineFragment(std::string piece, int pieceNum);
	/**
	 * Writes a particular line piece to the output.
	 * @param outputStream The stream to write the line piece to.
	 * @param piece The piece to write to the output.
	 * @param pieceNum The index of the piece being written.
	 */
	static void addPieceToLine(OUTSTREAM* outputStream, std::string piece, int pieceNum);
	/**
	 * Ends the line.
	 * @param outputStream The stream to write the file output to.
	 */
	static void endLine(OUTSTREAM* outputStream);
};

/**
 * A collection of functions for outputing particular lines of the continuity file.
 */
class ContFileFormatter {
public:
	/**
	 * Prints a tag that marks the beginning of a new sheet.
	 * @param outputStream The stream to print the file to.
	 * @param sheetName The name of the new sheet.
	 */
	static void outputNewSheetTag(OUTSTREAM* outputStream, std::string sheetName);
	/**
	 * Prints a line out to the continuity file telling the number of beats in the sheet.
	 * @param outputStream The stream to print the file to.
	 * @param numBeats The number of beats in the sheet.
	 */
	static void outputTotalBeatsLine(OUTSTREAM* outputStream, int numBeats);
	/**
	 * Writes a text message to the start of a continuity file.
	 * @param outputStream The stream to print the file to.
	 * @param message The message to print.
	 */
	static void outputMessage(OUTSTREAM* outputStream, std::string message);
	/**
	 * Makes a closing message for a continuity file ("Go bears! Beat the ___!").
	 * @param goBearsBeatTheWhat The name to print in the blank above.
	 * @return A closing message for the continuity file.
	 */
	static std::string makeClosing(std::string goBearsBeatTheWhat);
	/**
	 * Makes the opening message for the continuity file, to be printed on the top of the first sheet.
	 * @return The opening message for the continuity file.
	 */
	static std::string makeOpening();
	/**
	 * Writes a message containing the title for the show.
	 * @param showTitle The title of the show.
	 * @return A message containing the title of the show.
	 */
	static std::string makeShowTitle(std::string showTitle);
	/**
	 * Writes a message containing the song title.
	 * @param songTitle The title of the song.
	 * @return A message containing the title of the song.
	 */
	static std::string makeSongTitle(std::string songTitle);
	static std::string makeNewLineCharacter();
private:
	/**
	 * An array of the formatting tags to apply to messages being written to a continuity file.
	 */
	static ContFileFormatTag MESSAGE_TAGS_ARRAY[];
	/**
	 * A vector version of MESSAGE_TAGS_ARRAY, so that the tags can be passed as parameters to ContFileStreamer::formatMessage.
	 */
	static std::vector<ContFileFormatTag> MESSAGE_TAGS;
};


#endif