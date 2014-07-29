#include "json_unit_tests.h"

#include "json_file_formatter.h"

#include <wx/wfstream.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

extern int importJSON();
extern const char *json_inputbuffer;
extern JSONObjectValue *OutputMainObject;

void copyJSONFile(wxString inputFilePath, wxString outputFilePath);
bool checkForMatch(wxString inputFile, wxString standardFile);

class JSONUnitTestTraverser : public wxDirTraverser {
public:
	JSONUnitTestTraverser(wxDir& inputDirectory, wxDir& outputDirectory, wxDir& standardDirectory)
		: mOutputDir(outputDirectory), mStdDir(standardDirectory) {};
	virtual wxDirTraverseResult OnDir(const wxString &dirname) {
		return wxDIR_IGNORE;
	}
	virtual wxDirTraverseResult OnFile(const wxString &filename) {
		wxFileName inputFilename(filename);
		wxString sharedFilename = inputFilename.GetName();
		wxString fileExtension = inputFilename.GetExt();
		wxString outputFilename = wxFileName(mOutputDir.GetName(), sharedFilename, fileExtension).GetFullPath();
		wxString standardFilename = wxFileName(mStdDir.GetName(), sharedFilename, fileExtension).GetFullPath();
		copyJSONFile(filename, outputFilename);
		if (!checkForMatch(outputFilename, standardFilename)) {
			throw JSONUnitTestFailureException();
		}
		return wxDIR_CONTINUE;
	}
	virtual wxDirTraverseResult OnOpenError(const wxString &openerrorname) {
		return wxDIR_IGNORE;
	};
private:
	wxDir& mOutputDir;
	wxDir& mStdDir;

};

void testJSONFiles(std::string inputDirectory, std::string outputDirectory, std::string standardDirectory) {
	wxDir testInputDirectory, testOutputDirectory, testStandardDirectory;
	if (testInputDirectory.Open(inputDirectory) && testOutputDirectory.Open(outputDirectory) && testStandardDirectory.Open(standardDirectory)) {
		testInputDirectory.Traverse(JSONUnitTestTraverser(testInputDirectory, testOutputDirectory, testStandardDirectory));
	}
}


void copyJSONFile(wxString inputFilePath, wxString outputFilePath) {
	wxFileInputStream inputStream(inputFilePath);
	wxFileOutputStream outputStream(outputFilePath);
	char* input = new char[inputStream.GetSize()];
	inputStream.Read(input, inputStream.GetSize());
	json_inputbuffer = input;
	importJSON();
	if (OutputMainObject != nullptr) {
		JSONFormatter::exportJSON(&outputStream, *OutputMainObject);
		delete OutputMainObject;
	}
}

bool checkForMatch(wxString fileToCheck, wxString standardFile) {
	const int readSize = 16;
	wxFileInputStream check(fileToCheck);
	wxFileInputStream standard(standardFile);
	char checkChunk[readSize], standardChunk[readSize];
	while (!check.Eof()) {
		if (standard.Eof()) {
			return false;
		}
		check.ReadAll(&checkChunk, readSize);
		standard.ReadAll(&standardChunk, readSize);
		if (check.LastRead() != standard.LastRead()) {
			return false;
		}
		for (int counter = 0; counter < readSize; counter++) {
			if (checkChunk[counter] != standardChunk[counter]) {
				return false;
			}
		}
	}
	return true;
}

void json_UnitTests() {
	wxString baseDirectoryPath;
	wxFileName::SplitPath(wxStandardPaths::Get().GetExecutablePath(), &baseDirectoryPath, nullptr, nullptr);
	wxFileName jsonTestsBaseDir(baseDirectoryPath);
	jsonTestsBaseDir.AppendDir(wxT("unit_tests"));
	jsonTestsBaseDir.AppendDir(wxT("json_import_export"));
	wxFileName inputDir(jsonTestsBaseDir);
	inputDir.AppendDir(wxT("in"));
	wxFileName outputDir(jsonTestsBaseDir);
	outputDir.AppendDir(wxT("out"));
	wxFileName standardDir(jsonTestsBaseDir);
	standardDir.AppendDir(wxT("std"));
	testJSONFiles(inputDir.GetPath().ToStdString(), outputDir.GetPath().ToStdString(), standardDir.GetPath().ToStdString());
}