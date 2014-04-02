#include "ghost_module.h"

GhostModule::GhostModule()
: mDoc(nullptr), mCurrentSheet(0), mCurrentSource(nullptr), mIsActive(false), mIsResponsibleForSource(false)
{}

GhostModule::GhostModule(CalChartDoc* doc, int currentSheet)
: mDoc(doc), mCurrentSheet(currentSheet), mCurrentSource(nullptr), mIsActive(false), mIsResponsibleForSource(false)
{}

GhostModule::~GhostModule() {
	setGhostSource(nullptr);
}

void GhostModule::setGhostSource(GhostSource* source, bool isResponsible) {
	if (mIsResponsibleForSource && mCurrentSource != nullptr) {
		delete mCurrentSource;
	}
	mIsResponsibleForSource = isResponsible;
	mCurrentSource = source;
	updateCurrentGhostSource();
}

CC_sheet* GhostModule::getGhostSheet() {
	if (mIsActive && mCurrentSource != nullptr) {
		return mCurrentSource->getGhostSheet();
	} else {
		return nullptr;
	}
}

void GhostModule::update(CalChartDoc* doc, int currentSheet) {
	mDoc = doc;
	mCurrentSheet = currentSheet;
	updateCurrentGhostSource();
}

void GhostModule::setState(bool isActive) {
	bool wasActive = mIsActive;
	mIsActive = isActive;
	if (mIsActive && !wasActive) {
		updateCurrentGhostSource();
	}
}

bool GhostModule::isActive() {
	return mIsActive;
}

void GhostModule::updateCurrentGhostSource() {
	if (mIsActive && mCurrentSource != nullptr) {
		mCurrentSource->update(mDoc, mCurrentSheet);
	}
}
