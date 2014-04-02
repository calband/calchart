#ifndef _GHOST_SOURCE_H_
#define _GHOST_SOURCE_H_

#include "ghost_module.h"

class RelativeGhostSource : public GhostSource {
public:
	RelativeGhostSource(int relativeTarget);

	virtual CC_sheet* getGhostSheet();
	virtual void update(CalChartDoc* doc, int currentSheet);
private:
	int mRelativeSheet;
	CC_sheet* mGhost;
};

class AbsoluteGhostSource : public GhostSource {
public:
	AbsoluteGhostSource(int targetSheet);

	virtual CC_sheet* getGhostSheet();
	virtual void update(CalChartDoc* doc, int currentSheet);
private:
	int mTargetSheet;
	bool mIsActive;
	CC_sheet* mGhost;
};

#endif