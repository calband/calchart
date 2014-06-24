#include "cont_file_formatter.h"

ContFileFormatTag ContFileFormatter::MESSAGE_TAGS_ARRAY[] = { BOLD, CENTERED };
std::vector<ContFileFormatTag> ContFileFormatter::MESSAGE_TAGS = std::vector<ContFileFormatTag>(ContFileFormatter::MESSAGE_TAGS_ARRAY, ContFileFormatter::MESSAGE_TAGS_ARRAY + 2);


void ContFileStreamer::addLine(OUTSTREAM* outputStream, std::vector<std::string> pieces) {
	for (int pieceCounter = 0; pieceCounter < pieces.size(); pieceCounter++) {
		addPieceToLine(outputStream, pieces[pieceCounter], pieceCounter);
	}
	endLine(outputStream);
}

void ContFileStreamer::addLine(OUTSTREAM* outputStream, std::string line) {
	addPieceToLine(outputStream, line, 0);
	endLine(outputStream);
}

std::string ContFileStreamer::formatMessage(std::string originalMessage, std::vector<ContFileFormatTag> formattingTags) {
	std::string finalMessage = originalMessage;
	for (int tagCounter = 0; tagCounter < formattingTags.size(); tagCounter++) {
		finalMessage = formatMessage(finalMessage, formattingTags[tagCounter]);
	}
	return finalMessage;
}

std::string ContFileStreamer::formatMessage(std::string originalMessage, ContFileFormatTag formattingTag) {
	switch (formattingTag) {
	case BOLD:
		return "\\bs" + originalMessage + "\\be";
	case ITALIC:
		return "\\is" + originalMessage + "\\ie";
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

std::string ContFileStreamer::formatLineFragment(std::string piece, int pieceNum) {
	if (pieceNum > 0) {
		return '\t' + piece;
	} else {
		return piece;
	}
}

void ContFileStreamer::addPieceToLine(OUTSTREAM* outputStream, std::string piece, int pieceNum) {
	piece = formatLineFragment(piece, pieceNum);
	writeToStream(outputStream, piece.c_str(), piece.length());
}

void ContFileStreamer::endLine(OUTSTREAM* outputStream) {
	std::string newlineChar = ContFileFormatter::makeNewLineCharacter();
	writeToStream(outputStream, newlineChar.c_str(), newlineChar.size());
}


void ContFileFormatter::outputNewSheetTag(OUTSTREAM* outputStream, std::string sheetName) {
	ContFileStreamer::addLine(outputStream, "%%" + sheetName);
}

void ContFileFormatter::outputTotalBeatsLine(OUTSTREAM* outputStream, int numBeats) {
	ContFileStreamer::addLine(outputStream, std::to_string(numBeats) + " Beats Total");
}

void ContFileFormatter::outputMessage(OUTSTREAM* outputStream, std::string message) {
	ContFileStreamer::addLine(outputStream, message);
}

std::string ContFileFormatter::makeClosing(std::string goBearsBeatTheWhat) {
	return ContFileStreamer::formatMessage("GO BEARS! BEAT THE " + goBearsBeatTheWhat, MESSAGE_TAGS);
}

std::string ContFileFormatter::makeOpening() {
	return ContFileStreamer::formatMessage("UNIVERSITY OF CALIFORNIA MARCHING BAND", MESSAGE_TAGS);
}

std::string ContFileFormatter::makeShowTitle(std::string showTitle) {
	return ContFileStreamer::formatMessage(showTitle, MESSAGE_TAGS);
}

std::string ContFileFormatter::makeSongTitle(std::string songTitle) {
	return ContFileStreamer::formatMessage(songTitle, MESSAGE_TAGS);
}

std::string ContFileFormatter::makeNewLineCharacter() {
	return "\r\n";
}
