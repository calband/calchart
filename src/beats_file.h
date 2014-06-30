#ifndef _BEATS_FILE_H_
#define _BEATS_FILE_H_

#include <wx\stream.h>
#include "beat_info.h"

/**
 * Used to import/export beats files.
 */
class BeatsFileHandler {
public:
	/**
	 * Exports a beats file to the given stream.
	 * @param outputStream The stream to write the file output to.
	 * @param beatInfo The beats that will be written to the file.
	 */
	static void exportBeatsFile(wxOutputStream* outputStream, BeatInfo* beatInfo);
	/**
	 * Imports a beats file from the given stream.
	 * @param inputStream The stream to read the file from.
	 * @return The beats that were read from the file.
	 */
	static BeatInfo* importBeatsFile(wxInputStream* inputStream);
};

#endif