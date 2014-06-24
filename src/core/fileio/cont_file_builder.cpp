#include "cont_file_builder.h"

ContCollection::ContCollection(const CC_sheet& sheet) {
	for (SYMBOL_TYPE currSymbol = SYMBOLS_START; currSymbol < MAX_NUM_SYMBOLS; currSymbol = (SYMBOL_TYPE)(currSymbol + 1)) {
		if (sheet.SelectPointsBySymbol(currSymbol).size() > 0) {
			std::string contText = sheet.GetContinuityBySymbol(currSymbol).GetText();
			yyinputbuffer = contText.c_str();
			parsecontinuity();
			if (ParsedContinuity != nullptr) {
				mDotTypes.push_back(currSymbol);
				mProcedures.push_back(ParsedContinuity);
			}
		}
	}
}

ContCollection::~ContCollection() {
	for (int index = 0; index < mProcedures.size(); index++) {
		delete mProcedures[index];
	}
}

int ContCollection::getNumDotTypes() const {
	return mDotTypes.size();
}

ContProcedure* ContCollection::getContProcedures(int index) const {
	return mProcedures[index];
}

SYMBOL_TYPE ContCollection::getDotType(int index) const {
	return mDotTypes[index];
}

ContFileContBlockWriter::ContBlockData::ContBlockData()
: dotType(SYMBOLS_START), encompassesAllDotTypes(false), procedure(nullptr), printCont(""), hasKnownStartTime(false), startTime(0, 0), secondaryOrdering(0)
{}

ContFileContBlockWriter::ContBlockData::ContBlockData(SYMBOL_TYPE dotType, ContProcedure* procedure, std::string printCont, MeasureTime startTime, bool realStartTimeIsKnown, int secondaryOrdering)
: dotType(dotType), encompassesAllDotTypes(false), procedure(procedure), printCont(printCont), hasKnownStartTime(realStartTimeIsKnown), startTime(startTime), secondaryOrdering(secondaryOrdering)
{}

ContFileContBlockWriter::ContBlockData::~ContBlockData()
{}

bool ContFileContBlockWriter::ContBlockDataComparator::operator()(ContBlockData element1, ContBlockData element2) {
	int dotTypeCompare;
	if (element1.encompassesAllDotTypes || element2.encompassesAllDotTypes) {
		dotTypeCompare = 0;
	} else {
		dotTypeCompare = element1.dotType - element2.dotType;
	}
	if (dotTypeCompare != 0) {
		return dotTypeCompare < 0;
	}

	int timeCompare = element1.startTime.bar - element2.startTime.bar;
	if (timeCompare != 0) {
		return timeCompare < 0;
	}
	timeCompare = element1.startTime.beat - element2.startTime.beat;
	if (timeCompare != 0) {
		return timeCompare < 0;
	}
	
	return element1.secondaryOrdering - element2.secondaryOrdering < 0;
};


void ContFileContBlockWriter::writeContBlock(OUTSTREAM* output, const CalChartDoc* doc, int sheetIndex, std::string sheetNum, ContCollection& continuities, int sheetStartBeat) {
	std::vector<ContBlockData> blockData;
	makeContBlockData(blockData, doc, sheetIndex, continuities, sheetStartBeat, doc->GetMusicData()->getBeatsPerBar());
	writeContBlockData(output, blockData, sheetNum);
}

