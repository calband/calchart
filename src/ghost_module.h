#ifndef _GHOST_MODULE_H_
#define _GHOST_MODULE_H_

#include "core/cc_sheet.h"
#include "calchartdoc.h"

class GhostSource {
public:
	virtual CC_sheet* getGhostSheet() = 0;
	virtual void update(CalChartDoc* doc, int currentSheet) = 0;
};

class GhostModule {
public:
	GhostModule();
	GhostModule(CalChartDoc* show, int currentSheet);
	~GhostModule();

	void setGhostSource(GhostSource* source, bool isResponsible = false);
	CC_sheet* getGhostSheet();
	void update(CalChartDoc* doc, int currentSheet);

	void setState(bool isActive);
	bool isActive();
private:
	void updateCurrentGhostSource();

	GhostSource* mCurrentSource;
	CalChartDoc* mDoc;
	int mCurrentSheet;
	bool mIsActive;
	bool mIsResponsibleForSource;
};

#endif