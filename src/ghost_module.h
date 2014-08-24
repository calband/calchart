#ifndef _GHOST_MODULE_H_
#define _GHOST_MODULE_H_

class CC_sheet;
class CalChartDoc;
/**
 * The part of the Field Frame that controls the active ghosted formation.
 */
class GhostModule {
public:
	/**
	 * Sets the source from which the ghost sheet is extracted.
	 * @param source The source from which the active ghost sheet will be extracted.
	 * @param isResponsible True if the GhostModule should delete the source when it forgets it.
	 */
	enum GhostSource { disabled, next, previous, specific };
	void setGhostSource(GhostSource source, int which = 0);
	/**
	 * Returns the sheet having the ghost formation to draw.
	 * @return The sheet having the ghost formation to draw.
	 */
	CC_sheet* getGhostSheet(CalChartDoc* doc, int currentSheet) const;
	/**
	 * Returns whether or not ghost formations are active.
	 * @return True if the ghost formations are currently active; false otherwise.
	 */
	bool isActive() const;
private:

	GhostSource mCurrentSource = disabled;
	int mWhich = 0;
};

#endif
