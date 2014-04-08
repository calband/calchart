#ifndef _BEATS_FILE_H_
#define _BEATS_FILE_H_

#include <wx\stream.h>
#include "core\BeatInfo.h"

class BeatsFileHandler {
public:
	static void exportBeatsFile(wxOutputStream* outputStream, BeatInfo* beatInfo);
	static BeatInfo* importBeatsFile(wxInputStream* inputStream);
};

#endif