#include "ghost_tools.h"
#include "ui_enums.h"
#include <wx/textdlg.h>
#include <wx/msgdlg.h> 

GhostTool::GhostTool(GhostModule* ghostModule)
: mGhostModule(ghostModule)
{}

GhostModule* GhostTool::getGhostModule() {
	return mGhostModule;
}

DisableGhostTool::DisableGhostTool(GhostModule* ghostModule)
: GhostTool(ghostModule)
{}

int DisableGhostTool::getId() {
	return CALCHART__GhostOff;
}

void DisableGhostTool::activate() {
	getGhostModule()->setState(false);
}

bool DisableGhostTool::canActivate() {
	return getGhostModule()->isActive();
}

GhostSourceInstallerTool::GhostSourceInstallerTool(GhostModule* ghostModule)
: GhostTool(ghostModule)
{}

void GhostSourceInstallerTool::activate() {
	installGhostSource();
	getGhostModule()->setState(true);
}

bool GhostSourceInstallerTool::canActivate() {
	return true;
}

void GhostSourceInstallerTool::installGhostSource() {
	getGhostModule()->setGhostSource(getGhostSource(), ghostModuleIsResponsible());
}

GhostSingleSourceTool::GhostSingleSourceTool(GhostModule* ghostModule, GhostSource* source)
: GhostSourceInstallerTool(ghostModule), mSource(source)
{}

GhostSingleSourceTool::~GhostSingleSourceTool() {
	if (mSource != nullptr) {
		delete mSource;
	}
}

GhostSource* GhostSingleSourceTool::getGhostSource() {
	return mSource;
}

bool GhostSingleSourceTool::ghostModuleIsResponsible() {
	return false;
}

GhostSourceGeneratorTool::GhostSourceGeneratorTool(GhostModule* ghostModule)
: GhostSourceInstallerTool(ghostModule)
{}

GhostSource* GhostSourceGeneratorTool::getGhostSource() {
	return generateGhostSource();
}

bool GhostSourceGeneratorTool::ghostModuleIsResponsible() {
	return true;
}

GhostSheetRelativeTool::GhostSheetRelativeTool(GhostModule* ghostModule, int relativeSheet) 
: GhostSingleSourceTool(ghostModule, new RelativeGhostSource(relativeSheet))
{}


GhostSheetNextTool::GhostSheetNextTool(GhostModule* ghostModule)
: GhostSheetRelativeTool(ghostModule, 1)
{}

int GhostSheetNextTool::getId() {
	return CALCHART__GhostNextSheet;
}

GhostSheetPreviousTool::GhostSheetPreviousTool(GhostModule* ghostModule)
: GhostSheetRelativeTool(ghostModule, -1)
{}

int GhostSheetPreviousTool::getId() {
	return CALCHART__GhostPreviousSheet;
}
GhostSheetAbsoluteTool::GhostSheetAbsoluteTool(GhostModule* ghostModule, wxWindow* frame)
: GhostSourceGeneratorTool(ghostModule), mNextTargetSheet(0), mFrame(frame)
{}

void GhostSheetAbsoluteTool::activate() {
	wxString targetSheet = wxGetTextFromUser("Enter the sheet number to ghost:","Ghost Sheet", "1", mFrame);
	long targetSheetNum = 0;
	if (targetSheet.ToLong(&targetSheetNum)) {
		mNextTargetSheet = targetSheetNum - 1;
		super::activate();
	} else {
		wxMessageBox(wxT("The input must be a number."), wxT("Operation failed."));
	}
}

int GhostSheetAbsoluteTool::getId() {
	return CALCHART__GhostNthSheet;
}

GhostSource* GhostSheetAbsoluteTool::generateGhostSource() {
	return new AbsoluteGhostSource(mNextTargetSheet);
}




