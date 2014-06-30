#ifndef _GHOST_MODULE_H_
#define _GHOST_MODULE_H_

#include "cc_sheet.h"
#include "calchartdoc.h"


/**
* A source from which you can extract a ghost formation.
*/
class GhostSource {
public:
	/**
	* Returns the active ghost formation.
	* @return The active ghost formation.
	*/
	virtual CC_sheet* getGhostSheet() = 0;
	/**
	* Updates the source according to the current stuntsheet.
	* @param doc The CalChartDoc.
	* @param currentSheet The index of the active stuntsheet.
	*/
	virtual void update(CalChartDoc* doc, int currentSheet) = 0;
};


/**
 * The part of the Field Frame that controls the active ghosted formation.
 */
class GhostModule {
public:
	GhostModule();
	GhostModule(CalChartDoc* show, int currentSheet);
	~GhostModule();

	/**
	 * Sets the source from which the ghost sheet is extracted.
	 * @param source The source from which the active ghost sheet will be extracted.
	 * @param isResponsible True if the GhostModule should delete the source when it forgets it.
	 */
	void setGhostSource(GhostSource* source, bool isResponsible = false);
	/**
	 * Returns the sheet having the ghost formation to draw.
	 * @return The sheet having the ghost formation to draw.
	 */
	CC_sheet* getGhostSheet();
	/**
	 * Updates the module to respond to the active stuntsheet.
	 * @param doc The CalChartDoc.
	 * @param currentSheet The index of the current stuntsheet.
	 */
	void update(CalChartDoc* doc, int currentSheet);

	/**
	 * Activates/deactivates the ghost formations.
	 * @param isActive True if the ghost formations should be active; false otherwise.
	 */
	void setState(bool isActive);
	/**
	 * Returns whether or not ghost formations are active.
	 * @return True if the ghost formations are currently active; false otherwise.
	 */
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