#include "beats_file.h"
#include "core\fileio\json_file_formatter.h"
#include "core\BeatInfo.h"


extern int importJSON();
extern const char *json_inputbuffer;
extern JSONObjectValue *OutputMainObject;


void BeatsFileHandler::exportBeatsFile(wxOutputStream* outputStream, BeatInfo* beatInfo) {
	JSONObjectValue* mainObject = new JSONObjectValue();
	JSONObjectValue* metaObject = new JSONObjectValue();
	JSONArrayValue* beatsObject = new JSONArrayValue();
	metaObject->addValue("version", new JSONStringValue("1.0.0"));
	long lastBeatTime = 0;
	for (int index = 0; index < beatInfo->getNumBeats(); index++) {
		beatsObject->addValue(new JSONIntValue(beatInfo->getBeat(index) - lastBeatTime));
		lastBeatTime = beatInfo->getBeat(index);
	}
	mainObject->addValue("meta", metaObject);
	mainObject->addValue("beats", beatsObject);
	JSONFormatter::exportJSON(outputStream, *mainObject);
	delete mainObject;
}

BeatInfo* BeatsFileHandler::importBeatsFile(wxInputStream* inputStream) {
	char* input = new char[inputStream->GetSize()];
	inputStream->Read(input, inputStream->GetSize());
	json_inputbuffer = input;
	importJSON();
	BeatInfo* returnVal = new BeatInfo();
	long lastBeatTime = 0;
	JSONArrayValue* beatsArray = (JSONArrayValue*)(OutputMainObject->getContentValue("beats"));
	for (int index = 0; index < beatsArray->getNumContentValues(); index++) {
		lastBeatTime += ((JSONIntValue*)beatsArray->getContentValue(index))->getValue();
		returnVal->addBeat(lastBeatTime);
	}
	return returnVal;
}