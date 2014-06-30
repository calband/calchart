#include "ghost_source.h"

RelativeGhostSource::RelativeGhostSource(int relativeTarget)
: mRelativeSheet(relativeTarget), mDoc(nullptr), mCurrentSheet(0)
{}

CC_sheet* RelativeGhostSource::getGhostSheet() {
	if (mDoc == nullptr) {
		return nullptr;
	}
	int targetSheet = mCurrentSheet + mRelativeSheet;
	if (targetSheet >= 0 && targetSheet < mDoc->GetNumSheets()) {
		return &(*(mDoc->GetNthSheet(targetSheet)));
	} else {
		return nullptr;
	}
}

void RelativeGhostSource::update(CalChartDoc* doc, int currentSheet) {
	mCurrentSheet = currentSheet;
	mDoc = doc;
}

AbsoluteGhostSource::AbsoluteGhostSource(int targetSheet)
: mTargetSheet(targetSheet), mDoc(nullptr), mCurrentSheet(0)
{}

CC_sheet* AbsoluteGhostSource::getGhostSheet() {
	if (mCurrentSheet == mTargetSheet || mDoc == nullptr) {
		return nullptr;
	}
	if (mTargetSheet >= 0 && mTargetSheet < mDoc->GetNumSheets()) {
		return &(*(mDoc->GetNthSheet(mTargetSheet)));
	} else {
		return nullptr;
	}
}

void AbsoluteGhostSource::update(CalChartDoc* doc, int currentSheet) {
	mDoc = doc;
	mCurrentSheet = currentSheet;
}
