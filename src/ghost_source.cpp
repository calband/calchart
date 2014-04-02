#include "ghost_source.h"

RelativeGhostSource::RelativeGhostSource(int relativeTarget)
: mRelativeSheet(relativeTarget), mGhost(nullptr)
{}

CC_sheet* RelativeGhostSource::getGhostSheet() {
	return mGhost;
}

void RelativeGhostSource::update(CalChartDoc* doc, int currentSheet) {
	int targetSheet = currentSheet + mRelativeSheet;
	if (targetSheet >= 0 && targetSheet < doc->GetNumSheets()) {
		mGhost = &(*(doc->GetNthSheet(targetSheet)));
	} else {
		mGhost = nullptr;
	}
}

AbsoluteGhostSource::AbsoluteGhostSource(int targetSheet)
: mTargetSheet(targetSheet), mGhost(nullptr), mIsActive(false)
{}

CC_sheet* AbsoluteGhostSource::getGhostSheet() {
	if (mIsActive) {
		return mGhost;
	}
	return nullptr;
}

void AbsoluteGhostSource::update(CalChartDoc* doc, int currentSheet) {
	if (mTargetSheet >= 0 && mTargetSheet < doc->GetNumSheets()) {
		mGhost = &(*(doc->GetNthSheet(mTargetSheet)));
	}
	else {
		mGhost = nullptr;
	}
	mIsActive = (currentSheet != mTargetSheet);
}