void ContFileContBlockWriter::makeContBlockData(std::vector<ContFileContBlockWriter::ContBlockData>& blockData, const CalChartDoc* doc, int sheetIndex, ContCollection& continuities, int sheetStartBeat, const MeasureData* measureData) {
	AnimateCompile* compiler = new AnimateCompile(*doc->getShow(), AnimationVariables());
	compiler->setCurrentSheet(doc->GetNthSheet(sheetIndex));
	for (int index = 0; index < continuities.getNumDotTypes(); index++) {
		if (symbolTypeUnused(continuities.getDotType(index), *doc->GetNthSheet(sheetIndex))) {
			continue;
		}
		int startBeat = sheetStartBeat;
		int secondary = 0;
		bool dotTypeFirstProcedureFinished = false;
		bool startBeatKnown = true;
		if (measureData->getNumEvents() == 0) {
			startBeatKnown = false;
		}
		ContProcedure* currProcedure = continuities.getContProcedures(index);
		while (currProcedure != nullptr) {
			if (!shouldAppendContinuityTextToLastEntry(currProcedure) || !dotTypeFirstProcedureFinished) {
				dotTypeFirstProcedureFinished = true;
				ContBlockData newData;
				if (startBeatKnown) {
					secondary = 0;
					newData = ContBlockData(continuities.getDotType(index), currProcedure, getPrintContinuity(currProcedure, compiler), measureData->getNumEvents() > 0 ? measureData->getMeasureTime(startBeat) : MeasureTime(0,0));
				}
				else {
					secondary++;
					newData = ContBlockData(continuities.getDotType(index), currProcedure, getPrintContinuity(currProcedure, compiler), measureData->getNumEvents() > 0 ? measureData->getMeasureTime(startBeat) : MeasureTime(0,0), false, secondary);
				}
				blockData.push_back(newData);
			} else {
				(*(blockData.end() - 1)).printCont += getPrintContinuity(currProcedure, compiler, false);
			}
			if (hasExplicitDuration(currProcedure)) {
				startBeat += getExplicitDuration(currProcedure, compiler);
			} else {
				startBeatKnown = false;
			}
			
			currProcedure = currProcedure->next;
		}
	}
	delete compiler;
	mergeSimilarInstructions(continuities, blockData);
	removeRedundantTiming(blockData);
}

void ContFileContBlockWriter::writeContBlockData(OUTSTREAM* output, std::vector<ContBlockData> data, std::string sheetNum) {
	std::vector<std::string> thisRowData, lastRowData, mergedRowData;
	for (int index = 0; index < 4; index++) {
		lastRowData.push_back("");
	}
	for (int index = 0; index < data.size(); index++) {
		thisRowData.push_back(sheetNum);
		thisRowData.push_back(getTimeString(data[index]));
		thisRowData.push_back(getSymbolString(data[index]));
		thisRowData.push_back(getContString(data[index]));
		int startRecordIndex = thisRowData.size() - 1;
		for (int col = 0; col < thisRowData.size(); col++) {
			if (thisRowData[col] == "") {
				thisRowData[col] = lastRowData[col];
			}
			if (thisRowData[col] != lastRowData[col]) {
				startRecordIndex = col;
				break;
			}
		}
		for (int col = 0; col < startRecordIndex; col++) {
			mergedRowData.push_back("");
		}
		for (int col = startRecordIndex; col < thisRowData.size(); col++) {
			mergedRowData.push_back(thisRowData[col]);
		}
		ContFileStreamer::addLine(output, mergedRowData);
		lastRowData = thisRowData;
		mergedRowData.clear();
		thisRowData.clear();
	}
}

std::string ContFileContBlockWriter::getTimeString(ContBlockData& data) {
	if (data.hasKnownStartTime) {
		return std::to_string(data.startTime.bar + 1) + "/" + std::to_string(data.startTime.beat + 1);
	}
	else {
		return "";
	}
}
std::string ContFileContBlockWriter::getSymbolString(ContBlockData& data) {
	if (data.encompassesAllDotTypes) {
		return "All:";
	}
	switch (data.dotType) {
	case SYMBOL_PLAIN:
		return "\\po:";
	case SYMBOL_BKSL:
		return "\\pb:";
	case SYMBOL_SL:
		return "\\ps:";
	case SYMBOL_X:
		return "\\px:";
	case SYMBOL_SOL:
		return "\\so:";
	case SYMBOL_SOLBKSL:
		return "\\sb:";
	case SYMBOL_SOLSL:
		return "\\ss:";
	case SYMBOL_SOLX:
		return "\\sx:";
	default:
		return "ERR: UNKNOWN_DOT_TYPE";
	}
}

std::string ContFileContBlockWriter::getContString(ContBlockData& data) {
	return data.printCont;
}

