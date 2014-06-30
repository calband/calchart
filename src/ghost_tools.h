#ifndef _GHOST_TOOLS_H_
#define _GHOST_TOOLS_H_

#include "ghost_module.h"
#include "ghost_source.h"

#include <wx/window.h>

/**
 * A tool that modifies the GhostModule.
 */
class GhostTool {
public:
	GhostTool(GhostModule* ghostModule);

	/**
	 * Returns the identity of the tool, related to its wxWidgets menu item.
	 * @return The identity of the tool.
	 */
	virtual int getId() = 0;
	/**
	 * Activates the tool.
	 */
	virtual void activate() = 0;
	/**
	 * Returns whether or not the tool can activate.
	 * @return True if the tool can activate; false otherwise.
	 */
	virtual bool canActivate() = 0;
protected:
	/**
	 * Returns the module being modified by this tool.
	 * @return The module being modified by this tool.
	 */
	virtual GhostModule* getGhostModule();
private:
	GhostModule* mGhostModule;
};

/**
 * A tool that disables the ghost module.
 */
class DisableGhostTool : public GhostTool {
public:
	DisableGhostTool(GhostModule* ghostModule);

	virtual int getId();
	virtual void activate();
	virtual bool canActivate();
};

/**
 * A tool that installs a ghost formation into the ghost module.
 */
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

/**
 * A tool that installs a particular ghost formation source into the ghost module.
 */
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

/**
 * A tool that builds a new ghost formation source each time it installs it into the ghost module.
 */
class GhostSourceGeneratorTool : public GhostSourceInstallerTool {
public:
	GhostSourceGeneratorTool(GhostModule* ghostModule);
protected:
	virtual GhostSource* getGhostSource();
	virtual bool ghostModuleIsResponsible();

	virtual GhostSource* generateGhostSource() = 0;
};

/**
 * A tool that installs a GhostSourceRelative object to the Ghost Module.
 */
class GhostSheetRelativeTool : public GhostSingleSourceTool {
public:
	GhostSheetRelativeTool(GhostModule* ghostModule, int relativeSheet);
};

/**
 * A tool that ghosts the next stuntsheet.
 */
class GhostSheetNextTool : public GhostSheetRelativeTool {
public:
	GhostSheetNextTool(GhostModule* ghostModule);

	virtual int getId();
};

/**
 * A tool that ghosts the previous stuntsheet.
 */
class GhostSheetPreviousTool : public GhostSheetRelativeTool {
public:
	GhostSheetPreviousTool(GhostModule* ghostModule);

	virtual int getId();
};

/**
 * A tool that ghosts a particular stuntsheet.
 */
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