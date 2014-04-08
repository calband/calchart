#ifndef _CONT_FILE_FORMATTER_H_
#define _CONT_FILE_FORMATTER_H_

#include <iostream>
#include <vector>

enum ContFileFormatTag {
	FIRST_TAG = 0,
	BOLD = FIRST_TAG,
	ITALIC,
	CENTERED,
	CONT_SHEET_ONLY,
	STUNT_SHEET_ONLY,
	LAST_TAG
};


class ContFileFormatter {
public:
	static void addLine(std::ostream& outputStream, std::vector<std::string> pieces);
	static void addLine(std::ostream& outputStream, std::string line);
	static std::string formatMessage(std::string originalMessage, std::vector<ContFileFormatTag> formattingTags);
	static std::string formatMessage(std::string originalMessage, ContFileFormatTag formattingTag);
protected:
	static std::string formatLineFragment(std::string piece, int pieceNum);
	static void addPieceToLine(std::ostream& outputStream, std::string piece, int pieceNum);
	static void endLine(std::ostream& outputStream);
};

#endif