bool ContFileContBlockWriter::symbolTypeUnused(SYMBOL_TYPE dotType, const CC_sheet& sheet) {
	for (CC_point point : sheet.GetPoints()) {
		if (point.GetSymbol() == dotType) {
			return false;
		}
	}
	return true;
}

void ContFileContBlockWriter::sortBlockData(std::vector<ContBlockData>& data) {
	std::sort(data.begin(), data.end(), ContBlockDataComparator());
}

void ContFileContBlockWriter::mergeSimilarInstructions(ContCollection& continuities, std::vector<ContBlockData>& data) {
	if (continuities.getNumDotTypes() < 2) {
		return;
	}
	std::vector<ContBlockData> newBlocks;
	bool mergerFound = true;
	//Keep trying to merge instructions until no new mergers can be established
	while (mergerFound) {
		mergerFound = false;
		for (int index = 0; index < data.size() && data[index].dotType == continuities.getDotType(0); index++) {
			if (!data[index].hasKnownStartTime) {
				continue;
			}
			//Find all instructions which could be combined with the current continuity
			int expectedDotTypeIndex = 1;
			std::vector<int> mergeIndexes;
			mergeIndexes.push_back(index);
			for (int index2 = index + 1; index2 < data.size() && expectedDotTypeIndex < continuities.getNumDotTypes(); index2++) {
				if (data[index2].dotType == continuities.getDotType(expectedDotTypeIndex) 
					&& data[index2].hasKnownStartTime 
					&& data[index].startTime.bar == data[index2].startTime.bar && data[index].startTime.beat == data[index2].startTime.beat
					&& data[index].printCont == data[index2].printCont) {
					mergeIndexes.push_back(index2);
					expectedDotTypeIndex++;
				}
			}
			//Merge by adding a new 'all' block to newBlocks (to be added to the  final data later) and removing all individual elements which will be combined to make the 'all' block
			if (mergeIndexes.size() == continuities.getNumDotTypes()) {
				mergerFound = true;
				ContBlockData newAllBlock = data[mergeIndexes[0]];
				newAllBlock.encompassesAllDotTypes = true;
				newBlocks.push_back(newAllBlock);
				for (int mergeNum = mergeIndexes.size() - 1; mergeNum >= 0; mergeNum--) {
					data.erase(data.begin() + mergeIndexes[mergeNum]);
				}
			}
		}
	}
	//Add all of the 'all' blocks and make sure they end up in the right order
	for (int index = 0; index < newBlocks.size(); index++) {
		data.push_back(newBlocks[index]);
	}
	sortBlockData(data);
}

void ContFileContBlockWriter::removeRedundantTiming(std::vector<ContBlockData>& data) {
	for (int counter = 1; counter < data.size(); counter++) {
		if (data[counter].encompassesAllDotTypes == data[counter - 1].encompassesAllDotTypes && data[counter].dotType == data[counter - 1].dotType) {
			data[counter].hasKnownStartTime = false;
		}
	}
}

bool ContFileContBlockWriter::shouldAppendContinuityTextToLastEntry(ContProcedure* procedure) {
	return procedure->ShouldAppendPrintContinuityToPreviousLine();
}

std::string ContFileContBlockWriter::getPrintContinuity(ContProcedure* procedure, AnimateCompile* compiler, bool newLine) {
	return procedure->GetPrintContinuity(compiler, newLine);
}

bool ContFileContBlockWriter::hasExplicitDuration(ContProcedure* procedure) {
	return procedure->HasExplicitDuration();
}

int ContFileContBlockWriter::getExplicitDuration(ContProcedure* procedure, AnimateCompile* compiler) {
	return procedure->GetExplicitDuration(compiler);
}








void ContFileSheetWriter::writeContSheet(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum, std::string header) {
	writeSheetTag(output, doc, sheetNum);
	writeHeader(output, doc, sheetNum, header, calculateSheetStartTime(doc, sheetNum));
	writeContBlock(output, doc, sheetNum, calculateSheetStartBeat(doc, sheetNum));
	writeTotalBeats(output, doc, sheetNum);
}

