#ifndef _GHOST_TOOLS_H_
#define _GHOST_TOOLS_H_

#include "ghost_module.h"
#include "ghost_source.h"

#include <wx/window.h>

class GhostTool {
public:
	GhostTool(GhostModule* ghostModule);

	virtual int getId() = 0;
	virtual void activate() = 0;
	virtual bool canActivate() = 0;
protected:
	virtual GhostModule* getGhostModule();
private:
	GhostModule* mGhostModule;
};

class DisableGhostTool : public GhostTool {
public:
	DisableGhostTool(GhostModule* ghostModule);

	virtual int getId();
	virtual void activate();
	virtual bool canActivate();
};

class GhostSourceInstallerTool : public GhostTool {
public:
	GhostSourceInstallerTool(GhostModule* ghostModule);

	virtual void activate();
	virtual bool canActivate();
protected:
	virtual void installGhostSource();
	virtual GhostSource* getGhostSource() = 0;
	virtual bool ghostModuleIsResponsible() = 0;
};

class GhostSingleSourceTool : public GhostSourceInstallerTool {
public:
	GhostSingleSourceTool(GhostModule* ghostModule, GhostSource* source);
	virtual ~GhostSingleSourceTool();
protected:
	virtual GhostSource* getGhostSource();
	virtual bool ghostModuleIsResponsible();
private:
	GhostSource* mSource;
};

class GhostSourceGeneratorTool : public GhostSourceInstallerTool {
public:
	GhostSourceGeneratorTool(GhostModule* ghostModule);
protected:
	virtual GhostSource* getGhostSource();
	virtual bool ghostModuleIsResponsible();

	virtual GhostSource* generateGhostSource() = 0;
};

class GhostSheetRelativeTool : public GhostSingleSourceTool {
public:
	GhostSheetRelativeTool(GhostModule* ghostModule, int relativeSheet);
};

class GhostSheetNextTool : public GhostSheetRelativeTool {
public:
	GhostSheetNextTool(GhostModule* ghostModule);

	virtual int getId();
};

class GhostSheetPreviousTool : public GhostSheetRelativeTool {
public:
	GhostSheetPreviousTool(GhostModule* ghostModule);

	virtual int getId();
};

class GhostSheetAbsoluteTool : public GhostSourceGeneratorTool {
private:
	using super = GhostSourceGeneratorTool;
public:
	GhostSheetAbsoluteTool(GhostModule* ghostModule, wxWindow* frame);

	virtual void activate();

	virtual int getId();
protected:
	virtual GhostSource* generateGhostSource();
private:
	int mNextTargetSheet;
	wxWindow* mFrame;
};

#endif