#ifndef _GHOST_SOURCE_H_
#define _GHOST_SOURCE_H_

#include "ghost_module.h"


/**
 * A source whose ghost formation will be a stuntsheet relative to the current one.
 */
class RelativeGhostSource : public GhostSource {
public:
	/**
	 * Constructor.
	 * @param relativeTarget The amount to add to the active sheet index to get the ghost sheet index.
	 */
	RelativeGhostSource(int relativeTarget);

	virtual CC_sheet* getGhostSheet();
	virtual void update(CalChartDoc* doc, int currentSheet);
private:
	int mRelativeSheet;
	int mCurrentSheet;
	CalChartDoc* mDoc;
};

/**
 * A source that provides a particular stuntsheet for a ghost formation.
 */
class AbsoluteGhostSource : public GhostSource {
public:
	/**
	 * Constructor.
	 * @param targetSheet The index of the ghost sheet.
	 */
	AbsoluteGhostSource(int targetSheet);

	virtual CC_sheet* getGhostSheet();
	virtual void update(CalChartDoc* doc, int currentSheet);
private:
	int mTargetSheet;
	int mCurrentSheet;
	CalChartDoc* mDoc;
};

#endif