void ContFileSheetWriter::writeSheetTag(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum) {
	ContFileFormatter::outputNewSheetTag(output, getSheetName(doc, sheetNum));
	writeEmptyLine(output);
}

void ContFileSheetWriter::writeHeader(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum, std::string header, MeasureTime sheetStartTime) {
	if (!header.empty()) {
		ContFileFormatter::outputMessage(output, header);
		writeEmptyLine(output);
	}
	writeSongTitle(output, doc, sheetNum, sheetStartTime);
}

void ContFileSheetWriter::writeSongTitle(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum, MeasureTime sheetStartTime) {
	std::string songTitle = getSongTitle(doc, sheetStartTime);
	if (!songTitle.empty()) {
		ContFileFormatter::outputMessage(output, ContFileFormatter::makeSongTitle(songTitle));
		writeEmptyLine(output);
	}
}


void ContFileSheetWriter::writeContBlock(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum, int sheetStartBeat) {
	ContFileContBlockWriter::writeContBlock(output, doc, sheetNum, getSheetName(doc, sheetNum), ContCollection(*(doc->GetNthSheet(sheetNum))), sheetStartBeat);
	writeEmptyLine(output);
}


void ContFileSheetWriter::writeTotalBeats(OUTSTREAM* output, const CalChartDoc* doc, int sheetNum) {
	ContFileFormatter::outputTotalBeatsLine(output, getNumBeats(doc, sheetNum));
	writeEmptyLine(output);
}


void ContFileSheetWriter::writeEmptyLine(OUTSTREAM* output) {
	ContFileStreamer::addLine(output, "");
}

std::string ContFileSheetWriter::getSongTitle(const CalChartDoc* doc, MeasureTime sheetStartTime) {
	const SongData* songs = doc->GetMusicData()->getSongs();
	for (int currSongNum = 0; currSongNum < songs->getNumEvents(); currSongNum++) {
		SongChangeEvent songChange = songs->getEvent(currSongNum);
		if (songChange.bar == sheetStartTime.bar && songChange.beat == sheetStartTime.beat) {
			return songChange.songName;
		}
	}
	return "";
}

std::string ContFileSheetWriter::getSheetName(const CalChartDoc* doc, int sheetIndex) {
	std::string sheetNum = getSheetNumber(doc, sheetIndex);
	if (sheetNum == "13") {
		return "Hogballs";
	}
	return sheetNum;
}

std::string ContFileSheetWriter::getSheetNumber(const CalChartDoc* doc, int sheetIndex) {
	return std::to_string(sheetIndex + 1);
}

int ContFileSheetWriter::getNumBeats(const CalChartDoc* doc, int sheetIndex) {
	return (doc->GetNthSheet(sheetIndex))->GetBeats();
}

MeasureTime ContFileSheetWriter::calculateSheetStartTime(const CalChartDoc* doc, int sheetIndex) {
	if (doc->GetMusicData()->getBeatsPerBar()->getNumEvents() > 0) {
		return doc->GetMusicData()->getBeatsPerBar()->getMeasureTime(calculateSheetStartBeat(doc, sheetIndex));
	} else {
		return MeasureTime(0, 0);
	}
}

int ContFileSheetWriter::calculateSheetStartBeat(const CalChartDoc* doc, int sheetIndex) {
	int startBeat = 0;
	for (int index = 0; index < sheetIndex; index++) {
		startBeat += getNumBeats(doc, index);
	}
	return startBeat;
}

void ContFileWriter::writeContFile(OUTSTREAM* output, const CalChartDoc* doc) {
	for (int sheetNum = 0; sheetNum < doc->GetNumSheets(); sheetNum++) {
		ContFileSheetWriter::writeContSheet(output, doc, sheetNum, getSheetHeader(doc, sheetNum));
	}
}

std::string ContFileWriter::getSheetHeader(const CalChartDoc* doc, int sheetIndex) {
	if (sheetIndex == 0) {
		return ContFileFormatter::makeOpening();
	}
	if (sheetIndex == doc->GetNumSheets() - 1) {
		return ContFileFormatter::makeClosing("___ FIXME: STUNT FILL THIS IN!!! ___");
	}
	else {
		return "";
	}
}