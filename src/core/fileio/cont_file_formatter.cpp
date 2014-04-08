#include "cont_file_formatter.h"

void ContFileFormatter::addLine(std::ostream& outputStream, std::vector<std::string> pieces) {
	for (int pieceCounter = 0; pieceCounter < pieces.size(); pieceCounter++) {
		addPieceToLine(outputStream, pieces[pieceCounter], pieceCounter);
	}
}

void ContFileFormatter::addLine(std::ostream& outputStream, std::string line) {
	addPieceToLine(outputStream, line, 0);
	endLine(outputStream);
}

std::string ContFileFormatter::formatMessage(std::string originalMessage, std::vector<ContFileFormatTag> formattingTags) {
	std::string finalMessage = originalMessage;
	for (int tagCounter = 0; tagCounter < formattingTags.size(); tagCounter++) {
		finalMessage = formatMessage(finalMessage, formattingTags[tagCounter]);
	}
	return finalMessage;
}

std::string ContFileFormatter::formatMessage(std::string originalMessage, ContFileFormatTag formattingTag) {
	switch (formattingTag) {
	case BOLD:
		return "\bs" + originalMessage + "\be";
	case ITALIC:
		return "\is" + originalMessage + "\ie";
	case CENTERED:
		return "~" + originalMessage;
	case CONT_SHEET_ONLY:
		return "<" + originalMessage;
	case STUNT_SHEET_ONLY:
		return ">" + originalMessage;
	default:
		return originalMessage;
	}
}

std::string ContFileFormatter::formatLineFragment(std::string piece, int pieceNum) {
	if (pieceNum > 0) {
		return '\t' + piece;
	} else {
		return piece;
	}
}

void ContFileFormatter::addPieceToLine(std::ostream& outputStream, std::string piece, int pieceNum) {
	piece = formatLineFragment(piece, pieceNum);
	outputStream.write(piece.c_str(), piece.length);
}

void ContFileFormatter::endLine(std::ostream& outputStream) {
	char* newlineChar = "\n";
	outputStream.write(newlineChar, 1);
	delete newlineChar;
}